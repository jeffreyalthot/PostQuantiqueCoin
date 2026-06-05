#pragma once
#include <cstdint>
#include <string>
namespace pqc {
struct ChainParams {
    std::string name;
    std::string ticker;
    uint8_t decimals;
    uint64_t maxSupplyCoins;
    uint64_t maxSupplyAtoms;
    uint64_t targetBlockTimeSeconds;
    uint32_t difficultyAdjustmentInterval;
    uint64_t coinbaseMaturity;
    uint64_t initialSubsidyAtoms;
    uint64_t halvingInterval;
    uint32_t initialBits;
    std::string powHashAlgorithm;
    std::string defaultSignatureAlgorithm;
    std::string defaultKemAlgorithm;
    std::string expectedGenesisHash;
    static ChainParams Mainnet();
    static ChainParams Devnet();
    uint64_t GetBlockSubsidy(uint64_t height, uint64_t alreadyMinted) const;
};
}
