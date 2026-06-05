#include "postquantiquecoin/crypto/Address.h"
#include "postquantiquecoin/crypto/Hashing.h"
#include <algorithm>
#include <array>
#include <cctype>
namespace pqc {
namespace { constexpr char Alphabet[]="ABCDEFGHIJKLMNOPQRSTUVWXYZ234567"; int Val(char c){ c=static_cast<char>(std::toupper(static_cast<unsigned char>(c))); for(int i=0;i<32;i++) if(Alphabet[i]==c) return i; return -1; } std::string B32(const std::vector<uint8_t>& data){ std::string out; int bits=0; uint32_t acc=0; for(uint8_t b:data){ acc=(acc<<8)|b; bits+=8; while(bits>=5){ out.push_back(Alphabet[(acc>>(bits-5))&31]); bits-=5; } } if(bits>0) out.push_back(Alphabet[(acc<<(5-bits))&31]); return out; } Result<std::vector<uint8_t>> UnB32(const std::string& s){ std::vector<uint8_t> out; int bits=0; uint32_t acc=0; for(char c:s){ int v=Val(c); if(v<0) return Result<std::vector<uint8_t>>::Err("invalid address character"); acc=(acc<<5)|static_cast<uint32_t>(v); bits+=5; if(bits>=8){ out.push_back(static_cast<uint8_t>((acc>>(bits-8))&0xff)); bits-=8; } } return Result<std::vector<uint8_t>>::Ok(out); } }
std::string Address::FromPublicKey(const std::vector<uint8_t>& publicKey){ auto h=Hashing::Sha3_256(publicKey); std::vector<uint8_t> payload(h.begin(), h.begin()+20); return Encode(1,payload); }
std::string Address::Encode(uint8_t version,const std::vector<uint8_t>& payload){ std::vector<uint8_t> raw; raw.push_back(version); raw.insert(raw.end(),payload.begin(),payload.end()); auto chk=Hashing::Sha3_256(raw); raw.insert(raw.end(),chk.begin(),chk.begin()+4); return std::string("PQC1") + B32(raw); }
Result<DecodedAddress> Address::Decode(const std::string& address){ if(address.rfind("PQC1",0)!=0) return Result<DecodedAddress>::Err("bad prefix"); auto raw=UnB32(address.substr(4)); if(raw.IsErr()) return Result<DecodedAddress>::Err(raw.Error()); if(raw.Value().size()!=25) return Result<DecodedAddress>::Err("bad length"); std::vector<uint8_t> body(raw.Value().begin(),raw.Value().end()-4); auto chk=Hashing::Sha3_256(body); if(!std::equal(raw.Value().end()-4,raw.Value().end(),chk.begin())) return Result<DecodedAddress>::Err("bad checksum"); return Result<DecodedAddress>::Ok({body[0],std::vector<uint8_t>(body.begin()+1,body.end())}); }
bool Address::Validate(const std::string& address){ return Decode(address).IsOk(); }
}
