#include "postquantiquecoin/p2p/P2PClient.h"
#include "postquantiquecoin/p2p/Protocol.h"
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#endif

namespace pqc::p2p {
namespace {
#ifdef _WIN32
struct SocketRuntime { SocketRuntime(){ WSADATA w{}; WSAStartup(MAKEWORD(2,2), &w); } ~SocketRuntime(){ WSACleanup(); } };
void CloseSocket(SOCKET fd){ closesocket(fd); }
using SocketFd = SOCKET;
constexpr SocketFd InvalidSocket = INVALID_SOCKET;
#else
void CloseSocket(int fd){ close(fd); }
using SocketFd = int;
constexpr SocketFd InvalidSocket = -1;
#endif

Result<SocketFd> ConnectSocket(const std::string& host, uint16_t port, uint32_t timeoutMs) {
#ifdef _WIN32
    static SocketRuntime runtime;
#endif
    addrinfo hints{}; hints.ai_socktype = SOCK_STREAM; hints.ai_family = AF_UNSPEC;
    addrinfo* result = nullptr;
    const auto portText = std::to_string(port);
    if (getaddrinfo(host.c_str(), portText.c_str(), &hints, &result) != 0) return Result<SocketFd>::Err("unable to resolve peer");
    SocketFd fd = InvalidSocket;
    for (auto* ai = result; ai; ai = ai->ai_next) {
        fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (fd == InvalidSocket) continue;
#ifdef _WIN32
        DWORD tv = timeoutMs; setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv)); setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv));
#else
        timeval tv{static_cast<long>(timeoutMs / 1000), static_cast<long>((timeoutMs % 1000) * 1000)}; setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)); setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif
        if (connect(fd, ai->ai_addr, static_cast<int>(ai->ai_addrlen)) == 0) break;
        CloseSocket(fd); fd = InvalidSocket;
    }
    freeaddrinfo(result);
    if (fd == InvalidSocket) return Result<SocketFd>::Err("unable to connect peer");
    return Result<SocketFd>::Ok(fd);
}

Result<void> SendAll(SocketFd fd, const std::vector<uint8_t>& data) {
    size_t sent = 0;
    while (sent < data.size()) {
        const auto n = send(fd, reinterpret_cast<const char*>(data.data() + sent), static_cast<int>(data.size() - sent), 0);
        if (n <= 0) return Result<void>::Err("socket send failed");
        sent += static_cast<size_t>(n);
    }
    return Result<void>::Ok();
}

Result<std::vector<uint8_t>> ReceiveFrame(SocketFd fd) {
    constexpr size_t headerSize = 29;
    std::vector<uint8_t> header(headerSize);
    size_t got = 0;
    while (got < header.size()) {
        const auto n = recv(fd, reinterpret_cast<char*>(header.data() + got), static_cast<int>(header.size() - got), 0);
        if (n <= 0) return Result<std::vector<uint8_t>>::Err("socket receive header failed");
        got += static_cast<size_t>(n);
    }
    uint32_t payloadSize = static_cast<uint32_t>(header[17]) | (static_cast<uint32_t>(header[18]) << 8) | (static_cast<uint32_t>(header[19]) << 16) | (static_cast<uint32_t>(header[20]) << 24);
    if (payloadSize > MaxPayloadBytes) return Result<std::vector<uint8_t>>::Err("oversized p2p payload");
    std::vector<uint8_t> frame = header;
    frame.resize(headerSize + payloadSize);
    got = 0;
    while (got < payloadSize) {
        const auto n = recv(fd, reinterpret_cast<char*>(frame.data() + headerSize + got), static_cast<int>(payloadSize - got), 0);
        if (n <= 0) return Result<std::vector<uint8_t>>::Err("socket receive payload failed");
        got += static_cast<size_t>(n);
    }
    return Result<std::vector<uint8_t>>::Ok(std::move(frame));
}
}

P2PClient::P2PClient(uint8_t networkId) : networkId_(networkId) {}
Result<void> P2PClient::SendMessage(const std::string& host, uint16_t port, const P2PMessage& message, uint32_t timeoutMs) const {
    auto fd = ConnectSocket(host, port, timeoutMs); if (fd.IsErr()) return Result<void>::Err(fd.Error());
    auto bytes = message.Serialize(); if (bytes.empty()) { CloseSocket(fd.Value()); return Result<void>::Err("invalid p2p message"); }
    auto r = SendAll(fd.Value(), bytes); CloseSocket(fd.Value()); return r;
}
Result<P2PMessage> P2PClient::RoundTrip(const std::string& host, uint16_t port, const P2PMessage& message, uint32_t timeoutMs) const {
    auto fd = ConnectSocket(host, port, timeoutMs); if (fd.IsErr()) return Result<P2PMessage>::Err(fd.Error());
    auto bytes = message.Serialize(); if (bytes.empty()) { CloseSocket(fd.Value()); return Result<P2PMessage>::Err("invalid p2p message"); }
    auto s = SendAll(fd.Value(), bytes); if (s.IsErr()) { CloseSocket(fd.Value()); return Result<P2PMessage>::Err(s.Error()); }
    auto frame = ReceiveFrame(fd.Value()); CloseSocket(fd.Value()); if (frame.IsErr()) return Result<P2PMessage>::Err(frame.Error());
    return P2PMessage::Deserialize(frame.Value(), networkId_);
}
}
