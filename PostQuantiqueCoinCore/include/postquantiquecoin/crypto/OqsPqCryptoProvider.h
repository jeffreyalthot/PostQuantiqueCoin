#pragma once
#include "postquantiquecoin/crypto/PQCryptoProvider.h"
namespace pqc {
class OqsPqCryptoProvider final : public PQCryptoProvider {
public:
    PQKeyPair GenerateSigningKeyPair() override;
    std::vector<uint8_t> Sign(const std::vector<uint8_t>& privateKey, const std::vector<uint8_t>& message) override;
    bool Verify(const std::vector<uint8_t>& publicKey, const std::vector<uint8_t>& message, const std::vector<uint8_t>& signature) override;
    PQKemKeyPair GenerateKemKeyPair() override;
    KemEncapsulation Encapsulate(const std::vector<uint8_t>& publicKey) override;
    std::vector<uint8_t> Decapsulate(const std::vector<uint8_t>& privateKey, const std::vector<uint8_t>& ciphertext) override;
    std::string ProviderName() const override;
};
}
