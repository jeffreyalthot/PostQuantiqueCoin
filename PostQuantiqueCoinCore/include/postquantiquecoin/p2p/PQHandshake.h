#pragma once
#include "postquantiquecoin/crypto/PQCryptoProvider.h"
#include "postquantiquecoin/crypto/SecureBuffer.h"
#include "postquantiquecoin/core/Result.h"
#include <array>
#include <cstdint>
namespace pqc::p2p { struct PQHandshakeState { std::vector<uint8_t> ephemeralKemPublicKey; std::vector<uint8_t> ephemeralKemPrivateKey; std::vector<uint8_t> ciphertext; pqc::SecureBuffer sharedSecret; pqc::SecureBuffer sessionKey; std::array<uint8_t,32> transcriptHash{}; uint64_t timestamp{0}; std::array<uint8_t,16> sessionId{}; }; class PQHandshake { public: static PQHandshakeState CreateClientHello(pqc::PQCryptoProvider& provider); static Result<PQHandshakeState> CreateServerHello(pqc::PQCryptoProvider& provider,const std::vector<uint8_t>& clientKemPublicKey); static Result<void> CompleteClient(PQHandshakeState& state,pqc::PQCryptoProvider& provider,const std::vector<uint8_t>& ciphertext); static std::vector<uint8_t> Confirm(const PQHandshakeState& state); }; }
