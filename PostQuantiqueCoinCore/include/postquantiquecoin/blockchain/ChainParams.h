#pragma once
#include <cstdint>
#include <string>
namespace pqc {
enum class PowAlgorithm { DoubleSha256Legacy = 0, PQC_SHA3_SHAKE_V1 = 1, PQC_MEMORY_HARD_V2 = 2 };
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
    PowAlgorithm powAlgorithm{PowAlgorithm::PQC_SHA3_SHAKE_V1};
    uint64_t powScratchpadBytes{0};
    uint32_t powMemoryRounds{0};
    std::string powDomain{"PQC_POW_V1"};
    uint32_t powLimitBits{0x207fffff};
    std::string defaultSignatureAlgorithm;
    std::string defaultKemAlgorithm;
    std::string expectedGenesisHash;
    uint8_t networkId{1};
    std::string networkName{"mainnet"};
    bool allowDevCrypto{false};
    bool allowLegacyPow{false};
    bool requireLibOqs{true};
    static ChainParams Mainnet();
    static ChainParams Devnet();
    static ChainParams Testnet();
    static ChainParams Regtest();
    uint64_t GetBlockSubsidy(uint64_t height, uint64_t alreadyMinted) const;
};
}
