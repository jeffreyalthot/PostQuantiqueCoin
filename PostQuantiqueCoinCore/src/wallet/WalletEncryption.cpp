#include "postquantiquecoin/wallet/WalletEncryption.h"
#include "postquantiquecoin/crypto/Hashing.h"
#include "postquantiquecoin/crypto/SecureRandom.h"
#include "postquantiquecoin/crypto/WindowsAead.h"
#include "postquantiquecoin/wallet/WalletDatFormat.h"
#include "postquantiquecoin/wallet/WindowsWalletKdf.h"
namespace pqc {
WalletEncryptedBlob WalletEncryption::EncryptPayload(const std::vector<uint8_t>& plaintext,const std::string& password,uint32_t iterations){
    WalletEncryptedBlob b; b.version=2; b.networkId=1; b.kdfAlgorithm=static_cast<uint8_t>(WalletKdfAlgorithm::PBKDF2_HMAC_SHA256_BCRYPT); b.aeadAlgorithm=WindowsAead::AlgorithmIdAes256Gcm; b.kdfIterations=iterations==0?WindowsWalletKdf::ProductionIterations:iterations; b.salt=SecureRandom::Bytes(32); b.nonce=SecureRandom::Bytes(12);
    auto key=WindowsWalletKdf::DerivePbkdf2HmacSha256(password,b.salt,b.kdfIterations,32); if(key.IsErr()) return b;
    auto enc=WindowsAead::EncryptAes256Gcm(key.Value(),b.nonce,plaintext,WalletDatFormat::AssociatedData(b)); if(enc.IsOk()){ b.ciphertext=enc.Value().ciphertext; b.authTag=enc.Value().authTag; }
    auto serialized=WalletDatFormat::Serialize(b); auto parsed=WalletDatFormat::Deserialize(serialized); if(parsed.IsOk()) b=parsed.Value(); return b;
}
Result<std::vector<uint8_t>> WalletEncryption::DecryptPayload(const WalletEncryptedBlob& b,const std::string& password,uint32_t iterations){
    uint32_t useIterations = b.kdfIterations==0 ? iterations : b.kdfIterations;
    if(b.kdfAlgorithm==static_cast<uint8_t>(WalletKdfAlgorithm::DEV_ONLY_KMAC_LOOP) && b.networkId==1) return Result<std::vector<uint8_t>>::Err("dev-only wallet KDF is forbidden on mainnet");
    auto key=WindowsWalletKdf::DerivePbkdf2HmacSha256(password,b.salt,useIterations,32); if(key.IsErr()) return Result<std::vector<uint8_t>>::Err(key.Error());
    return WindowsAead::DecryptAes256Gcm(key.Value(),b.nonce,b.ciphertext,WalletDatFormat::AssociatedData(b),b.authTag);
}
}
