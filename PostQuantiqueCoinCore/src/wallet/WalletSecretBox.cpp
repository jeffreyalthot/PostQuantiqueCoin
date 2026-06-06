#include "postquantiquecoin/wallet/WalletSecretBox.h"
#include "postquantiquecoin/crypto/Hashing.h"
#include <stdexcept>
namespace pqc {
namespace {
void RequireDevOnlySecretBox() {
#ifndef PQC_DEV_ONLY_ALLOW_INSECURE_CRYPTO
    throw std::logic_error("WalletSecretBox is insecure dev-only fallback crypto and is disabled in production builds");
#endif
}
std::vector<uint8_t> Stream(const std::vector<uint8_t>& key,const std::vector<uint8_t>& nonce,size_t n){ std::vector<uint8_t> seed=nonce; return Hashing::Kmac256(key,seed,"PQC_WALLET_STREAM_V1",n); }
std::vector<uint8_t> Tag(const std::vector<uint8_t>& key,const std::vector<uint8_t>& nonce,const std::vector<uint8_t>& ct,const std::vector<uint8_t>& aad){ std::vector<uint8_t> m=nonce; m.insert(m.end(),aad.begin(),aad.end()); m.insert(m.end(),ct.begin(),ct.end()); return Hashing::Kmac256(key,m,"PQC_WALLET_AEAD_TAG_V1",16); }
}
WalletSecretBoxResult WalletSecretBox::Encrypt(const std::vector<uint8_t>& key,const std::vector<uint8_t>& nonce,const std::vector<uint8_t>& plaintext,const std::vector<uint8_t>& aad){ RequireDevOnlySecretBox(); auto ks=Stream(key,nonce,plaintext.size()); WalletSecretBoxResult r; r.ciphertext.resize(plaintext.size()); for(size_t i=0;i<plaintext.size();++i) r.ciphertext[i]=plaintext[i]^ks[i]; r.tag=Tag(key,nonce,r.ciphertext,aad); return r; }
Result<std::vector<uint8_t>> WalletSecretBox::Decrypt(const std::vector<uint8_t>& key,const std::vector<uint8_t>& nonce,const std::vector<uint8_t>& ciphertext,const std::vector<uint8_t>& aad,const std::vector<uint8_t>& tag){ RequireDevOnlySecretBox(); auto expected=Tag(key,nonce,ciphertext,aad); if(!Hashing::SecureCompare(expected,tag)) return Result<std::vector<uint8_t>>::Err("wallet authentication failed"); auto ks=Stream(key,nonce,ciphertext.size()); std::vector<uint8_t> pt(ciphertext.size()); for(size_t i=0;i<ciphertext.size();++i) pt[i]=ciphertext[i]^ks[i]; return Result<std::vector<uint8_t>>::Ok(pt); }
}
