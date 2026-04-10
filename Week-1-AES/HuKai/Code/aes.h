
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

void AES_Encrypt( word8 P[16], word8 K[16] );
void initialize_log_exp_tables();
void AES_Encrypt_logexp( word8 P[16], word8 K[16] );

#endif
