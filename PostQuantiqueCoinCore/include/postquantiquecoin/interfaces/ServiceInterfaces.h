#pragma once

#include "postquantiquecoin/core/Result.h"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace pqc::interfaces {

class IBlockStore {
public:
    virtual ~IBlockStore() = default;
    virtual Result<void> PutBlock(uint64_t height, const std::vector<uint8_t>& bytes) = 0;
    virtual Result<std::vector<uint8_t>> GetBlockByHeight(uint64_t height) const = 0;
    virtual Result<void> Flush() = 0;
};

class IUtxoStore {
public:
    virtual ~IUtxoStore() = default;
    virtual Result<void> PutUtxo(const std::string& outpoint, const std::vector<uint8_t>& bytes) = 0;
    virtual Result<void> DeleteUtxo(const std::string& outpoint) = 0;
    virtual Result<std::vector<uint8_t>> GetUtxo(const std::string& outpoint) const = 0;
};

class IPeerTransport {
public:
    virtual ~IPeerTransport() = default;
    virtual Result<void> Start() = 0;
    virtual void Stop() noexcept = 0;
    virtual Result<void> Send(const std::string& peerId, const std::vector<uint8_t>& frame) = 0;
};

class ICryptoProvider {
public:
    virtual ~ICryptoProvider() = default;
    virtual std::string ProviderName() const = 0;
    virtual bool ProductionAllowed() const = 0;
};

class IWalletStore {
public:
    virtual ~IWalletStore() = default;
    virtual Result<std::vector<uint8_t>> ReadWallet(const std::string& walletName) const = 0;
    virtual Result<void> WriteWalletAtomically(const std::string& walletName, const std::vector<uint8_t>& bytes) = 0;
};

class IMempoolPolicy {
public:
    virtual ~IMempoolPolicy() = default;
    virtual Result<void> CheckRelayPolicy(const std::vector<uint8_t>& serializedTx, uint64_t feeAtoms) const = 0;
};

} // namespace pqc::interfaces
