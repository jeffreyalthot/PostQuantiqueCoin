#pragma once
#include "postquantiquecoin/core/Result.h"
#include <array>
#include <cstdint>
#include <string>
#include <vector>
namespace pqc {
struct WalletKey {
    std::string label;
    std::string address;
    std::string signatureAlgorithm{"ML-DSA-65"};
    std::string kemAlgorithm{"ML-KEM-768"};
    std::vector<uint8_t> signingPublicKey;
    std::vector<uint8_t> kemPublicKey;
    std::vector<uint8_t> encryptedPrivateKeyRecord;
    std::array<uint8_t,32> keyFingerprint{};
    uint64_t createdTimestamp{0};
    // Compatibility aliases for older callers.
    std::vector<uint8_t> publicKey;
    std::vector<uint8_t> encryptedPrivateKey;
    std::vector<uint8_t> Serialize() const;
    static Result<WalletKey> Deserialize(const std::vector<uint8_t>& data);
    const std::vector<uint8_t>& EffectivePublicKey() const { return signingPublicKey.empty()?publicKey:signingPublicKey; }
    const std::vector<uint8_t>& EffectiveEncryptedRecord() const { return encryptedPrivateKeyRecord.empty()?encryptedPrivateKey:encryptedPrivateKeyRecord; }
};
}
