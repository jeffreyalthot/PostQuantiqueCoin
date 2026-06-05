#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace pqc { struct WalletHistoryEntry { std::string txid; uint64_t timestamp{0}; std::string direction; uint64_t amountAtoms{0}; uint64_t feeAtoms{0}; uint64_t blockHeight{0}; uint64_t confirmations{0}; std::string status; std::vector<std::string> fromAddresses; std::vector<std::string> toAddresses; }; }
