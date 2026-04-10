#include<iostream>
#include<iomanip>
#include<random>
#include<fstream>
#include<chrono>
#include<cstring>
#include"aes.h"

using namespace std;

const int N = 16 * 10000;

void encryptFile(const char* inputFile, const char* encrypted, word8 key[16]) 
{
    ifstream inFile(inputFile, std::ios::binary);
    ofstream outFile(encrypted, std::ios::binary);

    word8 RK[ROUND+1][16] = { 0 };
    KeyExpansion( key, RK );

    if (!inFile ) 
    {
        std::cerr << "Cannot open " << inputFile << std::endl;
        return;
    }

    if (!outFile ) 
    {
        std::cerr << "Cannot open " << encrypted << std::endl;
        return;
    }

    word8 block[N];
    word8 block1[N];
    // initialization
    memset(block, 0, N); 

    while (inFile.read((char*)block, N)) 
    {
        for ( int j = 0; j < 10000; j++ )
            //AES_Encrypt_NI(block + 16 * j, RK, block1 + 16 * j);
            AES_Encrypt(block + 16 * j, key);

        outFile.write((char*)block, N);
    }
    
    if (inFile.gcount() > 0) 
    {
        int remaining = inFile.gcount();
        for(int i = remaining; i < N; i++) 
            block[i] = 0;
        
        //AES_Encrypt_NI(block, RK, block1);
        AES_Encrypt(block, key);
        outFile.write((char*)block, N);
    }

    inFile.close();
    outFile.close();
}

int main()
{
    random_device rd;
    mt19937 gen( rd() );
    uniform_int_distribution<word8> dis(0, 255);

    //initialize_log_exp_tables();

    //word8 P[16] = { 0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34 };
    word8 K[16] = { 0x2b,0x7e,0x15,0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };

    const char* inputFile = "rijdael.pdf";
    const char* encFile = "encrypted.bin";

    long size = 1127384;

    auto start = std::chrono::high_resolution_clock::now();
    for ( int i = 0; i < 1; i++ )
        encryptFile(inputFile, encFile, K);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> encryptTime = end - start;
    double encryptSpeed = size / (1024.0 * 1024.0) / encryptTime.count(); // MB/s
    cout << encryptSpeed << endl;
}

