#pragma once
#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>
namespace pqc {
struct PQKeyPair { std::vector<uint8_t> publicKey; std::vector<uint8_t> privateKey; std::string algorithm; };
struct PQKemKeyPair { std::vector<uint8_t> publicKey; std::vector<uint8_t> privateKey; std::string algorithm; };
struct KemEncapsulation { std::vector<uint8_t> ciphertext; std::vector<uint8_t> sharedSecret; };
struct PQAlgorithmInfo {
    std::string name;
    size_t publicKeySize{0};
    size_t privateKeySize{0};
    size_t signatureSize{0};
    size_t ciphertextSize{0};
    size_t sharedSecretSize{0};
    bool productionAllowed{false};
};
class PQCryptoProvider {
public:
    virtual ~PQCryptoProvider() = default;
    virtual PQKeyPair GenerateSigningKeyPair() = 0;
    virtual std::vector<uint8_t> Sign(const std::vector<uint8_t>& privateKey, const std::vector<uint8_t>& message) = 0;
    virtual bool Verify(const std::vector<uint8_t>& publicKey, const std::vector<uint8_t>& message, const std::vector<uint8_t>& signature) = 0;
    virtual PQKemKeyPair GenerateKemKeyPair() = 0;
    virtual KemEncapsulation Encapsulate(const std::vector<uint8_t>& publicKey) = 0;
    virtual std::vector<uint8_t> Decapsulate(const std::vector<uint8_t>& privateKey, const std::vector<uint8_t>& ciphertext) = 0;
    virtual PQAlgorithmInfo GetSigningAlgorithmInfo() const = 0;
    virtual PQAlgorithmInfo GetKemAlgorithmInfo() const = 0;
    virtual bool IsSigningAlgorithmAllowed(const std::string& algorithm) const = 0;
    virtual bool IsKemAlgorithmAllowed(const std::string& algorithm) const = 0;
    virtual std::string ProviderName() const = 0;
};
std::unique_ptr<PQCryptoProvider> CreateDefaultCryptoProvider();
}
