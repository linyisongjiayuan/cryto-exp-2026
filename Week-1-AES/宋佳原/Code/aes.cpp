#include <iostream>
#include <iomanip>
#include <iostream>
#include <wmmintrin.h>
#include <cstring>
#include "aes.h"
#include <vector>

using namespace std;
using word32 = unsigned int;
word8 log_table[256];
word8 exp_table[512];
word32 T0[256];
word32 T1[256];
word32 T2[256];
word32 T3[256];
word32 Td0[256];
word32 Td1[256];
word32 Td2[256];
word32 Td3[256];

word8 Sbox[256] = {
    // 0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,  // 0
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,  // 1
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,  // 2
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,  // 3
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,  // 4
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,  // 5
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,  // 6
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,  // 7
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,  // 8
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,  // 9
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,  // a
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,  // b
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,  // c
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,  // d
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,  // e
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16}; // f

/*
 * Inverse S-box transformation table
 */
word8 inv_Sbox[256] = {
    // 0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,  // 0
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,  // 1
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,  // 2
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,  // 3
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,  // 4
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,  // 5
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,  // 6
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,  // 7
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,  // 8
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,  // 9
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,  // a
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,  // b
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,  // c
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,  // d
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,  // e
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d}; // f

void SubByte(word8 X[16])
{
    for (int i = 0; i < 16; i++)
        X[i] = Sbox[X[i]];
}

void InvSubByte(word8 X[16])
{
    for (int i = 0; i < 16; i++)
        X[i] = inv_Sbox[X[i]];
}

/*
 * 0  4  8  12
 * 1  5  9  13
 * 2  6  10 14
 * 3  7  11 15
 */
void ShiftRow(word8 X[16])
{
    word8 temp[16];
    memcpy(temp, X, 16);

    X[1] = temp[5];
    X[2] = temp[10];
    X[3] = temp[15];

    X[5] = temp[9];
    X[6] = temp[14];
    X[7] = temp[3];

    X[9] = temp[13];
    X[10] = temp[2];
    X[11] = temp[7];

    X[13] = temp[1];
    X[14] = temp[6];
    X[15] = temp[11];
}

void InvShiftRow(word8 X[16])
{
    word8 temp[16];
    memcpy(temp, X, 16);
    X[1] = temp[13];
    X[5] = temp[1];
    X[9] = temp[5];
    X[13] = temp[9];

    X[2] = temp[10];
    X[6] = temp[14];
    X[10] = temp[2];
    X[14] = temp[6];

    X[3] = temp[7];
    X[7] = temp[11];
    X[11] = temp[15];
    X[15] = temp[3];
}

word8 xtime(word8 x)
{
    if (x & 0b10000000) // 判断最高位是否为1
        return (x << 1) ^ 0x1b;
    else
        return x << 1;
}

word8 mul(word8 x, word8 y)
{
    word8 res = 0;
    word8 A[8] = {x};
    for (int i = 1; i < 8; i++)
        A[i] = xtime(A[i - 1]);
    for (int i = 0; i < 8; i++)
        if ((y >> i) & 1)
            res ^= A[i];
    return res;
}
word8 mul2(word8 x, word8 y)
{
    if (x == 0 || y == 0)
        return 0;
    else
        return exp_table[(log_table[x] + log_table[y]) % 255];
}

void MixColumn(word8 X[16])
{
    /* 2 3 1 1
     * 1 2 3 1
     * 1 1 2 3
     * 3 1 1 2
     */
    for (int i = 0; i < 4; i++)
    {
        word8 col[4];

        for (int j = 0; j < 4; j++)
            col[j] = X[4 * i + j];

        X[0 + 4 * i] = mul(0x2, col[0]) ^ mul(0x3, col[1]) ^ mul(0x1, col[2]) ^ mul(0x1, col[3]);
        X[1 + 4 * i] = mul(0x1, col[0]) ^ mul(0x2, col[1]) ^ mul(0x3, col[2]) ^ mul(0x1, col[3]);
        X[2 + 4 * i] = mul(0x1, col[0]) ^ mul(0x1, col[1]) ^ mul(0x2, col[2]) ^ mul(0x3, col[3]);
        X[3 + 4 * i] = mul(0x3, col[0]) ^ mul(0x1, col[1]) ^ mul(0x1, col[2]) ^ mul(0x2, col[3]);
    };
}

void MixColumn_logexp(word8 X[16])
{
    /* 2 3 1 1
     * 1 2 3 1
     * 1 1 2 3
     * 3 1 1 2
     */
    for (int i = 0; i < 4; i++)
    {
        word8 col[4];

        for (int j = 0; j < 4; j++)
            col[j] = X[4 * i + j];

        X[0 + 4 * i] = mul2(0x2, col[0]) ^ mul2(0x3, col[1]) ^ mul2(0x1, col[2]) ^ mul2(0x1, col[3]);
        X[1 + 4 * i] = mul2(0x1, col[0]) ^ mul2(0x2, col[1]) ^ mul2(0x3, col[2]) ^ mul2(0x1, col[3]);
        X[2 + 4 * i] = mul2(0x1, col[0]) ^ mul2(0x1, col[1]) ^ mul2(0x2, col[2]) ^ mul2(0x3, col[3]);
        X[3 + 4 * i] = mul2(0x3, col[0]) ^ mul2(0x1, col[1]) ^ mul2(0x1, col[2]) ^ mul2(0x2, col[3]);
    };
}

void InvmixColumn(word8 X[16])
{
    /* 0xe 0xb 0xd 0x9
     * 0x9 0xe 0xb 0xd
     * 0xd 0x9 0xe 0xb
     * 0xb 0xd 0x9 0xe
     */
    for (int i = 0; i < 4; i++)
    {
        word8 col[4];

        for (int j = 0; j < 4; j++)
            col[j] = X[4 * i + j];

        X[0 + 4 * i] = mul(0xE, col[0]) ^ mul(0xB, col[1]) ^ mul(0xD, col[2]) ^ mul(0x9, col[3]);
        X[1 + 4 * i] = mul(0x9, col[0]) ^ mul(0xE, col[1]) ^ mul(0xB, col[2]) ^ mul(0xD, col[3]);
        X[2 + 4 * i] = mul(0xD, col[0]) ^ mul(0x9, col[1]) ^ mul(0xE, col[2]) ^ mul(0xB, col[3]);
        X[3 + 4 * i] = mul(0xB, col[0]) ^ mul(0xD, col[1]) ^ mul(0x9, col[2]) ^ mul(0xE, col[3]);
    };
}
void InvMixColumn_logexp(word8 X[16])
{
    /* 0xe 0xb 0xd 0x9
     * 0x9 0xe 0xb 0xd
     * 0xd 0x9 0xe 0xb
     * 0xb 0xd 0x9 0xe
     */
    for (int i = 0; i < 4; i++)
    {
        word8 col[4];

        for (int j = 0; j < 4; j++)
            col[j] = X[4 * i + j];

        X[0 + 4 * i] = mul2(0xE, col[0]) ^ mul2(0xB, col[1]) ^ mul2(0xD, col[2]) ^ mul2(0x9, col[3]);
        X[1 + 4 * i] = mul2(0x9, col[0]) ^ mul2(0xE, col[1]) ^ mul2(0xB, col[2]) ^ mul2(0xD, col[3]);
        X[2 + 4 * i] = mul2(0xD, col[0]) ^ mul2(0x9, col[1]) ^ mul2(0xE, col[2]) ^ mul2(0xB, col[3]);
        X[3 + 4 * i] = mul2(0xB, col[0]) ^ mul2(0xD, col[1]) ^ mul2(0x9, col[2]) ^ mul2(0xE, col[3]);
    };
}
void generate_T_table()
{
    for (int i = 0; i < 256; i++)
    {
        word8 y = Sbox[i];

        T0[i] = ((word32)mul(0x02, y) << 24) ^
                ((word32)mul(0x01, y) << 16) ^
                ((word32)mul(0x01, y) << 8)  ^
                ((word32)mul(0x03, y));

        T1[i] = ((word32)mul(0x03, y) << 24) ^
                ((word32)mul(0x02, y) << 16) ^
                ((word32)mul(0x01, y) << 8)  ^
                ((word32)mul(0x01, y));

        T2[i] = ((word32)mul(0x01, y) << 24) ^
                ((word32)mul(0x03, y) << 16) ^
                ((word32)mul(0x02, y) << 8)  ^
                ((word32)mul(0x01, y));

        T3[i] = ((word32)mul(0x01, y) << 24) ^
                ((word32)mul(0x01, y) << 16) ^
                ((word32)mul(0x03, y) << 8)  ^
                ((word32)mul(0x02, y));
    }
}
void generate_Inv_T_tables()
{
    for (int i = 0; i < 256; i++)
    {
        word8 y = inv_Sbox[i];

        Td0[i] = ((word32)mul(0x0e, y) << 24) ^
                 ((word32)mul(0x09, y) << 16) ^
                 ((word32)mul(0x0d, y) << 8)  ^
                 ((word32)mul(0x0b, y));

        Td1[i] = ((word32)mul(0x0b, y) << 24) ^
                 ((word32)mul(0x0e, y) << 16) ^
                 ((word32)mul(0x09, y) << 8)  ^
                 ((word32)mul(0x0d, y));

        Td2[i] = ((word32)mul(0x0d, y) << 24) ^
                 ((word32)mul(0x0b, y) << 16) ^
                 ((word32)mul(0x0e, y) << 8)  ^
                 ((word32)mul(0x09, y));

        Td3[i] = ((word32)mul(0x09, y) << 24) ^
                 ((word32)mul(0x0d, y) << 16) ^
                 ((word32)mul(0x0b, y) << 8)  ^
                 ((word32)mul(0x0e, y));
    }
}



void AddRoundKey(word8 X[16], word8 K[16])
{
    for (int i = 0; i < 16; i++)
        X[i] ^= K[i];
}

void KeyExpansion(word8 k[16], word8 rk[][16])
{
    word8 RC[] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};

    for (int i = 0; i < 16; i++)
        rk[0][i] = k[i];

    for (int r = 0; r < ROUND; r++)
    {
        word8 col[4];
        col[0] = Sbox[rk[r][13]] ^ RC[r];
        col[1] = Sbox[rk[r][14]];
        col[2] = Sbox[rk[r][15]];
        col[3] = Sbox[rk[r][12]];

        for (int i = 0; i < 4; i++)
        {
            rk[r + 1][i] = col[i] ^ rk[r][i];
            rk[r + 1][4 + i] = rk[r + 1][i] ^ rk[r][4 + i];
            rk[r + 1][8 + i] = rk[r + 1][4 + i] ^ rk[r][8 + i];
            rk[r + 1][12 + i] = rk[r + 1][8 + i] ^ rk[r][12 + i];
        }
    }
  
}
void AES_Tbox_Encypt(word8 P[16], word8 K[16])
{
    word8 RK[ROUND + 1][16];
    word32 s0, s1, s2, s3;
    word32 t0, t1, t2, t3;

    KeyExpansion(K, RK);

    // 如果 main 里没调，这里也可以保证初始化
    

  
    s0 = ((word32)P[0] << 24) | ((word32)P[1] << 16) | ((word32)P[2] << 8) | (word32)P[3];
    s1 = ((word32)P[4] << 24) | ((word32)P[5] << 16) | ((word32)P[6] << 8) | (word32)P[7];
    s2 = ((word32)P[8] << 24) | ((word32)P[9] << 16) | ((word32)P[10] << 8) | (word32)P[11];
    s3 = ((word32)P[12] << 24) | ((word32)P[13] << 16) | ((word32)P[14] << 8) | (word32)P[15];

    // 初始轮密钥
    s0 ^= ((word32)RK[0][0] << 24) | ((word32)RK[0][1] << 16) | ((word32)RK[0][2] << 8) | (word32)RK[0][3];
    s1 ^= ((word32)RK[0][4] << 24) | ((word32)RK[0][5] << 16) | ((word32)RK[0][6] << 8) | (word32)RK[0][7];
    s2 ^= ((word32)RK[0][8] << 24) | ((word32)RK[0][9] << 16) | ((word32)RK[0][10] << 8) | (word32)RK[0][11];
    s3 ^= ((word32)RK[0][12] << 24) | ((word32)RK[0][13] << 16) | ((word32)RK[0][14] << 8) | (word32)RK[0][15];

    // 中间 ROUND-1 轮
    for (int r = 1; r < ROUND; r++)
    {
        word32 rk0 = ((word32)RK[r][0] << 24) | ((word32)RK[r][1] << 16) | ((word32)RK[r][2] << 8) | (word32)RK[r][3];
        word32 rk1 = ((word32)RK[r][4] << 24) | ((word32)RK[r][5] << 16) | ((word32)RK[r][6] << 8) | (word32)RK[r][7];
        word32 rk2 = ((word32)RK[r][8] << 24) | ((word32)RK[r][9] << 16) | ((word32)RK[r][10] << 8) | (word32)RK[r][11];
        word32 rk3 = ((word32)RK[r][12] << 24) | ((word32)RK[r][13] << 16) | ((word32)RK[r][14] << 8) | (word32)RK[r][15];

        t0 = T0[(s0 >> 24) & 0xff] ^ T1[(s1 >> 16) & 0xff] ^ T2[(s2 >> 8) & 0xff] ^ T3[s3 & 0xff] ^ rk0;
        t1 = T0[(s1 >> 24) & 0xff] ^ T1[(s2 >> 16) & 0xff] ^ T2[(s3 >> 8) & 0xff] ^ T3[s0 & 0xff] ^ rk1;
        t2 = T0[(s2 >> 24) & 0xff] ^ T1[(s3 >> 16) & 0xff] ^ T2[(s0 >> 8) & 0xff] ^ T3[s1 & 0xff] ^ rk2;
        t3 = T0[(s3 >> 24) & 0xff] ^ T1[(s0 >> 16) & 0xff] ^ T2[(s1 >> 8) & 0xff] ^ T3[s2 & 0xff] ^ rk3;

        s0 = t0;
        s1 = t1;
        s2 = t2;
        s3 = t3;
    }

    // 最后一轮：只有 SubBytes + ShiftRows + AddRoundKey
    t0 = ((word32)Sbox[(s0 >> 24) & 0xff] << 24) ^
         ((word32)Sbox[(s1 >> 16) & 0xff] << 16) ^
         ((word32)Sbox[(s2 >> 8) & 0xff] << 8) ^
         ((word32)Sbox[s3 & 0xff]);

    t1 = ((word32)Sbox[(s1 >> 24) & 0xff] << 24) ^
         ((word32)Sbox[(s2 >> 16) & 0xff] << 16) ^
         ((word32)Sbox[(s3 >> 8) & 0xff] << 8) ^
         ((word32)Sbox[s0 & 0xff]);

    t2 = ((word32)Sbox[(s2 >> 24) & 0xff] << 24) ^
         ((word32)Sbox[(s3 >> 16) & 0xff] << 16) ^
         ((word32)Sbox[(s0 >> 8) & 0xff] << 8) ^
         ((word32)Sbox[s1 & 0xff]);

    t3 = ((word32)Sbox[(s3 >> 24) & 0xff] << 24) ^
         ((word32)Sbox[(s0 >> 16) & 0xff] << 16) ^
         ((word32)Sbox[(s1 >> 8) & 0xff] << 8) ^
         ((word32)Sbox[s2 & 0xff]);

    t0 ^= ((word32)RK[ROUND][0] << 24) | ((word32)RK[ROUND][1] << 16) | ((word32)RK[ROUND][2] << 8) | (word32)RK[ROUND][3];
    t1 ^= ((word32)RK[ROUND][4] << 24) | ((word32)RK[ROUND][5] << 16) | ((word32)RK[ROUND][6] << 8) | (word32)RK[ROUND][7];
    t2 ^= ((word32)RK[ROUND][8] << 24) | ((word32)RK[ROUND][9] << 16) | ((word32)RK[ROUND][10] << 8) | (word32)RK[ROUND][11];
    t3 ^= ((word32)RK[ROUND][12] << 24) | ((word32)RK[ROUND][13] << 16) | ((word32)RK[ROUND][14] << 8) | (word32)RK[ROUND][15];

    P[0] = (t0 >> 24) & 0xff;  P[1] = (t0 >> 16) & 0xff;  P[2] = (t0 >> 8) & 0xff;  P[3] = t0 & 0xff;
    P[4] = (t1 >> 24) & 0xff;  P[5] = (t1 >> 16) & 0xff;  P[6] = (t1 >> 8) & 0xff;  P[7] = t1 & 0xff;
    P[8] = (t2 >> 24) & 0xff;  P[9] = (t2 >> 16) & 0xff;  P[10] = (t2 >> 8) & 0xff; P[11] = t2 & 0xff;
    P[12] = (t3 >> 24) & 0xff; P[13] = (t3 >> 16) & 0xff; P[14] = (t3 >> 8) & 0xff; P[15] = t3 & 0xff;
}

void AES_Tbox_Decrypt(word8 C[16], word8 K[16])
{
    word8 RK[ROUND + 1][16];
    word32 s0, s1, s2, s3;
    word32 t0, t1, t2, t3;

    KeyExpansion(K, RK);

    // 预处理：对中间 9 轮的轮密钥做一次 InvMixColumns

    for (int r = 1; r < ROUND; r++)
        InvmixColumn(RK[r]);

    // 载入密文
    s0 = ((word32)C[0]  << 24) | ((word32)C[1]  << 16) | ((word32)C[2]  << 8) | (word32)C[3];
    s1 = ((word32)C[4]  << 24) | ((word32)C[5]  << 16) | ((word32)C[6]  << 8) | (word32)C[7];
    s2 = ((word32)C[8]  << 24) | ((word32)C[9]  << 16) | ((word32)C[10] << 8) | (word32)C[11];
    s3 = ((word32)C[12] << 24) | ((word32)C[13] << 16) | ((word32)C[14] << 8) | (word32)C[15];

    // 白化：用 RK[ROUND]，原样异或
    s0 ^= ((word32)RK[ROUND][0]  << 24) | ((word32)RK[ROUND][1]  << 16) | ((word32)RK[ROUND][2]  << 8) | (word32)RK[ROUND][3];
    s1 ^= ((word32)RK[ROUND][4]  << 24) | ((word32)RK[ROUND][5]  << 16) | ((word32)RK[ROUND][6]  << 8) | (word32)RK[ROUND][7];
    s2 ^= ((word32)RK[ROUND][8]  << 24) | ((word32)RK[ROUND][9]  << 16) | ((word32)RK[ROUND][10] << 8) | (word32)RK[ROUND][11];
    s3 ^= ((word32)RK[ROUND][12] << 24) | ((word32)RK[ROUND][13] << 16) | ((word32)RK[ROUND][14] << 8) | (word32)RK[ROUND][15];

    // 中间 ROUND-1 轮：直接用预处理过的 RK[r]，不再 memcpy + InvmixColumn
    for (int r = ROUND - 1; r >= 1; r--)
    {
        word32 rk0 = ((word32)RK[r][0]  << 24) | ((word32)RK[r][1]  << 16) | ((word32)RK[r][2]  << 8) | (word32)RK[r][3];
        word32 rk1 = ((word32)RK[r][4]  << 24) | ((word32)RK[r][5]  << 16) | ((word32)RK[r][6]  << 8) | (word32)RK[r][7];
        word32 rk2 = ((word32)RK[r][8]  << 24) | ((word32)RK[r][9]  << 16) | ((word32)RK[r][10] << 8) | (word32)RK[r][11];
        word32 rk3 = ((word32)RK[r][12] << 24) | ((word32)RK[r][13] << 16) | ((word32)RK[r][14] << 8) | (word32)RK[r][15];

        t0 = Td0[(s0 >> 24) & 0xff] ^ Td1[(s3 >> 16) & 0xff] ^ Td2[(s2 >> 8) & 0xff] ^ Td3[s1 & 0xff] ^ rk0;
        t1 = Td0[(s1 >> 24) & 0xff] ^ Td1[(s0 >> 16) & 0xff] ^ Td2[(s3 >> 8) & 0xff] ^ Td3[s2 & 0xff] ^ rk1;
        t2 = Td0[(s2 >> 24) & 0xff] ^ Td1[(s1 >> 16) & 0xff] ^ Td2[(s0 >> 8) & 0xff] ^ Td3[s3 & 0xff] ^ rk2;
        t3 = Td0[(s3 >> 24) & 0xff] ^ Td1[(s2 >> 16) & 0xff] ^ Td2[(s1 >> 8) & 0xff] ^ Td3[s0 & 0xff] ^ rk3;

        s0 = t0;
        s1 = t1;
        s2 = t2;
        s3 = t3;
    }

    // 最后一轮：InvShiftRows + InvSubBytes + AddRoundKey（无 InvMixColumns）
    // 用 RK[0]，原样异或
    t0 = ((word32)inv_Sbox[(s0 >> 24) & 0xff] << 24)
       ^ ((word32)inv_Sbox[(s3 >> 16) & 0xff] << 16)
       ^ ((word32)inv_Sbox[(s2 >> 8)  & 0xff] << 8)
       ^ ((word32)inv_Sbox[ s1        & 0xff]);
    t1 = ((word32)inv_Sbox[(s1 >> 24) & 0xff] << 24)
       ^ ((word32)inv_Sbox[(s0 >> 16) & 0xff] << 16)
       ^ ((word32)inv_Sbox[(s3 >> 8)  & 0xff] << 8)
       ^ ((word32)inv_Sbox[ s2        & 0xff]);
    t2 = ((word32)inv_Sbox[(s2 >> 24) & 0xff] << 24)
       ^ ((word32)inv_Sbox[(s1 >> 16) & 0xff] << 16)
       ^ ((word32)inv_Sbox[(s0 >> 8)  & 0xff] << 8)
       ^ ((word32)inv_Sbox[ s3        & 0xff]);
    t3 = ((word32)inv_Sbox[(s3 >> 24) & 0xff] << 24)
       ^ ((word32)inv_Sbox[(s2 >> 16) & 0xff] << 16)
       ^ ((word32)inv_Sbox[(s1 >> 8)  & 0xff] << 8)
       ^ ((word32)inv_Sbox[ s0        & 0xff]);

    t0 ^= ((word32)RK[0][0]  << 24) | ((word32)RK[0][1]  << 16) | ((word32)RK[0][2]  << 8) | (word32)RK[0][3];
    t1 ^= ((word32)RK[0][4]  << 24) | ((word32)RK[0][5]  << 16) | ((word32)RK[0][6]  << 8) | (word32)RK[0][7];
    t2 ^= ((word32)RK[0][8]  << 24) | ((word32)RK[0][9]  << 16) | ((word32)RK[0][10] << 8) | (word32)RK[0][11];
    t3 ^= ((word32)RK[0][12] << 24) | ((word32)RK[0][13] << 16) | ((word32)RK[0][14] << 8) | (word32)RK[0][15];

    // 写回 C
    C[0]  = (t0 >> 24) & 0xff; C[1]  = (t0 >> 16) & 0xff; C[2]  = (t0 >> 8) & 0xff; C[3]  = t0 & 0xff;
    C[4]  = (t1 >> 24) & 0xff; C[5]  = (t1 >> 16) & 0xff; C[6]  = (t1 >> 8) & 0xff; C[7]  = t1 & 0xff;
    C[8]  = (t2 >> 24) & 0xff; C[9]  = (t2 >> 16) & 0xff; C[10] = (t2 >> 8) & 0xff; C[11] = t2 & 0xff;
    C[12] = (t3 >> 24) & 0xff; C[13] = (t3 >> 16) & 0xff; C[14] = (t3 >> 8) & 0xff; C[15] = t3 & 0xff;
}




// 加密：接受已展开的轮密钥，不再调用 KeyExpansion
void AES_Tbox_Encrypt_rk(word8 P[16], word8 RK[ROUND+1][16])
{
    word32 t0, t1, t2, t3;
    word32 s0, s1, s2, s3;

    // 载入明文（大端打包）
    s0 = ((word32)P[0]  << 24) | ((word32)P[1]  << 16) | ((word32)P[2]  << 8) | (word32)P[3];
    s1 = ((word32)P[4]  << 24) | ((word32)P[5]  << 16) | ((word32)P[6]  << 8) | (word32)P[7];
    s2 = ((word32)P[8]  << 24) | ((word32)P[9]  << 16) | ((word32)P[10] << 8) | (word32)P[11];
    s3 = ((word32)P[12] << 24) | ((word32)P[13] << 16) | ((word32)P[14] << 8) | (word32)P[15];

    // 白化
    s0 ^= ((word32)RK[0][0]  << 24) | ((word32)RK[0][1]  << 16) | ((word32)RK[0][2]  << 8) | (word32)RK[0][3];
    s1 ^= ((word32)RK[0][4]  << 24) | ((word32)RK[0][5]  << 16) | ((word32)RK[0][6]  << 8) | (word32)RK[0][7];
    s2 ^= ((word32)RK[0][8]  << 24) | ((word32)RK[0][9]  << 16) | ((word32)RK[0][10] << 8) | (word32)RK[0][11];
    s3 ^= ((word32)RK[0][12] << 24) | ((word32)RK[0][13] << 16) | ((word32)RK[0][14] << 8) | (word32)RK[0][15];

    // 中间 ROUND-1 轮
    for (int r = 0; r < ROUND - 1; r++)
    {
        word32 rk0 = ((word32)RK[r+1][0]  << 24) | ((word32)RK[r+1][1]  << 16) | ((word32)RK[r+1][2]  << 8) | (word32)RK[r+1][3];
        word32 rk1 = ((word32)RK[r+1][4]  << 24) | ((word32)RK[r+1][5]  << 16) | ((word32)RK[r+1][6]  << 8) | (word32)RK[r+1][7];
        word32 rk2 = ((word32)RK[r+1][8]  << 24) | ((word32)RK[r+1][9]  << 16) | ((word32)RK[r+1][10] << 8) | (word32)RK[r+1][11];
        word32 rk3 = ((word32)RK[r+1][12] << 24) | ((word32)RK[r+1][13] << 16) | ((word32)RK[r+1][14] << 8) | (word32)RK[r+1][15];

        // 大端下字节位置：行0在最高字节，行3在最低字节
        t0 = T0[(s0 >> 24) & 0xff] ^ T1[(s1 >> 16) & 0xff] ^ T2[(s2 >> 8) & 0xff] ^ T3[s3 & 0xff] ^ rk0;
        t1 = T0[(s1 >> 24) & 0xff] ^ T1[(s2 >> 16) & 0xff] ^ T2[(s3 >> 8) & 0xff] ^ T3[s0 & 0xff] ^ rk1;
        t2 = T0[(s2 >> 24) & 0xff] ^ T1[(s3 >> 16) & 0xff] ^ T2[(s0 >> 8) & 0xff] ^ T3[s1 & 0xff] ^ rk2;
        t3 = T0[(s3 >> 24) & 0xff] ^ T1[(s0 >> 16) & 0xff] ^ T2[(s1 >> 8) & 0xff] ^ T3[s2 & 0xff] ^ rk3;

        s0 = t0; s1 = t1; s2 = t2; s3 = t3;
    }

    // 最后一轮
    t0 = ((word32)Sbox[(s0 >> 24) & 0xff] << 24) | ((word32)Sbox[(s1 >> 16) & 0xff] << 16) | ((word32)Sbox[(s2 >> 8) & 0xff] << 8) | (word32)Sbox[s3 & 0xff];
    t1 = ((word32)Sbox[(s1 >> 24) & 0xff] << 24) | ((word32)Sbox[(s2 >> 16) & 0xff] << 16) | ((word32)Sbox[(s3 >> 8) & 0xff] << 8) | (word32)Sbox[s0 & 0xff];
    t2 = ((word32)Sbox[(s2 >> 24) & 0xff] << 24) | ((word32)Sbox[(s3 >> 16) & 0xff] << 16) | ((word32)Sbox[(s0 >> 8) & 0xff] << 8) | (word32)Sbox[s1 & 0xff];
    t3 = ((word32)Sbox[(s3 >> 24) & 0xff] << 24) | ((word32)Sbox[(s0 >> 16) & 0xff] << 16) | ((word32)Sbox[(s1 >> 8) & 0xff] << 8) | (word32)Sbox[s2 & 0xff];

    t0 ^= ((word32)RK[ROUND][0]  << 24) | ((word32)RK[ROUND][1]  << 16) | ((word32)RK[ROUND][2]  << 8) | (word32)RK[ROUND][3];
    t1 ^= ((word32)RK[ROUND][4]  << 24) | ((word32)RK[ROUND][5]  << 16) | ((word32)RK[ROUND][6]  << 8) | (word32)RK[ROUND][7];
    t2 ^= ((word32)RK[ROUND][8]  << 24) | ((word32)RK[ROUND][9]  << 16) | ((word32)RK[ROUND][10] << 8) | (word32)RK[ROUND][11];
    t3 ^= ((word32)RK[ROUND][12] << 24) | ((word32)RK[ROUND][13] << 16) | ((word32)RK[ROUND][14] << 8) | (word32)RK[ROUND][15];

    // 写回（大端）
    P[0]=(t0>>24)&0xff; P[1]=(t0>>16)&0xff; P[2]=(t0>>8)&0xff; P[3]=t0&0xff;
    P[4]=(t1>>24)&0xff; P[5]=(t1>>16)&0xff; P[6]=(t1>>8)&0xff; P[7]=t1&0xff;
    P[8]=(t2>>24)&0xff; P[9]=(t2>>16)&0xff; P[10]=(t2>>8)&0xff; P[11]=t2&0xff;
    P[12]=(t3>>24)&0xff; P[13]=(t3>>16)&0xff; P[14]=(t3>>8)&0xff; P[15]=t3&0xff;
}

// 解密：接受已预处理的轮密钥（中间轮已 InvMixColumns 过）
void AES_Tbox_Decrypt_rk(word8 C[16], word8 RK[ROUND+1][16])
{
    word32 s0, s1, s2, s3;
    word32 t0, t1, t2, t3;

    s0 = ((word32)C[0]  << 24) | ((word32)C[1]  << 16) | ((word32)C[2]  << 8) | (word32)C[3];
    s1 = ((word32)C[4]  << 24) | ((word32)C[5]  << 16) | ((word32)C[6]  << 8) | (word32)C[7];
    s2 = ((word32)C[8]  << 24) | ((word32)C[9]  << 16) | ((word32)C[10] << 8) | (word32)C[11];
    s3 = ((word32)C[12] << 24) | ((word32)C[13] << 16) | ((word32)C[14] << 8) | (word32)C[15];

    s0 ^= ((word32)RK[ROUND][0]  << 24) | ((word32)RK[ROUND][1]  << 16) | ((word32)RK[ROUND][2]  << 8) | (word32)RK[ROUND][3];
    s1 ^= ((word32)RK[ROUND][4]  << 24) | ((word32)RK[ROUND][5]  << 16) | ((word32)RK[ROUND][6]  << 8) | (word32)RK[ROUND][7];
    s2 ^= ((word32)RK[ROUND][8]  << 24) | ((word32)RK[ROUND][9]  << 16) | ((word32)RK[ROUND][10] << 8) | (word32)RK[ROUND][11];
    s3 ^= ((word32)RK[ROUND][12] << 24) | ((word32)RK[ROUND][13] << 16) | ((word32)RK[ROUND][14] << 8) | (word32)RK[ROUND][15];

    for (int r = ROUND - 1; r >= 1; r--)
    {
        word32 rk0 = ((word32)RK[r][0]  << 24) | ((word32)RK[r][1]  << 16) | ((word32)RK[r][2]  << 8) | (word32)RK[r][3];
        word32 rk1 = ((word32)RK[r][4]  << 24) | ((word32)RK[r][5]  << 16) | ((word32)RK[r][6]  << 8) | (word32)RK[r][7];
        word32 rk2 = ((word32)RK[r][8]  << 24) | ((word32)RK[r][9]  << 16) | ((word32)RK[r][10] << 8) | (word32)RK[r][11];
        word32 rk3 = ((word32)RK[r][12] << 24) | ((word32)RK[r][13] << 16) | ((word32)RK[r][14] << 8) | (word32)RK[r][15];

        t0 = Td0[(s0 >> 24) & 0xff] ^ Td1[(s3 >> 16) & 0xff] ^ Td2[(s2 >> 8) & 0xff] ^ Td3[s1 & 0xff] ^ rk0;
        t1 = Td0[(s1 >> 24) & 0xff] ^ Td1[(s0 >> 16) & 0xff] ^ Td2[(s3 >> 8) & 0xff] ^ Td3[s2 & 0xff] ^ rk1;
        t2 = Td0[(s2 >> 24) & 0xff] ^ Td1[(s1 >> 16) & 0xff] ^ Td2[(s0 >> 8) & 0xff] ^ Td3[s3 & 0xff] ^ rk2;
        t3 = Td0[(s3 >> 24) & 0xff] ^ Td1[(s2 >> 16) & 0xff] ^ Td2[(s1 >> 8) & 0xff] ^ Td3[s0 & 0xff] ^ rk3;

        s0 = t0; s1 = t1; s2 = t2; s3 = t3;
    }

    t0 = ((word32)inv_Sbox[(s0 >> 24) & 0xff] << 24) ^ ((word32)inv_Sbox[(s3 >> 16) & 0xff] << 16) ^ ((word32)inv_Sbox[(s2 >> 8) & 0xff] << 8) ^ ((word32)inv_Sbox[s1 & 0xff]);
    t1 = ((word32)inv_Sbox[(s1 >> 24) & 0xff] << 24) ^ ((word32)inv_Sbox[(s0 >> 16) & 0xff] << 16) ^ ((word32)inv_Sbox[(s3 >> 8) & 0xff] << 8) ^ ((word32)inv_Sbox[s2 & 0xff]);
    t2 = ((word32)inv_Sbox[(s2 >> 24) & 0xff] << 24) ^ ((word32)inv_Sbox[(s1 >> 16) & 0xff] << 16) ^ ((word32)inv_Sbox[(s0 >> 8) & 0xff] << 8) ^ ((word32)inv_Sbox[s3 & 0xff]);
    t3 = ((word32)inv_Sbox[(s3 >> 24) & 0xff] << 24) ^ ((word32)inv_Sbox[(s2 >> 16) & 0xff] << 16) ^ ((word32)inv_Sbox[(s1 >> 8) & 0xff] << 8) ^ ((word32)inv_Sbox[s0 & 0xff]);

    t0 ^= ((word32)RK[0][0]  << 24) | ((word32)RK[0][1]  << 16) | ((word32)RK[0][2]  << 8) | (word32)RK[0][3];
    t1 ^= ((word32)RK[0][4]  << 24) | ((word32)RK[0][5]  << 16) | ((word32)RK[0][6]  << 8) | (word32)RK[0][7];
    t2 ^= ((word32)RK[0][8]  << 24) | ((word32)RK[0][9]  << 16) | ((word32)RK[0][10] << 8) | (word32)RK[0][11];
    t3 ^= ((word32)RK[0][12] << 24) | ((word32)RK[0][13] << 16) | ((word32)RK[0][14] << 8) | (word32)RK[0][15];

    C[0]=(t0>>24)&0xff; C[1]=(t0>>16)&0xff; C[2]=(t0>>8)&0xff; C[3]=t0&0xff;
    C[4]=(t1>>24)&0xff; C[5]=(t1>>16)&0xff; C[6]=(t1>>8)&0xff; C[7]=t1&0xff;
    C[8]=(t2>>24)&0xff; C[9]=(t2>>16)&0xff; C[10]=(t2>>8)&0xff; C[11]=t2&0xff;
    C[12]=(t3>>24)&0xff; C[13]=(t3>>16)&0xff; C[14]=(t3>>8)&0xff; C[15]=t3&0xff;
}

void AES_Encrypt(word8 P[16], word8 K[16])
{
    word8 RK[ROUND + 1][16];

    KeyExpansion(K, RK);

    // whitening key
    AddRoundKey(P, RK[0]);

    // middle ROUND - 1 rounds
    for (int r = 0; r < ROUND - 1; r++)
    {
        SubByte(P);

        ShiftRow(P);

        MixColumn(P);

        AddRoundKey(P, RK[r + 1]);
    }

    // last roundP
    SubByte(P);
    ShiftRow(P);
    AddRoundKey(P, RK[ROUND]);
}

void AES_Decrypt(word8 C[16], word8 K[16])
{
    word8 RK[ROUND + 1][16];

    KeyExpansion(K, RK);

    // whitening key
    AddRoundKey(C, RK[ROUND]);

    // middle ROUND - 1 rounds
    for (int r = ROUND - 1; r > 0; r--)
    {
        InvShiftRow(C);
        InvSubByte(C);
        AddRoundKey(C, RK[r]);
        InvmixColumn(C);
    }

    // last round
    InvShiftRow(C);
    InvSubByte(C);
    AddRoundKey(C, RK[0]);
}

void AES_Encrypt_logexp(word8 P[16], word8 K[16])
{
    word8 RK[ROUND + 1][16];

    KeyExpansion(K, RK);

    // whitening key
    AddRoundKey(P, RK[0]);

    // middle ROUND - 1 rounds
    for (int r = 0; r < ROUND - 1; r++)
    {
        SubByte(P);

        ShiftRow(P);

        MixColumn_logexp(P);

        AddRoundKey(P, RK[r + 1]);
    }

    // last round
    SubByte(P);
    ShiftRow(P);
    AddRoundKey(P, RK[ROUND]);
}

void initialize_log_exp_tables()
{
    word8 g = 3;
    // exp_table[i] = g^i
    exp_table[0] = 1;
    exp_table[1] = g;
    for (int i = 2; i < 512; i++)
    {
        exp_table[i] = mul(g, exp_table[i - 1]);
    }
    // log_table[i]=k, 使得 g^k = i
    log_table[0] = 0; // log(0) 没有定义，但我们暂时设置为0
    for (int i = 0; i < 256; i++)
    {
        log_table[exp_table[i]] = i;
    }
}

void AES_Encrypt_NI(word8 P[16], word8 RK[ROUND + 1][16], word8 C[16])
{
    __m128i x;             // 数据块 (State)
    __m128i rk[ROUND + 1]; // 轮密钥 (Round Keys)
                           //
    for (int i = 0; i < ROUND + 1; i++)
        rk[i] = _mm_loadu_si128((__m128i *)RK[i]);

    // 1. 初始化：将输入明文 P 加载到 XMM 寄存器
    x = _mm_loadu_si128((__m128i *)P);

    // 3. 初始轮密钥加 (Whitening Key)
    x = _mm_xor_si128(x, rk[0]);

    // 4. 中间轮次 (AES-NI 将 SubBytes, ShiftRow, MixColumn 合并为一条指令)
    // 范围是 1 到 ROUND-1
    for (int r = 1; r < ROUND; r++)
    {
        // _mm_aesenc_si128 等同于：SubBytes -> ShiftRows -> MixColumns -> AddRoundKey
        x = _mm_aesenc_si128(x, rk[r]);
    }

    // 5. 最后一轮 (不包含 MixColumn)
    // _mm_aesenclast_si128 等同于：SubBytes -> ShiftRows -> AddRoundKey
    x = _mm_aesenclast_si128(x, rk[ROUND]);

    // 6. 输出密文
    _mm_storeu_si128((__m128i *)C, x);
}
