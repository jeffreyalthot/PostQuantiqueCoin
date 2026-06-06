#include "postquantiquecoin/p2p/P2PServer.h"
#include "postquantiquecoin/p2p/Protocol.h"
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace pqc::p2p {
namespace {
#ifdef _WIN32
struct SocketRuntime { SocketRuntime(){ WSADATA w{}; WSAStartup(MAKEWORD(2,2), &w); } ~SocketRuntime(){ WSACleanup(); } };
void CloseSocket(int fd){ closesocket(fd); }
#else
void CloseSocket(int fd){ close(fd); }
#endif

bool SendAll(int fd, const std::vector<uint8_t>& data) {
    size_t sent = 0;
    while (sent < data.size()) {
        auto n = send(fd, reinterpret_cast<const char*>(data.data() + sent), static_cast<int>(data.size() - sent), 0);
        if (n <= 0) return false;
        sent += static_cast<size_t>(n);
    }
    return true;
}

Result<std::vector<uint8_t>> ReceiveFrame(int fd) {
    constexpr size_t headerSize = 29;
    std::vector<uint8_t> frame(headerSize);
    size_t got = 0;
    while (got < headerSize) {
        auto n = recv(fd, reinterpret_cast<char*>(frame.data() + got), static_cast<int>(headerSize - got), 0);
        if (n <= 0) return Result<std::vector<uint8_t>>::Err("receive failed");
        got += static_cast<size_t>(n);
    }
    uint32_t payloadSize = static_cast<uint32_t>(frame[17]) | (static_cast<uint32_t>(frame[18]) << 8) | (static_cast<uint32_t>(frame[19]) << 16) | (static_cast<uint32_t>(frame[20]) << 24);
    if (payloadSize > MaxPayloadBytes) return Result<std::vector<uint8_t>>::Err("oversized payload");
    frame.resize(headerSize + payloadSize);
    got = 0;
    while (got < payloadSize) {
        auto n = recv(fd, reinterpret_cast<char*>(frame.data() + headerSize + got), static_cast<int>(payloadSize - got), 0);
        if (n <= 0) return Result<std::vector<uint8_t>>::Err("payload receive failed");
        got += static_cast<size_t>(n);
    }
    return Result<std::vector<uint8_t>>::Ok(std::move(frame));
}
}

P2PServer::P2PServer(uint8_t networkId, std::string bindHost, uint16_t port, PeerManager& peers)
    : networkId_(networkId), bindHost_(std::move(bindHost)), port_(port), peers_(peers) {}
P2PServer::~P2PServer() { Stop(); }

Result<void> P2PServer::Start(MessageHandler handler) {
#ifdef _WIN32
    static SocketRuntime runtime;
#endif
    if (running_) return Result<void>::Ok();
    addrinfo hints{}; hints.ai_family = AF_UNSPEC; hints.ai_socktype = SOCK_STREAM; hints.ai_flags = AI_PASSIVE;
    addrinfo* result = nullptr;
    auto rc = getaddrinfo(bindHost_.empty() ? nullptr : bindHost_.c_str(), std::to_string(port_).c_str(), &hints, &result);
    if (rc != 0) return Result<void>::Err("bind address resolution failed");
    int fd = -1;
    for (auto* ai = result; ai; ai = ai->ai_next) {
        fd = static_cast<int>(socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol));
        if (fd < 0) continue;
        int one = 1; setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&one), sizeof(one));
        if (bind(fd, ai->ai_addr, static_cast<int>(ai->ai_addrlen)) == 0 && listen(fd, 16) == 0) break;
        CloseSocket(fd); fd = -1;
    }
    freeaddrinfo(result);
    if (fd < 0) return Result<void>::Err("unable to bind p2p port");
    listenFd_ = fd; running_ = true; thread_ = std::thread(&P2PServer::AcceptLoop, this, std::move(handler));
    return Result<void>::Ok();
}

void P2PServer::Stop() {
    if (!running_) return;
    running_ = false;
    if (listenFd_ >= 0) { CloseSocket(listenFd_); listenFd_ = -1; }
    if (thread_.joinable()) thread_.join();
}
bool P2PServer::IsRunning() const noexcept { return running_; }

void P2PServer::AcceptLoop(MessageHandler handler) {
    while (running_) {
        sockaddr_storage remote{}; socklen_t len = sizeof(remote);
        int fd = static_cast<int>(accept(listenFd_, reinterpret_cast<sockaddr*>(&remote), &len));
        if (fd < 0) continue;
        auto frame = ReceiveFrame(fd);
        if (frame.IsOk()) {
            auto msg = P2PMessage::Deserialize(frame.Value(), networkId_);
            if (msg.IsOk()) {
                if (handler) {
                    auto reply = handler(msg.Value());
                    auto bytes = reply.Serialize();
                    if (!bytes.empty()) SendAll(fd, bytes);
                }
            }
        }
        CloseSocket(fd);
    }
}
}
