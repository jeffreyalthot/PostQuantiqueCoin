# PostQuantiqueCoin configuration reference

`postquantiquecoin.conf` is a versioned `key=value` file. Command-line arguments and `PQC_NETWORK` / `PQC_DATADIR` override file defaults.

## Core keys

- `config_version=1`: current configuration format.
- `network=mainnet|testnet|devnet|regtest`: selects immutable consensus, network id, default ML-DSA, and ML-KEM parameters.
- `datadir=<path>`: root data directory. Runtime data is separated under `<datadir>/<network>`.
- `safe_start=1`: refuses startup when network, consensus, or crypto provider are inconsistent.

## P2P keys

- `listen=0|1`
- `bind=<ip>`
- `port=<1..65535>`
- `maxconnections=<1..1024>`
- `connect=host:port,host:port`
- `addnode=host:port,host:port`
- `seednode=host:port,host:port`

## Security keys

- `require_liboqs=1`: production networks require liboqs.
- `allow_dev_crypto=0`: must remain disabled for mainnet.
- `allow_legacy_pow=0`: disables legacy PoW escape hatches.
