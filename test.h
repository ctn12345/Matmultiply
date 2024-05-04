#ifndef TEST
#define TEST
#include "matrix_multiply.h"
using namespace std;

DWORD WINAPI mat_multiply1(LPVOID lpPrama);
DWORD WINAPI mat_multiply2(LPVOID lpPrama);
void test1(vector<ThreadParams>& vec);
void test2(vector<ThreadParams>& vec);
void init_vector(vector<ThreadParams>& vec,int nums_threads,int rows,int cols,int right_cols,double** matrix_1,double** matrix_2,double** result);
#endif