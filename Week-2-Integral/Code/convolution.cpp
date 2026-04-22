#include<iostream>
#include<iomanip>
#include<vector>
#include<cmath>
#include<random>
#include<chrono>

using namespace std;

void fft( std::vector<double>& data ) 
{
    const size_t n = data.size();
    
    // 迭代实现FWT（蝴蝶操作）
    for (size_t len = 1; len < n; len <<= 1) 
    {
        for (size_t i = 0; i < n; i += 2 * len) 
        {
            for (size_t j = i; j < i + len; ++j) 
            {
                const double a = data[j];
                const double b = data[j + len];
                data[j] = a + b;     // 正变换: [a+b, a-b]
                data[j + len] = a - b;
            }
        }
    }
}

/*
void fmt( std::vector<int>& data ) 
{
    // 假设 data.size() 是 2^N
    int n = data.size();
    // 修正：Möbius 变换通常只需要两层循环
    for (int i = 0; (1 << i) <= n; ++i) 
    { // 遍历每一位
        for (int mask = 0; mask < n; ++mask) 
        {
            // 如果第 i 位是 1，则加上第 i 位是 0 的子集的值
            if (mask & (1 << i)) {
                data[mask] += data[mask ^ (1 << i)];
            }
        }
    }
}
*/

void fmt( std::vector<int>& data) 
{
    int size = data.size(); // 2^n
    int n = int( log2( size ) );

    for (int k = 1; k <= n; ++k) {
        int mask = 1 << (k - 1);      
        int block_size = 1 << k; 

        for (int i = 0; i < size; i += block_size) {
            for (int j = 0; j < mask; ++j) {
                int idx_subset = i + j;
                int idx_superset = i + mask + j;
                data[idx_superset] = (data[idx_subset] + data[idx_superset]) % 2;
            }
        }
    }
}

void convolution( vector<double>& a, vector<double>& b)
{
    int n = a.size();

    fft( a );
    fft( b );
    
    for (size_t i = 0; i < n; i++)
        a[i] *= b[i];
    
    fft( a );
    
    for (size_t i = 0; i < n; i++)
        a[i] /= n;
}

void convolution1( vector<int>& a, vector<int>& b)
{
    int n = a.size();

    fmt( a );

    fmt( b );
    
    for (size_t i = 0; i < n; i++)
        a[i] *= b[i];
    
    fmt( a );
}

int main()
{
    random_device rd {};

    mt19937 gen( rd () );

    uniform_int_distribution<int> dis( 0, 1 );

    cout << "=== FFT Test with 2^16 integer vector ===" << endl << endl;
    
    const int N = 1 << 3;
    cout << "Vector size: " << N << " (2^2)" << endl;
    
    vector<double> a(N), b(N);

    for (int i = 0; i < N; i++)
    {
        a[i] = dis( gen );
        b[i] = dis( gen );
    }

    vector<double> fa = a, fb = b;
    
    auto start = chrono::high_resolution_clock::now();
    convolution(a, b);
    auto end = chrono::high_resolution_clock::now();
    
    cout << "Convolution completed in " 
         << chrono::duration_cast<chrono::milliseconds>(end - start).count() 
         << " ms" << endl;
    
    cout << "Result size: " << a.size() << endl;
    for (int i = 0; i < N; i++)
        cout << a[i] << " ";
    cout << endl;
    
    cout << "\n=== Verification ===" << endl;

    for ( int y = 0; y < N; y++ )
    {
        double res = 0;
        for ( int x = 0; x < N; x++ )
            res += fa[x] * fb[x ^ y];
        cout << y << " " << res << endl;
    }

    
}


