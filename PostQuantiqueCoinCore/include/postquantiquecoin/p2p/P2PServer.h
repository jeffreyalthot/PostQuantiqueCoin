#pragma once
#include "postquantiquecoin/core/Result.h"
#include "postquantiquecoin/p2p/P2PMessage.h"
#include "postquantiquecoin/p2p/PeerManager.h"
#include <atomic>
#include <cstdint>
#include <functional>
#include <string>
#include <thread>

namespace pqc::p2p {
class P2PServer {
public:
    using MessageHandler = std::function<P2PMessage(const P2PMessage&)>;
    P2PServer(uint8_t networkId, std::string bindHost, uint16_t port, PeerManager& peers);
    ~P2PServer();
    Result<void> Start(MessageHandler handler = {});
    void Stop();
    bool IsRunning() const noexcept;
private:
    void AcceptLoop(MessageHandler handler);
    uint8_t networkId_{1};
    std::string bindHost_;
    uint16_t port_{0};
    PeerManager& peers_;
    std::atomic<bool> running_{false};
    std::thread thread_;
    int listenFd_{-1};
};
}
