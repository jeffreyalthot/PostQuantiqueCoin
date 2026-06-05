#pragma once
#include "postquantiquecoin/core/Result.h"
#include "postquantiquecoin/wallet/WalletDatFormat.h"
#include <string>
#include <vector>
namespace pqc {
class WalletEncryption {
public:
    static WalletEncryptedBlob EncryptPayload(const std::vector<uint8_t>& plaintext,
                                              const std::string& password,
                                              uint32_t iterations = 600000,
                                              const std::vector<uint8_t>& existingSalt = {});
    static Result<std::vector<uint8_t>> DecryptPayload(const WalletEncryptedBlob& blob,
                                                       const std::string& password,
                                                       uint32_t iterations = 600000);
    static Result<std::vector<uint8_t>> DerivePasswordKey(const std::string& password,
                                                          const std::vector<uint8_t>& salt,
                                                          uint32_t iterations,
                                                          size_t bytes = 32);
};
}
