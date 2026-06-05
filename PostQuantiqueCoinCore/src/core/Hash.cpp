#include "postquantiquecoin/core/Hash.h"
#include "postquantiquecoin/core/Hex.h"
#include <algorithm>

namespace pqc {
std::string Hash::Zero() { return std::string(64, '0'); }
bool Hash::IsValidHex256(const std::string& hex) { return hex.size() == 64 && Hex::IsHex(hex); }
std::string Hash::FromBytes(const Hash256Bytes& bytes) { return Hex::Encode(std::vector<uint8_t>(bytes.begin(), bytes.end())); }
Hash256Bytes Hash::ToBytes(const std::string& hex) { auto v = Hex::Decode(hex).Value(); Hash256Bytes out{}; std::copy_n(v.begin(), std::min<size_t>(32, v.size()), out.begin()); return out; }
}
