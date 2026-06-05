#include "postquantiquecoin/mining/PQMemoryPow.h"
#include "postquantiquecoin/crypto/Hashing.h"
#include <algorithm>

namespace pqc {
namespace {
void AddString(std::vector<uint8_t>& v, const char* s) { while (*s) v.push_back(static_cast<uint8_t>(*s++)); }
void AddU64(std::vector<uint8_t>& v, uint64_t x) { for (int i = 0; i < 8; ++i) v.push_back(static_cast<uint8_t>((x >> (8 * i)) & 0xff)); }
uint64_t Read64(const std::array<uint8_t,64>& h) { uint64_t x = 0; for (int i = 0; i < 8; ++i) x |= static_cast<uint64_t>(h[i]) << (8 * i); return x; }
uint64_t ScratchBytesFor(const ChainParams& params) { return params.powScratchpadBytes ? params.powScratchpadBytes : (params.networkId == 1 ? 128ULL * 1024 * 1024 : (params.networkId == 2 ? 16ULL * 1024 * 1024 : 1ULL * 1024 * 1024)); }
uint32_t RoundsFor(const ChainParams& params) { return params.powMemoryRounds ? params.powMemoryRounds : (params.networkId == 1 ? 4096U : (params.networkId == 2 ? 1024U : 64U)); }
}

void PQMemoryPowContext::BuildScratchpad(const BlockHeader& headerWithoutNonce, const ChainParams& params) {
    params_ = params;
    headerTemplate_ = headerWithoutNonce;
    headerTemplate_.nonce = 0;
    serializedWithoutNonce_ = headerTemplate_.Serialize();

    std::vector<uint8_t> seedInput;
    AddString(seedInput, "PQC_MEMORY_POW_SEED_V2");
    seedInput.insert(seedInput.end(), serializedWithoutNonce_.begin(), serializedWithoutNonce_.end());
    auto seed = Hashing::Shake256(seedInput, 64);
    std::copy(seed.begin(), seed.end(), seed_.begin());

    uint64_t entries = std::max<uint64_t>(1, ScratchBytesFor(params_) / 64);
    scratch_.assign(static_cast<size_t>(entries), {});
    for (uint64_t i = 0; i < entries; ++i) {
        std::vector<uint8_t> in(seed_.begin(), seed_.end());
        AddU64(in, i);
        auto block = Hashing::Shake256(in, 64);
        std::copy(block.begin(), block.end(), scratch_[static_cast<size_t>(i)].begin());
    }
}

std::array<uint8_t,32> PQMemoryPowContext::ComputeWithNonce(uint64_t nonce) const {
    BlockHeader h = headerTemplate_;
    h.nonce = nonce;
    auto serialized = h.Serialize();
    std::array<uint8_t,64> mix = seed_;
    const uint64_t entries = std::max<uint64_t>(1, scratch_.size());
    const uint32_t rounds = RoundsFor(params_);
    for (uint32_t r = 0; r < rounds; ++r) {
        std::vector<uint8_t> idxIn(mix.begin(), mix.end());
        AddU64(idxIn, r);
        idxIn.insert(idxIn.end(), serialized.begin(), serialized.end());
        auto idxHash = Hashing::Sha3_512(idxIn);
        uint64_t idx = Read64(idxHash) % entries;
        std::vector<uint8_t> mixIn(mix.begin(), mix.end());
        mixIn.insert(mixIn.end(), scratch_[static_cast<size_t>(idx)].begin(), scratch_[static_cast<size_t>(idx)].end());
        mixIn.insert(mixIn.end(), serialized.begin(), serialized.end());
        mix = Hashing::Sha3_512(mixIn);
    }
    std::vector<uint8_t> finalInput;
    AddString(finalInput, "PQC_MEMORY_POW_FINAL_V2");
    finalInput.insert(finalInput.end(), mix.begin(), mix.end());
    auto final = Hashing::Shake256(finalInput, 32);
    std::array<uint8_t,32> out{};
    std::copy(final.begin(), final.end(), out.begin());
    return out;
}

std::array<uint8_t,32> PQMemoryPow::ComputeMemoryHardV2(const BlockHeader& header, const ChainParams& params) {
    PQMemoryPowContext context;
    BlockHeader templateHeader = header;
    templateHeader.nonce = 0;
    context.BuildScratchpad(templateHeader, params);
    return context.ComputeWithNonce(header.nonce);
}
}
