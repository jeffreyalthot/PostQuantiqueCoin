#pragma once
#include "postquantiquecoin/blockchain/Block.h"
#include "postquantiquecoin/blockchain/ChainParams.h"
namespace pqc { class Genesis { public: static constexpr const char* CoinbaseText = "PostQuantiqueCoin Genesis - Proof of Work - 29M PQC - Post Quantum Signatures"; static Block BuildGenesisBlock(const ChainParams& params = ChainParams::Mainnet()); static bool ValidateGenesisBlock(const Block& block, const ChainParams& params); }; }
