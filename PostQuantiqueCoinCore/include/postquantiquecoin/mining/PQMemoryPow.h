#pragma once
#include "postquantiquecoin/blockchain/BlockHeader.h"
#include "postquantiquecoin/blockchain/ChainParams.h"
#include <array>
#include <cstdint>
namespace pqc { class PQMemoryPow { public: static std::array<uint8_t,32> ComputeMemoryHardV2(const BlockHeader& header,const ChainParams& params); }; }
