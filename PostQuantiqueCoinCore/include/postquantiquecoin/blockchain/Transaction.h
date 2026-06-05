#pragma once
#include "postquantiquecoin/blockchain/TxInput.h"
#include "postquantiquecoin/blockchain/TxOutput.h"
#include "postquantiquecoin/core/Result.h"
#include "postquantiquecoin/crypto/PQCryptoProvider.h"
#include <array>
#include <cstdint>
#include <string>
#include <vector>
namespace pqc {
class Transaction {
public:
    uint32_t version{1};
    std::vector<TxInput> inputs;
    std::vector<TxOutput> outputs;
    uint64_t locktime{0};
    std::string signatureAlgorithm{"ML-DSA-65"};
    std::vector<uint8_t> publicKey;
    std::vector<uint8_t> signature;
    uint64_t timestamp{0};
    std::string txid;
    bool IsCoinbase() const;
    std::vector<uint8_t> Serialize(bool includeSignature = true) const;
    static Result<Transaction> Deserialize(const std::vector<uint8_t>& data);
    std::vector<uint8_t> GetSigningDigest() const;
    std::vector<uint8_t> SerializeForSigningInput(size_t inputIndex, const TxOutput& referencedOutput) const;
    std::vector<uint8_t> GetSigningDigestForInput(size_t inputIndex, const TxOutput& referencedOutput) const;
    std::string ComputeTxId() const;
    uint64_t GetOutputSum() const;
    std::vector<std::pair<std::string,uint32_t>> GetInputOutpoints() const;
    size_t EstimatedSize() const;
    Result<void> ValidateBasic() const;
    void Sign(PQCryptoProvider& provider, const std::vector<uint8_t>& privateKey, const std::vector<uint8_t>& pubKey);
    Result<void> SignInput(size_t inputIndex, const TxOutput& referencedOutput, PQCryptoProvider& provider, const std::vector<uint8_t>& privateKey, const std::vector<uint8_t>& pubKey);
    bool VerifyInputSignature(size_t inputIndex, const TxOutput& referencedOutput, PQCryptoProvider& provider) const;
    bool VerifySignature(PQCryptoProvider& provider) const;
    bool Verify(PQCryptoProvider& provider) const;
};
}
