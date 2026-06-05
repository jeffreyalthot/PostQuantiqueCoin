#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "postquantiquecoin/core/Result.h"
namespace pqc {
class Serializer { public: void U32(uint32_t v); void U64(uint64_t v); void Bytes(const std::vector<uint8_t>& v); void String(const std::string& s); const std::vector<uint8_t>& Data() const; private: std::vector<uint8_t> data_; };
class Deserializer { public: explicit Deserializer(const std::vector<uint8_t>& data); Result<uint32_t> U32(); Result<uint64_t> U64(); Result<std::vector<uint8_t>> Bytes(); Result<std::string> String(); bool End() const; private: const std::vector<uint8_t>& data_; size_t pos_{0}; };
}
