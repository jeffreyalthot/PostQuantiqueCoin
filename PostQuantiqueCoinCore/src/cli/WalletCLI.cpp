#include "postquantiquecoin/cli/WalletCLI.h"

#include "postquantiquecoin/core/Amount.h"

#include <cctype>
#include <cstdlib>
#include <iostream>

namespace pqc {
namespace {

void PrintWalletHelp() {
    std::cout
        << "PostQuantiqueCoin wallet\n"
        << "Usage: pqc_wallet <command> [args]\n\n"
        << "Commands:\n"
        << "  create NAME PASSWORD        Create encrypted wallet (no default password)\n"
        << "  unlock NAME PASSWORD        Unlock wallet\n"
        << "  lock NAME                   Lock wallet\n"
        << "  newaddress NAME LABEL       Generate address\n"
        << "  listaddresses NAME          List wallet addresses\n"
        << "  balance NAME                Print balance\n"
        << "  send NAME TO AMOUNT FEE     Preview/build and queue transaction\n"
        << "  backup NAME PATH            Write encrypted wallet backup\n"
        << "  rescan NAME                 Rescan local chain\n";
}

bool StrongEnoughPassword(const std::string& password) {
    if (password.size() < 12) return false;
    bool upper = false, lower = false, digit = false;
    for (const unsigned char ch : password) {
        upper = upper || std::isupper(ch);
        lower = lower || std::islower(ch);
        digit = digit || std::isdigit(ch);
    }
    return upper && lower && digit;
}

} // namespace

WalletCLI::WalletCLI(WalletManager& w, Blockchain& b) : wm_(w), bc_(b) {}

int WalletCLI::Execute(const std::vector<std::string>& a) {
    if (a.empty() || a[0] == "help" || a[0] == "--help") {
        PrintWalletHelp();
        return 0;
    }
    const auto& c = a[0];
    if (c == "create") {
        if (a.size() < 3) {
            std::cout << "error: wallet creation requires an explicit password argument or secure prompt integration\n";
            return 1;
        }
        if (!StrongEnoughPassword(a[2])) {
            std::cout << "error: password policy requires at least 12 characters with upper/lowercase letters and a digit\n";
            return 1;
        }
        return WalletCommands::commandCreate(wm_, a[1], a[2]);
    }
    if (c == "unlock") {
        if (a.size() < 3) {
            std::cout << "error: unlock requires an explicit password; default passwords are forbidden\n";
            return 1;
        }
        return WalletCommands::commandUnlock(wm_, a[1], a[2]);
    }
    if (c == "lock" && a.size() > 1) return WalletCommands::commandLock(wm_, a[1]);
    if (c == "newaddress" && a.size() > 2) return WalletCommands::commandNewAddress(wm_, a[1], a[2]);
    if (c == "listaddresses" && a.size() > 1) return WalletCommands::commandListAddresses(wm_, a[1]);
    if (c == "balance" && a.size() > 1) return WalletCommands::commandBalance(wm_, a[1], bc_);
    if (c == "send" && a.size() > 4) {
        auto amt = Amount::FromCoinsString(a[3]);
        auto fee = Amount::FromCoinsString(a[4]);
        if (amt.IsErr() || fee.IsErr()) return 1;
        return WalletCommands::commandSend(wm_, a[1], a[2], amt.Value(), fee.Value(), bc_);
    }
    if (c == "backup" && a.size() > 2) return WalletCommands::commandBackup(wm_, a[1], a[2]);
    if (c == "rescan" && a.size() > 1) return WalletCommands::commandRescan(wm_, a[1], bc_);
    PrintWalletHelp();
    return 1;
}

} // namespace pqc
