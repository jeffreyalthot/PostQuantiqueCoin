#include "postquantiquecoin/cli/NodeCLI.h"
#include "postquantiquecoin/core/BuildInfo.h"
#include "postquantiquecoin/core/Config.h"
#include "postquantiquecoin/crypto/PQCryptoProvider.h"
#include <iostream>

namespace {
pqc::ChainParams ParamsFor(const std::string& network) {
    if (network == "mainnet") return pqc::ChainParams::Mainnet();
    if (network == "testnet") return pqc::ChainParams::Testnet();
    if (network == "regtest") return pqc::ChainParams::Regtest();
    return pqc::ChainParams::Devnet();
}
bool HasArg(int argc, char** argv, const std::string& flag) { for (int i=1;i<argc;++i) if (argv[i] == flag) return true; return false; }
}

int main(int argc,char** argv){
    try{
        if (HasArg(argc, argv, "--version")) { std::cout << pqc::build::BuildManifest(); return 0; }
        if (HasArg(argc, argv, "--help")) { std::cout << "pqc_node [--network mainnet|testnet|devnet|regtest] [--datadir path] [--conf path] [--json] <command>\n"; return 0; }
        auto cfg = pqc::Config::FromArgs(argc, argv);
        if(cfg.IsErr()){ std::cerr<<cfg.Error()<<'\n'; return 2; }
        auto params = ParamsFor(cfg.Value().network);
        auto crypto=pqc::CreateDefaultCryptoProvider();
        if (params.networkName == "mainnet" && crypto->ProviderName().find("DevOnly") != std::string::npos) { std::cerr << "dev-only crypto provider refused on mainnet\n"; return 1; }
        auto bc=pqc::Blockchain::OpenOrCreate(cfg.Value().datadir,params,crypto.get());
        if(bc.IsErr()){ std::cerr<<bc.Error()<<'\n'; return 1; }
        std::vector<std::string> args;
        for(int i=1;i<argc;++i){ std::string a=argv[i]; if(a.rfind("--",0)==0){ if(a=="--network"||a=="--datadir"||a=="--conf"||a=="--bind"||a=="--connect"||a=="--addnode"||a=="--seednode"||a=="--maxconnections") ++i; continue; } args.emplace_back(a); }
        pqc::NodeCLI cli(bc.Value(), cfg.Value());
        return cli.Execute(args);
    }catch(const std::exception& e){ std::cerr<<e.what()<<'\n'; return 1; }
}
