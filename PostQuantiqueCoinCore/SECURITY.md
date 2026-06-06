# Security Policy

PostQuantiqueCoin is experimental. Do not use it to protect real funds until the project has completed external cryptographic review, hostile-network testing and release hardening.

## Reporting vulnerabilities

Please report suspected vulnerabilities privately to the maintainers. Include reproduction steps, affected commit, platform, build flags and whether liboqs or the development fallback provider was active.

## Secure defaults

- Production builds require liboqs.
- Mainnet refuses development-only crypto.
- Wallet commands do not use a default password.
- Sensitive file names are validated to reduce path traversal risk.
