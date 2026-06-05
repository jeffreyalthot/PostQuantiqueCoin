#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace pqc { struct TxInput { std::string previousTxid; uint32_t outputIndex{0}; std::vector<uint8_t> unlockingSignature; std::vector<uint8_t> publicKey; bool IsCoinbase() const { return previousTxid == std::string(64,'0') && outputIndex == UINT32_MAX; } }; }
