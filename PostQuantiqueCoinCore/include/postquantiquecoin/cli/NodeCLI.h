#pragma once
#include "postquantiquecoin/blockchain/Blockchain.h"
namespace pqc { class NodeCLI { public: explicit NodeCLI(Blockchain& chain); int Execute(const std::vector<std::string>& args); private: Blockchain& chain_; }; }
