#include "postquantiquecoin/crypto/PQPrivateKeyRecord.h"
#include "postquantiquecoin/core/Serialization.h"
#include "postquantiquecoin/crypto/Hashing.h"
#include "postquantiquecoin/crypto/SecureBuffer.h"
#include <algorithm>

namespace pqc {
std::vector<uint8_t> PQPrivateKeyRecord::Serialize() const {
    Serializer s;
    s.String("PQC_PRIVATE_KEY_RECORD_V1");
    s.U32(version);
    s.String(signatureAlgorithm);
    s.String(kemAlgorithm);
    s.Bytes(signingPrivateKey);
    s.Bytes(signingPublicKey);
    s.Bytes(kemPrivateKey);
    s.Bytes(kemPublicKey);
    s.U64(createdTimestamp);
    auto fp = ComputeFingerprint();
    s.Bytes(std::vector<uint8_t>(fp.begin(), fp.end()));
    return s.Data();
}

Result<PQPrivateKeyRecord> PQPrivateKeyRecord::Deserialize(const std::vector<uint8_t>& data) {
    Deserializer d(data);
    auto magic = d.String(); if (magic.IsErr()) return Result<PQPrivateKeyRecord>::Err(magic.Error());
    if (magic.Value() != "PQC_PRIVATE_KEY_RECORD_V1") return Result<PQPrivateKeyRecord>::Err("bad private key record magic");
    PQPrivateKeyRecord r;
    auto ver = d.U32(); if (ver.IsErr()) return Result<PQPrivateKeyRecord>::Err(ver.Error()); r.version = ver.Value();
    auto sigAlg = d.String(); if (sigAlg.IsErr()) return Result<PQPrivateKeyRecord>::Err(sigAlg.Error()); r.signatureAlgorithm = sigAlg.Value();
    auto kemAlg = d.String(); if (kemAlg.IsErr()) return Result<PQPrivateKeyRecord>::Err(kemAlg.Error()); r.kemAlgorithm = kemAlg.Value();
    auto sk = d.Bytes(); if (sk.IsErr()) return Result<PQPrivateKeyRecord>::Err(sk.Error()); r.signingPrivateKey = sk.Value();
    auto spk = d.Bytes(); if (spk.IsErr()) return Result<PQPrivateKeyRecord>::Err(spk.Error()); r.signingPublicKey = spk.Value();
    auto kk = d.Bytes(); if (kk.IsErr()) return Result<PQPrivateKeyRecord>::Err(kk.Error()); r.kemPrivateKey = kk.Value();
    auto kpk = d.Bytes(); if (kpk.IsErr()) return Result<PQPrivateKeyRecord>::Err(kpk.Error()); r.kemPublicKey = kpk.Value();
    auto ts = d.U64(); if (ts.IsErr()) return Result<PQPrivateKeyRecord>::Err(ts.Error()); r.createdTimestamp = ts.Value();
    auto fp = d.Bytes(); if (fp.IsErr()) return Result<PQPrivateKeyRecord>::Err(fp.Error());
    if (fp.Value().size() != r.keyFingerprint.size()) return Result<PQPrivateKeyRecord>::Err("bad private key fingerprint length");
    std::copy(fp.Value().begin(), fp.Value().end(), r.keyFingerprint.begin());
    if (r.ComputeFingerprint() != r.keyFingerprint) return Result<PQPrivateKeyRecord>::Err("private key fingerprint mismatch");
    return Result<PQPrivateKeyRecord>::Ok(std::move(r));
}

Result<void> PQPrivateKeyRecord::ValidateSizes(size_t signingPublicKeySize, size_t signingPrivateKeySize, size_t kemPublicKeySize, size_t kemPrivateKeySize) const {
    if (signingPublicKey.size() != signingPublicKeySize) return Result<void>::Err("bad signing public key size");
    if (signingPrivateKey.size() != signingPrivateKeySize) return Result<void>::Err("bad signing private key size");
    if (!kemPublicKey.empty() && kemPublicKey.size() != kemPublicKeySize) return Result<void>::Err("bad KEM public key size");
    if (!kemPrivateKey.empty() && kemPrivateKey.size() != kemPrivateKeySize) return Result<void>::Err("bad KEM private key size");
    return Result<void>::Ok();
}

std::array<uint8_t, 32> PQPrivateKeyRecord::ComputeFingerprint() const {
    std::vector<uint8_t> d{'P','Q','C','_','K','E','Y','_','F','I','N','G','E','R','P','R','I','N','T','_','V','1'};
    d.insert(d.end(), signatureAlgorithm.begin(), signatureAlgorithm.end());
    d.insert(d.end(), signingPublicKey.begin(), signingPublicKey.end());
    d.insert(d.end(), kemAlgorithm.begin(), kemAlgorithm.end());
    d.insert(d.end(), kemPublicKey.begin(), kemPublicKey.end());
    return Hashing::Sha3_256(d);
}

void PQPrivateKeyRecord::WipeSecrets() {
    SecureBuffer signing(signingPrivateKey);
    signing.Wipe();
    std::fill(signingPrivateKey.begin(), signingPrivateKey.end(), 0);
    SecureBuffer kem(kemPrivateKey);
    kem.Wipe();
    std::fill(kemPrivateKey.begin(), kemPrivateKey.end(), 0);
}
PQPrivateKeyRecord::~PQPrivateKeyRecord() { WipeSecrets(); }
}
