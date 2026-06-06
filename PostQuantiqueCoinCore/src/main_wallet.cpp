#include "postquantiquecoin/cli/WalletCLI.h"
#include "postquantiquecoin/core/Config.h"
#include "postquantiquecoin/crypto/PQCryptoProvider.h"

#include <iostream>

int main(int argc, char** argv) {
    try {
        auto crypto = pqc::CreateDefaultCryptoProvider();
        std::vector<std::string> rawArgs;
        for (int i = 1; i < argc; ++i) rawArgs.emplace_back(argv[i]);
        auto loaded = pqc::Config::Load(pqc::Config::DefaultConfigPath("data"));
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
        const auto networkDir = cfg.datadir / params.Value().networkName;
        auto bc = pqc::Blockchain::OpenOrCreate(networkDir, params.Value(), crypto.get());
        if (bc.IsErr()) {
            std::cerr << bc.Error() << '\n';
            return 1;
        }
        pqc::WalletManager wm(networkDir / "wallets", crypto.get());
        pqc::WalletCLI cli(wm, bc.Value());
        return cli.Execute(positional);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
