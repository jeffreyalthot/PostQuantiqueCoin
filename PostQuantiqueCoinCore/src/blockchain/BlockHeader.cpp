#include "postquantiquecoin/blockchain/BlockHeader.h"
#include "postquantiquecoin/core/Constants.h"
#include "postquantiquecoin/core/Serialization.h"
#include "postquantiquecoin/crypto/Hashing.h"
namespace pqc { std::vector<uint8_t> BlockHeader::Serialize() const { Serializer s; s.String(constants::BLOCK_HEADER_DOMAIN); s.U32(version); s.String(previousHash); s.String(merkleRoot); s.U64(timestamp); s.U32(bits); s.U64(nonce); s.U64(height); return s.Data(); } std::string BlockHeader::GetHash() const { return Hashing::Hash256Hex(Serialize()); } }
