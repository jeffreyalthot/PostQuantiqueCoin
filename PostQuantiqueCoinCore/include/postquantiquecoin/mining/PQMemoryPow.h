#pragma once
#include "postquantiquecoin/blockchain/BlockHeader.h"
#include "postquantiquecoin/blockchain/ChainParams.h"
#include <array>
#include <cstdint>
#include <vector>

namespace pqc {
class PQMemoryPowContext {
public:
    void BuildScratchpad(const BlockHeader& headerWithoutNonce, const ChainParams& params);
    std::array<uint8_t,32> ComputeWithNonce(uint64_t nonce) const;
private:
    BlockHeader headerTemplate_;
    ChainParams params_{};
    std::vector<uint8_t> serializedWithoutNonce_;
    std::vector<std::array<uint8_t,64>> scratch_;
    std::array<uint8_t,64> seed_{};
};

class PQMemoryPow {
public:
    static std::array<uint8_t,32> ComputeMemoryHardV2(const BlockHeader& header, const ChainParams& params);
};
}
