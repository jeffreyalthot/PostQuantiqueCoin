#pragma once
#include "postquantiquecoin/blockchain/Block.h"
#include "postquantiquecoin/blockchain/ChainParams.h"
#include <atomic>
namespace pqc { struct MiningJob { Block candidateBlock; std::string minerAddress; uint64_t height{0}; uint64_t subsidyAtoms{0}; uint64_t feesAtoms{0}; uint32_t bits{0}; ChainParams params{}; std::atomic<bool> cancelled{false}; MiningJob()=default; MiningJob(const MiningJob& o):candidateBlock(o.candidateBlock),minerAddress(o.minerAddress),height(o.height),subsidyAtoms(o.subsidyAtoms),feesAtoms(o.feesAtoms),bits(o.bits),params(o.params),cancelled(o.cancelled.load()){} MiningJob& operator=(const MiningJob& o){ candidateBlock=o.candidateBlock; minerAddress=o.minerAddress; height=o.height; subsidyAtoms=o.subsidyAtoms; feesAtoms=o.feesAtoms; bits=o.bits; params=o.params; cancelled.store(o.cancelled.load()); return *this; } }; }
