#pragma once
#include <string>
#include <vector>
#include "postquantiquecoin/blockchain/Transaction.h"
namespace pqc { class MerkleTree { public: static std::string ComputeRoot(const std::vector<Transaction>& transactions); static std::string ComputeRootFromHashes(std::vector<std::string> hashes); }; }
