#pragma once
#include "postquantiquecoin/core/Result.h"
#include "postquantiquecoin/crypto/PQCryptoProvider.h"
#include "postquantiquecoin/wallet/WalletDatFormat.h"
#include "postquantiquecoin/wallet/WalletRecoveryFile.h"
#include <string>
namespace pqc {
class WalletRecovery {
public:
    static WalletRecoveryBundle Create(PQCryptoProvider& provider,
                                       const std::vector<uint8_t>& walletFingerprint,
                                       const std::string& recoveryPassword,
                                       uint8_t networkId = 1,
                                       uint32_t iterations = 600000);
    static Result<std::vector<uint8_t>> DecryptKemPrivateKey(const WalletRecoverySecretFile& secretFile,
                                                             const std::string& recoveryPassword,
                                                             uint32_t iterations = 600000);
    static Result<std::vector<uint8_t>> RecoverPayload(PQCryptoProvider& provider,
                                                       const WalletEncryptedBlob& walletBlob,
                                                       const WalletRecoverySecretFile& secretFile,
                                                       const std::string& recoveryPassword);
    static bool Test(PQCryptoProvider& provider,
                     const WalletRecoveryPublicFile& publicFile,
                     const WalletRecoverySecretFile& secretFile,
                     const std::string& recoveryPassword,
                     uint32_t iterations = 600000);
};
}
