#include "postquantiquecoin/p2p/PeerManager.h"
#include <algorithm>
#include <fstream>
#include <sstream>

namespace pqc::p2p {
namespace { std::string Endpoint(const std::string& host, uint16_t port){ return host + ":" + std::to_string(port); } }
PeerManager::PeerManager(size_t maxPeers) : maxPeers_(maxPeers) {}
Result<void> PeerManager::AddPeer(const std::string& host, uint16_t port) {
    if (host.empty() || port == 0) return Result<void>::Err("invalid peer endpoint");
    if (peers_.size() >= maxPeers_) return Result<void>::Err("peer table full");
    const auto ep = Endpoint(host, port);
    for (const auto& p : peers_) if (p.Endpoint() == ep) return Result<void>::Ok();
    peers_.emplace_back(host, port);
    return Result<void>::Ok();
}
bool PeerManager::RemovePeer(const std::string& endpoint) {
    const auto old = peers_.size();
    peers_.erase(std::remove_if(peers_.begin(), peers_.end(), [&](const Peer& p){ return p.Endpoint() == endpoint; }), peers_.end());
    return peers_.size() != old;
}
std::optional<Peer> PeerManager::GetPeer(const std::string& endpoint) const { for (const auto& p : peers_) if (p.Endpoint() == endpoint) return p; return std::nullopt; }
std::vector<Peer> PeerManager::ListPeers() const { return peers_; }
Result<void> PeerManager::Penalize(const std::string& endpoint, int delta) { for (auto& p : peers_) if (p.Endpoint() == endpoint) { p.Penalize(delta); if (p.Reputation() < 0) p.BanFor(std::chrono::minutes(10)); return Result<void>::Ok(); } return Result<void>::Err("peer not found"); }
Result<void> PeerManager::Ban(const std::string& endpoint, std::chrono::seconds duration) { for (auto& p : peers_) if (p.Endpoint() == endpoint) { p.BanFor(duration); return Result<void>::Ok(); } return Result<void>::Err("peer not found"); }
Result<void> PeerManager::Load(const std::filesystem::path& path) { std::ifstream in(path); if (!in) return Result<void>::Ok(); std::string host; uint16_t port; while (in >> host >> port) { auto r = AddPeer(host, port); if (r.IsErr()) return r; } return Result<void>::Ok(); }
Result<void> PeerManager::Save(const std::filesystem::path& path) const { std::filesystem::create_directories(path.parent_path()); std::ofstream out(path); if (!out) return Result<void>::Err("unable to write peers.dat"); for (const auto& p : peers_) out << p.Host() << ' ' << p.Port() << '\n'; return Result<void>::Ok(); }
}
