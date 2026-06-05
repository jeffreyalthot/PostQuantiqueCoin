#include "postquantiquecoin/mining/CpuPowEngine.h"
#include "postquantiquecoin/blockchain/Difficulty.h"
#include "postquantiquecoin/blockchain/MerkleTree.h"
#include "postquantiquecoin/core/Logger.h"
#include "postquantiquecoin/mining/PQPow.h"
#include <chrono>
#include <thread>

namespace pqc {
Result<Block> CpuPowEngine::Mine(MiningJob& job, uint32_t threads) {
    stopped_ = false;
    if (threads == 0) threads = 1;
    std::atomic<bool> found{false};
    std::atomic<uint64_t> hashes{0};
    currentHashes_ = 0;
    {
        std::lock_guard<std::mutex> lock(statsMutex_);
        bestHash_.clear();
        bestNonce_ = 0;
        miningStart_ = std::chrono::steady_clock::now();
        miningEnd_ = miningStart_;
    }
    Block result;
    std::vector<std::thread> workers;
    for (uint32_t i = 0; i < threads; ++i) workers.emplace_back(&CpuPowEngine::WorkerLoop, this, std::ref(job), i, threads, std::ref(found), std::ref(result), std::ref(hashes));
    for (auto& t : workers) t.join();
    lastHashes_ = hashes.load();
    currentHashes_ = hashes.load();
    {
        std::lock_guard<std::mutex> lock(statsMutex_);
        miningEnd_ = std::chrono::steady_clock::now();
    }
    if (found) return Result<Block>::Ok(result);
    return Result<Block>::Err("mining stopped");
}

void CpuPowEngine::Stop() { stopped_ = true; }

double CpuPowEngine::GetHashrate() const { return GetStats().hashRate; }

MiningStats CpuPowEngine::GetStats() const {
    std::lock_guard<std::mutex> lock(statsMutex_);
    const auto end = miningEnd_ == miningStart_ ? std::chrono::steady_clock::now() : miningEnd_;
    const double seconds = std::chrono::duration<double>(end - miningStart_).count();
    MiningStats stats;
    stats.hashesTried = currentHashes_.load();
    stats.secondsElapsed = seconds;
    stats.hashRate = seconds > 0.0 ? static_cast<double>(stats.hashesTried) / seconds : 0.0;
    stats.bestHash = bestHash_;
    stats.bestNonce = bestNonce_;
    return stats;
}

void CpuPowEngine::RecordBestHash(const std::string& hash, uint64_t nonce) {
    std::lock_guard<std::mutex> lock(statsMutex_);
    if (bestHash_.empty() || hash < bestHash_) {
        bestHash_ = hash;
        bestNonce_ = nonce;
    }
}

void CpuPowEngine::WorkerLoop(MiningJob& job, uint32_t idx, uint32_t threads, std::atomic<bool>& found, Block& result, std::atomic<uint64_t>& hashes) {
    Block b = job.candidateBlock;
    if (!b.transactions.empty()) {
        auto extra = job.BuildExtraNonce(idx, 0);
        b.transactions[0].inputs[0].unlockingSignature = extra;
        b.transactions[0].txid = b.transactions[0].ComputeTxId();
        b.header.merkleRoot = MerkleTree::ComputeRoot(b.transactions);
    }
    for (uint64_t n = idx; !stopped_ && !job.cancelled.load() && !found.load(); n += threads) {
        b.header.nonce = n;
        auto powHash = PQPow::ComputePowHash(b.header, job.params);
        auto h = PQPow::ToHex(powHash);
        ++hashes;
        currentHashes_ = hashes.load();
        RecordBestHash(h, n);
        if ((n % (threads * 200000ULL)) == idx) PrintProgress(job, n, h, threads);
        if (Difficulty::CheckProofOfWorkHash(powHash, b.header.bits)) {
            if (!found.exchange(true)) result = b;
            break;
        }
    }
}

void CpuPowEngine::PrintProgress(const MiningJob& job, uint64_t nonce, const std::string& hash, uint32_t threads) const {
    Logger::LogInfo("mining", "height=" + std::to_string(job.height) + " nonce=" + std::to_string(nonce) +
                               " pow_algorithm=" + job.params.powHashAlgorithm + " pow_hash=" + hash +
                               " bits=" + std::to_string(job.bits) + " threads=" + std::to_string(threads));
}
}
