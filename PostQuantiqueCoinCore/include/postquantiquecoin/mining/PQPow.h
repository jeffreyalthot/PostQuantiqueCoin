#pragma once
#include "postquantiquecoin/blockchain/BlockHeader.h"
#include "postquantiquecoin/blockchain/ChainParams.h"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace pqc {
class PQPow {
public:
    static std::array<uint8_t, 32> ComputePowHash(const BlockHeader& header, const ChainParams& params);
    static std::array<uint8_t, 32> ComputePqcSha3ShakeV1(const BlockHeader& header);
    static std::string ToHex(const std::array<uint8_t, 32>& hash);
};
}
