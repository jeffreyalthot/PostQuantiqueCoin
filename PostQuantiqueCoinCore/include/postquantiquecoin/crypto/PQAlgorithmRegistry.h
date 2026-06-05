#pragma once
#include <cstddef>
#include <string>
namespace pqc { class PQAlgorithmRegistry { public: static bool IsSignatureAlgorithmAllowed(const std::string& algorithm); static bool IsKemAlgorithmAllowed(const std::string& algorithm); static size_t GetExpectedPublicKeySize(const std::string& algorithm); static size_t GetExpectedPrivateKeySize(const std::string& algorithm); static size_t GetExpectedSignatureSize(const std::string& algorithm); static size_t GetExpectedCiphertextSize(const std::string& algorithm); static bool IsProductionAllowed(const std::string& algorithm); }; }
