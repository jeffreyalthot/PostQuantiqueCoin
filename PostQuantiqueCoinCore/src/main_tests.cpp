#include "postquantiquecoin/blockchain/Blockchain.h"
#include "postquantiquecoin/blockchain/ChainParams.h"
#include "postquantiquecoin/blockchain/Consensus.h"
#include "postquantiquecoin/blockchain/Difficulty.h"
#include "postquantiquecoin/blockchain/Genesis.h"
#include "postquantiquecoin/blockchain/Mempool.h"
#include "postquantiquecoin/blockchain/MerkleTree.h"
#include "postquantiquecoin/core/Amount.h"
#include "postquantiquecoin/core/Hex.h"
#include "postquantiquecoin/crypto/Address.h"
#include "postquantiquecoin/crypto/PQCAddress.h"
#include "postquantiquecoin/mining/PQPow.h"
#include "postquantiquecoin/wallet/WalletEncryption.h"
#include "postquantiquecoin/crypto/Hashing.h"
#include "postquantiquecoin/crypto/PQCryptoProvider.h"
#include "postquantiquecoin/mining/Miner.h"
#include "postquantiquecoin/storage/BlockIndex.h"
#include "postquantiquecoin/storage/FileStorage.h"
#include "postquantiquecoin/storage/UtxoStorage.h"
#include "postquantiquecoin/wallet/WalletManager.h"
#include <cstdlib>
#include <filesystem>
#include <cctype>
#include <iostream>
#include <stdexcept>

namespace {
int failed = 0;
void Check(bool condition, const char* name) {
    if (condition) std::cout << "[PASS] " << name << '\n';
    else { std::cerr << "[FAIL] " << name << '\n'; ++failed; }
}
std::filesystem::path TempDir(const std::string& name) {
    auto p = std::filesystem::temp_directory_path() / ("pqc_" + name + "_tests");
    std::filesystem::remove_all(p);
    std::filesystem::create_directories(p);
    return p;
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

        auto sha3abc = pqc::Hashing::Sha3_256(std::vector<uint8_t>{'a','b','c'});
        Check(pqc::Hex::Encode(std::vector<uint8_t>(sha3abc.begin(), sha3abc.end())) == "3a985da74fe225b2045c172d6bd390bd855f086e3e9d525b46bfe24511431532", "SHA3-256 known answer test");
        auto shake = pqc::Hashing::Shake256(std::vector<uint8_t>{'a','b','c'}, 64);
        Check(shake.size() == 64, "SHAKE256 variable output length");
        auto kp = provider->GenerateSigningKeyPair();
        auto address = pqc::Address::FromPublicKey(kp.publicKey);
        Check(pqc::Address::Validate(address), "Address generation/validation");
        auto decodedAddr = pqc::PQCAddress::Decode(address);
        Check(decodedAddr.IsOk(), "PQCAddress v2 decode");
        auto tamperedAddress = address; tamperedAddress.back() = tamperedAddress.back() == 'A' ? 'B' : 'A';
        Check(pqc::PQCAddress::Decode(tamperedAddress).IsErr(), "PQCAddress rejects bad checksum");
        auto lowerAddress = address; lowerAddress[5] = static_cast<char>(std::tolower(static_cast<unsigned char>(lowerAddress[5])));
        Check(pqc::PQCAddress::Decode(lowerAddress).IsErr(), "PQCAddress rejects lowercase ambiguity");

        std::vector<uint8_t> message{'P','Q','C'};
        auto sig = provider->Sign(kp.privateKey, message);
        Check(provider->Verify(kp.publicKey, message, sig), "PQ key generation/sign/verify");
        auto signInfo = provider->GetSigningAlgorithmInfo();
        auto kemInfo = provider->GetKemAlgorithmInfo();
        Check(provider->IsSigningAlgorithmAllowed(signInfo.name), "Signing algorithm allow-list reports active algorithm");
        Check(provider->IsKemAlgorithmAllowed(kemInfo.name), "KEM algorithm allow-list reports active algorithm");
        Check(!provider->Verify(std::vector<uint8_t>{1,2,3}, std::vector<uint8_t>{4}, std::vector<uint8_t>{5}), "Provider rejects invalid public key/signature sizes");
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
        Check(tx.GetOutputSum() == 1000 && tx.EstimatedSize() == tx.Serialize(true).size(), "Transaction helpers");

        std::vector<pqc::Transaction> txs{tx};
        Check(pqc::MerkleTree::ComputeRoot(txs) == pqc::MerkleTree::ComputeRoot(txs), "Merkle root stable");

        auto params = pqc::ChainParams::Devnet();
        auto genesis = pqc::Genesis::BuildGenesisBlock(params);
        Check(pqc::Genesis::ValidateGenesisBlock(genesis, params), "Genesis block deterministic");
        auto powHash1 = pqc::PQPow::ComputePowHash(genesis.header, params);
        auto powHash2 = pqc::PQPow::ComputePowHash(genesis.header, params);
        Check(powHash1 == powHash2, "PQC-PoW-v1 deterministic");
        auto changedHeader = genesis.header; ++changedHeader.nonce;
        Check(pqc::PQPow::ComputePowHash(changedHeader, params) != powHash1, "PQC-PoW-v1 changes with nonce");
        Check(pqc::Difficulty::CheckProofOfWorkHash(powHash1, genesis.header.bits), "PQC-PoW-v1 valid target accepted");
        Check(pqc::Consensus::ValidateProofOfWork(genesis.header, params), "PoW validation");

        uint64_t minted = 0;
        bool supplyOk = true;
        for (uint64_t height = 0; height < 10000000; ++height) {
            uint64_t s = params.GetBlockSubsidy(height, minted);
            if (minted > params.maxSupplyAtoms || UINT64_MAX - minted < s) { supplyOk = false; break; }
            minted += s;
            if (s == 0) break;
        }
        Check(supplyOk && minted <= params.maxSupplyAtoms, "Subsidy never exceeds max supply");
        Check(pqc::ChainParams::Mainnet().coinbaseMaturity == 100, "Coinbase maturity");

        auto basic = genesis.ValidateBasic();
        Check(basic.IsOk(), "Block validation");

        auto kem = provider->GenerateKemKeyPair();
        auto enc = provider->Encapsulate(kem.publicKey);
        auto dec = provider->Decapsulate(kem.privateKey, enc.ciphertext);
        Check(pqc::Hashing::SecureCompare(enc.sharedSecret, dec), "P2P KEM shared secret");

        pqc::UTXOSet set;
        pqc::UTXO u{std::string(64,'a'),0,5000,address,"PQC_PUBKEY_HASH",1,false,10};
        Check(set.AddUTXO(u).IsOk() && set.HasUTXO(u.txid,0), "UTXO add");
        Check(set.GetBalance(address, 2, params.coinbaseMaturity) == 5000, "UTXO balance calculation");
        pqc::Transaction spend;
        spend.timestamp = 20;
        spend.inputs.push_back({u.txid,0,{},kp.publicKey});
        spend.outputs.push_back({4000,address,"PQC_PUBKEY_HASH"});
        spend.Sign(*provider,kp.privateKey,kp.publicKey);
        auto spent = set.ApplyTransaction(spend,2,false,2,params.coinbaseMaturity);
        Check(spent.IsOk() && !set.HasUTXO(u.txid,0), "UTXO spend");
        Check(set.ApplyTransaction(spend,2,false,2,params.coinbaseMaturity).IsErr(), "UTXO reject double spend");
        pqc::Transaction missing = spend; missing.inputs[0].previousTxid = std::string(64,'b'); missing.txid.clear(); missing.Sign(*provider,kp.privateKey,kp.publicKey);
        Check(set.ApplyTransaction(missing,2,false,2,params.coinbaseMaturity).IsErr(), "UTXO reject missing input");
        pqc::UTXO immature{std::string(64,'c'),0,5000,address,"PQC_PUBKEY_HASH",10,true,10};
        pqc::UTXOSet cbset; cbset.AddUTXO(immature);
        pqc::Transaction cbspend; cbspend.timestamp=30; cbspend.inputs.push_back({immature.txid,0,{},kp.publicKey}); cbspend.outputs.push_back({4000,address,"PQC_PUBKEY_HASH"}); cbspend.Sign(*provider,kp.privateKey,kp.publicKey);
        Check(cbset.ApplyTransaction(cbspend,11,false,11,params.coinbaseMaturity).IsErr(), "UTXO coinbase immature rejected");
        Check(cbset.ApplyTransaction(cbspend,12,false,12,params.coinbaseMaturity).IsOk(), "UTXO coinbase mature accepted");

        pqc::UTXOSet mpSet; pqc::UTXO mu{std::string(64,'d'),0,10000,address,"PQC_PUBKEY_HASH",1,false,1}; mpSet.AddUTXO(mu);
        pqc::Transaction mptx; mptx.timestamp=40; mptx.inputs.push_back({mu.txid,0,{},kp.publicKey}); mptx.outputs.push_back({9000,address,"PQC_PUBKEY_HASH"}); mptx.Sign(*provider,kp.privateKey,kp.publicKey);
        pqc::Mempool mempool;
        Check(mempool.AddTransaction(mptx,mpSet,5,params,provider.get()).IsOk(), "Mempool add valid transaction");
        Check(mempool.AddTransaction(mptx,mpSet,5,params,provider.get()).IsErr(), "Mempool reject duplicate txid");
        pqc::Transaction conflict=mptx; conflict.outputs[0].amountAtoms=8000; conflict.txid.clear(); conflict.Sign(*provider,kp.privateKey,kp.publicKey);
        Check(mempool.AddTransaction(conflict,mpSet,5,params,provider.get()).IsErr(), "Mempool reject double spend");
        Check(mempool.GetTransactionsForBlock(10,100000).size()==1, "Mempool sort/select by fee rate");

        auto dataDir = TempDir("chain");
        auto bc = pqc::Blockchain::OpenOrCreate(dataDir, params, provider.get());
        Check(bc.IsOk() && bc.Value().GetHeight()==0, "Blockchain auto-create genesis");
        Check(bc.Value().ValidateChain().IsOk(), "Blockchain validate full chain");
        auto wrong = genesis; wrong.header.height=1; wrong.header.previousHash=std::string(64,'e'); wrong.header.merkleRoot=pqc::MerkleTree::ComputeRoot(wrong.transactions); wrong.header.nonce=0; while(!pqc::Consensus::ValidateProofOfWork(wrong.header, params)) ++wrong.header.nonce;
        Check(bc.Value().AddBlock(wrong).IsErr(), "Blockchain reject wrong previous hash");
        auto mineKey = provider->GenerateSigningKeyPair();
        auto mineAddr = pqc::Address::FromPublicKey(mineKey.publicKey);
        pqc::Miner miner(bc.Value());
        auto job = miner.BuildCandidateBlock(mineAddr);
        Check(job.IsOk() && !job.Value().candidateBlock.transactions.empty(), "Mining build candidate block");
        auto badMerkle = job.Value().candidateBlock; badMerkle.header.merkleRoot=std::string(64,'f');
        Check(bc.Value().AddBlock(badMerkle).IsErr(), "Blockchain reject invalid merkle root");
        auto excessive = job.Value().candidateBlock; excessive.transactions[0].outputs[0].amountAtoms += 1; excessive.transactions[0].txid = excessive.transactions[0].ComputeTxId(); excessive.header.merkleRoot=pqc::MerkleTree::ComputeRoot(excessive.transactions); excessive.header.nonce=0; while(!pqc::Consensus::ValidateProofOfWork(excessive.header, params)) ++excessive.header.nonce;
        Check(bc.Value().AddBlock(excessive).IsErr(), "Blockchain reject excessive coinbase reward");
        auto mined = miner.MineNextBlock(mineAddr,1);
        Check(mined.IsOk() && bc.Value().GetHeight()==1, "Mining mine and submit easy dev block");
        Check(bc.Value().RebuildUTXO().IsOk(), "Blockchain rebuild UTXO");

        auto storageDir = TempDir("storage");
        std::vector<uint8_t> bytes{'o','k'};
        Check(pqc::FileStorage::AtomicWrite(storageDir/"a.bin", bytes).IsOk() && pqc::FileStorage::ReadFile(storageDir/"a.bin").IsOk(), "Storage atomic write/read");
        pqc::BlockIndex bi(storageDir/"index.dat"); bi.Add({"hash","prev",7,1,2,0,0,10,"7"}); bi.Save(); pqc::BlockIndex bi2(storageDir/"index.dat"); bi2.Load();
        Check(bi2.GetByHeight(7).has_value(), "Storage save/load block index");
        pqc::UtxoStorage us(storageDir/"utxo.dat"); Check(us.SaveSnapshot(mpSet).IsOk() && us.LoadSnapshot().IsOk(), "Storage save/load UTXO snapshot");


        std::vector<uint8_t> walletPlain{'s','e','c','r','e','t'};
        auto blob = pqc::WalletEncryption::EncryptPayload(walletPlain, "correct", 1000);
        Check(blob.ciphertext != walletPlain, "wallet.dat encrypted payload unreadable");
        Check(pqc::WalletEncryption::DecryptPayload(blob, "wrong", 1000).IsErr(), "wallet.dat wrong password rejected");
        auto openedBlob = pqc::WalletEncryption::DecryptPayload(blob, "correct", 1000);
        Check(openedBlob.IsOk() && openedBlob.Value() == walletPlain, "wallet.dat unlock correct password");
        auto tamperedBlob = blob; tamperedBlob.ciphertext[0] ^= 1;
        Check(pqc::WalletEncryption::DecryptPayload(tamperedBlob, "correct", 1000).IsErr(), "wallet.dat tampered ciphertext rejected");

        auto walletDir = TempDir("wallets");
        pqc::WalletManager wm(walletDir, provider.get());
        Check(wm.CreateWallet("alice","pw").IsOk(), "Wallet create");
        auto* w = wm.GetWallet("alice");
        Check(w != nullptr && !w->ListAddresses().empty(), "Wallet generate address");
        pqc::Transaction unsignedTx; unsignedTx.inputs.push_back({std::string(64,'1'),0,{},kp.publicKey}); unsignedTx.outputs.push_back({1,address,"PQC_PUBKEY_HASH"});
        Check(w->SignTransaction(unsignedTx).IsErr(), "Wallet locked prevents signing");
        Check(w->Unlock("pw").IsOk() && w->SignTransaction(unsignedTx).IsErr(), "Wallet rejects transaction without matching key");
        auto walletPubs = w->ExportPublicKeys();
        pqc::Transaction ownedTx; ownedTx.inputs.push_back({std::string(64,'2'),0,{},walletPubs.empty()?std::vector<uint8_t>{}:walletPubs.front()}); ownedTx.inputs[0].signatureAlgorithm = provider->GetSigningAlgorithmInfo().name; ownedTx.outputs.push_back({1,w->ListAddresses().front(),"PQC_PUBKEY_HASH"});
        Check(!walletPubs.empty() && w->SignTransaction(ownedTx).IsOk(), "Wallet unlock allows owned key signing");
        Check(w->GetBalance(bc.Value()) == 0, "Wallet balance through blockchain");
        Check(w->Backup(walletDir/"alice.bak").IsOk(), "Wallet backup");

        Check(failed == 0, "Test suite aggregate");
    } catch (const std::exception& ex) {
        std::cerr << "[EXCEPTION] " << ex.what() << '\n';
        return 1;
    }
    return failed == 0 ? 0 : 1;
}
