# Security model

PostQuantiqueCoin is secure-by-default for production builds: mainnet requires liboqs and `safe_start` rejects dev-only cryptography. Development builds must be configured explicitly with `-DUSE_LIBOQS=OFF -DDEV_ONLY_ALLOW_INSECURE_CRYPTO=ON` and should only run on devnet/regtest.

## Wallet controls implemented

- Wallet creation and unlock no longer use a default password.
- Wallet creation enforces a minimum local password policy.
- Wallet data is separated by network under `<datadir>/<network>/wallets` to prevent accidental mainnet/devnet mixing.

## P2P controls implemented

- Message framing includes magic, network id, command, payload length, and checksum.
- Deserialization rejects unknown commands, oversized payloads, wrong networks, and checksum mismatches.
- Peer manager persists peers, tracks reputation, counts invalid messages, and temporarily bans abusive peers.
- TCP client/server primitives support IPv4/IPv6, `version`/`verack`, and bounded payload reads.

## Remaining release gates

Before real funds or public mainnet use, run an independent cryptographic review, add fuzzing for P2P/wallet/block parsing, and complete long-run multi-node sync/reorg tests.
