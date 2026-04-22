#include<iostream>
#include<iomanip>
#include<cstring>
#include<random>
#include<chrono>
#include<vector>
#include"aes.h"

using namespace std;

std::mt19937 gen(std::random_device{}());

void recover_4th_round_key( word8 K[16] )
{
    word8 P[16];
    word8 CT[256][16];
    

    cout << "Generating 256 plaintext-ciphertext pairs (integral set)..." << endl;

    word8 res = 0;

    for (int v = 0; v < 256; v++)
    {
        memset(P, 0, 16);
        P[0] = v;
        AES_Encrypt_noMC_superbox_rounds( P, K, 4 );
        memcpy( CT[v], P, 16 );
    }
    
    cout << "Finding candidates for the first key byte..." << endl;
    
    for (int guess = 0; guess < 256; guess++)
    {
        word8 xor_sum = 0;
        
        for (int v = 0; v < 256; v++)
        {
            word8 sbox_in = CT[v][0] ^ (word8)guess;
            xor_sum ^= inv_Sbox[sbox_in];
        }

        if ( xor_sum == 0 )
            cout << "Candidate " << hex << guess << endl;
    }
}

int main()
{
    initT();
    uniform_int_distribution<word8> dis(0, 255);
    
    cout << "=== AES 4-Round Key Recovery (No MixColumn) ===" << endl << endl;

    word8 K[16];
    for ( int i = 0; i < 16; i++ )
        K[i] = dis( gen );

    word8 RK[11][16];
    KeyExpansion( K, RK );

    cout << "The key byte at the first position " << RK[4][0] << endl;

    recover_4th_round_key( K );
    
    
    return 0;
}
