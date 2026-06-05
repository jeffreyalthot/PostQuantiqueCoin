#pragma once
#include "postquantiquecoin/blockchain/BlockHeader.h"
#include "postquantiquecoin/blockchain/Transaction.h"
#include "postquantiquecoin/core/Result.h"
#include <vector>
namespace pqc { class Block { public: BlockHeader header; std::vector<Transaction> transactions; std::string GetHash() const; std::vector<uint8_t> Serialize() const; static Result<Block> Deserialize(const std::vector<uint8_t>& data); Result<void> ValidateBasic() const; }; }
