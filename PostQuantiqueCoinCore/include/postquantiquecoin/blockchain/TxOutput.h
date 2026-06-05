#pragma once
#include <array>
#include <cstdint>
#include <string>
namespace pqc { struct TxOutput { uint64_t amountAtoms{0}; std::string address; std::string lockingScriptType{"PQC_PUBKEY_HASH"}; std::string requiredSignatureAlgorithm{"ML-DSA-65"}; std::array<uint8_t,32> publicKeyHash{}; }; }
