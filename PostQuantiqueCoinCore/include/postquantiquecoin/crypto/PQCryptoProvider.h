#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
namespace pqc {
struct PQKeyPair { std::vector<uint8_t> publicKey; std::vector<uint8_t> privateKey; std::string algorithm; };
struct PQKemKeyPair { std::vector<uint8_t> publicKey; std::vector<uint8_t> privateKey; std::string algorithm; };
struct KemEncapsulation { std::vector<uint8_t> ciphertext; std::vector<uint8_t> sharedSecret; };
class PQCryptoProvider {
public:
    virtual ~PQCryptoProvider() = default;
    virtual PQKeyPair GenerateSigningKeyPair() = 0;
    virtual std::vector<uint8_t> Sign(const std::vector<uint8_t>& privateKey, const std::vector<uint8_t>& message) = 0;
    virtual bool Verify(const std::vector<uint8_t>& publicKey, const std::vector<uint8_t>& message, const std::vector<uint8_t>& signature) = 0;
    virtual PQKemKeyPair GenerateKemKeyPair() = 0;
    virtual KemEncapsulation Encapsulate(const std::vector<uint8_t>& publicKey) = 0;
    virtual std::vector<uint8_t> Decapsulate(const std::vector<uint8_t>& privateKey, const std::vector<uint8_t>& ciphertext) = 0;
    virtual std::string ProviderName() const = 0;
};
std::unique_ptr<PQCryptoProvider> CreateDefaultCryptoProvider();
}
