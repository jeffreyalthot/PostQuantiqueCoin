#include "postquantiquecoin/mining/Miner.h"
#include "postquantiquecoin/crypto/PQCryptoProvider.h"
#include <iostream>
int main(int argc,char** argv){ try{ if(argc<3 || std::string(argv[1])!="mine"){ std::cout<<"usage: pqc_miner mine PQC1ADDRESS --threads 4\n"; return 1; } uint32_t threads=1; for(int i=3;i+1<argc;++i) if(std::string(argv[i])=="--threads") threads=static_cast<uint32_t>(std::stoul(argv[i+1])); auto crypto=pqc::CreateDefaultCryptoProvider(); auto bc=pqc::Blockchain::OpenOrCreate("data",pqc::ChainParams::Devnet(),crypto.get()); if(bc.IsErr()){ std::cerr<<bc.Error()<<'\n'; return 1; } pqc::Miner miner(bc.Value()); auto r=miner.MineNextBlock(argv[2],threads); return r.IsOk()?0:1; }catch(const std::exception& e){ std::cerr<<e.what()<<'\n'; return 1; } }
