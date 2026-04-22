#pragma once

#include <array>
#include <vector>
#include <cstdint>

const uint8_t NUM_ROUNDS = 5;

const uint8_t IRREDUCIBLE_POLY = 0x13;

const uint8_t SBOX[16] = {
    0x6, 0xB, 0x5, 0x4, 0x2, 0xE, 0x7, 0xA,
    0x9, 0xD, 0xF, 0xC, 0x3, 0x1, 0x0, 0x8
};

extern uint8_t INV_SBOX[16];

inline uint8_t gf_mul(uint8_t a, uint8_t b) {
    uint8_t result = 0;
    while (b) {
        if (b & 1) {
            result ^= a;
        }
        a = a << 1;
        if (a & 0x10) {
            a ^= IRREDUCIBLE_POLY;
        }
        a &= 0xF;
        b >>= 1;
    }
    return result;
}
uint8_t sub_nibble(uint8_t nibble);
uint8_t inv_sub_nibble(uint8_t nibble);

void init_inv_sbox();

using State = std::array<std::array<uint8_t, 4>, 4>;

State sub_bytes(const State& state);
State inv_sub_bytes(const State& state);
State shift_rows(const State& state);
State inv_shift_rows(const State& state);
State mix_column(const State& state);
State inv_mix_column(const State& state);

State bytes_to_state(const std::array<uint8_t, 8>& data);
std::array<uint8_t, 8> state_to_bytes(const State& state);

std::vector<State> key_expansion(const std::array<uint8_t, 8>& key);
State add_round_key(const State& state, const State& round_key);

std::array<uint8_t, 8> encrypt(const std::array<uint8_t, 8>& plaintext, const std::array<uint8_t, 8>& key);
std::array<uint8_t, 8> decrypt(const std::array<uint8_t, 8>& ciphertext, const std::array<uint8_t, 8>& key);
