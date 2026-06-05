#pragma once
#include "postquantiquecoin/blockchain/UTXO.h"
#include <vector>
namespace pqc { struct CoinSelectionResult { std::vector<UTXO> selected; uint64_t total{0}; bool success{false}; }; class CoinSelection { public: static CoinSelectionResult SelectSmallestAbove(std::vector<UTXO> utxos,uint64_t target); static CoinSelectionResult SelectBranchAndBound(std::vector<UTXO> utxos,uint64_t target); static CoinSelectionResult SelectPrivacyAware(std::vector<UTXO> utxos,uint64_t target); static CoinSelectionResult SelectConsolidation(std::vector<UTXO> utxos,uint64_t target); }; }
