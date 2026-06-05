#pragma once
#include "postquantiquecoin/blockchain/UTXO.h"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace pqc {
struct CoinSelectionPolicy {
    size_t maxInputsPerTransaction{128};
    uint64_t minChangeAtoms{1};
    uint64_t dustThreshold{1};
    bool preferConfirmed{true};
    bool avoidAddressReuse{true};
};

struct CoinSelectionResult {
    std::vector<UTXO> selected;
    uint64_t total{0};
    bool success{false};
    bool exactMatch{false};
};

class CoinSelection {
public:
    static CoinSelectionResult SelectSmallestAbove(std::vector<UTXO> utxos, uint64_t target, CoinSelectionPolicy policy = {});
    static CoinSelectionResult SelectBranchAndBound(std::vector<UTXO> utxos, uint64_t target, CoinSelectionPolicy policy = {});
    static CoinSelectionResult SelectPrivacyAware(std::vector<UTXO> utxos, uint64_t target, CoinSelectionPolicy policy = {});
    static CoinSelectionResult SelectConsolidation(std::vector<UTXO> utxos, uint64_t target, CoinSelectionPolicy policy = {});
};
}
