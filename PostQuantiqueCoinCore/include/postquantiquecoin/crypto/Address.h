#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "postquantiquecoin/core/Result.h"
namespace pqc {
struct DecodedAddress { uint8_t version; std::vector<uint8_t> payload; };
class Address {
public:
    static std::string FromPublicKey(const std::vector<uint8_t>& publicKey);
    static bool Validate(const std::string& address);
    static Result<DecodedAddress> Decode(const std::string& address);
    static std::string Encode(uint8_t version, const std::vector<uint8_t>& payload);
};
}
