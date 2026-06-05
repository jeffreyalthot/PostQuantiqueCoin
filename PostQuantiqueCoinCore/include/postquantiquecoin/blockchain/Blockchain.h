#pragma once
#include "postquantiquecoin/blockchain/Mempool.h"
#include "postquantiquecoin/storage/BlockIndex.h"
#include "postquantiquecoin/storage/FileStorage.h"
#include "postquantiquecoin/storage/UtxoStorage.h"
#include <filesystem>
namespace pqc {
class Blockchain {
public:
    Blockchain(std::filesystem::path dataDir, ChainParams params, PQCryptoProvider* crypto);
    static Result<Blockchain> OpenOrCreate(const std::filesystem::path& dataDir, const ChainParams& params, PQCryptoProvider* crypto);
    Result<void> Initialize(); Result<void> Load(); Result<void> Save(); Result<void> AddBlock(const Block& block); Result<void> ValidateBlock(const Block& block) const; Result<void> ConnectBlock(const Block& block); Result<void> DisconnectTip(); Result<void> RebuildUTXO(); Result<void> ValidateChain() const; Result<void> AddTransactionToMempool(const Transaction& tx);
    std::optional<Block> GetBlockByHash(const std::string& hash) const;
    uint64_t GetMedianTimePast(uint64_t height) const; std::vector<BlockHeader> GetLastHeaders(uint64_t count) const; std::optional<Block> GetBlockByHeight(uint64_t height) const; std::string GetTipHash() const; uint64_t GetHeight() const; uint64_t GetBalance(const std::string& address) const; std::vector<UTXO> GetSpendableUTXOs(const std::string& address) const; Mempool& GetMempool(); const UTXOSet& GetUTXOSet() const; uint64_t GetMintedSupply() const; uint64_t GetNextSubsidy() const; uint32_t GetNextWorkRequired() const; const ChainParams& Params() const;
private:
    std::filesystem::path BlockPath(uint64_t height) const;
    Result<void> StoreBlock(const Block& block);
    std::filesystem::path dataDir_; ChainParams params_; PQCryptoProvider* crypto_; FileStorage files_; BlockIndex index_; UTXOSet utxos_; Mempool mempool_; std::vector<Block> chain_; uint64_t mintedSupply_{0};
};
}
