#include "postquantiquecoin/wallet/WalletEncryption.h"
#include "postquantiquecoin/core/Serialization.h"
#include "postquantiquecoin/crypto/Hashing.h"
#include "postquantiquecoin/crypto/SecureBuffer.h"
#include "postquantiquecoin/crypto/SecureRandom.h"
#include "postquantiquecoin/crypto/WindowsAead.h"
#include "postquantiquecoin/wallet/WalletDatFormat.h"
#include "postquantiquecoin/wallet/WindowsWalletKdf.h"

#include <algorithm>

namespace pqc {
namespace {
std::vector<uint8_t> DekPasswordAssociatedData(const WalletEncryptedBlob& blob) {
    auto aad = WalletDatFormat::AssociatedData(blob);
    const char domain[] = "PQC_WALLET_DEK_BY_PASSWORD_V1";
    aad.insert(aad.end(), domain, domain + sizeof(domain) - 1);
    aad.insert(aad.end(), blob.walletFingerprint.begin(), blob.walletFingerprint.end());
    return aad;
}

std::array<uint8_t, 32> FingerprintFor(const std::vector<uint8_t>& plaintext) {
    std::vector<uint8_t> msg{'P','Q','C','_','W','A','L','L','E','T','_','F','I','N','G','E','R','P','R','I','N','T','_','V','1'};
    msg.insert(msg.end(), plaintext.begin(), plaintext.end());
    return Hashing::Sha3_256(msg);
}
}

Result<std::vector<uint8_t>> WalletEncryption::DerivePasswordKey(const std::string& password,
                                                                 const std::vector<uint8_t>& salt,
                                                                 uint32_t iterations,
                                                                 size_t bytes) {
    if (salt.size() < 16) return Result<std::vector<uint8_t>>::Err("wallet KDF salt is too short");
    const uint32_t useIterations = iterations == 0 ? WindowsWalletKdf::ProductionIterations : iterations;
    return WindowsWalletKdf::DerivePbkdf2HmacSha256(password, salt, useIterations, bytes);
}

WalletEncryptedBlob WalletEncryption::EncryptPayload(const std::vector<uint8_t>& plaintext,
                                                     const std::string& password,
                                                     uint32_t iterations,
                                                     const std::vector<uint8_t>& existingSalt) {
    WalletEncryptedBlob b;
    b.version = 3;
    b.networkId = 1;
    b.kdfAlgorithm = static_cast<uint8_t>(WalletKdfAlgorithm::PBKDF2_HMAC_SHA256_BCRYPT);
    b.aeadAlgorithm = WindowsAead::AlgorithmIdAes256Gcm;
    b.pqWrapAlgorithm = 0;
    b.kdfIterations = iterations == 0 ? WindowsWalletKdf::ProductionIterations : iterations;
    b.saltPassword = existingSalt.empty() ? SecureRandom::Bytes(32) : existingSalt;
    b.noncePayload = SecureRandom::Bytes(12);
    b.nonceDekPassword = SecureRandom::Bytes(12);
    b.walletFingerprint = FingerprintFor(plaintext);

    auto kekResult = DerivePasswordKey(password, b.saltPassword, b.kdfIterations, 32);
    if (kekResult.IsErr()) return b;

    SecureBuffer dek(SecureRandom::Bytes(32));
    SecureBuffer kek(kekResult.Value());
    auto dekCopy = dek.ToVectorCopy();
    auto payload = WindowsAead::EncryptAes256Gcm(dekCopy, b.noncePayload, plaintext, WalletDatFormat::AssociatedData(b));
    if (payload.IsErr()) return b;
    b.encryptedPayload = payload.Value().ciphertext;
    b.tagPayload = payload.Value().authTag;

    auto kekCopy = kek.ToVectorCopy();
    auto wrappedDek = WindowsAead::EncryptAes256Gcm(kekCopy, b.nonceDekPassword, dekCopy, DekPasswordAssociatedData(b));
    if (wrappedDek.IsErr()) return b;
    b.encryptedDekByPassword = wrappedDek.Value().ciphertext;
    b.tagDekPassword = wrappedDek.Value().authTag;

    std::fill(dekCopy.begin(), dekCopy.end(), 0);
    std::fill(kekCopy.begin(), kekCopy.end(), 0);

    auto serialized = WalletDatFormat::Serialize(b);
    auto parsed = WalletDatFormat::Deserialize(serialized);
    if (parsed.IsOk()) b = parsed.Value();
    return b;
}

Result<WalletEncryptedBlob> WalletEncryption::RotatePassword(const WalletEncryptedBlob& b,
                                                             const std::string& oldPassword,
                                                             const std::string& newPassword,
                                                             uint32_t iterations) {
    const uint32_t useIterations = b.kdfIterations == 0 ? iterations : b.kdfIterations;
    auto oldKek = DerivePasswordKey(oldPassword, b.saltPassword, useIterations, 32);
    if (oldKek.IsErr()) return Result<WalletEncryptedBlob>::Err(oldKek.Error());
    SecureBuffer oldKekBuffer(oldKek.Value());
    auto oldKekCopy = oldKekBuffer.ToVectorCopy();
    auto dek = WindowsAead::DecryptAes256Gcm(oldKekCopy, b.nonceDekPassword, b.encryptedDekByPassword, DekPasswordAssociatedData(b), b.tagDekPassword);
    std::fill(oldKekCopy.begin(), oldKekCopy.end(), 0);
    if (dek.IsErr()) return Result<WalletEncryptedBlob>::Err(dek.Error());

    WalletEncryptedBlob rotated = b;
    rotated.saltPassword = SecureRandom::Bytes(32);
    rotated.nonceDekPassword = SecureRandom::Bytes(12);
    rotated.kdfIterations = useIterations;
    auto newKek = DerivePasswordKey(newPassword, rotated.saltPassword, useIterations, 32);
    if (newKek.IsErr()) return Result<WalletEncryptedBlob>::Err(newKek.Error());
    SecureBuffer dekBuffer(dek.Value());
    SecureBuffer newKekBuffer(newKek.Value());
    auto dekCopy = dekBuffer.ToVectorCopy();
    auto newKekCopy = newKekBuffer.ToVectorCopy();
    auto rewrapped = WindowsAead::EncryptAes256Gcm(newKekCopy, rotated.nonceDekPassword, dekCopy, DekPasswordAssociatedData(rotated));
    std::fill(dekCopy.begin(), dekCopy.end(), 0);
    std::fill(newKekCopy.begin(), newKekCopy.end(), 0);
    if (rewrapped.IsErr()) return Result<WalletEncryptedBlob>::Err(rewrapped.Error());
    rotated.encryptedDekByPassword = rewrapped.Value().ciphertext;
    rotated.tagDekPassword = rewrapped.Value().authTag;
    auto serialized = WalletDatFormat::Serialize(rotated);
    auto parsed = WalletDatFormat::Deserialize(serialized);
    if (parsed.IsErr()) return Result<WalletEncryptedBlob>::Err(parsed.Error());
    return Result<WalletEncryptedBlob>::Ok(parsed.Value());
}

Result<std::vector<uint8_t>> WalletEncryption::DecryptPayload(const WalletEncryptedBlob& b,
                                                              const std::string& password,
                                                              uint32_t iterations) {
    const uint32_t useIterations = b.kdfIterations == 0 ? iterations : b.kdfIterations;
    if (b.kdfAlgorithm == static_cast<uint8_t>(WalletKdfAlgorithm::DEV_ONLY_KMAC_LOOP) && b.networkId == 1) {
        return Result<std::vector<uint8_t>>::Err("dev-only wallet KDF is forbidden on mainnet");
    }
    if (b.encryptedPayload.empty() || b.encryptedDekByPassword.empty()) {
        return Result<std::vector<uint8_t>>::Err("wallet is missing DEK/KEK encrypted fields");
    }
    auto kekResult = DerivePasswordKey(password, b.saltPassword, useIterations, 32);
    if (kekResult.IsErr()) return Result<std::vector<uint8_t>>::Err(kekResult.Error());

    SecureBuffer kek(kekResult.Value());
    auto kekCopy = kek.ToVectorCopy();
    auto dekResult = WindowsAead::DecryptAes256Gcm(kekCopy, b.nonceDekPassword, b.encryptedDekByPassword, DekPasswordAssociatedData(b), b.tagDekPassword);
    std::fill(kekCopy.begin(), kekCopy.end(), 0);
    if (dekResult.IsErr()) return Result<std::vector<uint8_t>>::Err(dekResult.Error());

    SecureBuffer dek(dekResult.Value());
    auto dekCopy = dek.ToVectorCopy();
    auto plaintext = WindowsAead::DecryptAes256Gcm(dekCopy, b.noncePayload, b.encryptedPayload, WalletDatFormat::AssociatedData(b), b.tagPayload);
    std::fill(dekCopy.begin(), dekCopy.end(), 0);
    return plaintext;
}
}
