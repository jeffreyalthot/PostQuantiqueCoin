#pragma once
#include "postquantiquecoin/core/Result.h"
#include <array>
#include <cstdint>
#include <string>
#include <vector>
namespace pqc {
struct UTXO {
    std::string txid;
    uint32_t outputIndex{0};
    uint64_t amountAtoms{0};
    std::string address;
    std::string lockingScriptType{"PQC_PUBKEY_HASH"};
    uint64_t height{0};
    bool coinbase{false};
    uint64_t createdTimestamp{0};
    std::string requiredSignatureAlgorithm{"ML-DSA-65"};
    std::array<uint8_t,32> publicKeyHash{};
    std::string OutpointKey() const;
    std::vector<uint8_t> Serialize() const;
    static Result<UTXO> Deserialize(const std::vector<uint8_t>& data);
    bool IsSpendable(uint64_t currentHeight, uint64_t coinbaseMaturity) const;
    Result<void> ValidateBasic() const;
};
}
