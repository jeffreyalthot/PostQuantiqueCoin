#include "postquantiquecoin/crypto/DevOnlyFallbackCryptoProvider.h"
#include "postquantiquecoin/crypto/Hashing.h"
#include "postquantiquecoin/crypto/SecureRandom.h"
#include <stdexcept>
namespace pqc {
PQKeyPair DevOnlyFallbackCryptoProvider::GenerateSigningKeyPair(){ auto priv=SecureRandom::Bytes(32); auto pub=Hashing::HmacSha256(priv, {'P','Q','C','_','D','E','V','_','P','U','B'}); return {pub,priv,"DEV-ONLY-HMAC-SHA256-NON-SECURE"}; }
std::vector<uint8_t> DevOnlyFallbackCryptoProvider::Sign(const std::vector<uint8_t>& privateKey,const std::vector<uint8_t>& message){ auto pub=Hashing::HmacSha256(privateKey, {'P','Q','C','_','D','E','V','_','P','U','B'}); return Hashing::HmacSha256(pub,message); }
bool DevOnlyFallbackCryptoProvider::Verify(const std::vector<uint8_t>& publicKey,const std::vector<uint8_t>& message,const std::vector<uint8_t>& signature){ return Hashing::SecureCompare(Hashing::HmacSha256(publicKey,message),signature); }
PQKemKeyPair DevOnlyFallbackCryptoProvider::GenerateKemKeyPair(){ auto priv=SecureRandom::Bytes(32); auto pub=Hashing::HmacSha256(priv, {'P','Q','C','_','D','E','V','_','K','E','M'}); return {pub,priv,"DEV-ONLY-KEM-NON-SECURE"}; }
KemEncapsulation DevOnlyFallbackCryptoProvider::Encapsulate(const std::vector<uint8_t>& publicKey){ auto eph=SecureRandom::Bytes(32); auto ct=eph; auto ss=Hashing::HmacSha256(publicKey,eph); return {ct,ss}; }
std::vector<uint8_t> DevOnlyFallbackCryptoProvider::Decapsulate(const std::vector<uint8_t>& privateKey,const std::vector<uint8_t>& ciphertext){ auto pub=Hashing::HmacSha256(privateKey, {'P','Q','C','_','D','E','V','_','K','E','M'}); return Hashing::HmacSha256(pub,ciphertext); }
std::string DevOnlyFallbackCryptoProvider::ProviderName() const { return "DevOnlyFallbackCryptoProvider (NON-SECURE LOCAL TESTING)"; }
}
