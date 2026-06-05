#pragma once
#include "postquantiquecoin/blockchain/ChainParams.h"
#include "postquantiquecoin/blockchain/UTXOSet.h"
#include <map>
#include <optional>
#include <set>
namespace pqc {
struct MempoolEntry { Transaction tx; uint64_t feeAtoms{0}; uint64_t receivedTimestamp{0}; size_t serializedSize{0}; double feeRate{0.0}; };
class Mempool {
public:
    Result<void> AddTransaction(const Transaction& tx, const UTXOSet& utxos, uint64_t currentHeight, const ChainParams& params, PQCryptoProvider* crypto);
    void RemoveTransaction(const std::string& txid);
    bool HasTransaction(const std::string& txid) const;
    std::optional<Transaction> GetTransaction(const std::string& txid) const;
    std::vector<Transaction> GetTransactionsForBlock(size_t maxCount, size_t maxBlockBytes) const;
    void RemoveConfirmedTransactions(const Block& block);
    void Clear();
    size_t Size() const;
    uint64_t GetTotalFeesEstimate() const;
    bool DetectConflicts(const Transaction& tx) const;
private:
    std::map<std::string,MempoolEntry> entries_;
    std::set<std::string> spent_;
};
}
