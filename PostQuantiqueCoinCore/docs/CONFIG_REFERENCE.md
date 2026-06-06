# Configuration Reference

The configuration file is `postquantiquecoin.conf`. It is a versioned `key=value` file.

| Key | Default | Description |
| --- | --- | --- |
| `config_version` | `1` | Configuration schema version. |
| `network` | `mainnet` | One of `mainnet`, `testnet`, `devnet`, `regtest`. |
| `datadir` | `data` | Node data directory. Avoid sharing between networks. |
| `wallet_dir` | `data/wallets` | Wallet directory. |
| `port` | `29444` | P2P TCP port. |
| `listen` | `1` | Whether the node accepts inbound peers. |
| `bind` | `0.0.0.0` | Bind address for inbound P2P. |
| `maxconnections` | `32` | Peer table limit. |
| `connect` | empty | Comma-separated outbound peers. |
| `addnode` | empty | Comma-separated extra peers. |
| `seednode` | empty | Comma-separated seed peers. |
| `require_liboqs` | `1` | Require liboqs cryptography. Must remain enabled for production. |
| `allow_dev_crypto` | `0` | Development-only fallback. Forbidden on mainnet. |
| `safe_start` | `1` | Refuse unsafe network/crypto/config combinations. |

CLI flags override important settings: `--network`, `--datadir`, `--conf`, `--bind`, `--connect`, `--addnode`, `--seednode`, `--maxconnections`, `--json`.
