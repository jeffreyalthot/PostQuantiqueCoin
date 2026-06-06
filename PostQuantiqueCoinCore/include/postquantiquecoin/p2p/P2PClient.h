#pragma once

#include "postquantiquecoin/core/Result.h"
#include "postquantiquecoin/p2p/P2PMessage.h"
#include "postquantiquecoin/p2p/Peer.h"

#include <chrono>
#include <cstdint>
#include <string>

namespace pqc::p2p {

class P2PClient {
public:
    P2PClient(uint8_t networkId, uint32_t protocolVersion = 1);

    Result<void> Connect(const PeerEndpoint& endpoint, std::chrono::milliseconds timeout = std::chrono::seconds(5));
    Result<void> Send(const P2PMessage& message);
    Result<P2PMessage> Receive(std::chrono::milliseconds timeout = std::chrono::seconds(5));
    Result<void> Handshake(uint64_t bestHeight, const std::string& userAgent);
    void Close() noexcept;
    bool IsConnected() const noexcept;

private:
    uint8_t networkId_{0};
    uint32_t protocolVersion_{1};
    int socket_{-1};
};

} // namespace pqc::p2p
