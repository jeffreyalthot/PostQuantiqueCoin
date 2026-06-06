#include "postquantiquecoin/p2p/Peer.h"
#include <sstream>

namespace pqc::p2p {
Peer::Peer(std::string host, uint16_t port) : host_(std::move(host)), port_(port) {}
const std::string& Peer::Host() const noexcept { return host_; }
uint16_t Peer::Port() const noexcept { return port_; }
std::string Peer::Endpoint() const { std::ostringstream out; out << host_ << ':' << port_; return out.str(); }
PeerConnectionState Peer::State() const noexcept { return state_; }
void Peer::SetState(PeerConnectionState state) noexcept { state_ = state; }
int Peer::Reputation() const noexcept { return reputation_; }
void Peer::Reward(int delta) noexcept { reputation_ += delta; if (reputation_ > 1000) reputation_ = 1000; }
void Peer::Penalize(int delta) noexcept { reputation_ -= delta; if (reputation_ < -1000) reputation_ = -1000; }
bool Peer::IsBanned() const noexcept { return state_ == PeerConnectionState::Banned && std::chrono::system_clock::now() < bannedUntil_; }
void Peer::BanFor(std::chrono::seconds duration) { state_ = PeerConnectionState::Banned; bannedUntil_ = std::chrono::system_clock::now() + duration; }
bool Peer::BanExpired() const { return state_ == PeerConnectionState::Banned && std::chrono::system_clock::now() >= bannedUntil_; }
void Peer::RecordBytesIn(uint64_t bytes) noexcept { bytesIn_ += bytes; }
void Peer::RecordBytesOut(uint64_t bytes) noexcept { bytesOut_ += bytes; }
uint64_t Peer::BytesIn() const noexcept { return bytesIn_; }
uint64_t Peer::BytesOut() const noexcept { return bytesOut_; }
}
