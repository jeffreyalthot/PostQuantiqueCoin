#include "postquantiquecoin/p2p/P2PClient.h"
#include "postquantiquecoin/p2p/Protocol.h"

#include <array>
#include <cstring>
#include <sstream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
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

std::vector<uint8_t> VersionPayload(uint32_t protocolVersion, uint64_t bestHeight, const std::string& userAgent) {
    std::vector<uint8_t> out;
    auto put32 = [&](uint32_t v) { for (int i = 0; i < 4; ++i) out.push_back(static_cast<uint8_t>((v >> (8 * i)) & 0xff)); };
    auto put64 = [&](uint64_t v) { for (int i = 0; i < 8; ++i) out.push_back(static_cast<uint8_t>((v >> (8 * i)) & 0xff)); };
    put32(protocolVersion);
    put64(bestHeight);
    out.push_back(static_cast<uint8_t>(std::min<size_t>(userAgent.size(), 252)));
    out.insert(out.end(), userAgent.begin(), userAgent.begin() + std::min<size_t>(userAgent.size(), 252));
    return out;
}

} // namespace

P2PClient::P2PClient(uint8_t networkId, uint32_t protocolVersion)
    : networkId_(networkId), protocolVersion_(protocolVersion) {}

Result<void> P2PClient::Connect(const PeerEndpoint& endpoint, std::chrono::milliseconds timeout) {
    (void)timeout;
    Close();
#ifdef _WIN32
    WSADATA wsa{};
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
    addrinfo hints{};
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    addrinfo* result = nullptr;
    const auto port = std::to_string(endpoint.port);
    const int rc = getaddrinfo(endpoint.host.c_str(), port.c_str(), &hints, &result);
    if (rc != 0 || result == nullptr) return Result<void>::Err("resolve failed for " + endpoint.ToString());

    for (addrinfo* p = result; p != nullptr; p = p->ai_next) {
        socket_ = static_cast<int>(socket(p->ai_family, p->ai_socktype, p->ai_protocol));
        if (socket_ < 0) continue;
        if (connect(socket_, p->ai_addr, static_cast<int>(p->ai_addrlen)) == 0) {
            freeaddrinfo(result);
            return Result<void>::Ok();
        }
        CloseSocket(socket_);
        socket_ = -1;
    }
    freeaddrinfo(result);
    return Result<void>::Err("connect failed to " + endpoint.ToString());
}

Result<void> P2PClient::Send(const P2PMessage& message) {
    if (socket_ < 0) return Result<void>::Err("not connected");
    auto data = message.Serialize();
    if (data.empty()) return Result<void>::Err("cannot serialize p2p message");
    if (!SendAll(socket_, data.data(), data.size())) return Result<void>::Err("send failed");
    return Result<void>::Ok();
}

Result<P2PMessage> P2PClient::Receive(std::chrono::milliseconds timeout) {
    (void)timeout;
    if (socket_ < 0) return Result<P2PMessage>::Err("not connected");
    std::array<uint8_t, HeaderSize> header{};
    if (!RecvAll(socket_, header.data(), header.size())) return Result<P2PMessage>::Err("receive header failed");
    const uint32_t payloadSize = ReadU32Le(header, 17);
    if (payloadSize > MaxPayloadBytes) return Result<P2PMessage>::Err("oversized payload");
    std::vector<uint8_t> frame(header.begin(), header.end());
    frame.resize(HeaderSize + payloadSize);
    if (payloadSize > 0 && !RecvAll(socket_, frame.data() + HeaderSize, payloadSize)) return Result<P2PMessage>::Err("receive payload failed");
    return P2PMessage::Deserialize(frame, networkId_);
}

Result<void> P2PClient::Handshake(uint64_t bestHeight, const std::string& userAgent) {
    P2PMessage version;
    version.networkId = networkId_;
    version.command = "version";
    version.payload = VersionPayload(protocolVersion_, bestHeight, userAgent);
    auto sendVersion = Send(version);
    if (sendVersion.IsErr()) return sendVersion;
    auto theirs = Receive();
    if (theirs.IsErr()) return Result<void>::Err(theirs.Error());
    if (theirs.Value().command != "version") return Result<void>::Err("expected version message");
    P2PMessage verack;
    verack.networkId = networkId_;
    verack.command = "verack";
    auto sendVerack = Send(verack);
    if (sendVerack.IsErr()) return sendVerack;
    auto ack = Receive();
    if (ack.IsErr()) return Result<void>::Err(ack.Error());
    if (ack.Value().command != "verack") return Result<void>::Err("expected verack message");
    return Result<void>::Ok();
}

void P2PClient::Close() noexcept {
    CloseSocket(socket_);
    socket_ = -1;
}

bool P2PClient::IsConnected() const noexcept { return socket_ >= 0; }

} // namespace pqc::p2p
