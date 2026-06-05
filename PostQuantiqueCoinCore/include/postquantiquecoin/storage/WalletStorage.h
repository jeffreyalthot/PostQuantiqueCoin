#pragma once
#include "postquantiquecoin/core/Result.h"
#include <filesystem>
#include <vector>
namespace pqc { class WalletStorage { public: explicit WalletStorage(std::filesystem::path path); Result<void> SaveEncryptedWallet(const std::vector<uint8_t>& bytes) const; Result<std::vector<uint8_t>> LoadEncryptedWallet() const; bool Exists() const; Result<std::filesystem::path> Backup(const std::filesystem::path& destination={}) const; Result<void> AtomicSave(const std::vector<uint8_t>& bytes) const; const std::filesystem::path& Path() const; private: std::filesystem::path path_; }; }
