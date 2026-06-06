#include "postquantiquecoin/cli/NodeCLI.h"
#include "postquantiquecoin/core/Config.h"
#include "postquantiquecoin/core/Constants.h"
#include "postquantiquecoin/crypto/PQCryptoProvider.h"

#include <algorithm>
#include <filesystem>
#include <iostream>

namespace {

std::filesystem::path FindConfigPath(const std::vector<std::string>& args, const std::filesystem::path& defaultDataDir) {
    for (size_t i = 0; i + 1 < args.size(); ++i) {
        if (args[i] == "--conf") return args[i + 1];
    }
    return pqc::Config::DefaultConfigPath(defaultDataDir);
}

void PrintHelp() {
    std::cout << "PostQuantiqueCoin node\n"
              << "Usage: pqc_node [--network mainnet|testnet|devnet|regtest] [--datadir PATH] [--conf PATH] <command>\n"
              << "Commands: help, getinfo, getblockchaininfo, getnetworkinfo, getmininginfo, printchain, getblockheight, getblock, mempool, validatechain, balance\n";
}

void PrintVersion(const pqc::PQCryptoProvider& crypto) {
    std::cout << pqc::constants::PROJECT_NAME << " " << pqc::constants::PROJECT_VERSION
              << " crypto_provider=" << crypto.ProviderName() << '\n';
}

} // namespace

int main(int argc, char** argv) {
    try {
        std::vector<std::string> rawArgs;
        for (int i = 1; i < argc; ++i) rawArgs.emplace_back(argv[i]);

        auto crypto = pqc::CreateDefaultCryptoProvider();
        if (std::find(rawArgs.begin(), rawArgs.end(), "--version") != rawArgs.end()) {
            PrintVersion(*crypto);
            return 0;
        }
        if (std::find(rawArgs.begin(), rawArgs.end(), "--help") != rawArgs.end() || std::find(rawArgs.begin(), rawArgs.end(), "help") != rawArgs.end()) {
            PrintHelp();
            return 0;
        }

        auto loaded = pqc::Config::Load(FindConfigPath(rawArgs, "data"));
        if (loaded.IsErr()) {
            std::cerr << loaded.Error() << '\n';
            return 1;
        }
        auto cfg = loaded.Value();
        std::vector<std::string> positional;
        auto argsOk = pqc::Config::ApplyArgs(cfg, rawArgs, positional);
        if (argsOk.IsErr()) {
            std::cerr << argsOk.Error() << '\n';
            return 1;
        }
        auto params = pqc::Config::ChainParamsForNetwork(cfg.network);
        if (params.IsErr()) {
            std::cerr << params.Error() << '\n';
            return 1;
        }
        auto valid = pqc::Config::Validate(cfg, params.Value(), crypto->ProviderName());
        if (valid.IsErr()) {
            std::cerr << "safe-start refused startup: " << valid.Error() << '\n';
            return 1;
        }
        auto bc = pqc::Blockchain::OpenOrCreate(cfg.datadir / params.Value().networkName, params.Value(), crypto.get());
        if (bc.IsErr()) {
            std::cerr << bc.Error() << '\n';
            return 1;
        }
        pqc::NodeCLI cli(bc.Value());
        return cli.Execute(positional);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
