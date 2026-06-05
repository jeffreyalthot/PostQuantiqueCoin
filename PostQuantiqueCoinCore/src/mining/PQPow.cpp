#include "postquantiquecoin/mining/PQPow.h"
#include "postquantiquecoin/core/Hex.h"
#include "postquantiquecoin/crypto/Hashing.h"
#include <algorithm>

namespace pqc {
namespace {
std::vector<uint8_t> WithDomain(const char* domain, const std::vector<uint8_t>& data) {
    std::vector<uint8_t> out;
    for (const char* p = domain; *p; ++p) out.push_back(static_cast<uint8_t>(*p));
    out.insert(out.end(), data.begin(), data.end());
    return out;
}
std::vector<uint8_t> WithDomainAndNonce(const char* domain, const std::array<uint8_t,64>& stage1, uint64_t nonce) {
    std::vector<uint8_t> out;
    for (const char* p = domain; *p; ++p) out.push_back(static_cast<uint8_t>(*p));
    out.insert(out.end(), stage1.begin(), stage1.end());
    for (int i = 0; i < 8; ++i) out.push_back(static_cast<uint8_t>((nonce >> (8 * i)) & 0xff));
    return out;
}
}

std::array<uint8_t, 32> PQPow::ComputePqcSha3ShakeV1(const BlockHeader& header) {
    auto serialized = header.Serialize();
    auto stage1 = Hashing::Sha3_512(WithDomain("PQC_POW_STAGE1_V1", serialized));
    auto stage2Input = WithDomainAndNonce("PQC_POW_STAGE2_V1", stage1, header.nonce);
    auto stage2 = Hashing::Shake256(stage2Input, 64);
    std::vector<uint8_t> stage3Input;
    for (const char* p = "PQC_POW_STAGE3_V1"; *p; ++p) stage3Input.push_back(static_cast<uint8_t>(*p));
    stage3Input.insert(stage3Input.end(), stage2.begin(), stage2.end());
    stage3Input.insert(stage3Input.end(), serialized.begin(), serialized.end());
    auto stage3 = Hashing::Sha3_512(stage3Input);
    std::vector<uint8_t> finalInput;
    for (const char* p = "PQC_POW_FINAL_V1"; *p; ++p) finalInput.push_back(static_cast<uint8_t>(*p));
    finalInput.insert(finalInput.end(), stage3.begin(), stage3.end());
    auto final = Hashing::Shake256(finalInput, 32);
    std::array<uint8_t, 32> out{};
    std::copy(final.begin(), final.end(), out.begin());
    return out;
}

std::array<uint8_t, 32> PQPow::ComputePowHash(const BlockHeader& header, const ChainParams& params) {
    if (params.powAlgorithm == PowAlgorithm::DoubleSha256Legacy) return Hashing::DoubleSha256(header.Serialize());
    return ComputePqcSha3ShakeV1(header);
}

std::string PQPow::ToHex(const std::array<uint8_t, 32>& hash) { return Hex::Encode(std::vector<uint8_t>(hash.begin(), hash.end())); }
}
