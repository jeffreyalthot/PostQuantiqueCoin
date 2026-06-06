#pragma once
#include "postquantiquecoin/blockchain/Blockchain.h"
#include "postquantiquecoin/core/Config.h"
#include <string>

namespace pqc::application {
struct NodeInfo {
    std::string network;
    std::string tipHash;
    uint64_t height{0};
    size_t mempoolSize{0};
    uint64_t mintedSupply{0};
};

class NodeService {
public:
    NodeService(Blockchain& chain, NodeConfig config);
    NodeInfo GetInfo() const;
    Result<void> ValidateStartup() const;
private:
    Blockchain& chain_;
    NodeConfig config_;
};
}
