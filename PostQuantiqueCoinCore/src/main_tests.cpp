#include "postquantiquecoin/blockchain/Block.h"
#include "postquantiquecoin/blockchain/ChainParams.h"
#include "postquantiquecoin/blockchain/Consensus.h"
#include "postquantiquecoin/blockchain/Genesis.h"
#include "postquantiquecoin/blockchain/MerkleTree.h"
#include "postquantiquecoin/core/Amount.h"
#include "postquantiquecoin/core/Hex.h"
#include "postquantiquecoin/crypto/Address.h"
#include "postquantiquecoin/crypto/Hashing.h"
#include "postquantiquecoin/crypto/PQCryptoProvider.h"
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace {
int failed = 0;
void Check(bool condition, const char* name) {
    if (condition) std::cout << "[PASS] " << name << '\n';
    else { std::cerr << "[FAIL] " << name << '\n'; ++failed; }
}
}

int main() {
    try {
        auto amount = pqc::Amount::FromCoinsString("29.00000001");
        Check(amount.IsOk() && amount.Value() == 2900000001ULL, "Amount conversion");
        Check(pqc::Amount::ToCoinsString(1) == "0.00000001", "Amount formatting");

        std::vector<uint8_t> abc{'a','b','c'};
        auto h1 = pqc::Hashing::Hash256Hex(abc);
        auto h2 = pqc::Hashing::Hash256Hex(abc);
        Check(h1 == h2 && h1.size() == 64, "Hash deterministic");

        auto provider = pqc::CreateDefaultCryptoProvider();
        auto kp = provider->GenerateSigningKeyPair();
        auto address = pqc::Address::FromPublicKey(kp.publicKey);
        Check(pqc::Address::Validate(address), "Address generation/validation");

        std::vector<uint8_t> message{'P','Q','C'};
        auto sig = provider->Sign(kp.privateKey, message);
        Check(provider->Verify(kp.publicKey, message, sig), "PQ key generation/sign/verify");
        message.push_back('!');
        Check(!provider->Verify(kp.publicKey, message, sig), "Bad signature rejected");

        pqc::Transaction tx;
        tx.timestamp = 1704067201ULL;
        tx.inputs.push_back({std::string(64, '1'), 0, {}, kp.publicKey});
        tx.outputs.push_back({1000, address, "PQC_PUBKEY_HASH"});
        tx.Sign(*provider, kp.privateKey, kp.publicKey);
        auto serialized = tx.Serialize(true);
        auto tx2 = pqc::Transaction::Deserialize(serialized);
        Check(tx2.IsOk() && tx2.Value().ComputeTxId() == tx.ComputeTxId(), "Transaction serialization stable");
        Check(tx.GetSigningDigest() == tx2.Value().GetSigningDigest(), "Transaction signing digest stable");

        std::vector<pqc::Transaction> txs{tx};
        Check(pqc::MerkleTree::ComputeRoot(txs) == pqc::MerkleTree::ComputeRoot(txs), "Merkle root stable");

        auto params = pqc::ChainParams::Mainnet();
        auto genesis = pqc::Genesis::BuildGenesisBlock(params);
        Check(pqc::Genesis::ValidateGenesisBlock(genesis, params), "Genesis block deterministic");
        Check(pqc::Consensus::ValidateProofOfWork(genesis.header), "PoW validation");

        uint64_t minted = 0;
        bool supplyOk = true;
        for (uint64_t height = 0; height < 10000000; ++height) {
            uint64_t s = params.GetBlockSubsidy(height, minted);
            if (minted > params.maxSupplyAtoms || UINT64_MAX - minted < s) { supplyOk = false; break; }
            minted += s;
            if (s == 0) break;
        }
        Check(supplyOk && minted <= params.maxSupplyAtoms, "Subsidy never exceeds max supply");
        Check(params.coinbaseMaturity == 100, "Coinbase maturity");

        auto basic = genesis.ValidateBasic();
        Check(basic.IsOk(), "Block validation");

        auto kem = provider->GenerateKemKeyPair();
        auto enc = provider->Encapsulate(kem.publicKey);
        auto dec = provider->Decapsulate(kem.privateKey, enc.ciphertext);
        Check(pqc::Hashing::SecureCompare(enc.sharedSecret, dec), "P2P KEM shared secret");

        Check(failed == 0, "Test suite aggregate");
    } catch (const std::exception& ex) {
        std::cerr << "[EXCEPTION] " << ex.what() << '\n';
        return 1;
    }
    return failed == 0 ? 0 : 1;
}
