#include "postquantiquecoin/wallet/WalletRecovery.h"
#include "postquantiquecoin/crypto/Hashing.h"
#include "postquantiquecoin/crypto/SecureBuffer.h"
#include "postquantiquecoin/crypto/SecureRandom.h"
#include "postquantiquecoin/crypto/WindowsAead.h"
#include "postquantiquecoin/wallet/WalletEncryption.h"
#include <algorithm>
namespace pqc {
namespace {
std::vector<uint8_t> RecoverySecretAad(const std::string& algorithm) {
    std::vector<uint8_t> aad{'P','Q','C','_','R','E','C','O','V','E','R','Y','_','S','E','C','R','E','T','_','K','E','M','_','V','1'};
    aad.insert(aad.end(), algorithm.begin(), algorithm.end());
    return aad;
}
std::vector<uint8_t> RecoveryDekAad(const WalletEncryptedBlob& blob) {
    auto aad = WalletDatFormat::AssociatedData(blob);
    const char domain[] = "PQC_WALLET_DEK_BY_KEM_V1";
    aad.insert(aad.end(), domain, domain + sizeof(domain) - 1);
    aad.insert(aad.end(), blob.walletFingerprint.begin(), blob.walletFingerprint.end());
    aad.insert(aad.end(), blob.kemPublicKeyFingerprint.begin(), blob.kemPublicKeyFingerprint.end());
    return aad;
}
std::array<uint8_t,32> ArrayFromVectorHash(const std::vector<uint8_t>& data) { return Hashing::Sha3_256(data); }
}
WalletRecoveryBundle WalletRecovery::Create(PQCryptoProvider& provider,
                                            const std::vector<uint8_t>& walletFingerprint,
                                            const std::string& recoveryPassword,
                                            uint8_t networkId,
                                            uint32_t iterations) {
    WalletRecoveryBundle bundle;
    auto kp = provider.GenerateKemKeyPair();
    bundle.publicFile.networkId = networkId;
    bundle.publicFile.kemAlgorithm = kp.algorithm;
    bundle.publicFile.kemPublicKey = kp.publicKey;
    if (walletFingerprint.size() == 32) std::copy(walletFingerprint.begin(), walletFingerprint.end(), bundle.publicFile.walletFingerprint.begin());

    bundle.secretFile.kemAlgorithm = kp.algorithm;
    bundle.secretFile.salt = SecureRandom::Bytes(32);
    bundle.secretFile.nonce = SecureRandom::Bytes(12);
    auto key = WalletEncryption::DerivePasswordKey(recoveryPassword, bundle.secretFile.salt, iterations, 32);
    if (key.IsOk()) {
        SecureBuffer kek(key.Value());
        auto kekCopy = kek.ToVectorCopy();
        auto enc = WindowsAead::EncryptAes256Gcm(kekCopy, bundle.secretFile.nonce, kp.privateKey, RecoverySecretAad(kp.algorithm));
        std::fill(kekCopy.begin(), kekCopy.end(), 0);
        if (enc.IsOk()) {
            bundle.secretFile.encryptedKemPrivateKey = enc.Value().ciphertext;
            bundle.secretFile.tag = enc.Value().authTag;
        }
    }
    return bundle;
}
Result<std::vector<uint8_t>> WalletRecovery::DecryptKemPrivateKey(const WalletRecoverySecretFile& secretFile,
                                                                  const std::string& recoveryPassword,
                                                                  uint32_t iterations) {
    auto key = WalletEncryption::DerivePasswordKey(recoveryPassword, secretFile.salt, iterations, 32);
    if (key.IsErr()) return Result<std::vector<uint8_t>>::Err(key.Error());
    SecureBuffer kek(key.Value());
    auto kekCopy = kek.ToVectorCopy();
    auto dec = WindowsAead::DecryptAes256Gcm(kekCopy, secretFile.nonce, secretFile.encryptedKemPrivateKey, RecoverySecretAad(secretFile.kemAlgorithm), secretFile.tag);
    std::fill(kekCopy.begin(), kekCopy.end(), 0);
    return dec;
}
Result<std::vector<uint8_t>> WalletRecovery::RecoverPayload(PQCryptoProvider& provider,
                                                            const WalletEncryptedBlob& walletBlob,
                                                            const WalletRecoverySecretFile& secretFile,
                                                            const std::string& recoveryPassword) {
    if (walletBlob.pqKemCiphertext.empty() || walletBlob.encryptedDekByKem.empty()) return Result<std::vector<uint8_t>>::Err("wallet blob has no KEM-wrapped DEK");
    auto kemPrivateKey = DecryptKemPrivateKey(secretFile, recoveryPassword, walletBlob.kdfIterations);
    if (kemPrivateKey.IsErr()) return Result<std::vector<uint8_t>>::Err(kemPrivateKey.Error());
    SecureBuffer privateKey(kemPrivateKey.Value());
    auto privateKeyCopy = privateKey.ToVectorCopy();
    auto sharedSecret = provider.Decapsulate(privateKeyCopy, walletBlob.pqKemCiphertext);
    std::fill(privateKeyCopy.begin(), privateKeyCopy.end(), 0);
    SecureBuffer shared(sharedSecret);
    auto kekPq = Hashing::Kmac256(sharedSecret, std::vector<uint8_t>(walletBlob.walletFingerprint.begin(), walletBlob.walletFingerprint.end()), "PQC_WALLET_RECOVERY_KEK_PQ_V1", 32);
    std::fill(sharedSecret.begin(), sharedSecret.end(), 0);
    auto dek = WindowsAead::DecryptAes256Gcm(kekPq, walletBlob.nonceDekKem, walletBlob.encryptedDekByKem, RecoveryDekAad(walletBlob), walletBlob.tagDekKem);
    std::fill(kekPq.begin(), kekPq.end(), 0);
    if (dek.IsErr()) return Result<std::vector<uint8_t>>::Err(dek.Error());
    SecureBuffer dekBuffer(dek.Value());
    auto dekCopy = dekBuffer.ToVectorCopy();
    auto payload = WindowsAead::DecryptAes256Gcm(dekCopy, walletBlob.noncePayload, walletBlob.encryptedPayload, WalletDatFormat::AssociatedData(walletBlob), walletBlob.tagPayload);
    std::fill(dekCopy.begin(), dekCopy.end(), 0);
    return payload;
}
bool WalletRecovery::Test(PQCryptoProvider& provider,
                          const WalletRecoveryPublicFile& publicFile,
                          const WalletRecoverySecretFile& secretFile,
                          const std::string& recoveryPassword,
                          uint32_t iterations) {
    auto secret = DecryptKemPrivateKey(secretFile, recoveryPassword, iterations);
    if (secret.IsErr()) return false;
    auto enc = provider.Encapsulate(publicFile.kemPublicKey);
    auto dec = provider.Decapsulate(secret.Value(), enc.ciphertext);
    return Hashing::SecureCompare(enc.sharedSecret, dec) && ArrayFromVectorHash(publicFile.kemPublicKey).size() == 32;
}
}
