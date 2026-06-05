#pragma once
#include "postquantiquecoin/core/Result.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
namespace pqc {
enum class WalletKdfAlgorithm : uint8_t { PBKDF2_HMAC_SHA256_BCRYPT = 1, ARGON2ID_OPTIONAL = 2, DEV_ONLY_KMAC_LOOP = 250 };
class WindowsWalletKdf {
public:
    static constexpr uint32_t ProductionIterations = 600000;
    static Result<std::vector<uint8_t>> DerivePbkdf2HmacSha256(const std::string& password,const std::vector<uint8_t>& salt,uint32_t iterations,size_t outputSize);
};
}
