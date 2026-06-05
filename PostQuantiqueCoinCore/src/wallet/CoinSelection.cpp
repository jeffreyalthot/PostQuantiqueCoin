#include "postquantiquecoin/wallet/CoinSelection.h"
#include <algorithm>
#include <limits>
#include <map>

namespace pqc {
namespace {
bool SafeAdd(uint64_t a, uint64_t b, uint64_t& out) {
    if (std::numeric_limits<uint64_t>::max() - a < b) return false;
    out = a + b;
    return true;
}

void Finalize(CoinSelectionResult& r, uint64_t target) {
    r.success = r.total >= target;
    r.exactMatch = r.success && r.total == target;
}
}

CoinSelectionResult CoinSelection::SelectSmallestAbove(std::vector<UTXO> u, uint64_t target, CoinSelectionPolicy policy) {
    std::sort(u.begin(), u.end(), [](const auto& a, const auto& b) {
        if (a.amountAtoms != b.amountAtoms) return a.amountAtoms < b.amountAtoms;
        return a.OutpointKey() < b.OutpointKey();
    });
    CoinSelectionResult r;
    for (const auto& x : u) {
        if (r.selected.size() >= policy.maxInputsPerTransaction) break;
        uint64_t next = 0;
        if (!SafeAdd(r.total, x.amountAtoms, next)) break;
        r.selected.push_back(x);
        r.total = next;
        if (r.total >= target) break;
    }
    Finalize(r, target);
    return r;
}

CoinSelectionResult CoinSelection::SelectBranchAndBound(std::vector<UTXO> u, uint64_t target, CoinSelectionPolicy policy) {
    u.erase(std::remove_if(u.begin(), u.end(), [&](const UTXO& x) { return x.amountAtoms < policy.dustThreshold; }), u.end());
    std::sort(u.begin(), u.end(), [](const auto& a, const auto& b) {
        if (a.amountAtoms != b.amountAtoms) return a.amountAtoms > b.amountAtoms;
        return a.OutpointKey() < b.OutpointKey();
    });

    CoinSelectionResult best;
    uint64_t bestWaste = std::numeric_limits<uint64_t>::max();
    const size_t n = std::min(u.size(), policy.maxInputsPerTransaction);
    const uint64_t states = n >= 20 ? 0 : (uint64_t{1} << n);
    if (states != 0) {
        for (uint64_t mask = 1; mask < states; ++mask) {
            CoinSelectionResult cur;
            for (size_t i = 0; i < n; ++i) {
                if ((mask & (uint64_t{1} << i)) == 0) continue;
                uint64_t next = 0;
                if (!SafeAdd(cur.total, u[i].amountAtoms, next)) { cur.total = std::numeric_limits<uint64_t>::max(); break; }
                cur.total = next;
                cur.selected.push_back(u[i]);
            }
            if (cur.total < target) continue;
            const uint64_t waste = cur.total - target;
            const bool dustChange = waste > 0 && waste < policy.minChangeAtoms;
            if (waste == 0 || (!dustChange && waste < bestWaste)) {
                cur.success = true;
                cur.exactMatch = waste == 0;
                best = cur;
                bestWaste = waste;
                if (waste == 0) break;
            }
        }
    }
    if (best.success) return best;
    return SelectSmallestAbove(std::move(u), target, policy);
}

CoinSelectionResult CoinSelection::SelectPrivacyAware(std::vector<UTXO> u, uint64_t target, CoinSelectionPolicy policy) {
    if (policy.avoidAddressReuse) {
        std::map<std::string, std::vector<UTXO>> byAddress;
        for (auto& x : u) byAddress[x.address].push_back(std::move(x));
        CoinSelectionResult best;
        uint64_t bestWaste = std::numeric_limits<uint64_t>::max();
        for (auto& [address, coins] : byAddress) {
            (void)address;
            auto r = SelectBranchAndBound(std::move(coins), target, policy);
            if (r.success && r.total - target < bestWaste) {
                best = std::move(r);
                bestWaste = best.total - target;
            }
        }
        if (best.success) return best;
    }
    return SelectBranchAndBound(std::move(u), target, policy);
}

CoinSelectionResult CoinSelection::SelectConsolidation(std::vector<UTXO> u, uint64_t target, CoinSelectionPolicy policy) {
    std::sort(u.begin(), u.end(), [](const auto& a, const auto& b) {
        if (a.amountAtoms != b.amountAtoms) return a.amountAtoms < b.amountAtoms;
        return a.OutpointKey() < b.OutpointKey();
    });
    return SelectSmallestAbove(std::move(u), target, policy);
}
}
