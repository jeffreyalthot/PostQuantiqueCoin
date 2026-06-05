#include "postquantiquecoin/crypto/SecureRandom.h"
#include <random>
namespace pqc { std::vector<uint8_t> SecureRandom::Bytes(size_t n){ std::vector<uint8_t> out(n); std::random_device rd; for(size_t i=0;i<n;i++) out[i]=static_cast<uint8_t>(rd()); return out; } }
