#pragma once
#include <cstdint>
#include <string>
#include "postquantiquecoin/core/Result.h"

namespace pqc {
class Amount {
public:
    static Result<uint64_t> FromCoinsString(const std::string& coins);
    static std::string ToCoinsString(uint64_t atoms);
    static bool Add(uint64_t a, uint64_t b, uint64_t& out);
    static bool Sub(uint64_t a, uint64_t b, uint64_t& out);
};
}
