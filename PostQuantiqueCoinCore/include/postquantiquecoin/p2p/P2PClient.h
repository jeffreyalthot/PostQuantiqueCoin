#pragma once
#include "postquantiquecoin/core/Result.h"
#include "postquantiquecoin/p2p/P2PMessage.h"
#include <cstdint>
#include <string>

namespace pqc::p2p {
class P2PClient {
public:
    explicit P2PClient(uint8_t networkId = 1);
    Result<void> SendMessage(const std::string& host, uint16_t port, const P2PMessage& message, uint32_t timeoutMs = 3000) const;
    Result<P2PMessage> RoundTrip(const std::string& host, uint16_t port, const P2PMessage& message, uint32_t timeoutMs = 3000) const;
private:
    uint8_t networkId_{1};
};
}
