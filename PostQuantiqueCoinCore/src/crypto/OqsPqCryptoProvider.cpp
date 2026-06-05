#include "postquantiquecoin/crypto/OqsPqCryptoProvider.h"
#include <stdexcept>
#ifdef PQC_USE_LIBOQS
#include <oqs/oqs.h>
#endif
namespace pqc {
#ifdef PQC_USE_LIBOQS
namespace { const char* SigAlg(){ return OQS_SIG_alg_ml_dsa_65; } const char* KemAlg(){ return OQS_KEM_alg_ml_kem_768; } }
PQKeyPair OqsPqCryptoProvider::GenerateSigningKeyPair(){ OQS_SIG* sig=OQS_SIG_new(SigAlg()); if(!sig) throw std::runtime_error("ML-DSA-65 unavailable in liboqs"); PQKeyPair kp; kp.publicKey.resize(sig->length_public_key); kp.privateKey.resize(sig->length_secret_key); if(OQS_SIG_keypair(sig,kp.publicKey.data(),kp.privateKey.data())!=OQS_SUCCESS){ OQS_SIG_free(sig); throw std::runtime_error("ML-DSA key generation failed"); } kp.algorithm=SigAlg(); OQS_SIG_free(sig); return kp; }
std::vector<uint8_t> OqsPqCryptoProvider::Sign(const std::vector<uint8_t>& privateKey,const std::vector<uint8_t>& message){ OQS_SIG* sig=OQS_SIG_new(SigAlg()); if(!sig) throw std::runtime_error("ML-DSA unavailable"); std::vector<uint8_t> out(sig->length_signature); size_t len=0; if(OQS_SIG_sign(sig,out.data(),&len,message.data(),message.size(),privateKey.data())!=OQS_SUCCESS){ OQS_SIG_free(sig); throw std::runtime_error("ML-DSA sign failed"); } out.resize(len); OQS_SIG_free(sig); return out; }
bool OqsPqCryptoProvider::Verify(const std::vector<uint8_t>& publicKey,const std::vector<uint8_t>& message,const std::vector<uint8_t>& signature){ OQS_SIG* sig=OQS_SIG_new(SigAlg()); if(!sig) return false; bool ok=OQS_SIG_verify(sig,message.data(),message.size(),signature.data(),signature.size(),publicKey.data())==OQS_SUCCESS; OQS_SIG_free(sig); return ok; }
PQKemKeyPair OqsPqCryptoProvider::GenerateKemKeyPair(){ OQS_KEM* kem=OQS_KEM_new(KemAlg()); if(!kem) throw std::runtime_error("ML-KEM-768 unavailable in liboqs"); PQKemKeyPair kp; kp.publicKey.resize(kem->length_public_key); kp.privateKey.resize(kem->length_secret_key); if(OQS_KEM_keypair(kem,kp.publicKey.data(),kp.privateKey.data())!=OQS_SUCCESS){ OQS_KEM_free(kem); throw std::runtime_error("ML-KEM key generation failed"); } kp.algorithm=KemAlg(); OQS_KEM_free(kem); return kp; }
KemEncapsulation OqsPqCryptoProvider::Encapsulate(const std::vector<uint8_t>& publicKey){ OQS_KEM* kem=OQS_KEM_new(KemAlg()); if(!kem) throw std::runtime_error("ML-KEM unavailable"); KemEncapsulation e; e.ciphertext.resize(kem->length_ciphertext); e.sharedSecret.resize(kem->length_shared_secret); if(OQS_KEM_encaps(kem,e.ciphertext.data(),e.sharedSecret.data(),publicKey.data())!=OQS_SUCCESS){ OQS_KEM_free(kem); throw std::runtime_error("ML-KEM encapsulation failed"); } OQS_KEM_free(kem); return e; }
std::vector<uint8_t> OqsPqCryptoProvider::Decapsulate(const std::vector<uint8_t>& privateKey,const std::vector<uint8_t>& ciphertext){ OQS_KEM* kem=OQS_KEM_new(KemAlg()); if(!kem) throw std::runtime_error("ML-KEM unavailable"); std::vector<uint8_t> ss(kem->length_shared_secret); if(OQS_KEM_decaps(kem,ss.data(),ciphertext.data(),privateKey.data())!=OQS_SUCCESS){ OQS_KEM_free(kem); throw std::runtime_error("ML-KEM decapsulation failed"); } OQS_KEM_free(kem); return ss; }
#else
PQKeyPair OqsPqCryptoProvider::GenerateSigningKeyPair(){ throw std::runtime_error("liboqs support is not compiled in"); }
std::vector<uint8_t> OqsPqCryptoProvider::Sign(const std::vector<uint8_t>&,const std::vector<uint8_t>&){ throw std::runtime_error("liboqs support is not compiled in"); }
bool OqsPqCryptoProvider::Verify(const std::vector<uint8_t>&,const std::vector<uint8_t>&,const std::vector<uint8_t>&){ return false; }
PQKemKeyPair OqsPqCryptoProvider::GenerateKemKeyPair(){ throw std::runtime_error("liboqs support is not compiled in"); }
KemEncapsulation OqsPqCryptoProvider::Encapsulate(const std::vector<uint8_t>&){ throw std::runtime_error("liboqs support is not compiled in"); }
std::vector<uint8_t> OqsPqCryptoProvider::Decapsulate(const std::vector<uint8_t>&,const std::vector<uint8_t>&){ throw std::runtime_error("liboqs support is not compiled in"); }
#endif
std::string OqsPqCryptoProvider::ProviderName() const { return "OqsPqCryptoProvider ML-DSA-65 ML-KEM-768"; }
}
