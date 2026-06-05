#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
namespace pqc {
class Hashing {
public:
    static std::array<uint8_t,32> Sha256(const std::vector<uint8_t>& data);
    static std::array<uint8_t,32> DoubleSha256(const std::vector<uint8_t>& data);
    static std::array<uint8_t,32> Sha3_256(const std::vector<uint8_t>& data);
    static std::string Hash256Hex(const std::vector<uint8_t>& data);
    static std::string Sha3_256Hex(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> HmacSha256(const std::vector<uint8_t>& key, const std::vector<uint8_t>& message);
    static bool SecureCompare(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b);
};
}
