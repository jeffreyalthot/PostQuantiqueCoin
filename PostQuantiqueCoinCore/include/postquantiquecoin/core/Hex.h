#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "postquantiquecoin/core/Result.h"
namespace pqc { class Hex { public: static std::string Encode(const std::vector<uint8_t>& data); static Result<std::vector<uint8_t>> Decode(const std::string& hex); static bool IsHex(const std::string& hex); }; }
