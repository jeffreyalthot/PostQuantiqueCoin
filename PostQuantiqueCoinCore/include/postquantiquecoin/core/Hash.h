#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace pqc {
using Hash256Bytes = std::array<uint8_t, 32>;
class Hash {
public:
    static std::string Zero();
    static bool IsValidHex256(const std::string& hex);
    static std::string FromBytes(const Hash256Bytes& bytes);
    static Hash256Bytes ToBytes(const std::string& hex);
};
}
