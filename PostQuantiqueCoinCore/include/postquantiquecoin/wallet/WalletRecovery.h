#pragma once
#include "postquantiquecoin/crypto/PQCryptoProvider.h"
#include "postquantiquecoin/wallet/WalletRecoveryFile.h"
namespace pqc { class WalletRecovery { public: static WalletRecoveryFile Create(PQCryptoProvider& provider,const std::vector<uint8_t>& dek); static bool Test(PQCryptoProvider& provider,const WalletRecoveryFile& file); }; }
