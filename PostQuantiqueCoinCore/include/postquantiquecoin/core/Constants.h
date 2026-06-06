#pragma once
#include <cstdint>
#include <string_view>

namespace pqc::constants {
inline constexpr std::string_view PROJECT_NAME = "PostQuantiqueCoin";
inline constexpr std::string_view PROJECT_VERSION = "0.2.0";
inline constexpr std::string_view COIN_SYMBOL = "PQC";
inline constexpr uint8_t COIN_DECIMALS = 8;
inline constexpr uint64_t COIN_ATOM = 100000000ULL;
inline constexpr uint64_t MAX_SUPPLY_COINS = 29000000ULL;
inline constexpr uint64_t MAX_SUPPLY_ATOMS = 2900000000000000ULL;
inline constexpr uint16_t DEFAULT_P2P_PORT = 29444;
inline constexpr uint16_t DEFAULT_RPC_PORT = 29445;
inline constexpr uint64_t TARGET_BLOCK_TIME_SECONDS = 120;
inline constexpr uint32_t DIFFICULTY_ADJUSTMENT_INTERVAL = 60;
inline constexpr uint64_t COINBASE_MATURITY = 100;
inline constexpr uint64_t INITIAL_SUBSIDY_ATOMS = 50ULL * COIN_ATOM;
inline constexpr uint64_t HALVING_INTERVAL = 210000;
inline constexpr size_t MAX_TRANSACTION_BYTES = 100000;
inline constexpr size_t MAX_BLOCK_BYTES = 4000000;
inline constexpr size_t MAX_TX_INPUTS = 1024;
inline constexpr size_t MAX_TX_OUTPUTS = 1024;
inline constexpr uint64_t MAX_FUTURE_BLOCK_TIME_SECONDS = 2 * 60 * 60;
inline constexpr const char* TX_SIGN_DOMAIN = "PQC_TX_SIGN_V1";
inline constexpr const char* BLOCK_HEADER_DOMAIN = "PQC_BLOCK_HEADER_V1";
inline constexpr const char* P2P_HANDSHAKE_DOMAIN = "PQC_P2P_HANDSHAKE_V1";
}
