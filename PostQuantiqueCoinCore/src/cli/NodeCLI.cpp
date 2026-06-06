#include "postquantiquecoin/cli/NodeCLI.h"
#include "postquantiquecoin/application/NodeService.h"
#include "postquantiquecoin/core/Amount.h"
#include <iostream>

namespace pqc {
namespace {
void PrintHelp(){ std::cout << "commands: help start status getinfo getblockchaininfo getnetworkinfo printchain getblockheight getblock mempool validatechain balance\n"; }
}
NodeCLI::NodeCLI(Blockchain& c, NodeConfig config):chain_(c),config_(std::move(config)){}
int NodeCLI::Execute(const std::vector<std::string>& a){
    std::string cmd=a.empty()?"status":a[0];
    if(cmd=="help"){ PrintHelp(); return 0; }
    if(cmd=="start"||cmd=="status"||cmd=="getinfo"){
        application::NodeService service(chain_, config_); auto info = service.GetInfo();
        if(config_.json) std::cout << "{\"network\":\""<<info.network<<"\",\"height\":"<<info.height<<",\"tip\":\""<<info.tipHash<<"\",\"mempool\":"<<info.mempoolSize<<"}\n";
        else std::cout<<"network="<<info.network<<" height="<<info.height<<" tip="<<info.tipHash<<" mempool="<<info.mempoolSize<<" minted="<<Amount::ToCoinsString(info.mintedSupply)<<" PQC\n";
        return 0;
    }
    if(cmd=="getblockchaininfo"){
        std::cout << "network="<<chain_.Params().networkName<<" height="<<chain_.GetHeight()<<" tip="<<chain_.GetTipHash()<<" next_subsidy="<<Amount::ToCoinsString(chain_.GetNextSubsidy())<<" PQC\n"; return 0;
    }
    if(cmd=="getnetworkinfo"){
        std::cout << "network="<<chain_.Params().networkName<<" listen="<<(config_.listen?1:0)<<" bind="<<config_.bind<<" maxconnections="<<config_.maxpeers<<" datadir="<<config_.datadir.string()<<"\n"; return 0;
    }
    if(cmd=="printchain"){ for(uint64_t h=0; h<=chain_.GetHeight(); ++h){ auto b=chain_.GetBlockByHeight(h); if(b) std::cout<<h<<' '<<b->GetHash()<<'\n'; } return 0; }
    if(cmd=="getblockheight"&&a.size()>1){ auto b=chain_.GetBlockByHeight(std::stoull(a[1])); if(!b) return 1; std::cout<<b->GetHash()<<'\n'; return 0; }
    if(cmd=="getblock"&&a.size()>1){ auto b=chain_.GetBlockByHash(a[1]); if(!b) return 1; std::cout<<"height="<<b->header.height<<" txs="<<b->transactions.size()<<'\n'; return 0; }
    if(cmd=="mempool"){ std::cout<<chain_.GetMempool().Size()<<" tx, fees="<<chain_.GetMempool().GetTotalFeesEstimate()<<'\n'; return 0; }
    if(cmd=="validatechain"){ auto r=chain_.ValidateChain(); std::cout<<(r.IsOk()?"valid":"invalid: "+r.Error())<<'\n'; return r.IsOk()?0:1; }
    if(cmd=="balance"&&a.size()>1){ std::cout<<Amount::ToCoinsString(chain_.GetBalance(a[1]))<<" PQC ("<<chain_.GetBalance(a[1])<<" atoms)\n"; return 0; }
    PrintHelp(); return 1;
}
}
