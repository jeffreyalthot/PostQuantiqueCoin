#pragma once
#include "postquantiquecoin/blockchain/Blockchain.h"
#include "postquantiquecoin/mining/CpuPowEngine.h"
namespace pqc { class Miner { public: explicit Miner(Blockchain& chain); Result<MiningJob> BuildCandidateBlock(const std::string& minerAddress); void AddCoinbaseTransaction(Block& block,const std::string& address,uint64_t reward); uint64_t SelectMempoolTransactions(Block& block); Result<Block> MineNextBlock(const std::string& minerAddress,uint32_t threadCount); Result<void> SubmitBlock(const Block& block); void StartLoop(const std::string& minerAddress,uint32_t threadCount); void Stop(); private: Blockchain& chain_; CpuPowEngine engine_; std::atomic<bool> stop_{false}; }; }
