#include "postquantiquecoin/mining/MiningJob.h"
#include "postquantiquecoin/core/Serialization.h"
#include <chrono>

namespace pqc {
std::vector<uint8_t> MiningJob::BuildExtraNonce(uint32_t threadId, uint64_t sequence) const {
    Serializer s;
    s.String("PQC_COINBASE_EXTRANONCE_V1");
    s.U64(extraNonceBase);
    s.U32(threadId);
    s.U64(sequence);
    s.U64(static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count()));
    s.String(minerTag);
    return s.Data();
}
}
