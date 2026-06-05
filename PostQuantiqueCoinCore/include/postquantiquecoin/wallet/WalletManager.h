#pragma once
#include "postquantiquecoin/wallet/Wallet.h"
#include <map>
#include <memory>
namespace pqc { class WalletManager { public: WalletManager(std::filesystem::path walletDir, PQCryptoProvider* crypto); Result<void> CreateWallet(const std::string& name,const std::string& password); Result<void> OpenWallet(const std::string& name); void CloseWallet(const std::string& name); Wallet* GetWallet(const std::string& name); std::vector<std::string> ListWallets() const; Result<void> DeleteWallet(const std::string& name,bool allowDangerousDelete=false); private: std::filesystem::path PathFor(const std::string& name) const; std::filesystem::path dir_; PQCryptoProvider* crypto_; std::map<std::string,std::unique_ptr<Wallet>> open_; }; }
