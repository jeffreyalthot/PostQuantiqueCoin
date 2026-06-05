#pragma once
#include "postquantiquecoin/mining/MiningJob.h"
#include <atomic>
#include <mutex>

namespace pqc {
struct MiningStats {
    uint64_t hashesTried{0};
    double secondsElapsed{0.0};
    double hashRate{0.0};
    std::string bestHash;
    uint64_t bestNonce{0};
};

class CpuPowEngine {
public:
    Result<Block> Mine(MiningJob& job, uint32_t threadCount);
    void Stop();
    double GetHashrate() const;
    MiningStats GetStats() const;
    void WorkerLoop(MiningJob& job, uint32_t index, uint32_t threads, std::atomic<bool>& found, Block& result, std::atomic<uint64_t>& hashes);
    void PrintProgress(const MiningJob& job, uint64_t nonce, const std::string& hash, uint32_t threads) const;
private:
    void RecordBestHash(const std::string& hash, uint64_t nonce);
    std::atomic<bool> stopped_{false};
    std::atomic<uint64_t> lastHashes_{0};
    std::atomic<uint64_t> currentHashes_{0};
    std::chrono::steady_clock::time_point miningStart_{};
    std::chrono::steady_clock::time_point miningEnd_{};
    mutable std::mutex statsMutex_;
    std::string bestHash_;
    uint64_t bestNonce_{0};
};
}
