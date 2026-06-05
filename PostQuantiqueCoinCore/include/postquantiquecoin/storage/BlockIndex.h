#pragma once
#include "postquantiquecoin/core/Result.h"
#include <filesystem>
#include <map>
#include <optional>
#include <string>
namespace pqc {
struct BlockIndexEntry { std::string hash; std::string previousHash; uint64_t height{0}; uint32_t bits{0}; uint64_t timestamp{0}; uint64_t fileNumber{0}; uint64_t fileOffset{0}; uint64_t serializedSize{0}; std::string cumulativeWork; };
class BlockIndex { public: explicit BlockIndex(std::filesystem::path path={}); Result<void> Add(const BlockIndexEntry& e); bool HasHash(const std::string& h) const; std::optional<BlockIndexEntry> GetByHash(const std::string& h) const; std::optional<BlockIndexEntry> GetByHeight(uint64_t h) const; std::optional<BlockIndexEntry> GetTip() const; Result<void> SetTip(const std::string& h); Result<void> Save() const; Result<void> Load(); void Clear(); void SetPath(std::filesystem::path p); private: std::filesystem::path path_; std::map<std::string,BlockIndexEntry> byHash_; std::map<uint64_t,std::string> byHeight_; std::string tip_; };
}
