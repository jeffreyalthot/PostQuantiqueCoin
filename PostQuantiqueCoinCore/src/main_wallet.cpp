#include "postquantiquecoin/cli/WalletCLI.h"
#include "postquantiquecoin/core/BuildInfo.h"
#include "postquantiquecoin/core/Config.h"
#include "postquantiquecoin/crypto/PQCryptoProvider.h"
#include <filesystem>
#include <iostream>

namespace {
pqc::ChainParams ParamsFor(const std::string& network) { if (network=="mainnet") return pqc::ChainParams::Mainnet(); if(network=="testnet") return pqc::ChainParams::Testnet(); if(network=="regtest") return pqc::ChainParams::Regtest(); return pqc::ChainParams::Devnet(); }
bool HasArg(int argc, char** argv, const std::string& flag) { for (int i=1;i<argc;++i) if (argv[i] == flag) return true; return false; }
}

int main(int argc,char** argv){
    try{
        if (HasArg(argc, argv, "--version")) { std::cout << pqc::build::BuildManifest(); return 0; }
        if (HasArg(argc, argv, "--help")) { std::cout << "pqc_wallet [--network mainnet|testnet|devnet|regtest] [--datadir path] [--conf path] <command>\n"; return 0; }
        auto cfg=pqc::Config::FromArgs(argc, argv); if(cfg.IsErr()){ std::cerr<<cfg.Error()<<'\n'; return 2; }
        auto crypto=pqc::CreateDefaultCryptoProvider();
        auto bc=pqc::Blockchain::OpenOrCreate(cfg.Value().datadir,ParamsFor(cfg.Value().network),crypto.get()); if(bc.IsErr()){ std::cerr<<bc.Error()<<'\n'; return 1; }
        auto walletDir = (cfg.Value().wallet_dir.empty() || cfg.Value().wallet_dir == std::filesystem::path("data/wallets")) ? cfg.Value().datadir/"wallets" : cfg.Value().wallet_dir;
        pqc::WalletManager wm(walletDir,crypto.get());
        std::vector<std::string> args; for(int i=1;i<argc;++i){ std::string a=argv[i]; if(a.rfind("--",0)==0){ if(a=="--network"||a=="--datadir"||a=="--conf") ++i; continue; } args.emplace_back(a); }
        pqc::WalletCLI cli(wm,bc.Value()); return cli.Execute(args);
    }catch(const std::exception& e){ std::cerr<<e.what()<<'\n'; return 1; }
}
