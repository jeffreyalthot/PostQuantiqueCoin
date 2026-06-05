#pragma once
#include "postquantiquecoin/wallet/WalletCommands.h"
namespace pqc { class WalletCLI { public: WalletCLI(WalletManager& wm, Blockchain& bc); int Execute(const std::vector<std::string>& args); private: WalletManager& wm_; Blockchain& bc_; }; }
