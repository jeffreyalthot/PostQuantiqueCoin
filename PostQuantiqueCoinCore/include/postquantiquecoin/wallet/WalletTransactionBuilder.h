#pragma once
#include "postquantiquecoin/blockchain/Mempool.h"
#include "postquantiquecoin/wallet/CoinSelection.h"
namespace pqc { class WalletTransactionBuilder { public: static uint64_t EstimateFee(size_t bytes,uint64_t feeRate); static void AddChangeOutput(Transaction& tx,const std::string& address,uint64_t amount); static Result<void> FinalizeTransaction(Transaction& tx); static Result<void> CommitToMempool(Mempool& mempool,const Transaction& tx,const UTXOSet& utxos,uint64_t height,const ChainParams& params,PQCryptoProvider* provider); }; }
