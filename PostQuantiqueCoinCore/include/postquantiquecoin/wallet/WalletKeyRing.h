#pragma once
#include "postquantiquecoin/blockchain/TxOutput.h"
#include "postquantiquecoin/core/Result.h"
#include "postquantiquecoin/crypto/PQCryptoProvider.h"
#include "postquantiquecoin/wallet/WalletKey.h"
#include <array>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace pqc {
class WalletKeyRing {
public:
    using PrivateKeyUnlocker = std::function<Result<std::vector<uint8_t>>(const WalletKey&)>;

    explicit WalletKeyRing(const std::vector<WalletKey>& keys);

    const WalletKey* FindKeyByAddress(const std::string& address) const;
    const WalletKey* FindKeyByPublicKeyHash(const std::array<uint8_t,32>& publicKeyHash) const;
    const WalletKey* FindKeyByFingerprint(const std::array<uint8_t,32>& fingerprint) const;
    bool HasPrivateKeyForAddress(const std::string& address) const;
    Result<std::vector<uint8_t>> GetSigningPublicKey(const std::string& address) const;
    Result<std::vector<uint8_t>> GetKemPublicKey(const std::string& address) const;
    Result<std::vector<uint8_t>> UnlockPrivateRecord(const WalletKey& key, const PrivateKeyUnlocker& unlocker) const;
    Result<void> SignDigestForAddress(const std::string& address,
                                      const std::vector<uint8_t>& digest,
                                      PQCryptoProvider& provider,
                                      const PrivateKeyUnlocker& unlocker,
                                      std::vector<uint8_t>& publicKeyOut,
                                      std::vector<uint8_t>& signatureOut) const;
private:
    const std::vector<WalletKey>& keys_;
};
}
