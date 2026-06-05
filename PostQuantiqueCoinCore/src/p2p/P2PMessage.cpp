#include "postquantiquecoin/p2p/P2PMessage.h"
#include "postquantiquecoin/crypto/Hashing.h"
#include "postquantiquecoin/p2p/Protocol.h"
#include <algorithm>
#include <array>
#include <cstring>

namespace pqc::p2p {
namespace {
void PutU32(std::vector<uint8_t>& out, uint32_t v) {
    for (int i = 0; i < 4; ++i) out.push_back(static_cast<uint8_t>((v >> (8 * i)) & 0xff));
}
uint32_t ReadU32(const std::vector<uint8_t>& data, size_t pos) {
    return static_cast<uint32_t>(data[pos]) |
           (static_cast<uint32_t>(data[pos + 1]) << 8) |
           (static_cast<uint32_t>(data[pos + 2]) << 16) |
           (static_cast<uint32_t>(data[pos + 3]) << 24);
}
std::array<uint8_t,12> CommandBytes(const std::string& command) {
    std::array<uint8_t,12> out{};
    std::copy(command.begin(), command.end(), out.begin());
    return out;
}
std::string CommandString(const uint8_t* command) {
    size_t len = 0;
    while (len < 12 && command[len] != 0) ++len;
    for (size_t i = len; i < 12; ++i) if (command[i] != 0) return {};
    return std::string(reinterpret_cast<const char*>(command), len);
}
}

std::vector<uint8_t> P2PMessage::Serialize() const {
    if (command.size() > 12) return {};
    if (payload.size() > MaxPayloadBytes) return {};
    std::vector<uint8_t> out;
    out.reserve(4 + 1 + 12 + 4 + 8 + payload.size());
    PutU32(out, magic);
    out.push_back(networkId);
    auto cmd = CommandBytes(command);
    out.insert(out.end(), cmd.begin(), cmd.end());
    PutU32(out, static_cast<uint32_t>(payload.size()));
    auto h = pqc::Hashing::Sha3_256(payload);
    out.insert(out.end(), h.begin(), h.begin() + 8);
    out.insert(out.end(), payload.begin(), payload.end());
    return out;
}

Result<P2PMessage> P2PMessage::Deserialize(const std::vector<uint8_t>& data, uint8_t expected) {
    constexpr size_t headerSize = 4 + 1 + 12 + 4 + 8;
    if (data.size() < headerSize) return Result<P2PMessage>::Err("short p2p message");
    P2PMessage m;
    m.magic = ReadU32(data, 0);
    if (m.magic != Magic) return Result<P2PMessage>::Err("bad p2p magic");
    m.networkId = data[4];
    if (expected && m.networkId != expected) return Result<P2PMessage>::Err("wrong network id");
    m.command = CommandString(data.data() + 5);
    if (m.command.empty()) return Result<P2PMessage>::Err("bad command encoding");
    if (m.command.size() > 12) return Result<P2PMessage>::Err("command too long");
    if (std::find(KnownCommands().begin(), KnownCommands().end(), m.command) == KnownCommands().end()) return Result<P2PMessage>::Err("unknown command");
    uint32_t payloadSize = ReadU32(data, 17);
    if (payloadSize > MaxPayloadBytes) return Result<P2PMessage>::Err("oversized payload");
    if (data.size() != headerSize + payloadSize) return Result<P2PMessage>::Err("trailing or truncated p2p message bytes");
    std::array<uint8_t,8> checksum{};
    std::copy(data.begin() + 21, data.begin() + 29, checksum.begin());
    m.payload.assign(data.begin() + headerSize, data.end());
    auto h = pqc::Hashing::Sha3_256(m.payload);
    if (!std::equal(checksum.begin(), checksum.end(), h.begin())) return Result<P2PMessage>::Err("message checksum mismatch");
    return Result<P2PMessage>::Ok(std::move(m));
}
}
