#include<iostream>
#include<iomanip>
#include<cstring>
#include<random>
#include"aes.h"

using namespace std;

std::mt19937 rng(std::random_device{}());

void integral_distinguisher(int rounds)
{
    word8 P[16], C[16], K[16];
    word8 xor_result[16];
    
    for (int i = 0; i < 16; i++)
        xor_result[i] = 0;

    std::uniform_int_distribution<int> dist(0, 255);

    for (int i = 0; i < 16; i++)
        K[i] = dist(rng);

    for (int i = 1; i < 16; i++)
        P[i] = 0x00;

    for (int v = 0; v < 256; v++)
    {
        P[0] = v;
        
        memcpy(C, P, 16);
        
        AES_Encrypt_NI_rounds(C, K, rounds);
        
        for (int i = 0; i < 16; i++)
            xor_result[i] ^= C[i];
    }

    cout << "Rounds = " << rounds << endl;
    cout << "XOR of all ciphertexts: " << endl;
    cout << "[";
    for (int i = 0; i < 16; i++)
    {
        if (i < 15)
            cout << hex << "0x" << int(xor_result[i]) << ", ";
        else
            cout << hex << "0x" << int(xor_result[i]) << "]" << endl;
    }

    bool all_zero = true;
    for (int i = 0; i < 16; i++)
    {
        if (xor_result[i] != 0)
        {
            all_zero = false;
            break;
        }
    }

    if (all_zero)
        cout << "Result: ALL ZERO (integral property holds)" << endl;
    else
        cout << "Result: NOT ALL ZERO (no integral property)" << endl;
    
    cout << dec << endl;
}

void diagonal_integral_distinguisher(int rounds)
{
    word8 P[16], C[16], K[16];
    word8 xor_result[16];
    
    for (int i = 0; i < 16; i++)
        xor_result[i] = 0;

    std::uniform_int_distribution<int> dist(0, 255);
    for (int i = 0; i < 16; i++)
        K[i] = dist(rng);

    cout << "Testing 2^32 plaintexts (diagonal variation)..." << endl;
    
    const long long total = 256LL * 256 * 256 * 256;
    long long count = 0;
    
    for (int d0 = 0; d0 < 256; d0++)
    {
        for (int d1 = 0; d1 < 256; d1++)
        {
            for (int d2 = 0; d2 < 256; d2++)
            {
                for (int d3 = 0; d3 < 256; d3++)
                {
                    memset(P, 0, 16);
                    P[0] = d0;
                    P[5] = d1;
                    P[10] = d2;
                    P[15] = d3;
                    
                    memcpy(C, P, 16);
                    AES_Encrypt_NI_rounds(C, K, rounds);
                    
                    for (int i = 0; i < 16; i++)
                        xor_result[i] ^= C[i];
                    
                    count++;
                    
                    if (count % 10000000 == 0)
                    {
                        double percent = (double)count / total * 100;
                        cout << "\rProgress: " << fixed << setprecision(1) << percent << "% (" << count << "/" << total << ")" << flush;
                    }
                }
            }
        }
    }
    
    cout << "\rProgress: 100% (" << total << "/" << total << ")" << endl;
    cout << "Processed " << count << " plaintexts" << endl;
    cout << "Rounds = " << rounds << endl;
    cout << "XOR of all ciphertexts: " << endl;
    cout << "[";
    for (int i = 0; i < 16; i++)
    {
        if (i < 15)
            cout << hex << "0x" << int(xor_result[i]) << ", ";
        else
            cout << hex << "0x" << int(xor_result[i]) << "]" << endl;
    }

    bool all_zero = true;
    for (int i = 0; i < 16; i++)
    {
        if (xor_result[i] != 0)
        {
            all_zero = false;
            break;
        }
    }

    if (all_zero)
        cout << "Result: ALL ZERO (integral property holds)" << endl;
    else
        cout << "Result: NOT ALL ZERO (no integral property)" << endl;
    
    cout << dec << endl;
}

int main()
{
    initT ();

    cout << "=== AES Integral Distinguisher Test ===" << endl << endl;

    cout << "Test 1: 3-round AES integral distinguisher" << endl;
    cout << "-------------------------------------------" << endl;
    integral_distinguisher(3);

    cout << "Test 2: 10-round AES behavior" << endl;
    cout << "--------------------------------------------" << endl;
    integral_distinguisher(10);

    cout << "Test 3: 4-round diagonal integral distinguisher" << endl;
    cout << "-----------------------------------------------" << endl;
    diagonal_integral_distinguisher(4);

    cout << "Test 4: 10-round diagonal behavior" << endl;
    cout << "-----------------------------------------------" << endl;
    diagonal_integral_distinguisher(10);


    return 0;
}
