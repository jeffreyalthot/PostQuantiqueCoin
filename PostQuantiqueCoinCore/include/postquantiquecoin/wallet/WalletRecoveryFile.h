#pragma once
#include "postquantiquecoin/core/Result.h"
#include <array>
#include <cstdint>
#include <string>
#include <vector>
namespace pqc {
struct WalletRecoveryPublicFile {
    uint32_t version{1};
    uint8_t networkId{1};
    std::string kemAlgorithm{"ML-KEM-768"};
    std::vector<uint8_t> kemPublicKey;
    std::array<uint8_t,32> walletFingerprint{};
    std::array<uint8_t,32> checksum{};
    std::vector<uint8_t> Serialize() const;
    static Result<WalletRecoveryPublicFile> Deserialize(const std::vector<uint8_t>& data);
};
struct WalletRecoverySecretFile {
    uint32_t version{1};
    std::string kemAlgorithm{"ML-KEM-768"};
    std::vector<uint8_t> encryptedKemPrivateKey;
    std::vector<uint8_t> salt;
    std::vector<uint8_t> nonce;
    std::vector<uint8_t> tag;
    std::array<uint8_t,32> checksum{};
    std::vector<uint8_t> Serialize() const;
    static Result<WalletRecoverySecretFile> Deserialize(const std::vector<uint8_t>& data);
};
struct WalletRecoveryBundle {
    WalletRecoveryPublicFile publicFile;
    WalletRecoverySecretFile secretFile;
};
}
