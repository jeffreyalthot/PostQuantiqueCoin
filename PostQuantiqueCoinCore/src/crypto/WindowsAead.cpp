#include "postquantiquecoin/crypto/WindowsAead.h"
#include "postquantiquecoin/crypto/Hashing.h"
#include "postquantiquecoin/wallet/WalletSecretBox.h"
#include <algorithm>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#endif
namespace pqc {
namespace {
Result<void> CheckInputs(const std::vector<uint8_t>& key,const std::vector<uint8_t>& nonce){
    if(key.size()!=32) return Result<void>::Err("AES-256-GCM requires a 32-byte key");
    if(nonce.size()!=12) return Result<void>::Err("AES-256-GCM requires a 12-byte nonce");
    return Result<void>::Ok();
}
}
Result<AeadCiphertext> WindowsAead::EncryptAes256Gcm(const std::vector<uint8_t>& key,const std::vector<uint8_t>& nonce,const std::vector<uint8_t>& plaintext,const std::vector<uint8_t>& associatedData){
    auto chk=CheckInputs(key,nonce); if(chk.IsErr()) return Result<AeadCiphertext>::Err(chk.Error());
#ifdef _WIN32
    struct AlgHandle { BCRYPT_ALG_HANDLE h{}; ~AlgHandle(){ if(h) BCryptCloseAlgorithmProvider(h,0); } } alg;
    if(BCryptOpenAlgorithmProvider(&alg.h, BCRYPT_AES_ALGORITHM, nullptr, 0) < 0) return Result<AeadCiphertext>::Err("BCryptOpenAlgorithmProvider(AES) failed");
    if(BCryptSetProperty(alg.h, BCRYPT_CHAINING_MODE, reinterpret_cast<PUCHAR>(const_cast<wchar_t*>(BCRYPT_CHAIN_MODE_GCM)), sizeof(BCRYPT_CHAIN_MODE_GCM), 0) < 0) return Result<AeadCiphertext>::Err("BCryptSetProperty(GCM) failed");
    DWORD objLen=0, cb=0; if(BCryptGetProperty(alg.h, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&objLen), sizeof(objLen), &cb, 0) < 0) return Result<AeadCiphertext>::Err("BCryptGetProperty(object length) failed");
    std::vector<uint8_t> obj(objLen);
    struct KeyHandle { BCRYPT_KEY_HANDLE h{}; ~KeyHandle(){ if(h) BCryptDestroyKey(h); } } kh;
    if(BCryptGenerateSymmetricKey(alg.h, &kh.h, obj.data(), objLen, const_cast<PUCHAR>(key.data()), static_cast<ULONG>(key.size()), 0) < 0) return Result<AeadCiphertext>::Err("BCryptGenerateSymmetricKey failed");
    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO info; BCRYPT_INIT_AUTH_MODE_INFO(info);
    std::vector<uint8_t> tag(16); info.pbNonce=const_cast<PUCHAR>(nonce.data()); info.cbNonce=static_cast<ULONG>(nonce.size()); info.pbAuthData=const_cast<PUCHAR>(associatedData.data()); info.cbAuthData=static_cast<ULONG>(associatedData.size()); info.pbTag=tag.data(); info.cbTag=static_cast<ULONG>(tag.size());
    AeadCiphertext out; out.ciphertext.resize(plaintext.size()); ULONG outLen=0;
    if(BCryptEncrypt(kh.h, const_cast<PUCHAR>(plaintext.data()), static_cast<ULONG>(plaintext.size()), &info, nullptr, 0, out.ciphertext.data(), static_cast<ULONG>(out.ciphertext.size()), &outLen, 0) < 0) return Result<AeadCiphertext>::Err("BCryptEncrypt(AES-GCM) failed");
    out.ciphertext.resize(outLen); out.authTag=std::move(tag); return Result<AeadCiphertext>::Ok(std::move(out));
#else
    auto fallback=WalletSecretBox::Encrypt(key,nonce,plaintext,associatedData);
    return Result<AeadCiphertext>::Ok({fallback.ciphertext,fallback.tag});
#endif
}
Result<std::vector<uint8_t>> WindowsAead::DecryptAes256Gcm(const std::vector<uint8_t>& key,const std::vector<uint8_t>& nonce,const std::vector<uint8_t>& ciphertext,const std::vector<uint8_t>& associatedData,const std::vector<uint8_t>& authTag){
    auto chk=CheckInputs(key,nonce); if(chk.IsErr()) return Result<std::vector<uint8_t>>::Err(chk.Error());
#ifdef _WIN32
    struct AlgHandle { BCRYPT_ALG_HANDLE h{}; ~AlgHandle(){ if(h) BCryptCloseAlgorithmProvider(h,0); } } alg;
    if(BCryptOpenAlgorithmProvider(&alg.h, BCRYPT_AES_ALGORITHM, nullptr, 0) < 0) return Result<std::vector<uint8_t>>::Err("BCryptOpenAlgorithmProvider(AES) failed");
    if(BCryptSetProperty(alg.h, BCRYPT_CHAINING_MODE, reinterpret_cast<PUCHAR>(const_cast<wchar_t*>(BCRYPT_CHAIN_MODE_GCM)), sizeof(BCRYPT_CHAIN_MODE_GCM), 0) < 0) return Result<std::vector<uint8_t>>::Err("BCryptSetProperty(GCM) failed");
    DWORD objLen=0, cb=0; if(BCryptGetProperty(alg.h, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&objLen), sizeof(objLen), &cb, 0) < 0) return Result<std::vector<uint8_t>>::Err("BCryptGetProperty(object length) failed");
    std::vector<uint8_t> obj(objLen); struct KeyHandle { BCRYPT_KEY_HANDLE h{}; ~KeyHandle(){ if(h) BCryptDestroyKey(h); } } kh;
    if(BCryptGenerateSymmetricKey(alg.h, &kh.h, obj.data(), objLen, const_cast<PUCHAR>(key.data()), static_cast<ULONG>(key.size()), 0) < 0) return Result<std::vector<uint8_t>>::Err("BCryptGenerateSymmetricKey failed");
    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO info; BCRYPT_INIT_AUTH_MODE_INFO(info); info.pbNonce=const_cast<PUCHAR>(nonce.data()); info.cbNonce=static_cast<ULONG>(nonce.size()); info.pbAuthData=const_cast<PUCHAR>(associatedData.data()); info.cbAuthData=static_cast<ULONG>(associatedData.size()); info.pbTag=const_cast<PUCHAR>(authTag.data()); info.cbTag=static_cast<ULONG>(authTag.size());
    std::vector<uint8_t> out(ciphertext.size()); ULONG outLen=0; if(BCryptDecrypt(kh.h, const_cast<PUCHAR>(ciphertext.data()), static_cast<ULONG>(ciphertext.size()), &info, nullptr, 0, out.data(), static_cast<ULONG>(out.size()), &outLen, 0) < 0) return Result<std::vector<uint8_t>>::Err("wallet authentication failed");
    out.resize(outLen); return Result<std::vector<uint8_t>>::Ok(out);
#else
    return WalletSecretBox::Decrypt(key,nonce,ciphertext,associatedData,authTag);
#endif
}
}
