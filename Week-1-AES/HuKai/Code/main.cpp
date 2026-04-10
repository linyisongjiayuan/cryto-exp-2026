#include<iostream>
#include<iomanip>
#include<random>
#include<fstream>
#include<chrono>
#include"aes.h"

using namespace std;

void encryptFile(const char* inputFile, const char* encrypted, word8 key[16]) 
{
    ifstream inFile(inputFile, std::ios::binary);
    ofstream outFile(encrypted, std::ios::binary);

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

    word8 block[16];
    // initialization
    memset(block, 0, 16); 

    while (inFile.read((char*)block, 16)) 
    {
        AES_Encrypt_logexp(block, key);
        outFile.write((char*)block, 16);
    }
    
    if (inFile.gcount() > 0) 
    {
        int remaining = inFile.gcount();
        for(int i = remaining; i < 16; i++) 
            block[i] = 0;
        
        AES_Encrypt_logexp(block, key);
        outFile.write((char*)block, 16);
    }

    inFile.close();
    outFile.close();
}

int main()
{
    random_device rd;
    mt19937 gen( rd() );
    uniform_int_distribution<word8> dis(0, 255);

    initialize_log_exp_tables();

    word8 K[16] = { 0x2b,0x7e,0x15,0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };

    const char* inputFile = "rijdael.pdf";
    const char* encFile = "encrypted.bin";

    long size = 1127384;

    auto start = std::chrono::high_resolution_clock::now();
    for ( int i = 0; i < 100; i++ )
        encryptFile(inputFile, encFile, K);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> encryptTime = end - start;
    double encryptSpeed = size * 100  / (1024.0 * 1024.0) / encryptTime.count(); // MB/s
    cout << encryptSpeed << endl;
}

