#pragma once
#include "postquantiquecoin/core/Result.h"
#include <filesystem>
#include <map>
#include <string>
namespace pqc { struct NodeConfig { std::string network{"mainnet"}; std::filesystem::path datadir{"data"}; uint16_t port{29444}; uint32_t maxpeers{32}; bool listen{true}; std::string seedpeer; uint32_t mining_threads{4}; std::string log_level{"info"}; std::filesystem::path wallet_dir{"data/wallets"}; bool require_liboqs{true}; bool allow_dev_crypto{false}; bool allow_legacy_pow{false}; }; class Config { public: static Result<NodeConfig> Load(const std::filesystem::path& path); static Result<void> Save(const std::filesystem::path& path,const NodeConfig& cfg); }; }
