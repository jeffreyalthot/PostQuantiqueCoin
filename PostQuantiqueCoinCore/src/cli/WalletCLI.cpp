#include "postquantiquecoin/cli/WalletCLI.h"
#include "postquantiquecoin/core/Amount.h"
#include <iostream>
namespace pqc {
WalletCLI::WalletCLI(WalletManager& w,Blockchain& b):wm_(w),bc_(b){}
int WalletCLI::Execute(const std::vector<std::string>& a){
    if(a.empty() || a[0]=="help"){ std::cout<<"commands: help create <name> <password> unlock <name> <password> lock newaddress listaddresses balance send backup rescan listwallets walletinfo\n"; return 0; }
    const auto& c=a[0];
    if(c=="create"&&a.size()>1){ if(a.size()<3){ std::cout<<"error: create requires an explicit password; no default password is allowed\n"; return 2; } return WalletCommands::commandCreate(wm_,a[1],a[2]); }
    if(c=="unlock"&&a.size()>1){ if(a.size()<3){ std::cout<<"error: unlock requires a password argument\n"; return 2; } return WalletCommands::commandUnlock(wm_,a[1],a[2]); }
    if(c=="lock"&&a.size()>1) return WalletCommands::commandLock(wm_,a[1]);
    if(c=="newaddress"&&a.size()>2) return WalletCommands::commandNewAddress(wm_,a[1],a[2]);
    if(c=="listaddresses"&&a.size()>1) return WalletCommands::commandListAddresses(wm_,a[1]);
    if(c=="listwallets"){ for(const auto& w: wm_.ListWallets()) std::cout<<w<<'\n'; return 0; }
    if(c=="walletinfo"&&a.size()>1){ auto* w=wm_.GetWallet(a[1]); if(!w){ auto r=wm_.OpenWallet(a[1]); if(r.IsErr()){ std::cout<<"error: "<<r.Error()<<'\n'; return 1; } w=wm_.GetWallet(a[1]); } std::cout<<"wallet="<<a[1]<<" addresses="<<(w?w->ListAddresses().size():0)<<"\n"; return w?0:1; }
    if(c=="balance"&&a.size()>1) return WalletCommands::commandBalance(wm_,a[1],bc_);
    if(c=="send"&&a.size()>4){ auto amt=Amount::FromCoinsString(a[3]); auto fee=Amount::FromCoinsString(a[4]); if(amt.IsErr()||fee.IsErr()) return 1; return WalletCommands::commandSend(wm_,a[1],a[2],amt.Value(),fee.Value(),bc_); }
    if(c=="backup"&&a.size()>2) return WalletCommands::commandBackup(wm_,a[1],a[2]);
    if(c=="rescan"&&a.size()>1) return WalletCommands::commandRescan(wm_,a[1],bc_);
    std::cout<<"bad wallet command\n"; return 1;
}
}
