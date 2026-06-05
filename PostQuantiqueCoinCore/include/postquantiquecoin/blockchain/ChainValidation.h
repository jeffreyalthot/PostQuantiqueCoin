#pragma once
#include "postquantiquecoin/blockchain/Block.h"
#include "postquantiquecoin/blockchain/ChainParams.h"
#include "postquantiquecoin/blockchain/UTXOSet.h"
namespace pqc { class ChainValidation { public: static Result<uint64_t> CalculateFees(const Block& block, const UTXOSet& view, uint64_t height, uint64_t maturity); static uint64_t CoinbaseOutputSum(const Block& block); static Result<void> ValidateSpendAuthorization(const TxInput& input,const UTXO& referencedUtxo,const Transaction& tx,size_t inputIndex,PQCryptoProvider& provider,const ChainParams& params); }; }
