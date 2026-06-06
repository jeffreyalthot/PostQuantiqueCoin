#pragma once

#include "postquantiquecoin/core/Result.h"
#include "postquantiquecoin/p2p/Peer.h"

#include <filesystem>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace pqc::p2p {

struct PeerManagerStats {
    size_t knownPeers{0};
    size_t bannedPeers{0};
    uint64_t bytesIn{0};
    uint64_t bytesOut{0};
};

class PeerManager {
public:
    explicit PeerManager(std::filesystem::path peersFile = {});

    Result<void> Load(uint16_t defaultPort);
    Result<void> Save() const;
    Result<void> AddOrUpdate(const PeerEndpoint& endpoint, bool outbound);
    Result<void> ReportGood(const std::string& peerId);
    Result<void> ReportInvalid(const std::string& peerId, const std::string& reason);
    Result<void> Ban(const std::string& peerId, std::chrono::seconds duration);
    Result<void> Unban(const std::string& peerId);

    std::optional<Peer> Get(const std::string& peerId) const;
    std::vector<Peer> SelectConnectable(size_t maxCount) const;
    std::vector<Peer> ListPeers() const;
    PeerManagerStats Stats() const;

private:
    mutable std::mutex mutex_;
    std::filesystem::path peersFile_;
    std::vector<Peer> peers_;
};

} // namespace pqc::p2p
