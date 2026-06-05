#include "postquantiquecoin/blockchain/BlockHeader.h"
#include "postquantiquecoin/core/Constants.h"
#include "postquantiquecoin/core/Hex.h"
#include "postquantiquecoin/core/Serialization.h"
#include "postquantiquecoin/crypto/Hashing.h"
namespace pqc { std::vector<uint8_t> BlockHeader::Serialize() const { Serializer s; s.String(constants::BLOCK_HEADER_DOMAIN); s.U32(version); s.U32(consensusVersion); s.U32(powAlgorithmId); s.String(previousHash); s.String(merkleRoot); s.U64(timestamp); s.U32(bits); s.U64(nonce); s.U64(height); return s.Data(); } std::array<uint8_t,32> BlockHeader::GetBlockIdBytes() const { return Hashing::DoubleSha256(Serialize()); } std::string BlockHeader::GetBlockId() const { auto h=GetBlockIdBytes(); return Hex::Encode(std::vector<uint8_t>(h.begin(),h.end())); } std::string BlockHeader::GetHash() const { return GetBlockId(); } }
