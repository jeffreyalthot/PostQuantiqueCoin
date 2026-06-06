#pragma once
#include "postquantiquecoin/blockchain/Blockchain.h"
#include "postquantiquecoin/core/Config.h"
#include <vector>
namespace pqc { class NodeCLI { public: explicit NodeCLI(Blockchain& chain, NodeConfig config = NodeConfig{}); int Execute(const std::vector<std::string>& args); private: Blockchain& chain_; NodeConfig config_; }; }
