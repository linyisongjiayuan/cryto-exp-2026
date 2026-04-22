#include <iostream>
#include "SmallScaleAES.h"

uint8_t INV_SBOX[16];

uint8_t RCON[] = {0x1, 0x2, 0x4, 0x8, 0x3, 0x6, 0xC, 0xB, 0x5};

uint8_t sub_nibble(uint8_t nibble) {
    return SBOX[nibble & 0xF];
}

uint8_t inv_sub_nibble(uint8_t nibble) {
    return INV_SBOX[nibble & 0xF];
}

void init_inv_sbox() {
    for (int i = 0; i < 16; i++) {
        INV_SBOX[SBOX[i]] = i;
    }
}

State sub_bytes(const State& state) {
    State result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result[i][j] = sub_nibble(state[i][j]);
        }
    }
    return result;
}

State inv_sub_bytes(const State& state) {
    State result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result[i][j] = inv_sub_nibble(state[i][j]);
        }
    }
    return result;
}

State shift_rows(const State& state) {
    State result = state;
    uint8_t temp1 = result[1][0];
    result[1][0] = result[1][1];
    result[1][1] = result[1][2];
    result[1][2] = result[1][3];
    result[1][3] = temp1;
    uint8_t temp2_0 = result[2][0];
    uint8_t temp2_1 = result[2][1];
    result[2][0] = result[2][2];
    result[2][1] = result[2][3];
    result[2][2] = temp2_0;
    result[2][3] = temp2_1;
    uint8_t temp3 = result[3][3];
    result[3][3] = result[3][2];
    result[3][2] = result[3][1];
    result[3][1] = result[3][0];
    result[3][0] = temp3;
    return result;
}

State inv_shift_rows(const State& state) {
    State result = state;
    uint8_t temp1 = result[1][3];
    result[1][3] = result[1][2];
    result[1][2] = result[1][1];
    result[1][1] = result[1][0];
    result[1][0] = temp1;
    uint8_t temp2_0 = result[2][0];
    uint8_t temp2_1 = result[2][1];
    result[2][0] = result[2][2];
    result[2][1] = result[2][3];
    result[2][2] = temp2_0;
    result[2][3] = temp2_1;
    uint8_t temp3 = result[3][0];
    result[3][0] = result[3][1];
    result[3][1] = result[3][2];
    result[3][2] = result[3][3];
    result[3][3] = temp3;
    return result;
}

State mix_column(const State& state) {
    State result = {};
    
    for (int col = 0; col < 4; col++) {
        uint8_t s0 = state[0][col];
        uint8_t s1 = state[1][col];
        uint8_t s2 = state[2][col];
        uint8_t s3 = state[3][col];
        
        result[0][col] = gf_mul(s0, 0x2) ^ gf_mul(s1, 0x3) ^ s2 ^ s3;
        result[1][col] = s0 ^ gf_mul(s1, 0x2) ^ gf_mul(s2, 0x3) ^ s3;
        result[2][col] = s0 ^ s1 ^ gf_mul(s2, 0x2) ^ gf_mul(s3, 0x3);
        result[3][col] = gf_mul(s0, 0x3) ^ s1 ^ s2 ^ gf_mul(s3, 0x2);
    }
    
    return result;
}

State inv_mix_column(const State& state) {
    State result = {};
    
    for (int col = 0; col < 4; col++) {
        uint8_t s0 = state[0][col];
        uint8_t s1 = state[1][col];
        uint8_t s2 = state[2][col];
        uint8_t s3 = state[3][col];
        
        result[0][col] = gf_mul(s0, 0xE) ^ gf_mul(s1, 0xB) ^ gf_mul(s2, 0xD) ^ gf_mul(s3, 0x9);
        result[1][col] = gf_mul(s0, 0x9) ^ gf_mul(s1, 0xE) ^ gf_mul(s2, 0xB) ^ gf_mul(s3, 0xD);
        result[2][col] = gf_mul(s0, 0xD) ^ gf_mul(s1, 0x9) ^ gf_mul(s2, 0xE) ^ gf_mul(s3, 0xB);
        result[3][col] = gf_mul(s0, 0xB) ^ gf_mul(s1, 0xD) ^ gf_mul(s2, 0x9) ^ gf_mul(s3, 0xE);
    }
    
    return result;
}

State bytes_to_state(const std::array<uint8_t, 8>& data) {
    State state = {};
    
    std::array<uint8_t, 16> nibbles;
    for (int i = 0; i < 8; i++) {
        nibbles[i * 2] = (data[i] >> 4) & 0xF;
        nibbles[i * 2 + 1] = data[i] & 0xF;
    }
    
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            int idx = col * 4 + row;
            state[row][col] = nibbles[idx];
        }
    }
    
    return state;
}

std::array<uint8_t, 8> state_to_bytes(const State& state) {
    std::array<uint8_t, 8> data = {};
    
    for (int col = 0; col < 4; col++) {
        int byte_idx = col * 2;
        data[byte_idx] = (state[0][col] << 4) | state[1][col];
        data[byte_idx + 1] = (state[2][col] << 4) | state[3][col];
    }
    
    return data;
}

std::vector<State> key_expansion(const std::array<uint8_t, 8>& key) {
    std::array<uint8_t, 16> nibbles;
    for (int i = 0; i < 8; i++) {
        nibbles[i * 2] = (key[i] >> 4) & 0xF;
        nibbles[i * 2 + 1] = key[i] & 0xF;
    }
    
    std::vector<std::array<uint8_t, 4>> w;
    for (int i = 0; i < 4; i++) {
        std::array<uint8_t, 4> word;
        for (int j = 0; j < 4; j++) {
            word[j] = nibbles[i * 4 + j];
        }
        w.push_back(word);
    }
    
    for (int round_num = 1; round_num <= NUM_ROUNDS; round_num++) {
        std::array<uint8_t, 4> last = w.back();
        std::array<uint8_t, 4> rotated = {last[1], last[2], last[3], last[0]};
        
        std::array<uint8_t, 4> subbed;
        for (int i = 0; i < 4; i++) {
            subbed[i] = sub_nibble(rotated[i]);
        }
        
        uint8_t rcon = RCON[round_num - 1];
        std::array<uint8_t, 4> new0 = {static_cast<uint8_t>(subbed[0] ^ rcon), subbed[1], subbed[2], subbed[3]};
        
        std::array<uint8_t, 4> new1, new2, new3;
        int w_size = w.size();
        for (int i = 0; i < 4; i++) {
            new1[i] = new0[i] ^ w[w_size - 4][i];
            new2[i] = new1[i] ^ w[w_size - 3][i];
            new3[i] = new2[i] ^ w[w_size - 2][i];
        }
        
        w.push_back(new0);
        w.push_back(new1);
        w.push_back(new2);
        w.push_back(new3);
    }
    
    std::vector<State> round_keys;
    for (int r = 0; r <= NUM_ROUNDS; r++) {
        State key_state = {};
        for (int word_idx = 0; word_idx < 4; word_idx++) {
            std::array<uint8_t, 4>& word = w[r * 4 + word_idx];
            key_state[0][word_idx] = word[0];
            key_state[1][word_idx] = word[1];
            key_state[2][word_idx] = word[2];
            key_state[3][word_idx] = word[3];
        }
        round_keys.push_back(key_state);
    }
    
    return round_keys;
}

State add_round_key(const State& state, const State& round_key) {
    State result = state;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result[i][j] ^= round_key[i][j];
        }
    }
    return result;
}

std::array<uint8_t, 8> encrypt(const std::array<uint8_t, 8>& plaintext, const std::array<uint8_t, 8>& key) {
    std::vector<State> round_keys = key_expansion(key);
    
    State state = bytes_to_state(plaintext);
    state = add_round_key(state, round_keys[0]);
    
    for (int r = 1; r < NUM_ROUNDS; r++) {
        state = sub_bytes(state);
        state = shift_rows(state);
        state = mix_column(state);
        state = add_round_key(state, round_keys[r]);
    }
    
    state = sub_bytes(state);
    state = shift_rows(state);
    state = add_round_key(state, round_keys[NUM_ROUNDS]);
    
    return state_to_bytes(state);
}

std::array<uint8_t, 8> decrypt(const std::array<uint8_t, 8>& ciphertext, const std::array<uint8_t, 8>& key) {
    std::vector<State> round_keys = key_expansion(key);
    
    State state = bytes_to_state(ciphertext);
    state = add_round_key(state, round_keys[NUM_ROUNDS]);
    state = inv_shift_rows(state);
    state = inv_sub_bytes(state);
    
    for (int r = NUM_ROUNDS - 1; r > 0; r--) {
        state = add_round_key(state, round_keys[r]);
        state = inv_mix_column(state);
        state = inv_shift_rows(state);
        state = inv_sub_bytes(state);
    }
    
    state = add_round_key(state, round_keys[0]);
    
    return state_to_bytes(state);
}

std::array<uint8_t, 8> encrypt_rounds(const std::array<uint8_t, 8>& plaintext, const std::array<uint8_t, 8>& key, int rounds) {
    std::vector<State> round_keys = key_expansion(key);
    
    State state = bytes_to_state(plaintext);
    state = add_round_key(state, round_keys[0]);
    
    for (int r = 1; r <= rounds; r++) {
        state = sub_bytes(state);
        state = shift_rows(state);
        state = mix_column(state);
        state = add_round_key(state, round_keys[r]);
    }
    
    return state_to_bytes(state);
}

int main() {
    init_inv_sbox();
    
    std::array<uint8_t, 8> key = {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6};
    std::array<uint8_t, 8> plaintext = {0x32, 0x43, 0xF6, 0xA8, 0x88, 0x5A, 0x30, 0x8D};
    
    auto ciphertext = encrypt(plaintext, key);
    
    std::cout << "Plaintext:  ";
    for (uint8_t b : plaintext) {
        std::cout << std::hex << (int)b;
    }
    std::cout << std::endl;
    
    std::cout << "Ciphertext: ";
    for (uint8_t b : ciphertext) {
        std::cout << std::hex << (int)b;
    }
    std::cout << std::endl;
    
    auto decrypted = decrypt(ciphertext, key);
    
    std::cout << "Decrypted:  ";
    for (uint8_t b : decrypted) {
        std::cout << std::hex << (int)b;
    }
    std::cout << std::endl;
    
    bool ok = (decrypted == plaintext);
    std::cout << (ok ? "OK" : "FAIL") << std::endl;
    
    std::cout << "\n=== 3-Full-Round Differentiator Test ===" << std::endl;
    std::array<uint8_t, 8> base_plaintext = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    State xor_state = {};
    
    for (int i = 0; i < 16; i++) {
        State pt_state = bytes_to_state(base_plaintext);
        pt_state[0][0] = i;
        std::array<uint8_t, 8> pt = state_to_bytes(pt_state);
        
        auto ct = encrypt_rounds(pt, key, 3);
        State ct_state = bytes_to_state(ct);
        
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                xor_state[r][c] ^= ct_state[r][c];
            }
        }
    }
    
    bool xor3_is_zero = true;
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (xor_state[r][c] != 0) xor3_is_zero = false;
        }
    }
    std::cout << "3 Full Rounds XOR: " << (xor3_is_zero ? "0 (PASS)" : "non-zero (FAIL)") << std::endl;
    
    std::cout << "\n=== 2-Full-Round Differentiator Test ===" << std::endl;
    xor_state = {};
    
    for (int i = 0; i < 16; i++) {
        State pt_state = bytes_to_state(base_plaintext);
        pt_state[0][0] = i;
        std::array<uint8_t, 8> pt = state_to_bytes(pt_state);
        
        auto ct = encrypt_rounds(pt, key, 2);
        State ct_state = bytes_to_state(ct);
        
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                xor_state[r][c] ^= ct_state[r][c];
            }
        }
    }
    
    bool xor2_is_zero = true;
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (xor_state[r][c] != 0) xor2_is_zero = false;
        }
    }
    std::cout << "2 Full Rounds XOR: " << (xor2_is_zero ? "0 (PASS)" : "non-zero (FAIL)") << std::endl;
    
    for (int test_rounds = 1; test_rounds <= 4; test_rounds++) {
        State xor_state = {};
        for (int i = 0; i < 16; i++) {
            State pt_state = bytes_to_state(base_plaintext);
            pt_state[0][0] = i;
            std::array<uint8_t, 8> pt = state_to_bytes(pt_state);
            auto ct = encrypt_rounds(pt, key, test_rounds);
            State ct_state = bytes_to_state(ct);
            for (int r = 0; r < 4; r++) {
                for (int c = 0; c < 4; c++) {
                    xor_state[r][c] ^= ct_state[r][c];
                }
            }
        }
        bool is_zero = true;
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                if (xor_state[r][c] != 0) is_zero = false;
            }
        }
        std::cout << test_rounds << " Full Round(s): " << (is_zero ? "XOR=0 PASS" : "XOR!=0") << std::endl;
    }
    
    std::cout << "\n=== 4-Round Diagonal Differentiator Test ===" << std::endl;
    printf("Expected: 16^4 = 65536 plaintexts\n");
    State xor_diag = {};
    int count = 0;
    for (int d0 = 0; d0 < 16; d0++) {
        for (int d1 = 0; d1 < 16; d1++) {
            for (int d2 = 0; d2 < 16; d2++) {
                for (int d3 = 0; d3 < 16; d3++) {
                    State pt_state = {};
                    pt_state[0][0] = d0;
                    pt_state[1][1] = d1;
                    pt_state[2][2] = d2;
                    pt_state[3][3] = d3;
                    std::array<uint8_t, 8> pt = state_to_bytes(pt_state);
                    auto ct = encrypt_rounds(pt, key, 4);
                    State ct_state = bytes_to_state(ct);
                    for (int r = 0; r < 4; r++) {
                        for (int c = 0; c < 4; c++) {
                            xor_diag[r][c] ^= ct_state[r][c];
                        }
                    }
                    count++;
                }
            }
        }
    }
    printf("Tested %d plaintexts\n", count);
    std::cout << "XOR of all ciphertexts (diagonal variation): ";
    bool is_zero_diag = true;
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (xor_diag[r][c] != 0) is_zero_diag = false;
        }
    }
    std::cout << (is_zero_diag ? "0 (PASS)" : "non-zero (FAIL)") << std::endl;
    
    return ok ? 0 : 1;
}
