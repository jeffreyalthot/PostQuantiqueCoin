#pragma once
#include <chrono>
#include <cstdint>
#include <string>

namespace pqc::p2p {
enum class PeerConnectionState { Disconnected, Connecting, Handshaking, Ready, Banned };

class Peer {
public:
    Peer() = default;
    Peer(std::string host, uint16_t port);
    const std::string& Host() const noexcept;
    uint16_t Port() const noexcept;
    std::string Endpoint() const;
    PeerConnectionState State() const noexcept;
    void SetState(PeerConnectionState state) noexcept;
    int Reputation() const noexcept;
    void Reward(int delta = 1) noexcept;
    void Penalize(int delta = 10) noexcept;
    bool IsBanned() const noexcept;
    void BanFor(std::chrono::seconds duration);
    bool BanExpired() const;
    void RecordBytesIn(uint64_t bytes) noexcept;
    void RecordBytesOut(uint64_t bytes) noexcept;
    uint64_t BytesIn() const noexcept;
    uint64_t BytesOut() const noexcept;
private:
    std::string host_;
    uint16_t port_{0};
    PeerConnectionState state_{PeerConnectionState::Disconnected};
    int reputation_{100};
    std::chrono::system_clock::time_point bannedUntil_{};
    uint64_t bytesIn_{0};
    uint64_t bytesOut_{0};
};
}
