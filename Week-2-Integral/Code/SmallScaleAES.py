"""
SmallScaleAES - 64-bit AES-like block cipher
State: 4x4 matrix of 4-bit elements
GF(2^4) with irreducible polynomial x^4 + x + 1 (0x13)
Sbox: [6, B, 5, 4, 2, E, 7, A, 9, D, F, C, 3, 1, 0, 8]
"""

IRREDUCIBLE_POLY = 0x13


def gf_mul(a, b):
    result = 0
    while b:
        if b & 1:
            result ^= a
        high_bit = (a >> 3) & 1
        a = (a << 1) & 0xF
        if high_bit:
            a ^= IRREDUCIBLE_POLY
        b >>= 1
    return result


def gf_inv(x):
    if x == 0:
        return 0
    for i in range(1, 16):
        if gf_mul(i, x) == 1:
            return i
    return 0


SBOX = [0x6, 0xB, 0x5, 0x4, 0x2, 0xE, 0x7, 0xA, 
        0x9, 0xD, 0xF, 0xC, 0x3, 0x1, 0x0, 0x8]

INV_SBOX = [0] * 16
for i, v in enumerate(SBOX):
    INV_SBOX[v] = i


def sub_nibble(nibble, sbox=SBOX):
    return sbox[nibble & 0xF]


def sub_bytes(state):
    for i in range(4):
        for j in range(4):
            state[i][j] = sub_nibble(state[i][j])
    return state


def inv_sub_bytes(state):
    for i in range(4):
        for j in range(4):
            state[i][j] = sub_nibble(state[i][j], INV_SBOX)
    return state


def shift_rows(state):
    new_state = [
        [state[0][0], state[0][1], state[0][2], state[0][3]],
        [state[1][1], state[1][2], state[1][3], state[1][0]],
        [state[2][2], state[2][3], state[2][0], state[2][1]],
        [state[3][3], state[3][0], state[3][1], state[3][2]]
    ]
    return new_state


def inv_shift_rows(state):
    new_state = [
        [state[0][0], state[0][1], state[0][2], state[0][3]],
        [state[1][3], state[1][0], state[1][1], state[1][2]],
        [state[2][2], state[2][3], state[2][0], state[2][1]],
        [state[3][1], state[3][2], state[3][3], state[3][0]]
    ]
    return new_state


MIX_COLUMN_MATRIX = [
    [0x1, 0x4, 0x4, 0x4],
    [0x4, 0x1, 0x4, 0x4],
    [0x4, 0x4, 0x1, 0x4],
    [0x4, 0x4, 0x4, 0x1]
]

INV_MIX_COLUMN_MATRIX = [
    [0xE, 0xB, 0xD, 0x9],
    [0x9, 0xE, 0xB, 0xD],
    [0xD, 0x9, 0xE, 0xB],
    [0xB, 0xD, 0x9, 0xE]
]


def mix_column(state):
    new_state = [[0]*4 for _ in range(4)]
    
    for col in range(4):
        s0 = state[0][col]
        s1 = state[1][col]
        s2 = state[2][col]
        s3 = state[3][col]
        
        new_state[0][col] = gf_mul(s0, 0x2) ^ gf_mul(s1, 0x3) ^ s2 ^ s3
        new_state[1][col] = s0 ^ gf_mul(s1, 0x2) ^ gf_mul(s2, 0x3) ^ s3
        new_state[2][col] = s0 ^ s1 ^ gf_mul(s2, 0x2) ^ gf_mul(s3, 0x3)
        new_state[3][col] = gf_mul(s0, 0x3) ^ s1 ^ s2 ^ gf_mul(s3, 0x2)
    
    return new_state


def inv_mix_column(state):
    new_state = [[0]*4 for _ in range(4)]
    
    for col in range(4):
        s0 = state[0][col]
        s1 = state[1][col]
        s2 = state[2][col]
        s3 = state[3][col]
        
        new_state[0][col] = gf_mul(s0, 0xE) ^ gf_mul(s1, 0xB) ^ gf_mul(s2, 0xD) ^ gf_mul(s3, 0x9)
        new_state[1][col] = gf_mul(s0, 0x9) ^ gf_mul(s1, 0xE) ^ gf_mul(s2, 0xB) ^ gf_mul(s3, 0xD)
        new_state[2][col] = gf_mul(s0, 0xD) ^ gf_mul(s1, 0x9) ^ gf_mul(s2, 0xE) ^ gf_mul(s3, 0xB)
        new_state[3][col] = gf_mul(s0, 0xB) ^ gf_mul(s1, 0xD) ^ gf_mul(s2, 0x9) ^ gf_mul(s3, 0xE)
    
    return new_state


def bytes_to_state(data):
    nibbles = []
    for b in data:
        nibbles.append((b >> 4) & 0xF)
        nibbles.append(b & 0xF)
    
    state = [[0]*4 for _ in range(4)]
    for col in range(4):
        for row in range(4):
            idx = col * 4 + row
            if idx < len(nibbles):
                state[row][col] = nibbles[idx]
    return state


def state_to_bytes(state):
    data = []
    for col in range(4):
        for row in range(0, 4, 2):
            high_nibble = state[row][col] & 0xF
            low_nibble = state[row+1][col] & 0xF
            data.append((high_nibble << 4) | low_nibble)
    return bytes(data)


def print_state(state, label="State"):
    print(f"{label}:")
    for row in state:
        print(" ".join(f"{x:01X}" for x in row))
    print()


RCON = [0x1, 0x2, 0x4, 0x8, 0x3]


def key_expansion(key):
    nibbles = []
    for b in key:
        nibbles.append((b >> 4) & 0xF)
        nibbles.append(b & 0xF)
    
    w = [nibbles[i:i+4] for i in range(0, 16, 4)]
    
    for round_num in range(1, 6):
        last = w[-1]
        rotated = last[1:] + last[:1]
        
        subbed = [sub_nibble(n) for n in rotated]
        
        rcon = RCON[round_num - 1]
        new0 = [(subbed[0] ^ rcon), subbed[1], subbed[2], subbed[3]]
        
        new1 = [new0[0] ^ w[-4][0], new0[1] ^ w[-4][1], new0[2] ^ w[-4][2], new0[3] ^ w[-4][3]]
        new2 = [new1[0] ^ w[-3][0], new1[1] ^ w[-3][1], new1[2] ^ w[-3][2], new1[3] ^ w[-3][3]]
        new3 = [new2[0] ^ w[-2][0], new2[1] ^ w[-2][1], new2[2] ^ w[-2][2], new2[3] ^ w[-2][3]]
        
        w.extend([new0, new1, new2, new3])
    
    round_keys = []
    for i in range(6):
        key_words = w[i*4:(i+1)*4]
        key_bytes = [(key_words[0][0] << 4) | key_words[0][1],
                     (key_words[0][2] << 4) | key_words[0][3],
                     (key_words[1][0] << 4) | key_words[1][1],
                     (key_words[1][2] << 4) | key_words[1][3],
                     (key_words[2][0] << 4) | key_words[2][1],
                     (key_words[2][2] << 4) | key_words[2][3],
                     (key_words[3][0] << 4) | key_words[3][1],
                     (key_words[3][2] << 4) | key_words[3][3]]
        round_keys.append(bytes(key_bytes))
    
    return round_keys


def add_round_key(state, round_key):
    nibbles = []
    for b in round_key:
        nibbles.append((b >> 4) & 0xF)
        nibbles.append(b & 0xF)
    
    idx = 0
    for col in range(4):
        for row in range(4):
            state[row][col] ^= nibbles[idx]
            idx += 1
    return state


def encrypt(plaintext, key):
    round_keys = key_expansion(key)
    state = bytes_to_state(plaintext)
    state = add_round_key(state, round_keys[0])
    
    for r in range(1, 5):
        state = sub_bytes(state)
        state = shift_rows(state)
        state = mix_column(state)
        state = add_round_key(state, round_keys[r])
    
    state = sub_bytes(state)
    state = shift_rows(state)
    state = add_round_key(state, round_keys[5])
    
    return state_to_bytes(state)


def decrypt(ciphertext, key):
    round_keys = key_expansion(key)
    state = bytes_to_state(ciphertext)
    state = add_round_key(state, round_keys[5])
    state = inv_shift_rows(state)
    state = inv_sub_bytes(state)
    
    for r in range(4, 0, -1):
        state = add_round_key(state, round_keys[r])
        state = inv_mix_column(state)
        state = inv_shift_rows(state)
        state = inv_sub_bytes(state)
    
    state = add_round_key(state, round_keys[0])
    return state_to_bytes(state)


if __name__ == "__main__":
    key = bytes([0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6])
    plaintext = bytes([0x32, 0x43, 0xF6, 0xA8, 0x88, 0x5A, 0x30, 0x8D])
    
    ciphertext = encrypt(plaintext, key)
    print(f"Ciphertext: {ciphertext.hex()}")
    
    decrypted = decrypt(ciphertext, key)
    print(f"Decrypted:  {decrypted.hex()}")
    
    if decrypted == plaintext:
        print("OK")
    else:
        print("FAIL")