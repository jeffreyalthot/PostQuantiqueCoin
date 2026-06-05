#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
namespace pqc { class WalletKdf { public: static std::vector<uint8_t> DeriveKey(const std::string& password,const std::vector<uint8_t>& salt,uint32_t iterations,size_t outputSize); }; }
