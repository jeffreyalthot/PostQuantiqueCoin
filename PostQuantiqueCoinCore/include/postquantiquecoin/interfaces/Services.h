#pragma once
#include "postquantiquecoin/blockchain/Mempool.h"
#include "postquantiquecoin/core/Result.h"
#include "postquantiquecoin/crypto/PQCryptoProvider.h"
#include "postquantiquecoin/p2p/P2PMessage.h"
#include <string>
#include <vector>

namespace pqc::interfaces {
class IPeerTransport {
public:
    virtual ~IPeerTransport() = default;
    virtual Result<void> Send(const std::string& host, uint16_t port, const pqc::p2p::P2PMessage& message) = 0;
};

class ICryptoProvider {
public:
    virtual ~ICryptoProvider() = default;
    virtual pqc::PQCryptoProvider& Provider() = 0;
};

class IMempoolPolicy {
public:
    virtual ~IMempoolPolicy() = default;
    virtual Result<void> ValidateRelay(const pqc::Transaction& tx, const pqc::UTXOSet& utxos, uint64_t height) const = 0;
};
}
