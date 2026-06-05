#pragma once
#include "postquantiquecoin/blockchain/UTXO.h"
#include "postquantiquecoin/core/Result.h"
#include <array>
#include <string>
#include <vector>
namespace pqc { struct TxUndo { std::vector<UTXO> spentOutputs; }; struct BlockUndo { std::string blockHash; uint64_t height{0}; std::vector<TxUndo> txUndos; std::array<uint8_t,32> checksum{}; std::vector<uint8_t> Serialize() const; static Result<BlockUndo> Deserialize(const std::vector<uint8_t>& data); }; }
