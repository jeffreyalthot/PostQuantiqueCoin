# P2P_PROTOCOL

PostQuantiqueCoin 0.0.9 internal technical note.

## Scope

This document records the versioned binary formats, consensus/wallet/network rules, size limits, and mainnet/devnet/regtest parameters introduced for the 0.0.9 hardening pass.

## Core rules

- Mainnet network id is 1, devnet is 2, and regtest is 3.
- Wallet files use the `PQCWALLET` encrypted blob format with KDF, AEAD, nonce, ciphertext, tag, optional post-quantum wrapping fields, and SHA3-256 checksum.
- Transactions use per-input signatures. Each spend binds the input, referenced output, output set, public key hash, and required signature algorithm.
- Blocks distinguish canonical block id from proof-of-work hash. Block ids link storage and previous-hash references; PoW hashes are compared against compact targets.
- P2P messages use magic bytes, network id, command, payload length, SHA3-256 checksum prefix, and bounded payloads.
- Mainnet forbids legacy Double-SHA256 proof of work. Regtest can enable it only through explicit configuration.

## Size and safety limits

Limits are enforced by the C++ serializers, transaction/block validation, wallet parser, and P2P framing parser. Parsers must reject malformed checksums, wrong network ids, unknown commands, oversized payloads, and trailing bytes.

## Testing expectations

Security tests cover wallet tampering, input-signature isolation, post-quantum PoW determinism, difficulty checks, undo/reorg primitives, P2P ML-KEM handshake, and fuzz-like deserialization of random inputs.
