#pragma once
#include <array>
#include <cstdint>
#include <string>
namespace pqc { class ChainWork { public: static std::array<uint8_t,32> CalculateBlockWork(uint32_t bits); static std::array<uint8_t,32> AddWork(const std::array<uint8_t,32>& a,const std::array<uint8_t,32>& b); static int CompareWork(const std::array<uint8_t,32>& a,const std::array<uint8_t,32>& b); static std::string WorkToHex(const std::array<uint8_t,32>& w); static std::array<uint8_t,32> WorkFromHex(const std::string& h); }; }
