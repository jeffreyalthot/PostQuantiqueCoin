#include "postquantiquecoin/blockchain/Difficulty.h"
#include "postquantiquecoin/core/Hex.h"
#include <algorithm>
#include <array>
namespace pqc {
std::vector<uint8_t> Difficulty::CompactToTarget(uint32_t bits){ uint32_t exponent=bits>>24; uint32_t mantissa=bits&0x007fffffU; std::vector<uint8_t> target(32,0); if(mantissa==0) return target; if(exponent<=3){ mantissa >>= 8*(3-exponent); target[31]=static_cast<uint8_t>(mantissa&0xff); if(exponent>=2) target[30]=static_cast<uint8_t>((mantissa>>8)&0xff); if(exponent>=3) target[29]=static_cast<uint8_t>((mantissa>>16)&0xff); } else { int idx=32-static_cast<int>(exponent); if(idx>=0&&idx<32) target[idx]=static_cast<uint8_t>((mantissa>>16)&0xff); if(idx+1>=0&&idx+1<32) target[idx+1]=static_cast<uint8_t>((mantissa>>8)&0xff); if(idx+2>=0&&idx+2<32) target[idx+2]=static_cast<uint8_t>(mantissa&0xff); } return target; }
uint32_t Difficulty::TargetToCompact(const std::vector<uint8_t>& target){ std::vector<uint8_t> t(32,0); if(target.size()>=32) std::copy(target.end()-32,target.end(),t.begin()); else std::copy(target.begin(),target.end(),t.begin()+(32-target.size())); size_t first=0; while(first<t.size()&&t[first]==0) ++first; if(first==t.size()) return 0; uint32_t exponent=static_cast<uint32_t>(32-first); uint32_t mantissa=0; if(exponent<=3){ for(uint32_t i=0;i<exponent;i++) mantissa=(mantissa<<8)|t[first+i]; mantissa <<= 8*(3-exponent); } else { mantissa=(static_cast<uint32_t>(t[first])<<16); if(first+1<t.size()) mantissa|=static_cast<uint32_t>(t[first+1])<<8; if(first+2<t.size()) mantissa|=t[first+2]; } if(mantissa&0x00800000U){ mantissa >>= 8; ++exponent; } return (exponent<<24)|(mantissa&0x007fffffU); }
int Difficulty::CompareHashToTarget(const std::string& hashHex,const std::vector<uint8_t>& target){ auto h=Hex::Decode(hashHex); if(h.IsErr()||h.Value().size()!=32) return 1; std::array<uint8_t,32> a{}; std::copy(h.Value().begin(),h.Value().end(),a.begin()); return CompareHashToTarget(a,target); }
int Difficulty::CompareHashToTarget(const std::array<uint8_t,32>& hash,const std::vector<uint8_t>& target){ std::vector<uint8_t> h(hash.begin(),hash.end()); std::vector<uint8_t> t(32,0); if(target.size()>=32) std::copy(target.end()-32,target.end(),t.begin()); else std::copy(target.begin(),target.end(),t.begin()+(32-target.size())); if(h<t) return -1; if(h>t) return 1; return 0; }
bool Difficulty::CheckProofOfWork(const std::string& hashHex,uint32_t bits){ return CompareHashToTarget(hashHex, CompactToTarget(bits))<=0; }
bool Difficulty::CheckProofOfWorkHash(const std::array<uint8_t,32>& powHash,uint32_t bits){ return CompareHashToTarget(powHash, CompactToTarget(bits))<=0; }
uint64_t Difficulty::ClampAdjustment(uint64_t actual,uint64_t expected){ uint64_t min=std::max<uint64_t>(1, expected/4); uint64_t max=expected*4; return std::min(max,std::max(min,actual)); }
namespace {
std::vector<uint8_t> MulDivTarget(std::vector<uint8_t> target, uint64_t mul, uint64_t div) {
    if (target.size() < 32) target.insert(target.begin(), 32 - target.size(), 0);
    if (target.size() > 32) target = std::vector<uint8_t>(target.end() - 32, target.end());
    std::vector<uint8_t> product(40, 0);
    uint64_t carry = 0;
    for (int i = 31; i >= 0; --i) {
        uint64_t v = static_cast<uint64_t>(target[i]) * mul + carry;
        product[i + 8] = static_cast<uint8_t>(v & 0xff);
        carry = v >> 8;
    }
    for (int i = 7; i >= 0; --i) { product[i] = static_cast<uint8_t>(carry & 0xff); carry >>= 8; }
    std::vector<uint8_t> quotient(40, 0);
    uint64_t rem = 0;
    for (size_t i = 0; i < product.size(); ++i) {
        rem = (rem << 8) | product[i];
        quotient[i] = static_cast<uint8_t>(rem / div);
        rem %= div;
    }
    return std::vector<uint8_t>(quotient.end() - 32, quotient.end());
}
}
uint32_t Difficulty::CalculateNextWorkRequired(const std::vector<Block>& blocks,const ChainParams& params){ if(blocks.empty()) return params.initialBits; const Block& last=blocks.back(); if(params.difficultyAdjustmentInterval==0 || (last.header.height+1)%params.difficultyAdjustmentInterval!=0) return last.header.bits; if(blocks.size()<params.difficultyAdjustmentInterval) return last.header.bits; uint64_t expected=params.targetBlockTimeSeconds*params.difficultyAdjustmentInterval; const Block& first=blocks[blocks.size()-params.difficultyAdjustmentInterval]; uint64_t actual=last.header.timestamp>first.header.timestamp?last.header.timestamp-first.header.timestamp:1; actual=ClampAdjustment(actual, expected); auto oldTarget=CompactToTarget(last.header.bits); auto newTarget=MulDivTarget(oldTarget,actual,expected==0?1:expected); auto powLimit=CompactToTarget(params.powLimitBits); if(newTarget>powLimit) newTarget=powLimit; return TargetToCompact(newTarget); }
}
