#include<iostream>
#include<iomanip>
#include<random>
#include<fstream>
#include<chrono>
#include<cstring>
#include"aes.h"

using namespace std;

using word32 = unsigned int;


const int N = 16 * 10000;

// void encryptFile(const char* inputFile, const char* encrypted, word8 key[16]) 
// {
//     ifstream inFile(inputFile, std::ios::binary);
//     ofstream outFile(encrypted, std::ios::binary);

//     word8 RK[ROUND+1][16] = { 0 };
//     KeyExpansion( key, RK );

//     if (!inFile ) 
//     {
//         std::cerr << "Cannot open " << inputFile << std::endl;
//         return;
//     }

//     if (!outFile ) 
//     {
//         std::cerr << "Cannot open " << encrypted << std::endl;
//         return;
//     }

//     word8 block[N];
//     word8 block1[N];
//     // initialization
//     memset(block, 0, N); 

//     while (inFile.read((char*)block, N)) 
//     {
//         for ( int j = 0; j < 10000; j++ )
//             //AES_Encrypt_NI(block + 16 * j, RK, block1 + 16 * j);
//             AES_Encrypt(block + 16 * j, key);

//         outFile.write((char*)block, N);
//     }
    
//     if (inFile.gcount() > 0) 
//     {
//         int remaining = inFile.gcount();
//         for(int i = remaining; i < N; i++) 
//             block[i] = 0;
        
//         //AES_Encrypt_NI(block, RK, block1);
//         AES_Encrypt(block, key);
//         outFile.write((char*)block, N);
//     }

//     inFile.close();
//     outFile.close();
// }



const int BLOCK_SIZE = 16;

bool encryptFile_normal(const char* inputFile, const char* outputFile, word8 key[16])
{
    ifstream inFile(inputFile, ios::binary);
    ofstream outFile(outputFile, ios::binary);

    if (!inFile)
    {
        cerr << "Cannot open input file: " << inputFile << endl;
        return false;
    }
    if (!outFile)
    {
        cerr << "Cannot open output file: " << outputFile << endl;
        return false;
    }

    word8 block[BLOCK_SIZE];

    while (true)
    {
        inFile.read((char*)block, BLOCK_SIZE);
        streamsize bytesRead = inFile.gcount();

        if (bytesRead == BLOCK_SIZE)
        {
            int nextByte = inFile.peek();
            if (nextByte == EOF)
            {
                AES_Encrypt(block, key);
                outFile.write((char*)block, BLOCK_SIZE);

                word8 padBlock[BLOCK_SIZE];
                memset(padBlock, BLOCK_SIZE, BLOCK_SIZE); // PKCS#7 padding
                AES_Encrypt(padBlock, key);
                outFile.write((char*)padBlock, BLOCK_SIZE);
                break;
            }
            else
            {
                AES_Encrypt(block, key);
                outFile.write((char*)block, BLOCK_SIZE);
            }
        }
        else
        {
            word8 pad = BLOCK_SIZE - (word8)bytesRead;
            for (int i = (int)bytesRead; i < BLOCK_SIZE; i++)
                block[i] = pad;

            AES_Encrypt(block, key);
            outFile.write((char*)block, BLOCK_SIZE);
            break;
        }
    }

    return true;
}

bool decryptFile_normal(const char* inputFile, const char* outputFile, word8 key[16])
{
    ifstream inFile(inputFile, ios::binary);
    ofstream outFile(outputFile, ios::binary);

    if (!inFile)
    {
        cerr << "Cannot open input file: " << inputFile << endl;
        return false;
    }
    if (!outFile)
    {
        cerr << "Cannot open output file: " << outputFile << endl;
        return false;
    }

    inFile.seekg(0, ios::end);
    streamoff fileSize = inFile.tellg();
    inFile.seekg(0, ios::beg);

    if (fileSize <= 0 || fileSize % BLOCK_SIZE != 0)
    {
        cerr << "Encrypted file size is invalid." << endl;
        return false;
    }

    word8 block[BLOCK_SIZE];
    word8 nextBlock[BLOCK_SIZE];

    inFile.read((char*)block, BLOCK_SIZE);

    while (true)
    {
        if (!inFile.read((char*)nextBlock, BLOCK_SIZE))
        {
            AES_Decrypt(block, key);

            word8 pad = block[BLOCK_SIZE - 1];
            if (pad < 1 || pad > BLOCK_SIZE)
            {
                cerr << "Invalid PKCS#7 padding." << endl;
                return false;
            }

            for (int i = BLOCK_SIZE - pad; i < BLOCK_SIZE; i++)
            {
                if (block[i] != pad)
                {
                    cerr << "Invalid PKCS#7 padding." << endl;
                    return false;
                }
            }

            outFile.write((char*)block, BLOCK_SIZE - pad);
            break;
        }
        else
        {
            AES_Decrypt(block, key);
            outFile.write((char*)block, BLOCK_SIZE);
            memcpy(block, nextBlock, BLOCK_SIZE);
        }
    }

    return true;
}

bool encryptFile_tbox(const char* inputFile, const char* outputFile, word8 key[16])
{
    ifstream inFile(inputFile, ios::binary);
    ofstream outFile(outputFile, ios::binary);

    if (!inFile)
    {
        cerr << "Cannot open input file: " << inputFile << endl;
        return false;
    }
    if (!outFile)
    {
        cerr << "Cannot open output file: " << outputFile << endl;
        return false;
    }

    generate_T_table();

    // ★ 密钥准备只做一次
    word8 RK[ROUND + 1][16];
    KeyExpansion(key, RK);

    word8 block[BLOCK_SIZE];

    while (true)
    {
        inFile.read((char*)block, BLOCK_SIZE);
        streamsize bytesRead = inFile.gcount();

        if (bytesRead == BLOCK_SIZE)
        {
            int nextByte = inFile.peek();
            if (nextByte == EOF)
            {
                AES_Tbox_Encrypt_rk(block, RK);
                outFile.write((char*)block, BLOCK_SIZE);

                word8 padBlock[BLOCK_SIZE];
                memset(padBlock, BLOCK_SIZE, BLOCK_SIZE);
                AES_Tbox_Encrypt_rk(padBlock, RK);
                outFile.write((char*)padBlock, BLOCK_SIZE);
                break;
            }
            else
            {
                AES_Tbox_Encrypt_rk(block, RK);
                outFile.write((char*)block, BLOCK_SIZE);
            }
        }
        else
        {
            word8 pad = BLOCK_SIZE - (word8)bytesRead;
            for (int i = (int)bytesRead; i < BLOCK_SIZE; i++)
                block[i] = pad;

            AES_Tbox_Encrypt_rk(block, RK);
            outFile.write((char*)block, BLOCK_SIZE);
            break;
        }
    }

    return true;
}

bool decryptFile_tbox(const char* inputFile, const char* outputFile, word8 key[16])
{
    ifstream inFile(inputFile, ios::binary);
    ofstream outFile(outputFile, ios::binary);

    if (!inFile)
    {
        cerr << "Cannot open input file: " << inputFile << endl;
        return false;
    }
    if (!outFile)
    {
        cerr << "Cannot open output file: " << outputFile << endl;
        return false;
    }

    generate_Inv_T_tables();

    // ★ 密钥准备只做一次：KeyExpansion + 对中间 9 轮做 InvMixColumns
    word8 RK[ROUND + 1][16];
    KeyExpansion(key, RK);
    for (int r = 1; r < ROUND; r++)
        InvmixColumn(RK[r]);

    inFile.seekg(0, ios::end);
    streamoff fileSize = inFile.tellg();
    inFile.seekg(0, ios::beg);

    if (fileSize <= 0 || fileSize % BLOCK_SIZE != 0)
    {
        cerr << "Encrypted file size is invalid." << endl;
        return false;
    }

    word8 block[BLOCK_SIZE];
    word8 nextBlock[BLOCK_SIZE];

    inFile.read((char*)block, BLOCK_SIZE);

    while (true)
    {
        if (!inFile.read((char*)nextBlock, BLOCK_SIZE))
        {
            AES_Tbox_Decrypt_rk(block, RK);

            word8 pad = block[BLOCK_SIZE - 1];
            if (pad < 1 || pad > BLOCK_SIZE)
            {
                cerr << "Invalid PKCS#7 padding." << endl;
                return false;
            }

            for (int i = BLOCK_SIZE - pad; i < BLOCK_SIZE; i++)
            {
                if (block[i] != pad)
                {
                    cerr << "Invalid PKCS#7 padding." << endl;
                    return false;
                }
            }

            outFile.write((char*)block, BLOCK_SIZE - pad);
            break;
        }
        else
        {
            AES_Tbox_Decrypt_rk(block, RK);
            outFile.write((char*)block, BLOCK_SIZE);
            memcpy(block, nextBlock, BLOCK_SIZE);
        }
    }

    return true;
}
// int main()
// {
//     random_device rd;
//     mt19937 gen( rd() );
//     uniform_int_distribution<word8> dis(0, 255);

//     //initialize_log_exp_tables();

//     //word8 P[16] = { 0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34 };
//     word8 K[16] = { 0x2b,0x7e,0x15,0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };

//     const char* inputFile = "rijdael.pdf";
//     const char* encFile = "encrypted.bin";

//     long size = 1127384;

//     auto start = std::chrono::high_resolution_clock::now();
//     for ( int i = 0; i < 1; i++ )
//         encryptFile(inputFile, encFile, K);
//     auto end = std::chrono::high_resolution_clock::now();

//     std::chrono::duration<double> encryptTime = end - start;
//     double encryptSpeed = size / (1024.0 * 1024.0) / encryptTime.count(); // MB/s
//     cout << encryptSpeed << endl;
// }

// int main(){



// word8 K[16] = { 0x2b,0x7e,0x15,0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
// word8 P1[16] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,
//                 0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
// word8 P2[16];
// generate_T_table();
// generate_Inv_T_tables();

// memcpy(P2, P1, 16);

// AES_Encrypt(P1, K);       // 字节级
// AES_Tbox_Encypt(P2, K);   // T 表
// cout << "AES_Encrypt: " << P1 << endl;
// cout << "AES_Tbox_Encypt: " << P2 << endl;
// auto start = std::chrono::high_resolution_clock::now();
// AES_Decrypt(P1,K);
// auto end = std::chrono::high_resolution_clock::now();
// auto decryptTime = end - start;
// auto start2 = std::chrono::high_resolution_clock::now();
// AES_Tbox_Decrypt(P2, K);
// auto end2 = std::chrono::high_resolution_clock::now();
// auto decryptTime2 = end2 - start2;


// cout << "AES_Decrypt: " << P1 << endl;
// cout << "AES_Tbox_Decrypt: " << P2 << endl;
// cout<< "AES_Decrypt time: " << decryptTime.count() <<   endl;
// cout<< "AES_Tbox_Decrypt time: " << decryptTime2.count() <<  endl;
// return 0;
// }
int main()
{
    word8 K[16] = {
        0x2b,0x7e,0x15,0x16,
        0x28,0xae,0xd2,0xa6,
        0xab,0xf7,0x15,0x88,
        0x09,0xcf,0x4f,0x3c
    };

    const char* inputFile = "rijdael.pdf";

    const char* enc_normal = "encrypted_normal.bin";
    const char* dec_normal = "decrypted_normal.pdf";

    const char* enc_tbox = "encrypted_tbox.bin";
    const char* dec_tbox = "decrypted_tbox.pdf";

    // 计算原文件大小
    ifstream fin(inputFile, ios::binary | ios::ate);
    if (!fin)
    {
        cerr << "Cannot open input file: " << inputFile << endl;
        return 1;
    }
    long long fileSize = fin.tellg();
    fin.close();

    double fileSizeMB = fileSize / (1024.0 * 1024.0);

    // =========================
    // 普通 AES 加密测速
    // =========================
    auto start1 = chrono::high_resolution_clock::now();
    bool ok1 = encryptFile_normal(inputFile, enc_normal, K);
    auto end1 = chrono::high_resolution_clock::now();

    if (!ok1)
    {
        cerr << "Normal AES encryption failed." << endl;
        return 1;
    }

    chrono::duration<double> encTime1 = end1 - start1;
    double encSpeed1 = fileSizeMB / encTime1.count();

    // =========================
    // 普通 AES 解密测速
    // =========================
    auto start2 = chrono::high_resolution_clock::now();
    bool ok2 = decryptFile_normal(enc_normal, dec_normal, K);
    auto end2 = chrono::high_resolution_clock::now();

    if (!ok2)
    {
        cerr << "Normal AES decryption failed." << endl;
        return 1;
    }

    chrono::duration<double> decTime1 = end2 - start2;
    double decSpeed1 = fileSizeMB / decTime1.count();

    // =========================
    // T盒 AES 加密测速
    // =========================
    auto start3 = chrono::high_resolution_clock::now();
    bool ok3 = encryptFile_tbox(inputFile, enc_tbox, K);
    auto end3 = chrono::high_resolution_clock::now();

    if (!ok3)
    {
        cerr << "T-box AES encryption failed." << endl;
        return 1;
    }

    chrono::duration<double> encTime2 = end3 - start3;
    double encSpeed2 = fileSizeMB / encTime2.count();

    // =========================
    // T盒 AES 解密测速
    // =========================
    auto start4 = chrono::high_resolution_clock::now();
    bool ok4 = decryptFile_tbox(enc_tbox, dec_tbox, K);
    auto end4 = chrono::high_resolution_clock::now();

    if (!ok4)
    {
        cerr << "T-box AES decryption failed." << endl;
        return 1;
    }

    chrono::duration<double> decTime2 = end4 - start4;
    double decSpeed2 = fileSizeMB / decTime2.count();

    cout << fixed << setprecision(6);

    cout << "File size: " << fileSizeMB << " MB" << endl;
    cout << endl;

    cout << "========== Normal AES ==========" << endl;
    cout << "Encrypt time : " << encTime1.count() << " s" << endl;
    cout << "Encrypt speed: " << encSpeed1 << " MB/s" << endl;
    cout << "Decrypt time : " << decTime1.count() << " s" << endl;
    cout << "Decrypt speed: " << decSpeed1 << " MB/s" << endl;
    cout << endl;

    cout << "========== T-box AES ==========" << endl;
    cout << "Encrypt time : " << encTime2.count() << " s" << endl;
    cout << "Encrypt speed: " << encSpeed2 << " MB/s" << endl;
    cout << "Decrypt time : " << decTime2.count() << " s" << endl;
    cout << "Decrypt speed: " << decSpeed2 << " MB/s" << endl;
    cout << endl;

    return 0;
}