# PostQuantiqueCoinCore

PostQuantiqueCoin (PQC) est un noyau blockchain C++17 Proof of Work pour Windows, conçu autour d'un modèle UTXO et de signatures post-quantiques via liboqs.

## Paramètres principaux

- Nom : PostQuantiqueCoin
- Symbole : PQC
- Offre maximale : 29 000 000.00000000 PQC
- Unités minimales : 2 900 000 000 000 000 atoms
- Décimales : 8
- Consensus : Proof of Work uniquement
- Temps cible : 120 secondes
- Récompense initiale : 50 PQC, halving tous les 210 000 blocs

## Compilation Windows

```bat
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

## Compilation locale de développement sans liboqs

Le fournisseur `DevOnlyFallbackCryptoProvider` est non sécurisé et réservé aux tests locaux :

```bat
cmake .. -G "Visual Studio 17 2022" -DUSE_LIBOQS=OFF -DDEV_ONLY_ALLOW_INSECURE_CRYPTO=ON
cmake --build . --config Debug
ctest -C Debug --output-on-failure
```

En production, compilez avec `USE_LIBOQS=ON` afin d'utiliser ML-DSA et ML-KEM.
