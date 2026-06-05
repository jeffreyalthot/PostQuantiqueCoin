#include "postquantiquecoin/wallet/WalletKeyRing.h"
#include "postquantiquecoin/crypto/PQCAddress.h"

namespace pqc {
WalletKeyRing::WalletKeyRing(const std::vector<WalletKey>& keys) : keys_(keys) {}

const WalletKey* WalletKeyRing::FindKeyByAddress(const std::string& address) const {
    for (const auto& key : keys_) if (key.address == address) return &key;
    return nullptr;
}

const WalletKey* WalletKeyRing::FindKeyByPublicKeyHash(const std::array<uint8_t,32>& publicKeyHash) const {
    for (const auto& key : keys_) {
        auto decoded = PQCAddress::Decode(key.address);
        if (decoded.IsOk() && decoded.Value().publicKeyHash == publicKeyHash) return &key;
        if (PQCAddress::HashPublicKey(key.EffectivePublicKey()) == publicKeyHash) return &key;
    }
    return nullptr;
}

const WalletKey* WalletKeyRing::FindKeyByFingerprint(const std::array<uint8_t,32>& fingerprint) const {
    for (const auto& key : keys_) if (key.keyFingerprint == fingerprint) return &key;
    return nullptr;
}

bool WalletKeyRing::HasPrivateKeyForAddress(const std::string& address) const {
    return FindKeyByAddress(address) != nullptr;
}

Result<std::vector<uint8_t>> WalletKeyRing::GetSigningPublicKey(const std::string& address) const {
    const auto* key = FindKeyByAddress(address);
    if (!key) return Result<std::vector<uint8_t>>::Err("no wallet key for address");
    return Result<std::vector<uint8_t>>::Ok(key->EffectivePublicKey());
}

Result<std::vector<uint8_t>> WalletKeyRing::GetKemPublicKey(const std::string& address) const {
    const auto* key = FindKeyByAddress(address);
    if (!key) return Result<std::vector<uint8_t>>::Err("no wallet key for address");
    return Result<std::vector<uint8_t>>::Ok(key->kemPublicKey);
}

Result<std::vector<uint8_t>> WalletKeyRing::UnlockPrivateRecord(const WalletKey& key, const PrivateKeyUnlocker& unlocker) const {
    return unlocker(key);
}

Result<void> WalletKeyRing::SignDigestForAddress(const std::string& address,
                                                 const std::vector<uint8_t>& digest,
                                                 PQCryptoProvider& provider,
                                                 const PrivateKeyUnlocker& unlocker,
                                                 std::vector<uint8_t>& publicKeyOut,
                                                 std::vector<uint8_t>& signatureOut) const {
    const auto* key = FindKeyByAddress(address);
    if (!key) return Result<void>::Err("no wallet key for address");
    auto privateKey = unlocker(*key);
    if (privateKey.IsErr()) return Result<void>::Err(privateKey.Error());
    publicKeyOut = key->EffectivePublicKey();
    signatureOut = provider.Sign(privateKey.Value(), digest);
    return Result<void>::Ok();
}
}
