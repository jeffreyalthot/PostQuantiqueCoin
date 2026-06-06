#include "postquantiquecoin/cli/NodeCLI.h"

#include "postquantiquecoin/core/Amount.h"

#include <iostream>

namespace pqc {
namespace {

void PrintHelp() {
    std::cout
        << "PostQuantiqueCoin node\n"
        << "Usage: pqc_node [--network mainnet|testnet|devnet|regtest] [--datadir PATH] [--conf PATH] <command>\n\n"
        << "Commands:\n"
        << "  help                 Show this help\n"
        << "  getinfo|status       Node summary\n"
        << "  getblockchaininfo    Chain height, tip, supply, next subsidy\n"
        << "  getnetworkinfo       Network and consensus parameters\n"
        << "  getmininginfo        Next PoW target and subsidy\n"
        << "  printchain           Print height/hash list\n"
        << "  getblockheight H     Print block hash at height H\n"
        << "  getblock HASH        Print block summary\n"
        << "  mempool              Print mempool summary\n"
        << "  validatechain        Re-validate local chain\n"
        << "  balance ADDRESS      Print confirmed spendable balance in atoms\n";
}

} // namespace

NodeCLI::NodeCLI(Blockchain& c) : chain_(c) {}

int NodeCLI::Execute(const std::vector<std::string>& a) {
    const std::string cmd = a.empty() ? "status" : a[0];
    if (cmd == "help" || cmd == "--help") {
        PrintHelp();
        return 0;
    }
    if (cmd == "start" || cmd == "status" || cmd == "getinfo") {
        std::cout << "status=READY network=" << chain_.Params().networkName << " height=" << chain_.GetHeight()
                  << " tip=" << chain_.GetTipHash() << " mempool=" << chain_.GetMempool().Size() << '\n';
        return 0;
    }
    if (cmd == "getblockchaininfo") {
        std::cout << "network=" << chain_.Params().networkName << '\n'
                  << "height=" << chain_.GetHeight() << '\n'
                  << "tip=" << chain_.GetTipHash() << '\n'
                  << "minted_atoms=" << chain_.GetMintedSupply() << '\n'
                  << "next_subsidy_atoms=" << chain_.GetNextSubsidy() << '\n';
        return 0;
    }
    if (cmd == "getnetworkinfo") {
        const auto& p = chain_.Params();
        std::cout << "network=" << p.networkName << '\n'
                  << "network_id=" << static_cast<int>(p.networkId) << '\n'
                  << "signature_algorithm=" << p.defaultSignatureAlgorithm << '\n'
                  << "kem_algorithm=" << p.defaultKemAlgorithm << '\n'
                  << "require_liboqs=" << (p.requireLibOqs ? "true" : "false") << '\n'
                  << "allow_dev_crypto=" << (p.allowDevCrypto ? "true" : "false") << '\n';
        return 0;
    }
    if (cmd == "getmininginfo") {
        std::cout << "height=" << chain_.GetHeight() << '\n'
                  << "next_bits=" << chain_.GetNextWorkRequired() << '\n'
                  << "next_reward_atoms=" << chain_.GetNextSubsidy() << '\n'
                  << "mempool_txs=" << chain_.GetMempool().Size() << '\n';
        return 0;
    }
    if (cmd == "printchain") {
        for (uint64_t h = 0; h <= chain_.GetHeight(); ++h) {
            auto b = chain_.GetBlockByHeight(h);
            if (b) std::cout << h << ' ' << b->GetHash() << '\n';
        }
        return 0;
    }
    if (cmd == "getblockheight" && a.size() > 1) {
        auto b = chain_.GetBlockByHeight(std::stoull(a[1]));
        if (!b) return 1;
        std::cout << b->GetHash() << '\n';
        return 0;
    }
    if (cmd == "getblock" && a.size() > 1) {
        auto b = chain_.GetBlockByHash(a[1]);
        if (!b) return 1;
        std::cout << "height=" << b->header.height << " txs=" << b->transactions.size() << " bits=" << b->header.bits << '\n';
        return 0;
    }
    if (cmd == "mempool") {
        std::cout << chain_.GetMempool().Size() << " tx, fees=" << chain_.GetMempool().GetTotalFeesEstimate() << '\n';
        return 0;
    }
    if (cmd == "validatechain") {
        auto r = chain_.ValidateChain();
        std::cout << (r.IsOk() ? "valid" : "invalid: " + r.Error()) << '\n';
        return r.IsOk() ? 0 : 1;
    }
    if (cmd == "balance" && a.size() > 1) {
        std::cout << chain_.GetBalance(a[1]) << " atoms\n";
        return 0;
    }
    PrintHelp();
    return 1;
}

} // namespace pqc
