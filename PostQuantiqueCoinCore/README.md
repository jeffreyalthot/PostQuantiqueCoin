# PostQuantiqueCoinCore

PostQuantiqueCoinCore is a local C++17 proof-of-work blockchain prototype with post-quantum signature provider integration. The monetary supply is fixed at 29,000,000.00000000 PQC and all accounting is performed in 8-decimal atoms.

## Features

- UTXO set with coinbase maturity and double-spend prevention.
- Fee-aware mempool for mining selection.
- Blockchain manager with genesis creation, block validation, disk persistence, and UTXO rebuild.
- CPU proof-of-work miner using compact target bits.
- Encrypted local wallet file format with per-wallet salt and per-key nonce.
- CLI executables for node, wallet, and miner operations.
- Post-quantum cryptography abstraction using liboqs for production builds.

## Security note

Production cryptography requires liboqs and ML-DSA/ML-KEM support. The development fallback provider is intentionally enabled only with `-DDEV_ONLY_ALLOW_INSECURE_CRYPTO=ON` and must not be used for production funds.

## Windows build

```bat
scripts\build_windows.bat
```

The script uses Visual Studio 17 2022 and creates:

- `build\Release\pqc_tests.exe`
- `build\Release\pqc_node.exe`
- `build\Release\pqc_wallet.exe`
- `build\Release\pqc_miner.exe`

For production, install liboqs and configure with `-DUSE_LIBOQS=ON`.

## Tests

```bat
scripts\run_tests.bat
```

Tests use devnet parameters with an easy target while preserving the same monetary rules.

## Node commands

```bat
build\Release\pqc_node.exe status
build\Release\pqc_node.exe printchain
build\Release\pqc_node.exe getblockheight 0
build\Release\pqc_node.exe getblock <hash>
build\Release\pqc_node.exe mempool
build\Release\pqc_node.exe validatechain
build\Release\pqc_node.exe balance <address>
```

## Wallet commands

```bat
build\Release\pqc_wallet.exe create <walletName> [password]
build\Release\pqc_wallet.exe unlock <walletName> [password]
build\Release\pqc_wallet.exe lock <walletName>
build\Release\pqc_wallet.exe newaddress <walletName> <label>
build\Release\pqc_wallet.exe listaddresses <walletName>
build\Release\pqc_wallet.exe balance <walletName>
build\Release\pqc_wallet.exe send <walletName> <toAddress> <amountPQC> <feePQC>
build\Release\pqc_wallet.exe backup <walletName> <path>
build\Release\pqc_wallet.exe rescan <walletName>
```

## Miner command

```bat
build\Release\pqc_miner.exe mine PQC1ADDRESS --threads 4
```

The miner builds a candidate block from mempool transactions, adds coinbase subsidy plus fees, computes the Merkle root, performs CPU PoW, and submits the block to the local chain.
