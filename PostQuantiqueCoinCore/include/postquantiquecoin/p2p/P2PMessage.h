#pragma once
#include "postquantiquecoin/core/Result.h"
#include <cstdint>
#include <string>
#include <vector>
namespace pqc::p2p { struct P2PMessage { uint32_t magic{0x51504350U}; uint8_t networkId{1}; std::string command; std::vector<uint8_t> payload; std::vector<uint8_t> Serialize() const; static Result<P2PMessage> Deserialize(const std::vector<uint8_t>& data,uint8_t expectedNetworkId); }; }
