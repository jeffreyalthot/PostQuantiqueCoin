#include "postquantiquecoin/wallet/WalletDatFormat.h"
#include "postquantiquecoin/core/Serialization.h"
#include "postquantiquecoin/crypto/Hashing.h"
#include <algorithm>
namespace pqc {
namespace {
void Append(std::vector<uint8_t>& out, const std::vector<uint8_t>& in) { out.insert(out.end(), in.begin(), in.end()); }
void Append(std::vector<uint8_t>& out, const std::array<uint8_t, 32>& in) { out.insert(out.end(), in.begin(), in.end()); }
std::array<uint8_t,32> ChecksumFor(const WalletEncryptedBlob& b){
    std::vector<uint8_t> d{'P','Q','C','W','A','L','L','E','T','_','B','L','O','B','_','V','3'};
    d.push_back(b.networkId); d.push_back(b.kdfAlgorithm); d.push_back(b.aeadAlgorithm); d.push_back(b.pqWrapAlgorithm);
    Append(d, b.encryptedPayload); Append(d, b.encryptedDekByPassword); Append(d, b.encryptedDekByKem); Append(d, b.pqKemCiphertext);
    Append(d, b.saltPassword); Append(d, b.noncePayload); Append(d, b.nonceDekPassword); Append(d, b.nonceDekKem);
    Append(d, b.tagPayload); Append(d, b.tagDekPassword); Append(d, b.tagDekKem); Append(d, b.walletFingerprint); Append(d, b.kemPublicKeyFingerprint);
    return Hashing::Sha3_256(d);
}
std::array<uint8_t,32> ReadArray32(Deserializer& d, const char* name, bool& ok, std::string& err) {
    std::array<uint8_t,32> out{};
    auto bytes = d.Bytes();
    if (bytes.IsErr()) { ok = false; err = bytes.Error(); return out; }
    if (bytes.Value().size() != 32) { ok = false; err = std::string("bad ") + name + " length"; return out; }
    std::copy(bytes.Value().begin(), bytes.Value().end(), out.begin());
    return out;
}
}
std::vector<uint8_t> WalletDatFormat::AssociatedData(const WalletEncryptedBlob& blob){
    std::vector<uint8_t> aad{'P','Q','C','W','A','L','L','E','T','_','V','3'};
    aad.push_back(static_cast<uint8_t>(blob.version & 0xff)); aad.push_back(blob.networkId); aad.push_back(blob.kdfAlgorithm); aad.push_back(blob.aeadAlgorithm); aad.push_back(blob.pqWrapAlgorithm); return aad;
}
std::vector<uint8_t> WalletDatFormat::Serialize(const WalletEncryptedBlob& in){
    WalletEncryptedBlob b=in; b.checksum=ChecksumFor(b); Serializer s; s.String("PQCWALLET"); s.U32(b.version); s.U32(b.networkId); s.U32(b.kdfAlgorithm); s.U32(b.aeadAlgorithm); s.U32(b.pqWrapAlgorithm); s.U32(b.kdfIterations); s.String(b.kdfName); s.String(b.aeadName); s.String(b.pqKemName);
    s.Bytes(b.encryptedPayload); s.Bytes(b.encryptedDekByPassword); s.Bytes(b.encryptedDekByKem); s.Bytes(b.pqKemCiphertext); s.Bytes(b.saltPassword); s.Bytes(b.noncePayload); s.Bytes(b.nonceDekPassword); s.Bytes(b.nonceDekKem); s.Bytes(b.tagPayload); s.Bytes(b.tagDekPassword); s.Bytes(b.tagDekKem); s.Bytes(std::vector<uint8_t>(b.walletFingerprint.begin(),b.walletFingerprint.end())); s.Bytes(std::vector<uint8_t>(b.kemPublicKeyFingerprint.begin(),b.kemPublicKeyFingerprint.end())); s.Bytes(std::vector<uint8_t>(b.checksum.begin(),b.checksum.end())); return s.Data();
}
Result<WalletEncryptedBlob> WalletDatFormat::Deserialize(const std::vector<uint8_t>& data){
    Deserializer d(data); WalletEncryptedBlob b; auto m=d.String(); if(m.IsErr()) return Result<WalletEncryptedBlob>::Err(m.Error()); if(m.Value()!="PQCWALLET") return Result<WalletEncryptedBlob>::Err("bad wallet magic");
    auto v=d.U32(); if(v.IsErr()) return Result<WalletEncryptedBlob>::Err(v.Error()); b.version=v.Value(); if(b.version!=3) return Result<WalletEncryptedBlob>::Err("unsupported wallet format version");
    auto nid=d.U32(); if(nid.IsErr()) return Result<WalletEncryptedBlob>::Err(nid.Error()); b.networkId=static_cast<uint8_t>(nid.Value());
    auto kdf=d.U32(); if(kdf.IsErr()) return Result<WalletEncryptedBlob>::Err(kdf.Error()); b.kdfAlgorithm=static_cast<uint8_t>(kdf.Value());
    auto aead=d.U32(); if(aead.IsErr()) return Result<WalletEncryptedBlob>::Err(aead.Error()); b.aeadAlgorithm=static_cast<uint8_t>(aead.Value());
    auto wrap=d.U32(); if(wrap.IsErr()) return Result<WalletEncryptedBlob>::Err(wrap.Error()); b.pqWrapAlgorithm=static_cast<uint8_t>(wrap.Value());
    auto iter=d.U32(); if(iter.IsErr()) return Result<WalletEncryptedBlob>::Err(iter.Error()); b.kdfIterations=iter.Value();
    auto kn=d.String(); if(kn.IsErr()) return Result<WalletEncryptedBlob>::Err(kn.Error()); b.kdfName=kn.Value(); auto an=d.String(); if(an.IsErr()) return Result<WalletEncryptedBlob>::Err(an.Error()); b.aeadName=an.Value(); auto pn=d.String(); if(pn.IsErr()) return Result<WalletEncryptedBlob>::Err(pn.Error()); b.pqKemName=pn.Value();
    auto readBytes=[&](std::vector<uint8_t>& target)->Result<void>{ auto r=d.Bytes(); if(r.IsErr()) return Result<void>::Err(r.Error()); target=r.Value(); return Result<void>::Ok(); };
    if(auto r=readBytes(b.encryptedPayload); r.IsErr()) return Result<WalletEncryptedBlob>::Err(r.Error());
    if(auto r=readBytes(b.encryptedDekByPassword); r.IsErr()) return Result<WalletEncryptedBlob>::Err(r.Error());
    if(auto r=readBytes(b.encryptedDekByKem); r.IsErr()) return Result<WalletEncryptedBlob>::Err(r.Error());
    if(auto r=readBytes(b.pqKemCiphertext); r.IsErr()) return Result<WalletEncryptedBlob>::Err(r.Error());
    if(auto r=readBytes(b.saltPassword); r.IsErr()) return Result<WalletEncryptedBlob>::Err(r.Error());
    if(auto r=readBytes(b.noncePayload); r.IsErr()) return Result<WalletEncryptedBlob>::Err(r.Error());
    if(auto r=readBytes(b.nonceDekPassword); r.IsErr()) return Result<WalletEncryptedBlob>::Err(r.Error());
    if(auto r=readBytes(b.nonceDekKem); r.IsErr()) return Result<WalletEncryptedBlob>::Err(r.Error());
    if(auto r=readBytes(b.tagPayload); r.IsErr()) return Result<WalletEncryptedBlob>::Err(r.Error());
    if(auto r=readBytes(b.tagDekPassword); r.IsErr()) return Result<WalletEncryptedBlob>::Err(r.Error());
    if(auto r=readBytes(b.tagDekKem); r.IsErr()) return Result<WalletEncryptedBlob>::Err(r.Error());
    bool ok=true; std::string err;
    b.walletFingerprint=ReadArray32(d,"wallet fingerprint",ok,err);
    if(!ok) return Result<WalletEncryptedBlob>::Err(err);
    b.kemPublicKeyFingerprint=ReadArray32(d,"KEM public key fingerprint",ok,err);
    if(!ok) return Result<WalletEncryptedBlob>::Err(err);
    b.checksum=ReadArray32(d,"wallet checksum",ok,err);
    if(!ok) return Result<WalletEncryptedBlob>::Err(err);
    if(!d.End()) return Result<WalletEncryptedBlob>::Err("trailing wallet bytes");
    if(ChecksumFor(b)!=b.checksum) return Result<WalletEncryptedBlob>::Err("wallet checksum mismatch");
    return Result<WalletEncryptedBlob>::Ok(std::move(b));
}
}
