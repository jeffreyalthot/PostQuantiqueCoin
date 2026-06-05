#include "postquantiquecoin/blockchain/ChainParams.h"
#include "postquantiquecoin/core/Constants.h"
namespace pqc {
ChainParams ChainParams::Mainnet(){ return {std::string(constants::PROJECT_NAME),std::string(constants::COIN_SYMBOL),constants::COIN_DECIMALS,constants::MAX_SUPPLY_COINS,constants::MAX_SUPPLY_ATOMS,constants::TARGET_BLOCK_TIME_SECONDS,constants::DIFFICULTY_ADJUSTMENT_INTERVAL,constants::COINBASE_MATURITY,constants::INITIAL_SUBSIDY_ATOMS,constants::HALVING_INTERVAL,0x207fffff,"DoubleSha256 header hash; SHA3-256 used for addresses and signing domain digests","ML-DSA-65","ML-KEM-768",""}; }
ChainParams ChainParams::Devnet(){ auto p=Mainnet(); p.name="PostQuantiqueCoin-devnet"; p.coinbaseMaturity=2; p.difficultyAdjustmentInterval=10; p.targetBlockTimeSeconds=15; p.initialBits=0x207fffff; p.expectedGenesisHash=""; return p; }
uint64_t ChainParams::GetBlockSubsidy(uint64_t height,uint64_t alreadyMinted) const { if(alreadyMinted>=maxSupplyAtoms) return 0; uint64_t halvings=halvingInterval==0?0:height/halvingInterval; uint64_t subsidy = halvings>=64 ? 0 : (initialSubsidyAtoms >> halvings); uint64_t remaining=maxSupplyAtoms-alreadyMinted; return subsidy>remaining?remaining:subsidy; }
}
