#pragma once
#include <cstddef>
#include <cstdint>
namespace pqc { struct MempoolPolicy { uint64_t minRelayFeeAtomsPerByte{1}; uint64_t dustThresholdAtoms{100}; size_t maxMempoolBytes{64*1024*1024}; uint64_t expirySeconds{14*24*60*60}; size_t maxAncestors{25}; size_t maxDescendants{25}; }; }
