#pragma once
#include <cstdint>
#include <string>
namespace pqc { struct TxOutput { uint64_t amountAtoms{0}; std::string address; std::string lockingScriptType{"PQC_PUBKEY_HASH"}; }; }
