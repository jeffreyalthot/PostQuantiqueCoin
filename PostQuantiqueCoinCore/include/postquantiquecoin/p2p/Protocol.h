#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
namespace pqc::p2p { constexpr uint32_t Magic = 0x51504350U; constexpr uint32_t MaxPayloadBytes = 4*1024*1024; const std::vector<std::string>& KnownCommands(); }
