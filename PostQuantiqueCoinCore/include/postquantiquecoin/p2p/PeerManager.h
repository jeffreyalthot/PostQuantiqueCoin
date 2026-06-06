#pragma once
#include "postquantiquecoin/core/Result.h"
#include "postquantiquecoin/p2p/Peer.h"
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace pqc::p2p {
class PeerManager {
public:
    explicit PeerManager(size_t maxPeers = 32);
    Result<void> AddPeer(const std::string& host, uint16_t port);
    bool RemovePeer(const std::string& endpoint);
    std::optional<Peer> GetPeer(const std::string& endpoint) const;
    std::vector<Peer> ListPeers() const;
    Result<void> Penalize(const std::string& endpoint, int delta);
    Result<void> Ban(const std::string& endpoint, std::chrono::seconds duration);
    Result<void> Load(const std::filesystem::path& path);
    Result<void> Save(const std::filesystem::path& path) const;
private:
    size_t maxPeers_{32};
    std::vector<Peer> peers_;
};
}
