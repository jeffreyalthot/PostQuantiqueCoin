#include "postquantiquecoin/mining/Miner.h"
#include "postquantiquecoin/blockchain/MerkleTree.h"
#include "postquantiquecoin/core/Logger.h"
#include "postquantiquecoin/crypto/PQCAddress.h"
#include <chrono>

namespace pqc {
namespace {
uint64_t NowSeconds() {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
}
}

Miner::Miner(Blockchain& c) : chain_(c) {}

void Miner::AddCoinbaseTransaction(Block& b, const std::string& addr, uint64_t reward) {
    auto decoded = PQCAddress::Decode(addr, chain_.Params().networkId);
    if (decoded.IsErr()) decoded = PQCAddress::Decode(addr);
    Transaction cb;
    cb.version = 2;
    cb.timestamp = NowSeconds();
    TxInput coinbase;
    coinbase.previousTxid = std::string(64, '0');
    coinbase.outputIndex = UINT32_MAX;
    coinbase.unlockingSignature = {'P','Q','C'};
    coinbase.signatureAlgorithm = chain_.Params().defaultSignatureAlgorithm;
    cb.inputs.push_back(std::move(coinbase));

    TxOutput out;
    out.amountAtoms = reward;
    out.address = addr;
    out.lockingScriptType = "PQC_PUBKEY_HASH";
    out.requiredSignatureAlgorithm = chain_.Params().defaultSignatureAlgorithm.empty() ? "ML-DSA-65" : chain_.Params().defaultSignatureAlgorithm;
    if (decoded.IsOk()) out.publicKeyHash = decoded.Value().publicKeyHash;
    cb.outputs.push_back(out);
    cb.txid = cb.ComputeTxId();
    b.transactions.insert(b.transactions.begin(), cb);
}

uint64_t Miner::SelectMempoolTransactions(Block& b) {
    auto txs = chain_.GetMempool().GetTransactionsForBlock(1000, 3000000);
    for (const auto& tx : txs) b.transactions.push_back(tx);
    return chain_.GetMempool().GetTotalFeesEstimate();
}

Result<MiningJob> Miner::BuildCandidateBlock(const std::string& addr) {
    auto decoded = PQCAddress::Decode(addr, chain_.Params().networkId);
    if (decoded.IsErr()) decoded = PQCAddress::Decode(addr);
    if (decoded.IsErr()) return Result<MiningJob>::Err("invalid miner address: " + decoded.Error());
    MiningJob j;
    j.minerAddress = addr;
    j.params = chain_.Params();
    j.height = chain_.GetHeight() + 1;
    j.subsidyAtoms = chain_.GetNextSubsidy();
    j.bits = chain_.GetNextWorkRequired();
    j.extraNonceBase = NowSeconds();
    j.candidateBlock.header.previousHash = chain_.GetTipHash();
    j.candidateBlock.header.height = j.height;
    j.candidateBlock.header.bits = j.bits;
    j.candidateBlock.header.powAlgorithmId = static_cast<uint16_t>(chain_.Params().powAlgorithm);
    j.candidateBlock.header.timestamp = NowSeconds();
    j.feesAtoms = SelectMempoolTransactions(j.candidateBlock);
    AddCoinbaseTransaction(j.candidateBlock, addr, j.subsidyAtoms + j.feesAtoms);
    j.candidateBlock.header.merkleRoot = MerkleTree::ComputeRoot(j.candidateBlock.transactions);
    return Result<MiningJob>::Ok(j);
}

Result<Block> Miner::MineNextBlock(const std::string& addr, uint32_t threads) {
    auto j = BuildCandidateBlock(addr);
    if (j.IsErr()) return Result<Block>::Err(j.Error());
    auto b = engine_.Mine(j.Value(), threads);
    if (b.IsErr()) return b;
    auto s = SubmitBlock(b.Value());
    if (s.IsErr()) return Result<Block>::Err(s.Error());
    Logger::LogInfo("mining", "mined block " + b.Value().header.GetBlockId());
    return b;
}

Result<void> Miner::SubmitBlock(const Block& b) { return chain_.AddBlock(b); }

void Miner::StartLoop(const std::string& a, uint32_t t) {
    stop_ = false;
    while (!stop_) {
        auto r = MineNextBlock(a, t);
        if (r.IsErr()) Logger::LogWarn("mining", "mining error: " + r.Error());
    }
}

void Miner::Stop() { stop_ = true; engine_.Stop(); }
}
