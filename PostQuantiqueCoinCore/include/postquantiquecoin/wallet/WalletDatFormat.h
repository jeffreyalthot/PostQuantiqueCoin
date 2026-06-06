#pragma once
#include "postquantiquecoin/core/Result.h"
#include <array>
#include <cstdint>
#include <string>
#include <vector>
namespace pqc {
struct WalletDatHeader {
    std::string magic{"PQCWALLET"};
    uint32_t formatVersion{3};
    uint8_t networkId{1};
    uint8_t kdfAlgorithm{1};
    uint8_t aeadAlgorithm{1};
    uint8_t pqWrapAlgorithm{0};
    uint32_t saltSize{32};
    uint32_t nonceSize{12};
    uint32_t tagSize{16};
    uint64_t encryptedPayloadSize{0};
    std::array<uint8_t,32> checksum{};
};
struct WalletEncryptedBlob {
    uint32_t version{3};
    uint8_t networkId{1};
    uint8_t kdfAlgorithm{1};
    uint8_t aeadAlgorithm{1};
    uint8_t pqWrapAlgorithm{0};
    std::string kdfName{"PBKDF2-HMAC-SHA256-BCRYPT"};
    std::string aeadName{"AES-256-GCM"};
    std::string pqKemName{"ML-KEM-768"};
    uint32_t kdfIterations{600000};

    std::vector<uint8_t> encryptedPayload;
    std::vector<uint8_t> encryptedDekByPassword;
    std::vector<uint8_t> encryptedDekByKem;
    std::vector<uint8_t> pqKemCiphertext;
    std::vector<uint8_t> saltPassword;
    std::vector<uint8_t> noncePayload;
    std::vector<uint8_t> nonceDekPassword;
    std::vector<uint8_t> nonceDekKem;
    std::vector<uint8_t> tagPayload;
    std::vector<uint8_t> tagDekPassword;
    std::vector<uint8_t> tagDekKem;
    std::array<uint8_t,32> walletFingerprint{};
    std::array<uint8_t,32> kemPublicKeyFingerprint{};
    std::array<uint8_t,32> checksum{};
};
class WalletDatFormat {
public:
    static std::vector<uint8_t> Serialize(const WalletEncryptedBlob& blob);
    static Result<WalletEncryptedBlob> Deserialize(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> AssociatedData(const WalletEncryptedBlob& blob);
};
}
