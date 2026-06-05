#pragma once
#include "postquantiquecoin/blockchain/Block.h"
#include "postquantiquecoin/blockchain/ChainParams.h"
#include <cstdint>
#include <string>
#include <vector>
namespace pqc {
class Difficulty {
public:
    static std::vector<uint8_t> CompactToTarget(uint32_t bits);
    static uint32_t TargetToCompact(const std::vector<uint8_t>& target);
    static bool CheckProofOfWork(const std::string& hashHex, uint32_t bits);
    static uint32_t CalculateNextWorkRequired(const std::vector<Block>& previousBlocks, const ChainParams& params);
    static uint64_t ClampAdjustment(uint64_t actualTimespan, uint64_t expectedTimespan);
    static int CompareHashToTarget(const std::string& hashHex, const std::vector<uint8_t>& target);
};
}
