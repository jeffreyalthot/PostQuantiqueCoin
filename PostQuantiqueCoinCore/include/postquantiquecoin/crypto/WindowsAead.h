#pragma once
#include "postquantiquecoin/core/Result.h"
#include <cstdint>
#include <vector>
namespace pqc {
struct AeadCiphertext { std::vector<uint8_t> ciphertext; std::vector<uint8_t> authTag; };
class WindowsAead {
public:
    static constexpr uint8_t AlgorithmIdAes256Gcm = 1;
    static Result<AeadCiphertext> EncryptAes256Gcm(const std::vector<uint8_t>& key,
                                                   const std::vector<uint8_t>& nonce,
                                                   const std::vector<uint8_t>& plaintext,
                                                   const std::vector<uint8_t>& associatedData);
    static Result<std::vector<uint8_t>> DecryptAes256Gcm(const std::vector<uint8_t>& key,
                                                         const std::vector<uint8_t>& nonce,
                                                         const std::vector<uint8_t>& ciphertext,
                                                         const std::vector<uint8_t>& associatedData,
                                                         const std::vector<uint8_t>& authTag);
};
}
