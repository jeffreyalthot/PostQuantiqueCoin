# Threat Model

## Primary assets

- Consensus integrity and chain selection.
- Wallet private keys and encrypted `wallet.dat` contents.
- P2P session integrity, peer reputation and ban state.
- Disk chainstate, block index and UTXO snapshots.

## Current controls

- Mainnet builds require liboqs unless an explicit development CMake override is used.
- Runtime startup refuses dev-only crypto on mainnet.
- P2P messages enforce magic, network id, known commands, payload limits and checksums.
- Wallet creation rejects implicit/default passwords and validates wallet names to prevent path traversal.
- Configuration validates network names, datadir, port, peer limits and unsafe crypto flags.

## Required before real funds

- Independent cryptographic review.
- Production AEAD/KDF hardening across every supported OS.
- Fuzzing for P2P, wallet, block, transaction and address deserialization.
- Crash-recovery tests for disk writes and chainstate rollback.
- Multi-node adversarial sync tests covering forks, invalid objects and slow peers.
