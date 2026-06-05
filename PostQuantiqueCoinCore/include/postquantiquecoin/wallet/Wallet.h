#pragma once
#include "postquantiquecoin/blockchain/Blockchain.h"
#include "postquantiquecoin/crypto/SecureBuffer.h"
#include "postquantiquecoin/storage/WalletStorage.h"
#include "postquantiquecoin/wallet/WalletKey.h"
namespace pqc {
class Wallet {
public:
    Wallet(std::string name, std::filesystem::path path, PQCryptoProvider* crypto);
    static Result<Wallet> CreateNew(const std::string& name, const std::string& password, const std::filesystem::path& path, PQCryptoProvider* crypto);
    Result<void> Load(); Result<void> Save(); Result<void> Unlock(const std::string& password); Result<void> Unlock(const std::string& password, uint64_t timeoutSeconds); void Lock(); bool IsLocked() const;
    void MaybeAutoLock(); void SetAutoLockTimeout(uint64_t seconds);
    Result<std::string> GenerateNewAddress(const std::string& label); std::vector<std::string> ListAddresses() const; uint64_t GetBalance(Blockchain& chain) const;
    uint64_t GetConfirmedBalance(Blockchain& chain) const; uint64_t GetImmatureBalance(Blockchain& chain) const; uint64_t GetPendingIncomingBalance(Blockchain& chain) const; uint64_t GetPendingOutgoingBalance(Blockchain& chain) const; uint64_t GetAvailableBalance(Blockchain& chain) const; uint64_t GetTotalBalance(Blockchain& chain) const;
    Result<Transaction> CreateTransaction(const std::string& toAddress, uint64_t amountAtoms, uint64_t feeAtoms, Blockchain& chain); Result<void> SignTransaction(Transaction& tx); Result<void> ImportPrivateKey(const std::string& label,const std::vector<uint8_t>& pub,const std::vector<uint8_t>& priv); std::vector<std::vector<uint8_t>> ExportPublicKeys() const; Result<void> Backup(const std::filesystem::path& path) const; Result<void> Rescan(Blockchain& chain); const std::string& Name() const;
private:
    std::vector<uint8_t> SerializePlaintext() const; Result<void> DeserializePlaintext(const std::vector<uint8_t>& plaintext); Result<std::vector<uint8_t>> DecryptPrivate(const WalletKey& key) const; Result<void> EncryptAndStorePlaintext(const std::string& password);
    std::string name_; WalletStorage storage_; PQCryptoProvider* crypto_; bool locked_{true}; SecureBuffer masterKey_; std::vector<WalletKey> keys_; std::string unlockPassword_; uint64_t unlockedUntil{0}; uint64_t autoLockSeconds{300};
};
}
