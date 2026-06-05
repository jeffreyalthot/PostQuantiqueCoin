#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace pqc { class BlockHeader { public: uint32_t version{1}; std::string previousHash; std::string merkleRoot; uint64_t timestamp{0}; uint32_t bits{0x207fffff}; uint64_t nonce{0}; uint64_t height{0}; std::vector<uint8_t> Serialize() const; std::string GetHash() const; }; }
