#include "postquantiquecoin/wallet/WalletDatFormat.h"
#include "postquantiquecoin/core/Serialization.h"
#include "postquantiquecoin/crypto/Hashing.h"
#include <algorithm>
namespace pqc {
namespace {
std::array<uint8_t,32> ChecksumFor(const WalletEncryptedBlob& b){
    std::vector<uint8_t> d{'P','Q','C','W','A','L','L','E','T','_','B','L','O','B','_','V','2'};
    d.push_back(b.networkId); d.push_back(b.kdfAlgorithm); d.push_back(b.aeadAlgorithm); d.push_back(b.pqWrapAlgorithm);
    d.insert(d.end(), b.salt.begin(), b.salt.end()); d.insert(d.end(), b.nonce.begin(), b.nonce.end()); d.insert(d.end(), b.ciphertext.begin(), b.ciphertext.end()); d.insert(d.end(), b.authTag.begin(), b.authTag.end()); d.insert(d.end(), b.pqKemPublicKey.begin(), b.pqKemPublicKey.end()); d.insert(d.end(), b.pqKemCiphertext.begin(), b.pqKemCiphertext.end()); d.insert(d.end(), b.encryptedDekByPassword.begin(), b.encryptedDekByPassword.end()); d.insert(d.end(), b.encryptedDekByKem.begin(), b.encryptedDekByKem.end());
    return Hashing::Sha3_256(d);
}
}
std::vector<uint8_t> WalletDatFormat::AssociatedData(const WalletEncryptedBlob& blob){
    std::vector<uint8_t> aad{'P','Q','C','W','A','L','L','E','T'};
    aad.push_back(static_cast<uint8_t>(blob.version & 0xff)); aad.push_back(blob.networkId); aad.push_back(blob.kdfAlgorithm); aad.push_back(blob.aeadAlgorithm); aad.push_back(blob.pqWrapAlgorithm); return aad;
}
std::vector<uint8_t> WalletDatFormat::Serialize(const WalletEncryptedBlob& in){
    WalletEncryptedBlob b=in; b.checksum=ChecksumFor(b); Serializer s; s.String("PQCWALLET"); s.U32(b.version); s.U32(b.networkId); s.U32(b.kdfAlgorithm); s.U32(b.aeadAlgorithm); s.U32(b.pqWrapAlgorithm); s.U32(b.kdfIterations); s.String(b.kdfName); s.String(b.aeadName); s.String(b.pqKemName); s.Bytes(b.salt); s.Bytes(b.nonce); s.Bytes(b.ciphertext); s.Bytes(b.authTag); s.Bytes(b.pqKemPublicKey); s.Bytes(b.pqKemCiphertext); s.Bytes(b.encryptedDekByPassword); s.Bytes(b.encryptedDekByKem); s.Bytes(std::vector<uint8_t>(b.checksum.begin(),b.checksum.end())); return s.Data();
}
Result<WalletEncryptedBlob> WalletDatFormat::Deserialize(const std::vector<uint8_t>& data){
    Deserializer d(data); WalletEncryptedBlob b; auto m=d.String(); if(m.IsErr()) return Result<WalletEncryptedBlob>::Err(m.Error()); if(m.Value()!="PQCWALLET") return Result<WalletEncryptedBlob>::Err("bad wallet magic");
    auto v=d.U32(); if(v.IsErr()) return Result<WalletEncryptedBlob>::Err(v.Error()); b.version=v.Value(); if(b.version!=2) return Result<WalletEncryptedBlob>::Err("unsupported wallet format version");
    auto nid=d.U32(); if(nid.IsErr()) return Result<WalletEncryptedBlob>::Err(nid.Error()); b.networkId=static_cast<uint8_t>(nid.Value());
    auto kdf=d.U32(); if(kdf.IsErr()) return Result<WalletEncryptedBlob>::Err(kdf.Error()); b.kdfAlgorithm=static_cast<uint8_t>(kdf.Value());
    auto aead=d.U32(); if(aead.IsErr()) return Result<WalletEncryptedBlob>::Err(aead.Error()); b.aeadAlgorithm=static_cast<uint8_t>(aead.Value());
    auto wrap=d.U32(); if(wrap.IsErr()) return Result<WalletEncryptedBlob>::Err(wrap.Error()); b.pqWrapAlgorithm=static_cast<uint8_t>(wrap.Value());
    auto iter=d.U32(); if(iter.IsErr()) return Result<WalletEncryptedBlob>::Err(iter.Error()); b.kdfIterations=iter.Value();
    auto kn=d.String(); if(kn.IsErr()) return Result<WalletEncryptedBlob>::Err(kn.Error()); b.kdfName=kn.Value(); auto an=d.String(); if(an.IsErr()) return Result<WalletEncryptedBlob>::Err(an.Error()); b.aeadName=an.Value(); auto pn=d.String(); if(pn.IsErr()) return Result<WalletEncryptedBlob>::Err(pn.Error()); b.pqKemName=pn.Value();
    auto salt=d.Bytes(); if(salt.IsErr()) return Result<WalletEncryptedBlob>::Err(salt.Error()); b.salt=salt.Value(); auto nonce=d.Bytes(); if(nonce.IsErr()) return Result<WalletEncryptedBlob>::Err(nonce.Error()); b.nonce=nonce.Value(); auto ct=d.Bytes(); if(ct.IsErr()) return Result<WalletEncryptedBlob>::Err(ct.Error()); b.ciphertext=ct.Value(); auto tag=d.Bytes(); if(tag.IsErr()) return Result<WalletEncryptedBlob>::Err(tag.Error()); b.authTag=tag.Value();
    auto pk=d.Bytes(); if(pk.IsErr()) return Result<WalletEncryptedBlob>::Err(pk.Error()); b.pqKemPublicKey=pk.Value(); auto pct=d.Bytes(); if(pct.IsErr()) return Result<WalletEncryptedBlob>::Err(pct.Error()); b.pqKemCiphertext=pct.Value(); auto ep=d.Bytes(); if(ep.IsErr()) return Result<WalletEncryptedBlob>::Err(ep.Error()); b.encryptedDekByPassword=ep.Value(); auto ek=d.Bytes(); if(ek.IsErr()) return Result<WalletEncryptedBlob>::Err(ek.Error()); b.encryptedDekByKem=ek.Value(); auto chk=d.Bytes(); if(chk.IsErr()) return Result<WalletEncryptedBlob>::Err(chk.Error()); if(chk.Value().size()!=32) return Result<WalletEncryptedBlob>::Err("bad wallet checksum length"); std::copy(chk.Value().begin(),chk.Value().end(),b.checksum.begin()); if(!d.End()) return Result<WalletEncryptedBlob>::Err("trailing wallet bytes"); if(ChecksumFor(b)!=b.checksum) return Result<WalletEncryptedBlob>::Err("wallet checksum mismatch"); return Result<WalletEncryptedBlob>::Ok(std::move(b));
}
}
