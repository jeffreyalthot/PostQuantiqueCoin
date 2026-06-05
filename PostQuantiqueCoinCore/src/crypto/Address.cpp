#include "postquantiquecoin/crypto/Address.h"
#include "postquantiquecoin/crypto/PQCAddress.h"
#include <algorithm>

namespace pqc {
std::string Address::FromPublicKey(const std::vector<uint8_t>& publicKey) { return PQCAddress::FromPublicKey(publicKey); }

std::string Address::Encode(uint8_t version, const std::vector<uint8_t>& payload) {
    PQCAddressPayload p;
    p.formatVersion = version == 1 ? 2 : version;
    if (payload.size() >= p.publicKeyHash.size()) std::copy(payload.begin(), payload.begin() + static_cast<std::ptrdiff_t>(p.publicKeyHash.size()), p.publicKeyHash.begin());
    else std::copy(payload.begin(), payload.end(), p.publicKeyHash.begin());
    return PQCAddress::Encode(p);
}

Result<DecodedAddress> Address::Decode(const std::string& address) {
    auto decoded = PQCAddress::Decode(address);
    if (decoded.IsErr()) return Result<DecodedAddress>::Err(decoded.Error());
    std::vector<uint8_t> payload(decoded.Value().publicKeyHash.begin(), decoded.Value().publicKeyHash.end());
    return Result<DecodedAddress>::Ok({decoded.Value().formatVersion, payload});
}

bool Address::Validate(const std::string& address) { return PQCAddress::Validate(address); }
}
