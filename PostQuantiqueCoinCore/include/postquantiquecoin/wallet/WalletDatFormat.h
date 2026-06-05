#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
namespace pqc {
struct WalletDatHeader { std::string magic{"PQCWALLET"}; uint32_t formatVersion{2}; uint8_t networkId{1}; uint8_t kdfAlgorithm{1}; uint8_t aeadAlgorithm{1}; uint8_t pqWrapAlgorithm{0}; uint32_t saltSize{32}; uint32_t nonceSize{12}; uint32_t tagSize{32}; uint64_t encryptedPayloadSize{0}; std::array<uint8_t,32> checksum{}; };
struct WalletEncryptedBlob { uint32_t version{2}; std::string kdfName{"PBKDF2-HMAC-SHA256"}; std::string aeadName{"PQC-SECRETBOX-KMAC256"}; std::string pqKemName{"ML-KEM-768"}; std::vector<uint8_t> salt; std::vector<uint8_t> nonce; std::vector<uint8_t> ciphertext; std::vector<uint8_t> authTag; std::vector<uint8_t> pqKemCiphertext; std::vector<uint8_t> encryptedDekByPassword; std::vector<uint8_t> encryptedDekByKem; std::array<uint8_t,32> checksum{}; };
}
