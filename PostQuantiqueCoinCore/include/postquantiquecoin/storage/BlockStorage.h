#pragma once
#include "postquantiquecoin/blockchain/Block.h"
#include "postquantiquecoin/core/Result.h"
#include <filesystem>
#include <vector>
namespace pqc { class BlockStorage { public: explicit BlockStorage(std::filesystem::path dir,uint8_t networkId); Result<void> WriteBlockAtomic(const Block& block); Result<Block> ReadBlock(const std::filesystem::path& path) const; Result<Block> ReadBlockByHeight(uint64_t height) const; Result<Block> ReadBlockByHash(const std::string& hash) const; Result<void> VerifyBlockRecord(const std::filesystem::path& path) const; std::vector<std::filesystem::path> ListStoredBlocks() const; Result<void> RepairIndexFromDisk(); private: std::filesystem::path dir_; uint8_t networkId_; std::filesystem::path PathForHeight(uint64_t height) const; }; }
