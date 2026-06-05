#pragma once
#include "postquantiquecoin/mining/MiningJob.h"
#include <atomic>
namespace pqc { class CpuPowEngine { public: Result<Block> Mine(MiningJob& job,uint32_t threadCount); void Stop(); double GetHashrate() const; void WorkerLoop(MiningJob& job,uint32_t index,uint32_t threads,std::atomic<bool>& found,Block& result,std::atomic<uint64_t>& hashes); void PrintProgress(const MiningJob& job,uint64_t nonce,const std::string& hash,uint32_t threads) const; private: std::atomic<bool> stopped_{false}; std::atomic<uint64_t> lastHashes_{0}; }; }
