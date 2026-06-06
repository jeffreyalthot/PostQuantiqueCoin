#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace pqc::p2p {

enum class PeerState { Disconnected, Connecting, Handshaking, Established, Banned };

struct PeerEndpoint {
    std::string host;
    uint16_t port{0};

    std::string ToString() const;
    static PeerEndpoint FromString(const std::string& value, uint16_t defaultPort);
};

class Peer {
public:
    Peer() = default;
    Peer(PeerEndpoint endpoint, bool outbound);

    const PeerEndpoint& Endpoint() const noexcept;
    const std::string& Id() const noexcept;
    PeerState State() const noexcept;
    int Reputation() const noexcept;
    bool Outbound() const noexcept;
    uint64_t BytesIn() const noexcept;
    uint64_t BytesOut() const noexcept;
    uint64_t InvalidMessages() const noexcept;
    std::chrono::system_clock::time_point BannedUntil() const noexcept;

    void SetState(PeerState state) noexcept;
    void MarkBytesIn(uint64_t bytes) noexcept;
    void MarkBytesOut(uint64_t bytes) noexcept;
    void Reward(int delta = 1) noexcept;
    void Penalize(int delta = 10) noexcept;
    void BanFor(std::chrono::seconds duration) noexcept;
    bool IsBanned(std::chrono::system_clock::time_point now = std::chrono::system_clock::now()) const noexcept;

private:
    PeerEndpoint endpoint_{};
    std::string id_;
    PeerState state_{PeerState::Disconnected};
    bool outbound_{false};
    int reputation_{0};
    uint64_t bytesIn_{0};
    uint64_t bytesOut_{0};
    uint64_t invalidMessages_{0};
    std::chrono::system_clock::time_point bannedUntil_{};
};

} // namespace pqc::p2p
