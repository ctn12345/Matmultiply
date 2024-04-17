#include"matrix_multiply.h"
#include<iostream>
#include<windows.h>
#include <random>
#include <chrono>
#include <time.h>    //引入头文件
#include<vector>
using namespace std;
void init_mat(uniform_real_distribution<double> dis, double** MAT,int rows){
    random_device rd;
    mt19937 gen(rd());
     for (int i = 0; i < rows; ++i) {
        MAT[i] = new double[rows];
    }

    // 随机初始化矩阵
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < rows; ++j) {
            MAT[i][j] = dis(gen);
        }
    }
}


// 计算是先计算result数组的每一列
void volient_method1(ThreadParams parama){
    double** result = parama.result;
    double** matrix_1 = parama.matrix_1;
    double** matrix_2 = parama.matrix_2;
    // n left_row  p left_col    m right_col
    int n = parama.n,p = parama.p,m=parama.m;
    clock_t start,end;//定义clock_t变量
    start = clock();//开始时间
    for(int i = 0;i < n;++i){
        for(int j = 0;j < p;j++){
            result[i][j] = 0.0;
            for(int k = 0;k<m;++k){
                result[i][j] += matrix_1[i][k] * matrix_2[k][j];
            }
        }
    }
    end = clock();     //结束时间
    cout << "暴力求法串行1  ---  " << double(end-start)/CLOCKS_PER_SEC << endl;
}

void volient_method2(ThreadParams parama){
    double** result = parama.result;
    double** matrix_1 = parama.matrix_1;
    double** matrix_2 = parama.matrix_2;
    int n = parama.n,p = parama.p,m=parama.m;
    clock_t start,end;//定义clock_t变量
    start = clock();//开始时间
    // n left_row  p left_col    m right_col
    for(int i = 0;i < n;++i){
        for(int j = 0;j < m;j++){
            // result[i][j] = 0.0;
            for(int k = 0;k< p;++k){
                result[i][k] += matrix_1[i][j] * matrix_2[j][k];
            }
        }
    }
    end = clock();     //结束时间
    cout << "暴力求法串行2  ---  " << double(end-start)/CLOCKS_PER_SEC << endl;
}


void volient_method_local(ThreadParams parama){
    double** result = parama.result;
    double** matrix_1 = parama.matrix_1;
    double** matrix_2 = parama.matrix_2;
    int n = parama.n,p = parama.p,m=parama.m;
    clock_t start,end;//定义clock_t变量
    start = clock();//开始时间
    // n left_row  p left_col    m right_col
    for(int i = 0;i < n;++i){
        for(int j = 0;j < m;j++){
            // result[i][j] = 0.0;
            double temp = matrix_1[i][j];
            for(int k = 0;k< p;++k){
                result[i][k] += temp * matrix_2[j][k];
            }
        }
    }
    end = clock();     //结束时间
    cout << "暴力求法串行local variable  ---  " << double(end-start)/CLOCKS_PER_SEC << endl;
}




DWORD WINAPI mat_multiply1(LPVOID lpPrama){

    ThreadParams* parama = (ThreadParams*) lpPrama;
    double** matrix_1 =parama ->matrix_1;
    double** matrix_2 = parama->matrix_2;
    double** result = parama->result;
    
    int row = parama->row;
    int col = parama->col;
    int m = parama->m;
    int n = parama->n;
    int p = parama->p;


    int times = parama->thread_times;
    // 分成四个子任务
    int x_times = 0;
    for(;x_times*x_times<times;++x_times);
    // cout << "x_times   " << x_times << endl;
    int start_x = parama->m * row / x_times;
    int start_y = parama->n * col / x_times;
    int x= 0,y=0;

    for(int left_row = 0;left_row < m / x_times;++left_row){
        x = left_row + start_x;
        for(int right_col = 0;right_col < p/x_times;right_col++){
            y = right_col + start_y;
           //  result[x][y] = 0;
            // cout << n << endl;
            for(int left_col = 0;left_col< n; ++left_col){
                // cout << x << "  "<< y  << "   " << left_col<< endl;
                result[x][y] += matrix_1[x][left_col] * matrix_2[left_col][y];
            }
        }
    }
    return 0;
}


DWORD WINAPI mat_multiply2(LPVOID lpPrama){
    ThreadParams* parama = (ThreadParams*) lpPrama;
    double** matrix_1 =parama ->matrix_1;
    double** matrix_2 = parama->matrix_2;
    double** result = parama->result;

    int row = parama->row;
    int col = parama->col;
    int m = parama->m;
    int n = parama->n;
    int p = parama->p;

    int times = parama->thread_times;
    // 分成x个子任务
    int x_times = 0;
    for(;x_times*x_times<times;++x_times);
    // times是线程数量，但是它的x索引和y索引有差别
    int start_x = parama->m * row / x_times;
    int start_y = parama->n * col / x_times;
    int x= 0,y=0;

    for(int left_row = 0;left_row < m / x_times; ++left_row){
        x = left_row + start_x;
        for(int left_col = 0; left_col < n; ++left_col){
            // temp 会被编译器优化到寄存器上
            int temp = matrix_1[x][left_col];
            for(int right_col = 0;right_col < p / x_times;++right_col ){
                y = right_col + start_y;
                // cout << x << "  " << y << "  " << left_col << "  " << endl;
                result[x][y] += temp * matrix_2[left_col][y];
            }
        }
    }
    return 0;
}


void violent_method3(vector<ThreadParams>& vec){
    clock_t start,end;//定义clock_t变量
    start = clock();//开始时间
    for(int i = 0;i < vec.size(); ++i){
        ThreadParams* parama = &vec[i];

        double** matrix_1 =parama ->matrix_1;
        double** matrix_2 = parama->matrix_2;
        double** result = parama->result;
        
        int row = parama->row;
        int col = parama->col;
        int m = parama->m;
        int n = parama->n;
        int p = parama->p;


        int times = parama->thread_times;
        // 分成四个子任务
        int start_x = parama->m * row / times;
        int start_y = parama->n * col / times;
        int x= 0,y=0;

        for(int left_row = 0;left_row < m / times;++left_row){
            x = left_row + start_x;
            for(int right_col = 0;right_col < p/times;right_col++){
                result[left_row][right_col] = 0;
                y = right_col + start_y;
                // cout << n << endl;
                for(int left_col = 0;left_col< n;++left_col){
                    // cout << x << "  "<< y  << "   " << left_col<< endl;
                    result[x][y] += matrix_1[x][left_col] * matrix_2[left_col][y];
                }
            }
        }
    }
    end = clock();     //结束时间
    cout << "暴力查找的时间3clock  ---  " << double(end-start)/CLOCKS_PER_SEC << endl; 
}

void violent_method4(vector<ThreadParams>& vec){
    clock_t start,end;//定义clock_t变量
    start = clock();//开始时间
    for(int i = 0;i < vec.size(); ++i){
        ThreadParams* parama = &vec[i];
        double** matrix_1 =parama ->matrix_1;
        double** matrix_2 = parama->matrix_2;
        double** result = parama->result;

        int row = parama->row;
        int col = parama->col;
        int m = parama->m;
        int n = parama->n;
        int p = parama->p;

        int times = parama->thread_times;
        // 分成四个子任务
        int x_times = 0;
        for(;x_times*x_times<times;++x_times);
        int start_x = parama->m * row / x_times;
        int start_y = parama->n * col / x_times;
        int x= 0,y=0;

        for(int left_row = 0;left_row < m / x_times; ++left_row){
            x = left_row + start_x;
            for(int left_col = 0; left_col < n; ++left_col){
                for(int right_col = 0;right_col < p / x_times;++right_col ){
                    y = right_col + start_y;
                    // cout << x << "  " << y << "  " << left_col << "  " << endl;
                    result[x][y] = matrix_1[x][left_col] * matrix_2[left_col][y];
                }

            }
    }
    }
    end = clock();     //结束时间
    cout << "暴力查找的时间4clock  ---  " << double(end-start)/CLOCKS_PER_SEC << endl; 
}


void volient_method5(ThreadParams parama){
    double** result = parama.result;
    double** matrix_1 = parama.matrix_1;
    double** matrix_2 = parama.matrix_2;
    // n left_row  p left_col    m right_col
    int n = parama.n,p = parama.p,m=parama.m;
    clock_t start,end;//定义clock_t变量
    start = clock();//开始时间
    int block_nums = 64;
    for(int row = 0; row < n; ++row){
        for(int col = 0;col < m;col+=block_nums)
        {
            for(int k = 0;k < p;++k){
                for(int time=0;time < block_nums;++time)
                result[row][col+time] += matrix_1[row][k] * matrix_2[k][col+time];
               
            }
        }
    }
    end = clock();
    cout << "暴力查找的时间5clock  ---  " << double(end-start)/CLOCKS_PER_SEC << endl; 



}

void volient_method6(ThreadParams parama){
    double** result = parama.result;
    double** matrix_1 = parama.matrix_1;
    double** matrix_2 = parama.matrix_2;
    // n left_row  p left_col    m right_col
    int n = parama.n,p = parama.p,m=parama.m;
    clock_t start,end;//定义clock_t变量
    start = clock();//开始时间
    int block_nums = 2;
    for(int row = 0; row < n; ++row){
       for(int k = 0;k<p;++k){
        for(int col = 0;col < m;col+=block_nums){
            for(int time=0;time < block_nums;++time)
                result[row][col+time] += matrix_1[row][k] * matrix_2[k][col+time];
        }
       }
    }
    end = clock();
    cout << "暴力查找的时间6clock  ---  " << double(end-start)/CLOCKS_PER_SEC << endl; 

}

void test1(vector<ThreadParams>& vec){
     // 矩阵运算
    HANDLE* handle_array = new HANDLE[vec.size()];
    DWORD ThreadId;
    clock_t start,end;//定义clock_t变量
    start = clock();//开始时间
    for(int i = 0;i < vec.size();++i){
        HANDLE hThread;
        hThread = CreateThread(
                    NULL,                   // default security attributes
                    0,                      // default stack size
                    mat_multiply1,         // thread function name
                    &vec[i],                // argument to thread function o
                    0,                      // default creation flags
                    &ThreadId);             // returns the thread identifier
        handle_array[i] = hThread;
    }

    WaitForMultipleObjects(vec.size(), handle_array, TRUE, INFINITE);

    // 关闭线程句柄
    for(int i = 0;i < vec.size();++i){
        CloseHandle(handle_array[i]);
    }

    end = clock();     //结束时间
    cout << "线程数:"<< vec[0].thread_times << " 需要的时间并发1clock  ---  " << double(end-start)/CLOCKS_PER_SEC << endl;
    // time1 = double(end-start)/CLOCKS_PER_SEC;

}


void test2(vector<ThreadParams>& vec){
     // 矩阵运算
    HANDLE* handle_array = new HANDLE[vec.size()];
    DWORD ThreadId;
    clock_t start,end;//定义clock_t变量
    start = clock();//开始时间
    for(int i = 0;i < vec.size();++i){
        HANDLE hThread;
        hThread = CreateThread(
                    NULL,                   // default security attributes
                    0,                      // default stack size
                    mat_multiply2,         // thread function name
                    &vec[i],                // argument to thread function o
                    0,                      // default creation flags
                    &ThreadId);             // returns the thread identifier
        handle_array[i] = hThread;
    }

    WaitForMultipleObjects(vec.size(), handle_array, TRUE, INFINITE);

    // 关闭线程句柄
    for(int i = 0;i < vec.size();++i){
        CloseHandle(handle_array[i]);
    }

    end = clock();     //结束时间
    cout << "线程数:"<< vec[0].thread_times <<" 需要的时间并发2clock  ---  " << double(end-start)/CLOCKS_PER_SEC << endl;
   // time2 = double(end-start)/CLOCKS_PER_SEC;
}


void init_vector(vector<ThreadParams>& vec,int nums_threads,int rows,int cols,int right_cols,double** matrix_1,double** matrix_2,double** result){
    for(int i = 0; i * i < nums_threads;++i){
        for(int j = 0;j * j < nums_threads;++j){
            ThreadParams threadid(rows,cols,right_cols,matrix_1,matrix_2,result,i,j,nums_threads);
            vec.push_back(threadid);
        }
    }
}

bool check(double** result1, double** result2,int total){
    for(int i = 0;i < total;++i){
        for(int j = 0;j<total;++j){
            if(abs(result1[i][j]-result2[i][j]) < 0.01){

            }else{
                cout << i << "   " << j << endl;
                cout << "result "<< result1[i][j] << "  " << result2[i][j] << endl;
                return false;
            }
        }
    }
    return true;
    
}