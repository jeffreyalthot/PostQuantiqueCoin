#include "postquantiquecoin/crypto/Keccak.h"
#include <algorithm>
#include <array>
#include <cstring>
#include <stdexcept>

namespace pqc {
namespace {
uint64_t Rotl(uint64_t x, unsigned n) { return n == 0 ? x : ((x << n) | (x >> (64U - n))); }

constexpr uint64_t RoundConstants[24] = {
    0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL,
    0x8000000080008000ULL, 0x000000000000808bULL, 0x0000000080000001ULL,
    0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL,
    0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
    0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
    0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
    0x000000000000800aULL, 0x800000008000000aULL, 0x8000000080008081ULL,
    0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
};

constexpr unsigned Rho[25] = {
    0, 1, 62, 28, 27,
    36, 44, 6, 55, 20,
    3, 10, 43, 25, 39,
    41, 45, 15, 21, 8,
    18, 2, 61, 56, 14
};

void KeccakF1600(std::array<uint64_t, 25>& a) {
    for (uint64_t rc : RoundConstants) {
        uint64_t c[5]{};
        uint64_t d[5]{};
        for (int x = 0; x < 5; ++x) c[x] = a[x] ^ a[x + 5] ^ a[x + 10] ^ a[x + 15] ^ a[x + 20];
        for (int x = 0; x < 5; ++x) d[x] = c[(x + 4) % 5] ^ Rotl(c[(x + 1) % 5], 1);
        for (int y = 0; y < 5; ++y) for (int x = 0; x < 5; ++x) a[x + 5 * y] ^= d[x];

        std::array<uint64_t, 25> b{};
        for (int y = 0; y < 5; ++y) {
            for (int x = 0; x < 5; ++x) {
                const int src = x + 5 * y;
                const int nx = y;
                const int ny = (2 * x + 3 * y) % 5;
                b[nx + 5 * ny] = Rotl(a[src], Rho[src]);
            }
        }

        for (int y = 0; y < 5; ++y) {
            for (int x = 0; x < 5; ++x) {
                a[x + 5 * y] = b[x + 5 * y] ^ ((~b[((x + 1) % 5) + 5 * y]) & b[((x + 2) % 5) + 5 * y]);
            }
        }
        a[0] ^= rc;
    }
}

std::vector<uint8_t> Sponge(const std::vector<uint8_t>& input, size_t rateBytes, uint8_t suffix, size_t outputSize) {
    if (rateBytes == 0 || rateBytes > 200) throw std::invalid_argument("invalid Keccak rate");
    std::array<uint64_t, 25> state{};
    size_t offset = 0;
    while (input.size() - offset >= rateBytes) {
        for (size_t i = 0; i < rateBytes; ++i) state[i / 8] ^= static_cast<uint64_t>(input[offset + i]) << (8 * (i % 8));
        KeccakF1600(state);
        offset += rateBytes;
    }

    std::array<uint8_t, 200> block{};
    const size_t remaining = input.size() - offset;
    std::copy(input.begin() + static_cast<std::ptrdiff_t>(offset), input.end(), block.begin());
    block[remaining] ^= suffix;
    block[rateBytes - 1] ^= 0x80;
    for (size_t i = 0; i < rateBytes; ++i) state[i / 8] ^= static_cast<uint64_t>(block[i]) << (8 * (i % 8));
    KeccakF1600(state);

    std::vector<uint8_t> out;
    out.reserve(outputSize);
    while (out.size() < outputSize) {
        for (size_t i = 0; i < rateBytes && out.size() < outputSize; ++i) {
            out.push_back(static_cast<uint8_t>((state[i / 8] >> (8 * (i % 8))) & 0xff));
        }
        if (out.size() < outputSize) KeccakF1600(state);
    }
    return out;
}

void AppendU64Le(std::vector<uint8_t>& out, uint64_t v) {
    for (int i = 0; i < 8; ++i) out.push_back(static_cast<uint8_t>((v >> (8 * i)) & 0xff));
}

void AppendString(std::vector<uint8_t>& out, const std::string& s) {
    AppendU64Le(out, s.size());
    out.insert(out.end(), s.begin(), s.end());
}
}

std::array<uint8_t, 32> Keccak::Sha3_256(const std::vector<uint8_t>& data) {
    auto v = Sponge(data, 136, 0x06, 32);
    std::array<uint8_t, 32> out{};
    std::copy(v.begin(), v.end(), out.begin());
    return out;
}

std::array<uint8_t, 64> Keccak::Sha3_512(const std::vector<uint8_t>& data) {
    auto v = Sponge(data, 72, 0x06, 64);
    std::array<uint8_t, 64> out{};
    std::copy(v.begin(), v.end(), out.begin());
    return out;
}

std::vector<uint8_t> Keccak::Shake128(const std::vector<uint8_t>& data, size_t outputSize) { return Sponge(data, 168, 0x1f, outputSize); }
std::vector<uint8_t> Keccak::Shake256(const std::vector<uint8_t>& data, size_t outputSize) { return Sponge(data, 136, 0x1f, outputSize); }

std::vector<uint8_t> Keccak::Kmac256(const std::vector<uint8_t>& key, const std::vector<uint8_t>& message, const std::string& customization, size_t outputSize) {
    std::vector<uint8_t> framed;
    AppendString(framed, "PQC-KMAC256-V1");
    AppendString(framed, customization);
    AppendU64Le(framed, key.size());
    framed.insert(framed.end(), key.begin(), key.end());
    AppendU64Le(framed, message.size());
    framed.insert(framed.end(), message.begin(), message.end());
    AppendU64Le(framed, outputSize);
    return Shake256(framed, outputSize);
}
}
