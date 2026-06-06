#pragma once

#include "postquantiquecoin/core/Result.h"
#include "postquantiquecoin/p2p/P2PMessage.h"
#include "postquantiquecoin/p2p/Protocol.h"
#include "postquantiquecoin/p2p/PeerManager.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <string>
#include <thread>

namespace pqc::p2p {

struct P2PServerConfig {
    uint8_t networkId{1};
    uint16_t port{29444};
    std::string bind{"0.0.0.0"};
    uint32_t maxConnections{32};
    uint32_t maxPayloadBytes{MaxPayloadBytes};
    bool listen{true};
    std::chrono::seconds handshakeTimeout{10};
};

struct P2PServerMetrics {
    uint64_t acceptedConnections{0};
    uint64_t rejectedConnections{0};
    uint64_t bytesIn{0};
    uint64_t bytesOut{0};
    uint64_t invalidMessages{0};
};

class P2PServer {
public:
    using MessageHandler = std::function<Result<void>(const PeerEndpoint&, const P2PMessage&)>;

    P2PServer(P2PServerConfig config, PeerManager& peers);
    ~P2PServer();

    Result<void> Start(MessageHandler handler);
    void Stop() noexcept;
    bool IsRunning() const noexcept;
    P2PServerMetrics Metrics() const noexcept;

private:
    void AcceptLoop();
    void HandleClient(int clientSocket, PeerEndpoint endpoint);

    P2PServerConfig config_;
    PeerManager& peers_;
    MessageHandler handler_;
    std::atomic<bool> running_{false};
    std::thread acceptThread_;
    int listenSocket_{-1};
    P2PServerMetrics metrics_{};
};

} // namespace pqc::p2p
