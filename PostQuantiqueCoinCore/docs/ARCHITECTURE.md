# PostQuantiqueCoin Architecture

PostQuantiqueCoin is split into explicit layers so consensus code remains deterministic and presentation code stays replaceable.

## Layers

- `domain`: immutable blockchain concepts (`Block`, `BlockHeader`, `Transaction`, `UTXO`, addresses, wallet records, peer sessions).
- `core consensus`: block, transaction, PoW, difficulty, UTXO and chain validation rules. These rules are network-versioned and must not depend on CLI, sockets or wall-clock side effects except where consensus explicitly requires timestamps.
- `application/service`: orchestration between CLI/API and domain services. `NodeService` is the first service façade and owns startup validation plus node status composition.
- `infrastructure`: disk, sockets, cryptography providers, logging and platform primitives.
- `presentation`: CLI executables now parse config/network/datadir flags and call application services instead of embedding more business logic.

## Stable interfaces

The `include/postquantiquecoin/interfaces` directory defines the dependency-injection seams used for long-term modularity:

- `IBlockStore`
- `IUtxoStore`
- `IPeerTransport`
- `ICryptoProvider`
- `IWalletStore`
- `IMempoolPolicy`

Production adapters should implement those contracts; tests can use in-memory fakes.

## Network separation

The supported network names are `mainnet`, `testnet`, `devnet` and `regtest`. Startup configuration can be supplied by `postquantiquecoin.conf`, environment-specific datadirs, or CLI flags such as:

```bash
pqc_node --network regtest --datadir ./regtest-data getinfo
pqc_node --conf ./postquantiquecoin.conf getnetworkinfo
```

Mainnet refuses dev-only crypto in CMake and during startup safe-start checks.
