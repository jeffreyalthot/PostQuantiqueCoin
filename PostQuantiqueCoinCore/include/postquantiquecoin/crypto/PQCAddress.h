#pragma once
#include "postquantiquecoin/core/Result.h"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace pqc {
enum class AddressType : uint8_t {
    PublicKeyHashMLDSA = 1,
    PublicKeyHashSLHDSA = 2,
    MultisigPostQuantum = 3,
    ScriptHashPostQuantum = 4
};

enum class PQSignatureAlgorithmId : uint8_t {
    ML_DSA_44 = 1,
    ML_DSA_65 = 2,
    ML_DSA_87 = 3,
    SLH_DSA_SHA2_128S = 10,
    SLH_DSA_SHAKE_128S = 11,
    SLH_DSA_SHA2_192S = 12,
    SLH_DSA_SHAKE_192S = 13,
    SLH_DSA_SHA2_256S = 14,
    SLH_DSA_SHAKE_256S = 15
};

enum class PQCNetworkId : uint8_t {
    Mainnet = 1,
    Devnet = 2,
    Regtest = 3
};

struct PQCAddressPayload {
    uint8_t formatVersion{2};
    uint8_t networkId{static_cast<uint8_t>(PQCNetworkId::Mainnet)};
    uint8_t addressType{static_cast<uint8_t>(AddressType::PublicKeyHashMLDSA)};
    uint8_t signatureAlgorithmId{static_cast<uint8_t>(PQSignatureAlgorithmId::ML_DSA_65)};
    std::array<uint8_t, 32> publicKeyHash{};
    std::array<uint8_t, 6> checksum{};
};

class PQCAddress {
public:
    static constexpr const char* Prefix = "PQC1";
    static constexpr size_t SerializedSize = 42;
    static std::array<uint8_t, 32> HashPublicKey(const std::vector<uint8_t>& publicKey);
    static std::string Encode(const PQCAddressPayload& payloadWithoutChecksum);
    static Result<PQCAddressPayload> Decode(const std::string& address, uint8_t expectedNetworkId = 0);
    static bool Validate(const std::string& address, uint8_t expectedNetworkId = 0);
    static std::string FromPublicKey(const std::vector<uint8_t>& publicKey,
                                     PQCNetworkId network = PQCNetworkId::Mainnet,
                                     AddressType type = AddressType::PublicKeyHashMLDSA,
                                     PQSignatureAlgorithmId algorithm = PQSignatureAlgorithmId::ML_DSA_65);
    static bool IsSupportedType(uint8_t type);
    static bool IsAllowedSignatureAlgorithm(uint8_t algorithm);
};
}
