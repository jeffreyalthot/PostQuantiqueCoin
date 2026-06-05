#include "postquantiquecoin/wallet/Wallet.h"
#include "postquantiquecoin/core/Serialization.h"
#include "postquantiquecoin/crypto/PQCAddress.h"
#include "postquantiquecoin/crypto/PQPrivateKeyRecord.h"
#include "postquantiquecoin/crypto/SecureRandom.h"
#include "postquantiquecoin/crypto/WindowsAead.h"
#include "postquantiquecoin/wallet/CoinSelection.h"
#include "postquantiquecoin/wallet/WalletDatFormat.h"
#include "postquantiquecoin/wallet/WalletEncryption.h"
#include "postquantiquecoin/wallet/WalletKeyRing.h"
#include <algorithm>
#include <chrono>
#include <limits>

namespace pqc {
namespace {
uint64_t Now() {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
}

std::vector<uint8_t> PrivateRecordAssociatedData(const WalletKey& k) {
    std::vector<uint8_t> aad{'P','Q','C','_','W','A','L','L','E','T','_','P','R','I','V','A','T','E','_','K','E','Y','_','A','E','A','D','_','V','1'};
    aad.insert(aad.end(), k.address.begin(), k.address.end());
    aad.insert(aad.end(), k.signatureAlgorithm.begin(), k.signatureAlgorithm.end());
    aad.insert(aad.end(), k.kemAlgorithm.begin(), k.kemAlgorithm.end());
    aad.insert(aad.end(), k.keyFingerprint.begin(), k.keyFingerprint.end());
    aad.push_back(2);
    return aad;
}

Result<std::vector<uint8_t>> DeriveWalletMasterKey(const std::string& password,
                                                   const std::vector<uint8_t>& salt,
                                                   uint32_t iterations) {
    return WalletEncryption::DerivePasswordKey(password, salt, iterations, 32);
}

Result<std::vector<uint8_t>> EncryptPrivateRecord(const std::vector<uint8_t>& masterKey,
                                                  const WalletKey& key,
                                                  const PQPrivateKeyRecord& record) {
    auto nonce = SecureRandom::Bytes(12);
    auto enc = WindowsAead::EncryptAes256Gcm(masterKey, nonce, record.Serialize(), PrivateRecordAssociatedData(key));
    if (enc.IsErr()) return Result<std::vector<uint8_t>>::Err(enc.Error());
    std::vector<uint8_t> packed;
    packed.reserve(12 + 16 + enc.Value().ciphertext.size());
    packed.insert(packed.end(), nonce.begin(), nonce.end());
    packed.insert(packed.end(), enc.Value().authTag.begin(), enc.Value().authTag.end());
    packed.insert(packed.end(), enc.Value().ciphertext.begin(), enc.Value().ciphertext.end());
    return Result<std::vector<uint8_t>>::Ok(std::move(packed));
}

TxOutput ReferencedOutputFromUtxo(const UTXO& utxo) {
    TxOutput ref;
    ref.amountAtoms = utxo.amountAtoms;
    ref.address = utxo.address;
    ref.lockingScriptType = utxo.lockingScriptType;
    ref.requiredSignatureAlgorithm = utxo.requiredSignatureAlgorithm;
    ref.publicKeyHash = utxo.publicKeyHash;
    return ref;
}

bool AddOverflow(uint64_t a, uint64_t b, uint64_t& out) {
    if (std::numeric_limits<uint64_t>::max() - a < b) return true;
    out = a + b;
    return false;
}
}

Wallet::Wallet(std::string n, std::filesystem::path p, PQCryptoProvider* c)
    : name_(std::move(n)), storage_(std::move(p)), crypto_(c) {}

Result<Wallet> Wallet::CreateNew(const std::string& name,
                                 const std::string& password,
                                 const std::filesystem::path& path,
                                 PQCryptoProvider* crypto) {
    Wallet w(name, path, crypto);
    w.locked_ = false;
    w.unlockPassword_ = password;
    w.walletSalt_ = SecureRandom::Bytes(32);
    auto master = DeriveWalletMasterKey(password, w.walletSalt_, w.kdfIterations_);
    if (master.IsErr()) return Result<Wallet>::Err(master.Error());
    w.masterKey_ = SecureBuffer(master.Value());
    w.unlockedUntil = Now() + w.autoLockSeconds;
    auto a = w.GenerateNewAddress("default");
    if (a.IsErr()) return Result<Wallet>::Err(a.Error());
    auto s = w.EncryptAndStorePlaintext(password);
    if (s.IsErr()) return Result<Wallet>::Err(s.Error());
    w.Lock();
    return Result<Wallet>::Ok(std::move(w));
}

std::vector<uint8_t> Wallet::SerializePlaintext() const {
    Serializer s;
    s.String("PQC_WALLET_KEYS_V2");
    s.U64(keys_.size());
    for (const auto& k : keys_) s.Bytes(k.Serialize());
    return s.Data();
}

Result<void> Wallet::DeserializePlaintext(const std::vector<uint8_t>& plaintext) {
    Deserializer d(plaintext);
    auto magic = d.String();
    if (magic.IsErr()) return Result<void>::Err(magic.Error());
    if (magic.Value() != "PQC_WALLET_KEYS_V2") return Result<void>::Err("bad wallet plaintext magic");
    auto n = d.U64();
    if (n.IsErr()) return Result<void>::Err(n.Error());
    keys_.clear();
    for (uint64_t i = 0; i < n.Value(); ++i) {
        auto kb = d.Bytes();
        if (kb.IsErr()) return Result<void>::Err(kb.Error());
        auto k = WalletKey::Deserialize(kb.Value());
        if (k.IsErr()) return Result<void>::Err(k.Error());
        keys_.push_back(k.Value());
    }
    if (!d.End()) return Result<void>::Err("trailing wallet plaintext bytes");
    return Result<void>::Ok();
}

Result<void> Wallet::EncryptAndStorePlaintext(const std::string& password) {
    if (walletSalt_.empty()) walletSalt_ = SecureRandom::Bytes(32);
    auto plaintext = SerializePlaintext();
    auto blob = WalletEncryption::EncryptPayload(plaintext, password, kdfIterations_, walletSalt_);
    if (blob.ciphertext.empty() && !plaintext.empty()) return Result<void>::Err("wallet encryption failed");
    walletSalt_ = blob.salt;
    kdfIterations_ = blob.kdfIterations;
    return storage_.SaveEncryptedWallet(WalletDatFormat::Serialize(blob));
}

Result<void> Wallet::Load() {
    if (!storage_.Exists()) return Result<void>::Err("wallet not found");
    auto b = storage_.LoadEncryptedWallet();
    if (b.IsErr()) return Result<void>::Err(b.Error());
    auto blob = WalletDatFormat::Deserialize(b.Value());
    if (blob.IsErr()) return Result<void>::Err(blob.Error());
    walletSalt_ = blob.Value().salt;
    kdfIterations_ = blob.Value().kdfIterations;
    locked_ = true;
    masterKey_.Wipe();
    unlockPassword_.clear();
    keys_.clear();
    return Result<void>::Ok();
}

Result<void> Wallet::Save() {
    if (locked_) return Result<void>::Err("wallet locked");
    if (unlockPassword_.empty()) return Result<void>::Err("wallet save requires unlocked password context");
    return EncryptAndStorePlaintext(unlockPassword_);
}

Result<void> Wallet::Unlock(const std::string& password) { return Unlock(password, autoLockSeconds); }

Result<void> Wallet::Unlock(const std::string& password, uint64_t timeoutSeconds) {
    auto b = storage_.LoadEncryptedWallet();
    if (b.IsErr()) return Result<void>::Err(b.Error());
    auto blob = WalletDatFormat::Deserialize(b.Value());
    if (blob.IsErr()) return Result<void>::Err(blob.Error());
    auto pt = WalletEncryption::DecryptPayload(blob.Value(), password);
    if (pt.IsErr()) return Result<void>::Err(pt.Error());
    auto dp = DeserializePlaintext(pt.Value());
    if (dp.IsErr()) return dp;
    auto master = DeriveWalletMasterKey(password, blob.Value().salt, blob.Value().kdfIterations);
    if (master.IsErr()) return Result<void>::Err(master.Error());
    walletSalt_ = blob.Value().salt;
    kdfIterations_ = blob.Value().kdfIterations;
    masterKey_ = SecureBuffer(master.Value());
    unlockPassword_ = password;
    locked_ = false;
    unlockedUntil = Now() + timeoutSeconds;
    return Result<void>::Ok();
}

void Wallet::Lock() {
    locked_ = true;
    masterKey_.Wipe();
    unlockPassword_.clear();
    unlockedUntil = 0;
}

bool Wallet::IsLocked() const { return locked_; }

void Wallet::MaybeAutoLock() { if (!locked_ && unlockedUntil != 0 && Now() >= unlockedUntil) Lock(); }
void Wallet::SetAutoLockTimeout(uint64_t seconds) { autoLockSeconds = seconds; if (!locked_) unlockedUntil = Now() + seconds; }

Result<std::vector<uint8_t>> Wallet::DecryptPrivate(const WalletKey& k) const {
    if (locked_) return Result<std::vector<uint8_t>>::Err("wallet locked");
    const auto& record = k.EffectiveEncryptedRecord();
    if (record.size() < 12 + 16) return Result<std::vector<uint8_t>>::Err("bad encrypted key record");
    std::vector<uint8_t> nonce(record.begin(), record.begin() + 12);
    std::vector<uint8_t> tag(record.begin() + 12, record.begin() + 28);
    std::vector<uint8_t> ct(record.begin() + 28, record.end());
    auto pt = WindowsAead::DecryptAes256Gcm(masterKey_.ToVectorCopy(), nonce, ct, PrivateRecordAssociatedData(k), tag);
    if (pt.IsErr()) return Result<std::vector<uint8_t>>::Err(pt.Error());
    auto rec = PQPrivateKeyRecord::Deserialize(pt.Value());
    if (rec.IsErr()) return Result<std::vector<uint8_t>>::Err(rec.Error());
    return Result<std::vector<uint8_t>>::Ok(rec.Value().signingPrivateKey);
}

Result<std::string> Wallet::GenerateNewAddress(const std::string& label) {
    MaybeAutoLock();
    if (locked_) return Result<std::string>::Err("wallet locked");
    auto kp = crypto_->GenerateSigningKeyPair();
    auto kem = crypto_->GenerateKemKeyPair();

    PQPrivateKeyRecord rec;
    rec.signatureAlgorithm = kp.algorithm.empty() ? crypto_->GetSigningAlgorithmInfo().name : kp.algorithm;
    rec.kemAlgorithm = kem.algorithm.empty() ? crypto_->GetKemAlgorithmInfo().name : kem.algorithm;
    rec.signingPrivateKey = kp.privateKey;
    rec.signingPublicKey = kp.publicKey;
    rec.kemPrivateKey = kem.privateKey;
    rec.kemPublicKey = kem.publicKey;
    rec.createdTimestamp = Now();
    rec.keyFingerprint = rec.ComputeFingerprint();

    WalletKey wk;
    wk.label = label;
    wk.signatureAlgorithm = rec.signatureAlgorithm;
    wk.kemAlgorithm = rec.kemAlgorithm;
    wk.signingPublicKey = kp.publicKey;
    wk.publicKey = kp.publicKey;
    wk.kemPublicKey = kem.publicKey;
    wk.keyFingerprint = rec.keyFingerprint;
    wk.createdTimestamp = rec.createdTimestamp;
    wk.address = PQCAddress::FromPublicKey(kp.publicKey);

    auto enc = EncryptPrivateRecord(masterKey_.ToVectorCopy(), wk, rec);
    if (enc.IsErr()) return Result<std::string>::Err(enc.Error());
    wk.encryptedPrivateKeyRecord = enc.Value();
    wk.encryptedPrivateKey = wk.encryptedPrivateKeyRecord;
    keys_.push_back(wk);
    return Result<std::string>::Ok(wk.address);
}

std::vector<std::string> Wallet::ListAddresses() const {
    std::vector<std::string> v;
    for (const auto& k : keys_) v.push_back(k.address);
    return v;
}

uint64_t Wallet::GetBalance(Blockchain& chain) const { return GetAvailableBalance(chain); }
uint64_t Wallet::GetConfirmedBalance(Blockchain& chain) const {
    uint64_t s = 0;
    for (const auto& a : ListAddresses()) {
        uint64_t next = 0;
        if (!AddOverflow(s, chain.GetBalance(a), next)) s = next;
    }
    return s;
}
uint64_t Wallet::GetImmatureBalance(Blockchain& chain) const {
    uint64_t sum = 0;
    for (const auto& utxo : chain.GetUTXOSet().Snapshot()) {
        if (WalletKeyRing(keys_).FindKeyByAddress(utxo.address) && !utxo.IsSpendable(chain.GetHeight(), chain.Params().coinbaseMaturity)) {
            uint64_t next = 0;
            if (!AddOverflow(sum, utxo.amountAtoms, next)) sum = next;
        }
    }
    return sum;
}
uint64_t Wallet::GetPendingIncomingBalance(Blockchain&) const { return 0; }
uint64_t Wallet::GetPendingOutgoingBalance(Blockchain&) const { return 0; }
uint64_t Wallet::GetAvailableBalance(Blockchain& chain) const { return GetConfirmedBalance(chain); }
uint64_t Wallet::GetTotalBalance(Blockchain& chain) const {
    uint64_t total = GetConfirmedBalance(chain);
    uint64_t next = 0;
    if (!AddOverflow(total, GetImmatureBalance(chain), next)) total = next;
    if (!AddOverflow(total, GetPendingIncomingBalance(chain), next)) total = next;
    return total;
}

Result<Transaction> Wallet::CreateTransaction(const std::string& to, uint64_t amount, uint64_t fee, Blockchain& chain) {
    MaybeAutoLock();
    if (locked_) return Result<Transaction>::Err("wallet locked");
    uint64_t target = 0;
    if (AddOverflow(amount, fee, target)) return Result<Transaction>::Err("transaction amount overflow");

    auto toDecoded = PQCAddress::Decode(to, chain.Params().networkId);
    if (toDecoded.IsErr()) return Result<Transaction>::Err(toDecoded.Error());

    WalletKeyRing ring(keys_);
    std::vector<UTXO> candidates;
    for (const auto& k : keys_) {
        auto us = chain.GetSpendableUTXOs(k.address);
        candidates.insert(candidates.end(), us.begin(), us.end());
    }

    auto selected = CoinSelection::SelectBranchAndBound(std::move(candidates), target);
    if (!selected.success) return Result<Transaction>::Err("insufficient funds");

    Transaction tx;
    tx.version = 2;
    tx.timestamp = Now();
    tx.publicKey.clear();
    tx.signature.clear();

    for (const auto& u : selected.selected) {
        const auto* key = ring.FindKeyByAddress(u.address);
        if (!key) return Result<Transaction>::Err("selected UTXO has no matching wallet key");
        TxInput in;
        in.previousTxid = u.txid;
        in.outputIndex = u.outputIndex;
        in.publicKey = key->EffectivePublicKey();
        in.signatureAlgorithm = u.requiredSignatureAlgorithm.empty() ? key->signatureAlgorithm : u.requiredSignatureAlgorithm;
        tx.inputs.push_back(in);
    }

    TxOutput out;
    out.amountAtoms = amount;
    out.address = to;
    out.requiredSignatureAlgorithm = chain.Params().defaultSignatureAlgorithm.empty() ? "ML-DSA-65" : chain.Params().defaultSignatureAlgorithm;
    out.publicKeyHash = toDecoded.Value().publicKeyHash;
    tx.outputs.push_back(out);

    if (selected.total > target) {
        const auto& changeKey = *ring.FindKeyByAddress(selected.selected.front().address);
        auto changeDecoded = PQCAddress::Decode(changeKey.address, chain.Params().networkId);
        TxOutput change;
        change.amountAtoms = selected.total - target;
        change.address = changeKey.address;
        change.requiredSignatureAlgorithm = changeKey.signatureAlgorithm;
        if (changeDecoded.IsOk()) change.publicKeyHash = changeDecoded.Value().publicKeyHash;
        tx.outputs.push_back(change);
    }

    for (size_t i = 0; i < tx.inputs.size(); ++i) {
        const auto& utxo = selected.selected[i];
        const auto* key = ring.FindKeyByAddress(utxo.address);
        if (!key) return Result<Transaction>::Err("no wallet key for input");
        if (PQCAddress::HashPublicKey(key->EffectivePublicKey()) != utxo.publicKeyHash) return Result<Transaction>::Err("wallet key publicKeyHash mismatch");
        if (!utxo.requiredSignatureAlgorithm.empty() && key->signatureAlgorithm != utxo.requiredSignatureAlgorithm) return Result<Transaction>::Err("wallet key signature algorithm mismatch");
        auto pk = DecryptPrivate(*key);
        if (pk.IsErr()) return Result<Transaction>::Err(pk.Error());
        auto si = tx.SignInput(i, ReferencedOutputFromUtxo(utxo), *crypto_, pk.Value(), key->EffectivePublicKey());
        if (si.IsErr()) return Result<Transaction>::Err(si.Error());
    }
    tx.txid = tx.ComputeTxId();
    return Result<Transaction>::Ok(tx);
}

Result<void> Wallet::SignTransaction(Transaction& tx) {
    MaybeAutoLock();
    if (locked_) return Result<void>::Err("wallet locked");
    WalletKeyRing ring(keys_);
    for (size_t i = 0; i < tx.inputs.size(); ++i) {
        if (tx.inputs[i].publicKey.empty()) return Result<void>::Err("input has no public key to match wallet key");
        auto pkh = PQCAddress::HashPublicKey(tx.inputs[i].publicKey);
        const auto* key = ring.FindKeyByPublicKeyHash(pkh);
        if (!key) return Result<void>::Err("no matching wallet key for input publicKeyHash");
        if (key->EffectivePublicKey() != tx.inputs[i].publicKey) return Result<void>::Err("input public key is not owned by wallet");
        if (!tx.inputs[i].signatureAlgorithm.empty() && key->signatureAlgorithm != tx.inputs[i].signatureAlgorithm) return Result<void>::Err("input signature algorithm mismatch");
        auto pk = DecryptPrivate(*key);
        if (pk.IsErr()) return Result<void>::Err(pk.Error());
        TxOutput ref;
        ref.address = key->address;
        ref.requiredSignatureAlgorithm = key->signatureAlgorithm;
        ref.publicKeyHash = pkh;
        auto r = tx.SignInput(i, ref, *crypto_, pk.Value(), key->EffectivePublicKey());
        if (r.IsErr()) return r;
    }
    return Result<void>::Ok();
}

Result<void> Wallet::ImportPrivateKey(const std::string& label,
                                      const std::vector<uint8_t>& pub,
                                      const std::vector<uint8_t>& priv) {
    MaybeAutoLock();
    if (locked_) return Result<void>::Err("wallet locked");
    PQPrivateKeyRecord rec;
    rec.signingPrivateKey = priv;
    rec.signingPublicKey = pub;
    rec.signatureAlgorithm = crypto_->GetSigningAlgorithmInfo().name;
    rec.createdTimestamp = Now();
    rec.keyFingerprint = rec.ComputeFingerprint();

    WalletKey wk;
    wk.label = label;
    wk.address = PQCAddress::FromPublicKey(pub);
    wk.signatureAlgorithm = rec.signatureAlgorithm;
    wk.signingPublicKey = pub;
    wk.publicKey = pub;
    wk.keyFingerprint = rec.keyFingerprint;
    wk.createdTimestamp = rec.createdTimestamp;
    auto enc = EncryptPrivateRecord(masterKey_.ToVectorCopy(), wk, rec);
    if (enc.IsErr()) return Result<void>::Err(enc.Error());
    wk.encryptedPrivateKeyRecord = enc.Value();
    wk.encryptedPrivateKey = wk.encryptedPrivateKeyRecord;
    keys_.push_back(wk);
    return Result<void>::Ok();
}

std::vector<std::vector<uint8_t>> Wallet::ExportPublicKeys() const {
    std::vector<std::vector<uint8_t>> v;
    for (const auto& k : keys_) v.push_back(k.EffectivePublicKey());
    return v;
}

Result<void> Wallet::Backup(const std::filesystem::path& p) const {
    auto r = storage_.Backup(p);
    return r.IsOk() ? Result<void>::Ok() : Result<void>::Err(r.Error());
}

Result<void> Wallet::Rescan(Blockchain& chain) {
    WalletKeyRing ring(keys_);
    for (uint64_t h = 0; h <= chain.GetHeight(); ++h) {
        auto block = chain.GetBlockByHeight(h);
        if (!block) continue;
        for (const auto& tx : block->transactions) {
            for (const auto& out : tx.outputs) {
                if (!out.address.empty() && ring.FindKeyByAddress(out.address)) {
                    (void)out;
                }
            }
        }
        if (h == std::numeric_limits<uint64_t>::max()) break;
    }
    return Result<void>::Ok();
}

const std::string& Wallet::Name() const { return name_; }
}
