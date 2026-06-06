#pragma once
#include "postquantiquecoin/core/Result.h"
#include <filesystem>
#include <map>
#include <string>
#include <vector>
namespace pqc {
struct NodeConfig {
    uint32_t config_version{1};
    std::string network{"mainnet"};
    std::filesystem::path datadir{"data"};
    std::filesystem::path conf_path{};
    uint16_t port{29444};
    uint32_t maxpeers{32};
    bool listen{true};
    std::string bind{"0.0.0.0"};
    std::vector<std::string> connect;
    std::vector<std::string> addnode;
    std::vector<std::string> seednode;
    std::string seedpeer;
    uint32_t mining_threads{4};
    std::string log_level{"info"};
    std::filesystem::path wallet_dir{"data/wallets"};
    bool require_liboqs{true};
    bool allow_dev_crypto{false};
    bool allow_legacy_pow{false};
    bool safe_start{true};
    bool json{false};
};
class Config {
public:
    static Result<NodeConfig> Load(const std::filesystem::path& path);
    static Result<void> Save(const std::filesystem::path& path,const NodeConfig& cfg);
    static Result<NodeConfig> FromArgs(int argc, char** argv, const NodeConfig& base = NodeConfig{});
    static Result<void> Validate(const NodeConfig& cfg);
};
}
