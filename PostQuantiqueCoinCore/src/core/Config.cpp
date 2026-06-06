#include "postquantiquecoin/core/Config.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <unordered_set>

namespace pqc {
namespace {

std::string Trim(const std::string& in) {
    const auto first = in.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = in.find_last_not_of(" \t\r\n");
    return in.substr(first, last - first + 1);
}

bool On(const std::string& v) {
    auto s = Trim(v);
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s == "1" || s == "true" || s == "yes" || s == "on";
}

std::vector<std::string> SplitList(const std::string& value) {
    std::vector<std::string> out;
    std::stringstream ss(value);
    std::string item;
    while (std::getline(ss, item, ',')) {
        item = Trim(item);
        if (!item.empty()) out.push_back(item);
    }
    return out;
}

uint16_t ParsePort(const std::string& raw) {
    const auto v = std::stoul(raw);
    if (v == 0 || v > 65535) throw std::out_of_range("port out of range");
    return static_cast<uint16_t>(v);
}

void ApplyKeyValue(NodeConfig& c, const std::string& rawKey, const std::string& rawValue) {
    const auto k = Trim(rawKey);
    const auto v = Trim(rawValue);
    if (k == "config_version") c.formatVersion = static_cast<uint32_t>(std::stoul(v));
    else if (k == "network") c.network = v;
    else if (k == "datadir") c.datadir = v;
    else if (k == "port") c.port = ParsePort(v);
    else if (k == "maxpeers" || k == "maxconnections") c.maxpeers = static_cast<uint32_t>(std::stoul(v));
    else if (k == "listen") c.listen = On(v);
    else if (k == "bind") c.bind = v;
    else if (k == "connect") c.connect = SplitList(v);
    else if (k == "addnode") c.addnode = SplitList(v);
    else if (k == "seednode" || k == "seedpeer") c.seednode = SplitList(v);
    else if (k == "mining_threads") c.mining_threads = static_cast<uint32_t>(std::stoul(v));
    else if (k == "log_level") c.log_level = v;
    else if (k == "wallet_dir") c.wallet_dir = v;
    else if (k == "require_liboqs") c.require_liboqs = On(v);
    else if (k == "allow_dev_crypto") c.allow_dev_crypto = On(v);
    else if (k == "allow_legacy_pow") c.allow_legacy_pow = On(v);
    else if (k == "safe_start") c.safe_start = On(v);
    else if (k == "json") c.json = On(v);
    else if (k == "no_color") c.no_color = On(v);
}

std::string Join(const std::vector<std::string>& values) {
    std::string out;
    for (size_t i = 0; i < values.size(); ++i) {
        if (i) out += ',';
        out += values[i];
    }
    return out;
}

} // namespace

Result<NodeConfig> Config::Load(const std::filesystem::path& path) {
    NodeConfig c;
    if (const char* env = std::getenv("PQC_NETWORK")) c.network = env;
    if (const char* env = std::getenv("PQC_DATADIR")) c.datadir = env;

    std::ifstream in(path);
    if (!in) return Result<NodeConfig>::Ok(c);

    std::string line;
    uint64_t lineNumber = 0;
    try {
        while (std::getline(in, line)) {
            ++lineNumber;
            line = Trim(line);
            if (line.empty() || line[0] == '#') continue;
            const auto comment = line.find('#');
            if (comment != std::string::npos) line = Trim(line.substr(0, comment));
            const auto pos = line.find('=');
            if (pos == std::string::npos) {
                return Result<NodeConfig>::Err("invalid config line " + std::to_string(lineNumber));
            }
            ApplyKeyValue(c, line.substr(0, pos), line.substr(pos + 1));
        }
    } catch (const std::exception& e) {
        return Result<NodeConfig>::Err("invalid config value at line " + std::to_string(lineNumber) + ": " + e.what());
    }
    return Result<NodeConfig>::Ok(c);
}

Result<void> Config::Save(const std::filesystem::path& path, const NodeConfig& c) {
    if (!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path);
    if (!out) return Result<void>::Err("config write failed");
    out << "config_version=" << c.formatVersion << '\n'
        << "network=" << c.network << '\n'
        << "datadir=" << c.datadir.string() << '\n'
        << "port=" << c.port << '\n'
        << "maxconnections=" << c.maxpeers << '\n'
        << "listen=" << (c.listen ? 1 : 0) << '\n'
        << "bind=" << c.bind << '\n'
        << "connect=" << Join(c.connect) << '\n'
        << "addnode=" << Join(c.addnode) << '\n'
        << "seednode=" << Join(c.seednode) << '\n'
        << "mining_threads=" << c.mining_threads << '\n'
        << "log_level=" << c.log_level << '\n'
        << "wallet_dir=" << c.wallet_dir.string() << '\n'
        << "require_liboqs=" << (c.require_liboqs ? 1 : 0) << '\n'
        << "allow_dev_crypto=" << (c.allow_dev_crypto ? 1 : 0) << '\n'
        << "allow_legacy_pow=" << (c.allow_legacy_pow ? 1 : 0) << '\n'
        << "safe_start=" << (c.safe_start ? 1 : 0) << '\n';
    return Result<void>::Ok();
}

Result<void> Config::ApplyArgs(NodeConfig& cfg, const std::vector<std::string>& args, std::vector<std::string>& positional) {
    positional.clear();
    try {
        for (size_t i = 0; i < args.size(); ++i) {
            const auto& arg = args[i];
            auto needValue = [&](const std::string& name) -> std::string {
                if (i + 1 >= args.size()) throw std::runtime_error(name + " requires a value");
                return args[++i];
            };
            if (arg == "--network") cfg.network = needValue(arg);
            else if (arg == "--datadir") cfg.datadir = needValue(arg);
            else if (arg == "--port") cfg.port = ParsePort(needValue(arg));
            else if (arg == "--conf") { (void)needValue(arg); }
            else if (arg == "--maxconnections") cfg.maxpeers = static_cast<uint32_t>(std::stoul(needValue(arg)));
            else if (arg == "--bind") cfg.bind = needValue(arg);
            else if (arg == "--connect") cfg.connect.push_back(needValue(arg));
            else if (arg == "--addnode") cfg.addnode.push_back(needValue(arg));
            else if (arg == "--seednode") cfg.seednode.push_back(needValue(arg));
            else if (arg == "--listen") cfg.listen = On(needValue(arg));
            else if (arg == "--json") cfg.json = true;
            else if (arg == "--no-color") cfg.no_color = true;
            else if (arg == "--safe-start") cfg.safe_start = true;
            else if (arg == "--unsafe-start") cfg.safe_start = false;
            else if (arg == "--help" || arg == "--version") positional.push_back(arg);
            else if (!arg.empty() && arg[0] == '-') return Result<void>::Err("unknown option: " + arg);
            else positional.push_back(arg);
        }
    } catch (const std::exception& e) {
        return Result<void>::Err(e.what());
    }
    return Result<void>::Ok();
}

Result<ChainParams> Config::ChainParamsForNetwork(const std::string& network) {
    if (network == "mainnet") return Result<ChainParams>::Ok(ChainParams::Mainnet());
    if (network == "testnet") return Result<ChainParams>::Ok(ChainParams::Testnet());
    if (network == "devnet") return Result<ChainParams>::Ok(ChainParams::Devnet());
    if (network == "regtest") return Result<ChainParams>::Ok(ChainParams::Regtest());
    return Result<ChainParams>::Err("unknown network: " + network);
}

Result<void> Config::Validate(const NodeConfig& cfg, const ChainParams& params, const std::string& provider) {
    static const std::unordered_set<std::string> validLogs{"trace", "debug", "info", "warn", "error", "fatal"};
    if (cfg.formatVersion != 1) return Result<void>::Err("unsupported config version");
    if (cfg.datadir.empty()) return Result<void>::Err("datadir is required");
    if (cfg.maxpeers == 0 || cfg.maxpeers > 1024) return Result<void>::Err("maxconnections must be in [1,1024]");
    if (!validLogs.count(cfg.log_level)) return Result<void>::Err("invalid log_level");
    if (cfg.safe_start) {
        if (params.networkName != cfg.network) return Result<void>::Err("network parameters do not match config network");
        if (params.networkName == "mainnet" && (params.allowDevCrypto || cfg.allow_dev_crypto || provider.find("DevOnly") != std::string::npos)) {
            return Result<void>::Err("mainnet refuses dev-only crypto provider");
        }
        if (params.requireLibOqs && provider.find("liboqs") == std::string::npos && provider.find("OQS") == std::string::npos) {
            return Result<void>::Err("selected network requires liboqs provider");
        }
    }
    return Result<void>::Ok();
}

std::filesystem::path Config::DefaultConfigPath(const std::filesystem::path& datadir) {
    return datadir / "postquantiquecoin.conf";
}

} // namespace pqc
