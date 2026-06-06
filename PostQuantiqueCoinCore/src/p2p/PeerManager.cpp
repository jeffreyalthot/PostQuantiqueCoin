#include "postquantiquecoin/p2p/PeerManager.h"

#include <algorithm>
#include <fstream>

namespace pqc::p2p {

PeerManager::PeerManager(std::filesystem::path peersFile) : peersFile_(std::move(peersFile)) {}

Result<void> PeerManager::Load(uint16_t defaultPort) {
    std::lock_guard<std::mutex> lock(mutex_);
    peers_.clear();
    if (peersFile_.empty()) return Result<void>::Ok();
    std::ifstream in(peersFile_);
    if (!in) return Result<void>::Ok();
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        auto ep = PeerEndpoint::FromString(line, defaultPort);
        if (!ep.host.empty() && ep.port != 0) peers_.emplace_back(ep, true);
    }
    return Result<void>::Ok();
}

Result<void> PeerManager::Save() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (peersFile_.empty()) return Result<void>::Ok();
    std::filesystem::create_directories(peersFile_.parent_path());
    std::ofstream out(peersFile_);
    if (!out) return Result<void>::Err("cannot write peers.dat");
    for (const auto& peer : peers_) {
        if (!peer.IsBanned()) out << peer.Endpoint().ToString() << '\n';
    }
    return Result<void>::Ok();
}

Result<void> PeerManager::AddOrUpdate(const PeerEndpoint& endpoint, bool outbound) {
    if (endpoint.host.empty() || endpoint.port == 0) return Result<void>::Err("invalid peer endpoint");
    std::lock_guard<std::mutex> lock(mutex_);
    const auto id = endpoint.ToString();
    auto it = std::find_if(peers_.begin(), peers_.end(), [&](const Peer& p) { return p.Id() == id; });
    if (it == peers_.end()) peers_.emplace_back(endpoint, outbound);
    else it->Reward(1);
    return Result<void>::Ok();
}

Result<void> PeerManager::ReportGood(const std::string& peerId) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& peer : peers_) if (peer.Id() == peerId) { peer.Reward(5); return Result<void>::Ok(); }
    return Result<void>::Err("unknown peer");
}

Result<void> PeerManager::ReportInvalid(const std::string& peerId, const std::string& reason) {
    (void)reason;
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& peer : peers_) {
        if (peer.Id() == peerId) {
            peer.Penalize(25);
            if (peer.Reputation() < -100 || peer.InvalidMessages() >= 10) peer.BanFor(std::chrono::hours(24));
            return Result<void>::Ok();
        }
    }
    return Result<void>::Err("unknown peer");
}

Result<void> PeerManager::Ban(const std::string& peerId, std::chrono::seconds duration) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& peer : peers_) if (peer.Id() == peerId) { peer.BanFor(duration); return Result<void>::Ok(); }
    return Result<void>::Err("unknown peer");
}

Result<void> PeerManager::Unban(const std::string& peerId) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& peer : peers_) if (peer.Id() == peerId) { peer.SetState(PeerState::Disconnected); return Result<void>::Ok(); }
    return Result<void>::Err("unknown peer");
}

std::optional<Peer> PeerManager::Get(const std::string& peerId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& peer : peers_) if (peer.Id() == peerId) return peer;
    return std::nullopt;
}

std::vector<Peer> PeerManager::SelectConnectable(size_t maxCount) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Peer> out;
    for (const auto& peer : peers_) {
        if (out.size() >= maxCount) break;
        if (!peer.IsBanned()) out.push_back(peer);
    }
    return out;
}

std::vector<Peer> PeerManager::ListPeers() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return peers_;
}

PeerManagerStats PeerManager::Stats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    PeerManagerStats stats;
    stats.knownPeers = peers_.size();
    for (const auto& peer : peers_) {
        if (peer.IsBanned()) ++stats.bannedPeers;
        stats.bytesIn += peer.BytesIn();
        stats.bytesOut += peer.BytesOut();
    }
    return stats;
}

} // namespace pqc::p2p
