#pragma once
#include "postquantiquecoin/core/Result.h"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace pqc {
struct PQPrivateKeyRecord {
    uint32_t version{1};
    std::string signatureAlgorithm{"ML-DSA-65"};
    std::string kemAlgorithm{"ML-KEM-768"};
    std::vector<uint8_t> signingPrivateKey;
    std::vector<uint8_t> signingPublicKey;
    std::vector<uint8_t> kemPrivateKey;
    std::vector<uint8_t> kemPublicKey;
    std::array<uint8_t, 32> keyFingerprint{};
    uint64_t createdTimestamp{0};

    std::vector<uint8_t> Serialize() const;
    static Result<PQPrivateKeyRecord> Deserialize(const std::vector<uint8_t>& data);
    Result<void> ValidateSizes(size_t signingPublicKeySize, size_t signingPrivateKeySize, size_t kemPublicKeySize, size_t kemPrivateKeySize) const;
    std::array<uint8_t, 32> ComputeFingerprint() const;
    void WipeSecrets();
    ~PQPrivateKeyRecord();
};
}
