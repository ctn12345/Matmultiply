


// 初始化一个很大的矩阵
// 全并行观察时间
// 多线程会怎样 任务分成多份来完成 (先初始化偶数的)
#include<iostream>
#include<windows.h>
#include <random>
#include <chrono>
#include <time.h>    //引入头文件
#include<vector>
#include"matrix_multiply.cpp"
using namespace std;
int num = 1024*4;   // 矩阵维度
double time1 = 0,time2 = 0;
int main(){  

    // 设置随机数生成器
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> dis(0.0, 10); // 在0到1之间生成均匀分布的随机数

    // 动态分配内存创建矩阵
    int rows = num;
    int cols = num;
    double** matrix_1 = new double*[rows];
    double** matrix_2 = new double* [rows];
    double** result = new double* [rows];
    double** result1 = new double* [rows];
    double** result2 = new double* [rows];
    double** result3 = new double* [rows];
    double** result4 = new double* [rows];
    init_mat(dis,matrix_1,cols);
    init_mat(dis,matrix_2,cols);
    for (int i = 0; i < rows; ++i) {
        result[i] = new double[rows];
        result1[i] = new double[rows];
        result2[i] = new double[rows];
        result3[i] = new double[rows];
        result4[i] = new double[rows];
        for(int x = 0; x < rows; ++x){
            result[i][x] = 0;
            result1[i][x] = 0;
            result2[i][x] = 0;
            result3[i][x] = 0;
            result4[i][x] = 0;
        }
    }
    cout << "init over " << endl;
    // vector<ThreadParams> vec_16,vec_4,vec_8,vec_64;
    // init_vector(vec_4,4,rows,cols,cols,matrix_1,matrix_2,result);
    // init_vector(vec_16,16,rows,cols,cols,matrix_1,matrix_2,result1);
    // init_vector(vec_64,64,rows,cols,cols,matrix_1,matrix_2,result2);
    // init_vector(vec_8,25,rows,cols,cols,matrix_1,matrix_2,result3);
    // test2(vec_4);
    // test2(vec_64);
    // test2(vec_16);
    // test2(vec_8);
    // test2(vec_32);

    // local variable 1 compute the paramater of speeding  2 compute the f and m 3 base befores
    for(int s = 512;s<1024*4;s+=512){
        rows = s;
        cols = s;
        ThreadParams threadparamas(rows,cols,rows,matrix_1,matrix_2,result4,0,0,0);
        ThreadParams threadparamas1(rows,cols,rows,matrix_1,matrix_2,result3,0,0,0);
        ThreadParams threadparamas2(rows,cols,rows,matrix_1,matrix_2,result2,0,0,0);
        volient_method2(threadparamas);
        volient_method_local(threadparamas1);
        volient_method1(threadparamas2);
        check(result4,result3,num);
        cout<< '\n';
    }


    // 线程数的对比 4线程 8线程 16线程 32线程
    
    return 0;

}