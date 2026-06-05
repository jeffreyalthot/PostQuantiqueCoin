#pragma once
#include "postquantiquecoin/core/Result.h"
#include <cstdint>
#include <vector>
namespace pqc { struct WalletRecoveryFile { uint32_t version{1}; std::vector<uint8_t> kemPublicKey; std::vector<uint8_t> kemPrivateKey; std::vector<uint8_t> wrappedDek; std::vector<uint8_t> Serialize() const; static Result<WalletRecoveryFile> Deserialize(const std::vector<uint8_t>& data); }; }
