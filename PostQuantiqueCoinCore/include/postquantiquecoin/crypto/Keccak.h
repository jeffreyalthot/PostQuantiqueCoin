#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace pqc {
class Keccak {
public:
    static std::array<uint8_t, 32> Sha3_256(const std::vector<uint8_t>& data);
    static std::array<uint8_t, 64> Sha3_512(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> Shake128(const std::vector<uint8_t>& data, size_t outputSize);
    static std::vector<uint8_t> Shake256(const std::vector<uint8_t>& data, size_t outputSize);
    static std::vector<uint8_t> Kmac256(const std::vector<uint8_t>& key,
                                        const std::vector<uint8_t>& message,
                                        const std::string& customization,
                                        size_t outputSize);
};
}
