#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace pqc {
struct PQPublicIdentity {
    std::string address;
    std::string signatureAlgorithm;
    std::string kemAlgorithm;
    std::vector<uint8_t> signingPublicKey;
    std::vector<uint8_t> kemPublicKey;
    std::array<uint8_t, 32> signingPublicKeyHash{};
    std::array<uint8_t, 32> kemPublicKeyHash{};
    uint64_t createdTimestamp{0};
};
}
