#pragma once
#include "postquantiquecoin/blockchain/Block.h"
#include "postquantiquecoin/blockchain/UTXO.h"
#include "postquantiquecoin/core/Result.h"
#include <map>
#include <optional>
#include <vector>
namespace pqc {
class UTXOSet {
public:
    Result<void> AddUTXO(const UTXO& utxo);
    Result<void> RemoveUTXO(const std::string& txid, uint32_t outputIndex);
    bool HasUTXO(const std::string& txid, uint32_t outputIndex) const;
    std::optional<UTXO> GetUTXO(const std::string& txid, uint32_t outputIndex) const;
    uint64_t GetBalance(const std::string& address, uint64_t currentHeight, uint64_t coinbaseMaturity) const;
    std::vector<UTXO> GetSpendableUTXOs(const std::string& address, uint64_t currentHeight, uint64_t coinbaseMaturity) const;
    Result<std::vector<UTXO>> ApplyTransaction(const Transaction& tx, uint64_t height, bool isCoinbase, uint64_t currentHeight, uint64_t coinbaseMaturity);
    Result<void> RevertTransaction(const Transaction& tx, const std::vector<UTXO>& spentUtxos);
    Result<void> ApplyBlock(const Block& block, uint64_t height, uint64_t coinbaseMaturity);
    std::vector<UTXO> Snapshot() const;
    Result<void> LoadSnapshot(const std::vector<UTXO>& snapshot);
    Result<void> LoadSnapshot();
    void Clear();
    size_t Size() const;
private:
    static std::string Key(const std::string& txid, uint32_t outputIndex);
    std::map<std::string, UTXO> entries_;
};
}
