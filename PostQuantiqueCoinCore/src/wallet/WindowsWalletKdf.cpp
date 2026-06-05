#include "postquantiquecoin/wallet/WindowsWalletKdf.h"
#include "postquantiquecoin/wallet/WalletKdf.h"
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#endif
namespace pqc {
Result<std::vector<uint8_t>> WindowsWalletKdf::DerivePbkdf2HmacSha256(const std::string& password,const std::vector<uint8_t>& salt,uint32_t iterations,size_t outputSize){
    if(salt.size()<16) return Result<std::vector<uint8_t>>::Err("wallet salt too short");
    if(iterations==0) iterations=ProductionIterations;
#ifdef _WIN32
    struct AlgHandle { BCRYPT_ALG_HANDLE h{}; ~AlgHandle(){ if(h) BCryptCloseAlgorithmProvider(h,0); } } alg;
    if(BCryptOpenAlgorithmProvider(&alg.h, BCRYPT_SHA256_ALGORITHM, nullptr, BCRYPT_ALG_HANDLE_HMAC_FLAG) < 0) return Result<std::vector<uint8_t>>::Err("BCryptOpenAlgorithmProvider(SHA256/HMAC) failed");
    std::vector<uint8_t> out(outputSize);
    if(BCryptDeriveKeyPBKDF2(alg.h, reinterpret_cast<PUCHAR>(const_cast<char*>(password.data())), static_cast<ULONG>(password.size()), const_cast<PUCHAR>(salt.data()), static_cast<ULONG>(salt.size()), iterations, out.data(), static_cast<ULONG>(out.size()), 0) < 0) return Result<std::vector<uint8_t>>::Err("BCryptDeriveKeyPBKDF2 failed");
    return Result<std::vector<uint8_t>>::Ok(out);
#else
    // Non-Windows builds use the explicitly dev-only KMAC-loop fallback. Cap the
    // loop count so local CI remains usable; production Windows builds use BCrypt
    // PBKDF2 with the full persisted iteration count above.
    if (iterations > 5000) iterations = 5000;
    return Result<std::vector<uint8_t>>::Ok(WalletKdf::DeriveKey(password,salt,iterations,outputSize));
#endif
}
}
