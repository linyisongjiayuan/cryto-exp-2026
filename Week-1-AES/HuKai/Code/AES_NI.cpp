#include<iostream>
#include<wmmintrin.h>

using namespace std;

const int ROUND = 10;


void AES_Encrypt_NI( word8 P[16], word8 K[16], word8 C[16] )
{
    __m128i x; // 数据块 (State)
    __m128i rk[ROUND + 1]; // 轮密钥 (Round Keys)

    // 1. 初始化：将输入明文 P 加载到 XMM 寄存器
    x = _mm_loadu_si128((__m128i*)P);

    // 2. 密钥扩展
    // 注意：这里需要调用适配 AES-NI 的密钥扩展函数
    // 该函数会生成 __m128i 格式的轮密钥
    KeyExpansion_NI( K, rk );

    // 3. 初始轮密钥加 (Whitening Key)
    x = _mm_xor_si128(x, rk[0]);

    // 4. 中间轮次 (AES-NI 将 SubBytes, ShiftRow, MixColumn 合并为一条指令)
    // 范围是 1 到 ROUND-1
    for ( int r = 1; r < ROUND; r++ )
    {
        // _mm_aesenc_si128 等同于：SubBytes -> ShiftRows -> MixColumns -> AddRoundKey
        x = _mm_aesenc_si128(x, rk[r]);
    }

    // 5. 最后一轮 (不包含 MixColumn)
    // _mm_aesenclast_si128 等同于：SubBytes -> ShiftRows -> AddRoundKey
    x = _mm_aesenclast_si128(x, rk[ROUND]);

    // 6. 输出密文
    _mm_storeu_si128((__m128i*)C, x);
}

int main()
{
    ;
}
