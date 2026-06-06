#include "postquantiquecoin/p2p/P2PServer.h"

#include <array>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace pqc::p2p {
namespace {

constexpr size_t HeaderSize = 29;

void CloseSocket(int s) noexcept {
    if (s < 0) return;
#ifdef _WIN32
    closesocket(s);
#else
    close(s);
#endif
}

bool SendAll(int socket, const uint8_t* data, size_t size) {
    size_t sent = 0;
    while (sent < size) {
#ifdef _WIN32
        const int n = send(socket, reinterpret_cast<const char*>(data + sent), static_cast<int>(size - sent), 0);
#else
        const ssize_t n = send(socket, data + sent, size - sent, 0);
#endif
        if (n <= 0) return false;
        sent += static_cast<size_t>(n);
    }
    return true;
}

bool RecvAll(int socket, uint8_t* data, size_t size) {
    size_t received = 0;
    while (received < size) {
#ifdef _WIN32
        const int n = recv(socket, reinterpret_cast<char*>(data + received), static_cast<int>(size - received), 0);
#else
        const ssize_t n = recv(socket, data + received, size - received, 0);
#endif
        if (n <= 0) return false;
        received += static_cast<size_t>(n);
    }
    return true;
}

uint32_t ReadU32Le(const std::array<uint8_t, HeaderSize>& data, size_t pos) {
    return static_cast<uint32_t>(data[pos]) |
           (static_cast<uint32_t>(data[pos + 1]) << 8) |
           (static_cast<uint32_t>(data[pos + 2]) << 16) |
           (static_cast<uint32_t>(data[pos + 3]) << 24);
}

Result<P2PMessage> ReadMessage(int socket, uint8_t networkId) {
    std::array<uint8_t, HeaderSize> header{};
    if (!RecvAll(socket, header.data(), header.size())) return Result<P2PMessage>::Err("receive header failed");
    const uint32_t payloadSize = ReadU32Le(header, 17);
    if (payloadSize > MaxPayloadBytes) return Result<P2PMessage>::Err("oversized payload");
    std::vector<uint8_t> frame(header.begin(), header.end());
    frame.resize(HeaderSize + payloadSize);
    if (payloadSize > 0 && !RecvAll(socket, frame.data() + HeaderSize, payloadSize)) return Result<P2PMessage>::Err("receive payload failed");
    return P2PMessage::Deserialize(frame, networkId);
}

bool WriteMessage(int socket, const P2PMessage& message) {
    auto data = message.Serialize();
    return !data.empty() && SendAll(socket, data.data(), data.size());
}

PeerEndpoint EndpointFromSockaddr(const sockaddr_storage& storage, uint16_t fallbackPort) {
    char host[NI_MAXHOST]{};
    char service[NI_MAXSERV]{};
    const auto* sa = reinterpret_cast<const sockaddr*>(&storage);
    const auto len = storage.ss_family == AF_INET6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in);
    if (getnameinfo(sa, static_cast<socklen_t>(len), host, sizeof(host), service, sizeof(service), NI_NUMERICHOST | NI_NUMERICSERV) != 0) {
        return {"unknown", fallbackPort};
    }
    return {host, static_cast<uint16_t>(std::stoul(service))};
}

} // namespace

P2PServer::P2PServer(P2PServerConfig config, PeerManager& peers)
    : config_(std::move(config)), peers_(peers) {}

P2PServer::~P2PServer() { Stop(); }

Result<void> P2PServer::Start(MessageHandler handler) {
    if (!config_.listen) return Result<void>::Ok();
    if (running_) return Result<void>::Err("p2p server already running");
    handler_ = std::move(handler);
#ifdef _WIN32
    WSADATA wsa{};
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    addrinfo* result = nullptr;
    const auto port = std::to_string(config_.port);
    const char* bindHost = config_.bind.empty() || config_.bind == "0.0.0.0" ? nullptr : config_.bind.c_str();
    if (getaddrinfo(bindHost, port.c_str(), &hints, &result) != 0 || result == nullptr) return Result<void>::Err("bind resolve failed");

    for (addrinfo* p = result; p != nullptr; p = p->ai_next) {
        listenSocket_ = static_cast<int>(socket(p->ai_family, p->ai_socktype, p->ai_protocol));
        if (listenSocket_ < 0) continue;
        int yes = 1;
        setsockopt(listenSocket_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(yes));
        if (bind(listenSocket_, p->ai_addr, static_cast<int>(p->ai_addrlen)) == 0 && listen(listenSocket_, static_cast<int>(config_.maxConnections)) == 0) break;
        CloseSocket(listenSocket_);
        listenSocket_ = -1;
    }
    freeaddrinfo(result);
    if (listenSocket_ < 0) return Result<void>::Err("p2p listen failed");
    running_ = true;
    acceptThread_ = std::thread(&P2PServer::AcceptLoop, this);
    return Result<void>::Ok();
}

void P2PServer::Stop() noexcept {
    running_ = false;
    CloseSocket(listenSocket_);
    listenSocket_ = -1;
    if (acceptThread_.joinable()) acceptThread_.join();
}

bool P2PServer::IsRunning() const noexcept { return running_; }
P2PServerMetrics P2PServer::Metrics() const noexcept { return metrics_; }

void P2PServer::AcceptLoop() {
    while (running_) {
        sockaddr_storage addr{};
        socklen_t len = sizeof(addr);
        const int client = static_cast<int>(accept(listenSocket_, reinterpret_cast<sockaddr*>(&addr), &len));
        if (client < 0) {
            if (running_) ++metrics_.rejectedConnections;
            continue;
        }
        ++metrics_.acceptedConnections;
        const auto endpoint = EndpointFromSockaddr(addr, config_.port);
        peers_.AddOrUpdate(endpoint, false);
        std::thread(&P2PServer::HandleClient, this, client, endpoint).detach();
    }
}

void P2PServer::HandleClient(int clientSocket, PeerEndpoint endpoint) {
    auto first = ReadMessage(clientSocket, config_.networkId);
    if (first.IsErr()) {
        ++metrics_.invalidMessages;
        peers_.ReportInvalid(endpoint.ToString(), first.Error());
        CloseSocket(clientSocket);
        return;
    }
    metrics_.bytesIn += first.Value().payload.size() + HeaderSize;
    if (first.Value().command == "version") {
        P2PMessage version;
        version.networkId = config_.networkId;
        version.command = "version";
        if (!WriteMessage(clientSocket, version)) ++metrics_.invalidMessages;
        P2PMessage verack;
        verack.networkId = config_.networkId;
        verack.command = "verack";
        if (!WriteMessage(clientSocket, verack)) ++metrics_.invalidMessages;
    } else if (handler_) {
        auto r = handler_(endpoint, first.Value());
        if (r.IsErr()) {
            ++metrics_.invalidMessages;
            peers_.ReportInvalid(endpoint.ToString(), r.Error());
        } else {
            peers_.ReportGood(endpoint.ToString());
        }
    }
    CloseSocket(clientSocket);
}

} // namespace pqc::p2p
