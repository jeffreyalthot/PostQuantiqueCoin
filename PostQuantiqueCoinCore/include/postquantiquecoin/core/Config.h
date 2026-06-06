#pragma once

#include "postquantiquecoin/blockchain/ChainParams.h"
#include "postquantiquecoin/core/Result.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace pqc {

struct NodeConfig {
    uint32_t formatVersion{1};
    std::string network{"mainnet"};
    std::filesystem::path datadir{"data"};
    uint16_t port{29444};
    uint32_t maxpeers{32};
    bool listen{true};
    std::string bind{"0.0.0.0"};
    std::vector<std::string> connect;
    std::vector<std::string> addnode;
    std::vector<std::string> seednode;
    uint32_t mining_threads{4};
    std::string log_level{"info"};
    std::filesystem::path wallet_dir{"data/wallets"};
    bool require_liboqs{true};
    bool allow_dev_crypto{false};
    bool allow_legacy_pow{false};
    bool safe_start{true};
    bool json{false};
    bool no_color{false};
};

class Config {
public:
    static Result<NodeConfig> Load(const std::filesystem::path& path);
    static Result<void> Save(const std::filesystem::path& path, const NodeConfig& cfg);
    static Result<void> ApplyArgs(NodeConfig& cfg, const std::vector<std::string>& args, std::vector<std::string>& positional);
    static Result<void> Validate(const NodeConfig& cfg, const ChainParams& params, const std::string& cryptoProviderName);
    static Result<ChainParams> ChainParamsForNetwork(const std::string& network);
    static std::filesystem::path DefaultConfigPath(const std::filesystem::path& datadir);
};

} // namespace pqc
