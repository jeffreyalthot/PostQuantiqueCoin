#include "postquantiquecoin/wallet/WalletEncryption.h"
#include "postquantiquecoin/core/Serialization.h"
#include "postquantiquecoin/crypto/SecureRandom.h"
#include "postquantiquecoin/crypto/WindowsAead.h"
#include "postquantiquecoin/wallet/WalletDatFormat.h"
#include "postquantiquecoin/wallet/WindowsWalletKdf.h"

namespace pqc {
namespace {
std::vector<uint8_t> DekAssociatedData(const WalletEncryptedBlob& blob) {
    auto aad = WalletDatFormat::AssociatedData(blob);
    const char domain[] = "PQC_WALLET_DEK_BY_PASSWORD_V1";
    aad.insert(aad.end(), domain, domain + sizeof(domain) - 1);
    return aad;
}

std::vector<uint8_t> PackAead(const std::vector<uint8_t>& nonce, const AeadCiphertext& enc) {
    std::vector<uint8_t> out;
    out.reserve(nonce.size() + enc.authTag.size() + enc.ciphertext.size());
    out.insert(out.end(), nonce.begin(), nonce.end());
    out.insert(out.end(), enc.authTag.begin(), enc.authTag.end());
    out.insert(out.end(), enc.ciphertext.begin(), enc.ciphertext.end());
    return out;
}

Result<std::vector<uint8_t>> UnpackAndDecryptAead(const std::vector<uint8_t>& key,
                                                  const std::vector<uint8_t>& packed,
                                                  const std::vector<uint8_t>& aad) {
    if (packed.size() < 12 + 16) return Result<std::vector<uint8_t>>::Err("packed AEAD value is too short");
    std::vector<uint8_t> nonce(packed.begin(), packed.begin() + 12);
    std::vector<uint8_t> tag(packed.begin() + 12, packed.begin() + 28);
    std::vector<uint8_t> ciphertext(packed.begin() + 28, packed.end());
    return WindowsAead::DecryptAes256Gcm(key, nonce, ciphertext, aad, tag);
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
    b.version = 2;
    b.networkId = 1;
    b.kdfAlgorithm = static_cast<uint8_t>(WalletKdfAlgorithm::PBKDF2_HMAC_SHA256_BCRYPT);
    b.aeadAlgorithm = WindowsAead::AlgorithmIdAes256Gcm;
    b.pqWrapAlgorithm = 0;
    b.kdfIterations = iterations == 0 ? WindowsWalletKdf::ProductionIterations : iterations;
    b.salt = existingSalt.empty() ? SecureRandom::Bytes(32) : existingSalt;
    b.nonce = SecureRandom::Bytes(12);

    auto kek = DerivePasswordKey(password, b.salt, b.kdfIterations, 32);
    if (kek.IsErr()) return b;

    auto dek = SecureRandom::Bytes(32);
    auto payload = WindowsAead::EncryptAes256Gcm(dek, b.nonce, plaintext, WalletDatFormat::AssociatedData(b));
    if (payload.IsErr()) return b;
    b.ciphertext = payload.Value().ciphertext;
    b.authTag = payload.Value().authTag;

    auto dekNonce = SecureRandom::Bytes(12);
    auto wrappedDek = WindowsAead::EncryptAes256Gcm(kek.Value(), dekNonce, dek, DekAssociatedData(b));
    if (wrappedDek.IsErr()) return b;
    b.encryptedDekByPassword = PackAead(dekNonce, wrappedDek.Value());

    auto serialized = WalletDatFormat::Serialize(b);
    auto parsed = WalletDatFormat::Deserialize(serialized);
    if (parsed.IsOk()) b = parsed.Value();
    return b;
}

Result<std::vector<uint8_t>> WalletEncryption::DecryptPayload(const WalletEncryptedBlob& b,
                                                              const std::string& password,
                                                              uint32_t iterations) {
    const uint32_t useIterations = b.kdfIterations == 0 ? iterations : b.kdfIterations;
    if (b.kdfAlgorithm == static_cast<uint8_t>(WalletKdfAlgorithm::DEV_ONLY_KMAC_LOOP) && b.networkId == 1) {
        return Result<std::vector<uint8_t>>::Err("dev-only wallet KDF is forbidden on mainnet");
    }
    auto kek = DerivePasswordKey(password, b.salt, useIterations, 32);
    if (kek.IsErr()) return Result<std::vector<uint8_t>>::Err(kek.Error());

    if (!b.encryptedDekByPassword.empty()) {
        auto dek = UnpackAndDecryptAead(kek.Value(), b.encryptedDekByPassword, DekAssociatedData(b));
        if (dek.IsErr()) return Result<std::vector<uint8_t>>::Err(dek.Error());
        return WindowsAead::DecryptAes256Gcm(dek.Value(), b.nonce, b.ciphertext, WalletDatFormat::AssociatedData(b), b.authTag);
    }

    return WindowsAead::DecryptAes256Gcm(kek.Value(), b.nonce, b.ciphertext, WalletDatFormat::AssociatedData(b), b.authTag);
}
}
