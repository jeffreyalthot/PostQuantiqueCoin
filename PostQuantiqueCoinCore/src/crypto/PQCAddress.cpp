#include "postquantiquecoin/crypto/PQCAddress.h"
#include "postquantiquecoin/crypto/Hashing.h"
#include <algorithm>
#include <array>
#include <cctype>

namespace pqc {
namespace {
constexpr char Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

int Val(char c) {
    for (int i = 0; i < 32; ++i) if (Alphabet[i] == c) return i;
    return -1;
}

std::string Base32Encode(const std::vector<uint8_t>& data) {
    std::string out;
    int bits = 0;
    uint32_t acc = 0;
    for (uint8_t b : data) {
        acc = (acc << 8) | b;
        bits += 8;
        while (bits >= 5) {
            out.push_back(Alphabet[(acc >> (bits - 5)) & 31]);
            bits -= 5;
        }
    }
    if (bits > 0) out.push_back(Alphabet[(acc << (5 - bits)) & 31]);
    return out;
}

Result<std::vector<uint8_t>> Base32DecodeStrict(const std::string& s) {
    std::vector<uint8_t> out;
    int bits = 0;
    uint32_t acc = 0;
    for (char c : s) {
        if (std::islower(static_cast<unsigned char>(c))) return Result<std::vector<uint8_t>>::Err("address must be uppercase");
        int v = Val(c);
        if (v < 0) return Result<std::vector<uint8_t>>::Err("invalid address character");
        acc = (acc << 5) | static_cast<uint32_t>(v);
        bits += 5;
        if (bits >= 8) {
            out.push_back(static_cast<uint8_t>((acc >> (bits - 8)) & 0xff));
            bits -= 8;
        }
    }
    if (bits > 0 && ((acc << (8 - bits)) & 0xffU) != 0) return Result<std::vector<uint8_t>>::Err("non-zero base32 padding bits");
    return Result<std::vector<uint8_t>>::Ok(out);
}

std::vector<uint8_t> BodyBytes(const PQCAddressPayload& p) {
    std::vector<uint8_t> body;
    body.reserve(36);
    body.push_back(p.formatVersion);
    body.push_back(p.networkId);
    body.push_back(p.addressType);
    body.push_back(p.signatureAlgorithmId);
    body.insert(body.end(), p.publicKeyHash.begin(), p.publicKeyHash.end());
    return body;
}

std::array<uint8_t, 6> Checksum(const std::vector<uint8_t>& body) {
    std::vector<uint8_t> domain{'P','Q','C','_','A','D','D','R','E','S','S','_','V','2'};
    domain.insert(domain.end(), body.begin(), body.end());
    auto h = Hashing::Sha3_256(domain);
    std::array<uint8_t, 6> out{};
    std::copy(h.begin(), h.begin() + 6, out.begin());
    return out;
}
}

std::array<uint8_t, 32> PQCAddress::HashPublicKey(const std::vector<uint8_t>& publicKey) {
    std::vector<uint8_t> domain{'P','Q','C','_','P','U','B','K','E','Y','_','H','A','S','H','_','V','2'};
    domain.insert(domain.end(), publicKey.begin(), publicKey.end());
    return Hashing::Sha3_256(domain);
}

bool PQCAddress::IsSupportedType(uint8_t type) {
    return type == static_cast<uint8_t>(AddressType::PublicKeyHashMLDSA) ||
           type == static_cast<uint8_t>(AddressType::PublicKeyHashSLHDSA) ||
           type == static_cast<uint8_t>(AddressType::MultisigPostQuantum) ||
           type == static_cast<uint8_t>(AddressType::ScriptHashPostQuantum);
}

bool PQCAddress::IsAllowedSignatureAlgorithm(uint8_t algorithm) {
    switch (static_cast<PQSignatureAlgorithmId>(algorithm)) {
        case PQSignatureAlgorithmId::ML_DSA_44:
        case PQSignatureAlgorithmId::ML_DSA_65:
        case PQSignatureAlgorithmId::ML_DSA_87:
        case PQSignatureAlgorithmId::SLH_DSA_SHA2_128S:
        case PQSignatureAlgorithmId::SLH_DSA_SHAKE_128S:
        case PQSignatureAlgorithmId::SLH_DSA_SHA2_192S:
        case PQSignatureAlgorithmId::SLH_DSA_SHAKE_192S:
        case PQSignatureAlgorithmId::SLH_DSA_SHA2_256S:
        case PQSignatureAlgorithmId::SLH_DSA_SHAKE_256S:
            return true;
    }
    return false;
}

std::string PQCAddress::Encode(const PQCAddressPayload& payloadWithoutChecksum) {
    auto body = BodyBytes(payloadWithoutChecksum);
    auto checksum = Checksum(body);
    body.insert(body.end(), checksum.begin(), checksum.end());
    return std::string(Prefix) + Base32Encode(body);
}

Result<PQCAddressPayload> PQCAddress::Decode(const std::string& address, uint8_t expectedNetworkId) {
    if (address.rfind(Prefix, 0) != 0) return Result<PQCAddressPayload>::Err("bad prefix");
    if (address.substr(0, 4) != Prefix) return Result<PQCAddressPayload>::Err("address prefix must be uppercase PQC1");
    auto raw = Base32DecodeStrict(address.substr(4));
    if (raw.IsErr()) return Result<PQCAddressPayload>::Err(raw.Error());
    if (raw.Value().size() != SerializedSize) return Result<PQCAddressPayload>::Err("bad length");

    PQCAddressPayload p;
    p.formatVersion = raw.Value()[0];
    p.networkId = raw.Value()[1];
    p.addressType = raw.Value()[2];
    p.signatureAlgorithmId = raw.Value()[3];
    std::copy(raw.Value().begin() + 4, raw.Value().begin() + 36, p.publicKeyHash.begin());
    std::copy(raw.Value().begin() + 36, raw.Value().end(), p.checksum.begin());

    if (p.formatVersion != 2) return Result<PQCAddressPayload>::Err("unsupported address version");
    if (expectedNetworkId != 0 && p.networkId != expectedNetworkId) return Result<PQCAddressPayload>::Err("wrong network");
    if (p.networkId != static_cast<uint8_t>(PQCNetworkId::Mainnet) && p.networkId != static_cast<uint8_t>(PQCNetworkId::Devnet) && p.networkId != static_cast<uint8_t>(PQCNetworkId::Regtest)) return Result<PQCAddressPayload>::Err("unsupported network");
    if (!IsSupportedType(p.addressType)) return Result<PQCAddressPayload>::Err("unsupported address type");
    if (!IsAllowedSignatureAlgorithm(p.signatureAlgorithmId)) return Result<PQCAddressPayload>::Err("unsupported signature algorithm");

    std::vector<uint8_t> body(raw.Value().begin(), raw.Value().begin() + 36);
    auto expected = Checksum(body);
    if (!std::equal(expected.begin(), expected.end(), p.checksum.begin())) return Result<PQCAddressPayload>::Err("bad checksum");
    return Result<PQCAddressPayload>::Ok(p);
}

bool PQCAddress::Validate(const std::string& address, uint8_t expectedNetworkId) { return Decode(address, expectedNetworkId).IsOk(); }

std::string PQCAddress::FromPublicKey(const std::vector<uint8_t>& publicKey, PQCNetworkId network, AddressType type, PQSignatureAlgorithmId algorithm) {
    PQCAddressPayload p;
    p.networkId = static_cast<uint8_t>(network);
    p.addressType = static_cast<uint8_t>(type);
    p.signatureAlgorithmId = static_cast<uint8_t>(algorithm);
    p.publicKeyHash = HashPublicKey(publicKey);
    return Encode(p);
}
}
