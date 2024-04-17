#include<iostream>
#include<windows.h>
#include <random>
#include <chrono>
#include <time.h>    //引入头文件
#include<vector>
using namespace std;
// 定义一个结构体来存储参数
struct ThreadParams {
    int n;
    int m;
    int p;
    double** matrix_1;
    double** matrix_2;
    double** result;
    int row;
    int col;
    int thread_times;
    ThreadParams(int n,int m,int p,double** matrix_1,double** matrix_2,double** result,int row,int col,int thread_times){
        this->n = n;
        this->m = m;
        this->p = p;
        this->matrix_1 = matrix_1;
        this->matrix_2 = matrix_2;
        this->result = result;
        this->row = row;
        this->col = col;
        this->thread_times = thread_times;
    }
};
void init_mat(uniform_real_distribution<double> dis, double** MAT,int rows);
void volient_method1(ThreadParams parama);
void volient_method2(ThreadParams parama);
DWORD WINAPI mat_multiply1(LPVOID lpPrama);
DWORD WINAPI mat_multiply2(LPVOID lpPrama);
void violent_method3(vector<ThreadParams>& vec);
void violent_method4(vector<ThreadParams>& vec);
void volient_method5(ThreadParams parama);
void volient_method6(ThreadParams parama);
void test1(vector<ThreadParams>& vec);
void test2(vector<ThreadParams>& vec);
void init_vector(vector<ThreadParams>& vec,int nums_threads,int rows,int cols,int right_cols,double** matrix_1,double** matrix_2,double** result);
bool check(double** result1, double** result2,int total);
void volient_method_local(ThreadParams parama);