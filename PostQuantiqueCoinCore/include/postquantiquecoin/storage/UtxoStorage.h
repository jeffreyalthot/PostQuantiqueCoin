#pragma once
#include "postquantiquecoin/blockchain/UTXOSet.h"
#include <filesystem>
namespace pqc { class UtxoStorage { public: explicit UtxoStorage(std::filesystem::path path); Result<void> SaveSnapshot(const UTXOSet& set) const; Result<UTXOSet> LoadSnapshot() const; Result<void> SaveAtomic(const UTXOSet& set) const; Result<void> ValidateSnapshotChecksum() const; private: std::filesystem::path path_; }; }
