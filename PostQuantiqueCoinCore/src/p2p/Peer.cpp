#include "postquantiquecoin/p2p/Peer.h"

#include <algorithm>
#include <sstream>

namespace pqc::p2p {

std::string PeerEndpoint::ToString() const {
    return host + ":" + std::to_string(port);
}

PeerEndpoint PeerEndpoint::FromString(const std::string& value, uint16_t defaultPort) {
    if (value.empty()) return {};
    if (value.front() == '[') {
        const auto close = value.find(']');
        if (close == std::string::npos) return {value, defaultPort};
        PeerEndpoint ep{value.substr(1, close - 1), defaultPort};
        if (close + 2 < value.size() && value[close + 1] == ':') ep.port = static_cast<uint16_t>(std::stoul(value.substr(close + 2)));
        return ep;
    }
    const auto colon = value.rfind(':');
    if (colon == std::string::npos || value.find(':') != colon) return {value, defaultPort};
    return {value.substr(0, colon), static_cast<uint16_t>(std::stoul(value.substr(colon + 1)))};
}

Peer::Peer(PeerEndpoint endpoint, bool outbound)
    : endpoint_(std::move(endpoint)), id_(endpoint_.ToString()), outbound_(outbound) {}

const PeerEndpoint& Peer::Endpoint() const noexcept { return endpoint_; }
const std::string& Peer::Id() const noexcept { return id_; }
PeerState Peer::State() const noexcept { return state_; }
int Peer::Reputation() const noexcept { return reputation_; }
bool Peer::Outbound() const noexcept { return outbound_; }
uint64_t Peer::BytesIn() const noexcept { return bytesIn_; }
uint64_t Peer::BytesOut() const noexcept { return bytesOut_; }
uint64_t Peer::InvalidMessages() const noexcept { return invalidMessages_; }
std::chrono::system_clock::time_point Peer::BannedUntil() const noexcept { return bannedUntil_; }

void Peer::SetState(PeerState state) noexcept { state_ = state; }
void Peer::MarkBytesIn(uint64_t bytes) noexcept { bytesIn_ += bytes; }
void Peer::MarkBytesOut(uint64_t bytes) noexcept { bytesOut_ += bytes; }
void Peer::Reward(int delta) noexcept { reputation_ = std::min(1000, reputation_ + delta); }
void Peer::Penalize(int delta) noexcept {
    reputation_ = std::max(-1000, reputation_ - delta);
    ++invalidMessages_;
}
void Peer::BanFor(std::chrono::seconds duration) noexcept {
    state_ = PeerState::Banned;
    bannedUntil_ = std::chrono::system_clock::now() + duration;
}

bool Peer::IsBanned(std::chrono::system_clock::time_point now) const noexcept {
    return state_ == PeerState::Banned && now < bannedUntil_;
}

} // namespace pqc::p2p
