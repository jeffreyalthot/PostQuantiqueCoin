#include "postquantiquecoin/core/Serialization.h"
#include <cstring>
namespace pqc {
void Serializer::U32(uint32_t v){ for(int i=0;i<4;i++) data_.push_back(static_cast<uint8_t>((v>>(8*i))&0xff)); }
void Serializer::U64(uint64_t v){ for(int i=0;i<8;i++) data_.push_back(static_cast<uint8_t>((v>>(8*i))&0xff)); }
void Serializer::Bytes(const std::vector<uint8_t>& v){ U64(static_cast<uint64_t>(v.size())); data_.insert(data_.end(), v.begin(), v.end()); }
void Serializer::String(const std::string& s){ Bytes(std::vector<uint8_t>(s.begin(), s.end())); }
const std::vector<uint8_t>& Serializer::Data() const { return data_; }
Deserializer::Deserializer(const std::vector<uint8_t>& data):data_(data){}
Result<uint32_t> Deserializer::U32(){ if(pos_+4>data_.size()) return Result<uint32_t>::Err("unexpected end"); uint32_t v=0; for(int i=0;i<4;i++) v|=static_cast<uint32_t>(data_[pos_++])<<(8*i); return Result<uint32_t>::Ok(v); }
Result<uint64_t> Deserializer::U64(){ if(pos_+8>data_.size()) return Result<uint64_t>::Err("unexpected end"); uint64_t v=0; for(int i=0;i<8;i++) v|=static_cast<uint64_t>(data_[pos_++])<<(8*i); return Result<uint64_t>::Ok(v); }
Result<std::vector<uint8_t>> Deserializer::Bytes(){ auto n=U64(); if(n.IsErr()) return Result<std::vector<uint8_t>>::Err(n.Error()); if(n.Value()>data_.size()-pos_) return Result<std::vector<uint8_t>>::Err("length exceeds buffer"); std::vector<uint8_t> v(data_.begin()+static_cast<std::ptrdiff_t>(pos_), data_.begin()+static_cast<std::ptrdiff_t>(pos_+n.Value())); pos_+=static_cast<size_t>(n.Value()); return Result<std::vector<uint8_t>>::Ok(v); }
Result<std::string> Deserializer::String(){ auto b=Bytes(); if(b.IsErr()) return Result<std::string>::Err(b.Error()); return Result<std::string>::Ok(std::string(b.Value().begin(), b.Value().end())); }
bool Deserializer::End() const { return pos_==data_.size(); }
}
