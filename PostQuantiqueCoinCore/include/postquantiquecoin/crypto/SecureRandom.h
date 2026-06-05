#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
namespace pqc { class SecureRandom { public: static std::vector<uint8_t> Bytes(size_t n); }; }
