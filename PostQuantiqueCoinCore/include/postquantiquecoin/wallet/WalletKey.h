#pragma once
#include "postquantiquecoin/core/Result.h"
#include <cstdint>
#include <string>
#include <vector>
namespace pqc { struct WalletKey { std::string label; std::vector<uint8_t> publicKey; std::vector<uint8_t> encryptedPrivateKey; std::string address; std::string signatureAlgorithm; uint64_t createdTimestamp{0}; std::vector<uint8_t> Serialize() const; static Result<WalletKey> Deserialize(const std::vector<uint8_t>& data); }; }
