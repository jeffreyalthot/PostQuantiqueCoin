#include "postquantiquecoin/core/Amount.h"
#include "postquantiquecoin/core/Constants.h"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace pqc {
Result<uint64_t> Amount::FromCoinsString(const std::string& coins) {
    if (coins.empty()) return Result<uint64_t>::Err("empty amount");
    auto dot = coins.find('.');
    std::string whole = dot == std::string::npos ? coins : coins.substr(0, dot);
    std::string frac = dot == std::string::npos ? std::string() : coins.substr(dot + 1);
    if (whole.empty()) whole = "0";
    if (frac.size() > constants::COIN_DECIMALS) return Result<uint64_t>::Err("too many decimals");
    auto digits = [](char c){ return std::isdigit(static_cast<unsigned char>(c)) != 0; };
    if (!std::all_of(whole.begin(), whole.end(), digits) || !std::all_of(frac.begin(), frac.end(), digits)) return Result<uint64_t>::Err("invalid amount");
    uint64_t wholeValue = 0;
    for (char c : whole) {
        uint64_t digit = static_cast<uint64_t>(c - '0');
        if (wholeValue > (UINT64_MAX - digit) / 10ULL) return Result<uint64_t>::Err("amount overflow");
        wholeValue = wholeValue * 10ULL + digit;
    }
    if (wholeValue > constants::MAX_SUPPLY_ATOMS / constants::COIN_ATOM) return Result<uint64_t>::Err("amount exceeds supply");
    uint64_t atoms = wholeValue * constants::COIN_ATOM;
    frac.append(constants::COIN_DECIMALS - frac.size(), '0');
    uint64_t fracValue = 0;
    for (char c : frac) fracValue = fracValue * 10ULL + static_cast<uint64_t>(c - '0');
    if (!Add(atoms, fracValue, atoms)) return Result<uint64_t>::Err("amount overflow");
    if (atoms > constants::MAX_SUPPLY_ATOMS) return Result<uint64_t>::Err("amount exceeds supply");
    return Result<uint64_t>::Ok(atoms);
}
std::string Amount::ToCoinsString(uint64_t atoms) {
    std::ostringstream oss;
    oss << (atoms / constants::COIN_ATOM) << '.' << std::setw(constants::COIN_DECIMALS) << std::setfill('0') << (atoms % constants::COIN_ATOM);
    return oss.str();
}
bool Amount::Add(uint64_t a, uint64_t b, uint64_t& out) { if (UINT64_MAX - a < b) return false; out = a + b; return true; }
bool Amount::Sub(uint64_t a, uint64_t b, uint64_t& out) { if (a < b) return false; out = a - b; return true; }
}
