#pragma once
#include "postquantiquecoin/core/Result.h"
#include <cstdint>
#include <vector>
namespace pqc { struct WalletSecretBoxResult { std::vector<uint8_t> ciphertext; std::vector<uint8_t> tag; }; class WalletSecretBox { public: static WalletSecretBoxResult Encrypt(const std::vector<uint8_t>& key,const std::vector<uint8_t>& nonce,const std::vector<uint8_t>& plaintext,const std::vector<uint8_t>& aad); static Result<std::vector<uint8_t>> Decrypt(const std::vector<uint8_t>& key,const std::vector<uint8_t>& nonce,const std::vector<uint8_t>& ciphertext,const std::vector<uint8_t>& aad,const std::vector<uint8_t>& tag); }; }
