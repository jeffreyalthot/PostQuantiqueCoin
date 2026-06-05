#include "postquantiquecoin/cli/WalletCLI.h"
#include "postquantiquecoin/crypto/PQCryptoProvider.h"
#include <iostream>
int main(int argc,char** argv){ try{ auto crypto=pqc::CreateDefaultCryptoProvider(); auto bc=pqc::Blockchain::OpenOrCreate("data",pqc::ChainParams::Devnet(),crypto.get()); if(bc.IsErr()){ std::cerr<<bc.Error()<<'\n'; return 1; } pqc::WalletManager wm("data/wallets",crypto.get()); std::vector<std::string> args; for(int i=1;i<argc;++i) args.emplace_back(argv[i]); pqc::WalletCLI cli(wm,bc.Value()); return cli.Execute(args); }catch(const std::exception& e){ std::cerr<<e.what()<<'\n'; return 1; } }
