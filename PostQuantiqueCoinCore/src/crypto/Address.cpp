#include "postquantiquecoin/crypto/Address.h"
#include "postquantiquecoin/crypto/PQCAddress.h"
#include <algorithm>
namespace pqc {
std::string Address::FromPublicKey(const std::vector<uint8_t>& publicKey) {
    return PQCAddress::FromPublicKey(publicKey, PQCNetworkId::Mainnet, AddressType::PublicKeyHashMLDSA, PQSignatureAlgorithmId::ML_DSA_65);
}
std::string Address::Encode(uint8_t version, const std::vector<uint8_t>& payload) {
    PQCAddressPayload p; p.formatVersion = version == 1 ? 2 : version; p.networkId=static_cast<uint8_t>(PQCNetworkId::Mainnet); p.addressType=static_cast<uint8_t>(AddressType::PublicKeyHashMLDSA); p.signatureAlgorithmId=static_cast<uint8_t>(PQSignatureAlgorithmId::ML_DSA_65);
    if (payload.size() >= p.publicKeyHash.size()) std::copy(payload.begin(), payload.begin() + static_cast<std::ptrdiff_t>(p.publicKeyHash.size()), p.publicKeyHash.begin()); else std::copy(payload.begin(), payload.end(), p.publicKeyHash.begin());
    return PQCAddress::Encode(p);
}
Result<DecodedAddress> Address::Decode(const std::string& address) {
    auto decoded = PQCAddress::Decode(address, static_cast<uint8_t>(PQCNetworkId::Mainnet));
    if (decoded.IsErr()) return Result<DecodedAddress>::Err(decoded.Error());
    if(!PQCAddress::IsSupportedType(decoded.Value().addressType) || !PQCAddress::IsAllowedSignatureAlgorithm(decoded.Value().signatureAlgorithmId)) return Result<DecodedAddress>::Err("unsupported post-quantum address parameters");
    std::vector<uint8_t> payload(decoded.Value().publicKeyHash.begin(), decoded.Value().publicKeyHash.end());
    return Result<DecodedAddress>::Ok({decoded.Value().formatVersion, payload});
}
bool Address::Validate(const std::string& address) { return Decode(address).IsOk(); }
}
