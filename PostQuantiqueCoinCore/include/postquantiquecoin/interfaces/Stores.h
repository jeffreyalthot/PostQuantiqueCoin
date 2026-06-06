#pragma once
#include "postquantiquecoin/blockchain/Block.h"
#include "postquantiquecoin/blockchain/Transaction.h"
#include "postquantiquecoin/blockchain/UTXO.h"
#include "postquantiquecoin/core/Result.h"
#include <optional>
#include <string>
#include <vector>

namespace pqc::interfaces {
class IBlockStore {
public:
    virtual ~IBlockStore() = default;
    virtual Result<void> PutBlock(const pqc::Block& block) = 0;
    virtual std::optional<pqc::Block> GetBlockByHash(const std::string& hash) const = 0;
    virtual std::optional<pqc::Block> GetBlockByHeight(uint64_t height) const = 0;
};

class IUtxoStore {
public:
    virtual ~IUtxoStore() = default;
    virtual Result<void> PutUtxo(const pqc::UTXO& utxo) = 0;
    virtual Result<void> DeleteUtxo(const std::string& txid, uint32_t outputIndex) = 0;
    virtual std::optional<pqc::UTXO> GetUtxo(const std::string& txid, uint32_t outputIndex) const = 0;
};

class IWalletStore {
public:
    virtual ~IWalletStore() = default;
    virtual Result<std::vector<uint8_t>> ReadWallet(const std::string& walletName) const = 0;
    virtual Result<void> WriteWalletAtomically(const std::string& walletName, const std::vector<uint8_t>& bytes) = 0;
};
}
