#pragma once
#include "postquantiquecoin/blockchain/Blockchain.h"
#include "postquantiquecoin/storage/WalletStorage.h"
#include "postquantiquecoin/wallet/WalletKey.h"
namespace pqc {
class Wallet {
public:
    Wallet(std::string name, std::filesystem::path path, PQCryptoProvider* crypto);
    static Result<Wallet> CreateNew(const std::string& name, const std::string& password, const std::filesystem::path& path, PQCryptoProvider* crypto);
    Result<void> Load(); Result<void> Save(); Result<void> Unlock(const std::string& password); void Lock(); bool IsLocked() const;
    Result<std::string> GenerateNewAddress(const std::string& label); std::vector<std::string> ListAddresses() const; uint64_t GetBalance(Blockchain& chain) const;
    Result<Transaction> CreateTransaction(const std::string& toAddress, uint64_t amountAtoms, uint64_t feeAtoms, Blockchain& chain); Result<void> SignTransaction(Transaction& tx); Result<void> ImportPrivateKey(const std::string& label,const std::vector<uint8_t>& pub,const std::vector<uint8_t>& priv); std::vector<std::vector<uint8_t>> ExportPublicKeys() const; Result<void> Backup(const std::filesystem::path& path) const; Result<void> Rescan(Blockchain& chain); const std::string& Name() const;
private:
    std::vector<uint8_t> DeriveKey(const std::string& password) const; std::vector<uint8_t> Crypt(const std::vector<uint8_t>& in,const std::vector<uint8_t>& key,const std::vector<uint8_t>& nonce) const; Result<std::vector<uint8_t>> DecryptPrivate(const WalletKey& key) const;
    std::string name_; WalletStorage storage_; PQCryptoProvider* crypto_; bool locked_{true}; std::vector<uint8_t> salt_; std::vector<uint8_t> masterKey_; std::vector<WalletKey> keys_;
};
}
