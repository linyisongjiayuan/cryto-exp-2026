
#ifndef __AES_H__
#define __AES_H__

#include<iostream>
#include<iomanip>

using namespace std;

using word8 = unsigned char;

const int ROUND = 10;

inline ostream & operator << ( ostream & os, word8 x )  
{
    os << setw(2) << setfill('0') << hex << int( x );
    os << dec;
    return os;
}

inline ostream & operator << ( ostream & os, word8 X[16] )  
{
    for ( int i = 0; i < 16; i++ )
    {
        os << setw(2) << setfill('0') << hex <<  int( X[i] ) << "\t";
    }
    os << dec;

    return os;
}

inline void printA( word8 X[16] )
{
    cout << "{";
    for ( int i = 0; i < 16; i++ )
    {
        if ( i < 15 )
            cout << hex << "0x" << X[i] << ",";
        else
            cout << hex << "0x" << X[i] << "}";
    }
}

void AES_Encrypt( word8 P[16], word8 K[16] );
void AES_Decrypt( word8 C[16], word8 K[16] );
void initialize_log_exp_tables();
void AES_Encrypt_logexp( word8 P[16], word8 K[16] );
void AES_Encrypt_NI( word8 P[16], word8 RK[ROUND+1][16], word8 C[16] );
void KeyExpansion( word8 k[16], word8 rk[][16] );
void AES_Tbox_Encypt( word8 P[16], word8 K[16] );
void AES_Tbox_Decrypt( word8 C[16], word8 K[16] );
void generate_T_table();
void generate_Inv_T_tables();
void AES_Tbox_Encrypt_rk(word8 P[16], word8 RK[ROUND+1][16]);
void AES_Tbox_Decrypt_rk(word8 C[16], word8 RK[ROUND+1][16]);
void InvmixColumn(word8 X[16]);
#endif
