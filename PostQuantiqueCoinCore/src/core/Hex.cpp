#include "postquantiquecoin/core/Hex.h"
#include <array>
#include <cctype>
namespace pqc {
std::string Hex::Encode(const std::vector<uint8_t>& data) { static constexpr char h[]="0123456789abcdef"; std::string out; out.reserve(data.size()*2); for(uint8_t b:data){ out.push_back(h[b>>4]); out.push_back(h[b&15]); } return out; }
Result<std::vector<uint8_t>> Hex::Decode(const std::string& hex) { if(hex.size()%2) return Result<std::vector<uint8_t>>::Err("odd hex length"); if(!IsHex(hex)) return Result<std::vector<uint8_t>>::Err("non-hex character"); auto val=[](char c)->uint8_t{ if(c>='0'&&c<='9') return c-'0'; c=static_cast<char>(std::tolower(static_cast<unsigned char>(c))); return static_cast<uint8_t>(10+c-'a');}; std::vector<uint8_t> out; out.reserve(hex.size()/2); for(size_t i=0;i<hex.size();i+=2) out.push_back(static_cast<uint8_t>((val(hex[i])<<4)|val(hex[i+1]))); return Result<std::vector<uint8_t>>::Ok(out); }
bool Hex::IsHex(const std::string& hex) { for(char c:hex) if(!std::isxdigit(static_cast<unsigned char>(c))) return false; return true; }
}
