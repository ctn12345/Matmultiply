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


// 计算是先计算C数组的每一列
void violent_method1(ThreadParams parama){
    double** result = parama.result;
    double** matrix_1 = parama.matrix_1;
    double** matrix_2 = parama.matrix_2;
    // n left_row  p left_col    m right_col
    int n = parama.n,p = parama.p,m=parama.m;
    clock_t start,end;//定义clock_t变量
    for(int a = 0;a < n;++a){
        for(int h = 0;h < p;++h)
            result[a][h] = 0.0;
    }
    start = clock();//开始时间
    for(int i = 0;i < n;++i){
        for(int j = 0;j < p;j++){
            for(int k = 0;k<m;++k){
                result[i][j] += matrix_1[i][k] * matrix_2[k][j];
            }
        }
    }
    end = clock();     //结束时间
    cout << "暴力求法串行1  ---  " << double(end-start)/CLOCKS_PER_SEC << endl;
}

// 计算C矩阵的每一行
void violent_method2(ThreadParams parama){
    double** result = parama.result;
    double** matrix_1 = parama.matrix_1;
    double** matrix_2 = parama.matrix_2;
    int n = parama.n,p = parama.p,m=parama.m;
    clock_t start,end,begin_1,end_1;//定义clock_t变量
    for(int a = 0;a < n;++a){
        for(int h = 0;h < p;++h)
            result[a][h] = 0.0;
    }
    start = clock();//开始时间
    // n left_row  p left_col    m right_col
    for(int i = 0;i < n;++i){
        for(int j = 0;j < m;j++){
            for(int k = 0;k< p;++k){
                result[i][k] += matrix_1[i][j] * matrix_2[j][k];
            }
        }
    }
    end = clock();     //结束时间
    cout << "暴力求法串行2  ---  " << double(end-start)/CLOCKS_PER_SEC << endl;
}

// 本地寄存器+C矩阵列
void violent_method_local(ThreadParams parama){
    double** result = parama.result;
    double** matrix_1 = parama.matrix_1;
    double** matrix_2 = parama.matrix_2;
    int n = parama.n,p = parama.p,m=parama.m;
    clock_t start,end,begin_1,end_1;//定义clock_t变量
    for(int a = 0;a < n;++a){
        for(int h = 0;h < p;++h)
            result[a][h] = 0.0;
    }
    start = clock();//开始时间
    // n left_row  p left_col    m right_col
    for(int i = 0;i < n;++i){
        for(int j = 0;j < m;j++){
             // 本地寄存器变量
            double temp = matrix_1[i][j];
            for(int k = 0;k< p;++k){
                result[i][k] += temp * matrix_2[j][k];
            }
        }
    }
    end = clock();     //结束时间
    cout << "本地寄存器local variable  ---  " << double(end-start)/CLOCKS_PER_SEC << endl;
}

// 
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
                // result[left_row][right_col] = 0;
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

        int x_times = 0;
        for(;x_times*x_times<times;++x_times);
        // cout << "times  index " << x_times << endl;
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

void tile_method1(ThreadParams parama,int block_nums){
    double** result = parama.result;
    double** matrix_1 = parama.matrix_1;
    double** matrix_2 = parama.matrix_2;
    // n left_row  p left_col    m right_col
    int n = parama.n,p = parama.p,m=parama.m;\
    for(int a = 0;a < n;++a){
        for(int h = 0;h < p;++h)
            result[a][h] = 0.0;
    }
    clock_t start,end;//定义clock_t变量
    start = clock();//开始时间
    for(int row = 0; row < n; row+=block_nums){
        for(int col = 0;col < m;col+=block_nums){
            // 计算分块矩阵相乘
           for(int time = 0;time < n / block_nums;++time){
                for(int block_row = 0;block_row < block_nums;++block_row){
                    int block_x = block_row + row;
                    for(int block_col = 0;block_col < block_nums;++block_col){
                        int block_y = block_col + col;
                        int bias_y = time * block_nums;
                        for(int block_p = 0;block_p < block_nums;++block_p){
                            result[block_x][block_y] += matrix_1[block_x][bias_y+block_p] * matrix_2[bias_y+block_p][block_y];
                        }  
                    }
            }
           }
        }
    }
    end = clock();
    cout << "分块矩阵乘法1 block_nums="<<block_nums << "    clock  ---  " << double(end-start)/CLOCKS_PER_SEC << endl; 

}


void tile_method2(ThreadParams parama,int block_nums){
    double** result = parama.result;
    double** matrix_1 = parama.matrix_1;
    double** matrix_2 = parama.matrix_2;
    // n left_row  p left_col    m right_col
    int n = parama.n,p = parama.p,m=parama.m;
    for(int a = 0;a < n;++a){
        for(int h = 0;h < p;++h)
            result[a][h] = 0.0;
    }
    clock_t start,end;//定义clock_t变量
    
    clock_t begin_1,end_1;
    double record_1 = 0;
    int times = n ;
    int block_bias = 0;
    start = clock();//开始时间
    for(int row = 0; row < n; row+=block_nums){
        for(int col = 0;col < m;col+=block_nums){
            // 计算分块矩阵相乘
           for(int time = 0;time < times;time += block_nums){
                for(int block_row = 0;block_row < block_nums;++block_row){
                    int block_x = block_row + row;
                    for(int block_p = 0;block_p < block_nums;++block_p){
                        block_bias = block_p + time;
                        switch(block_nums){
                            case 16:
                                result[block_x][col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][col];
                                result[block_x][1+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][1+ col];
                                result[block_x][2+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][2+ col];
                                result[block_x][3+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][3+ col];
                                result[block_x][4+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][4+ col];
                                result[block_x][5+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][5+ col];
                                result[block_x][6+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][6+ col];
                                result[block_x][7+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][7+ col];
                                result[block_x][8+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][8+ col];
                                result[block_x][9+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][9+ col];
                                result[block_x][10+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][10+ col];
                                result[block_x][11+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][11+ col];
                                result[block_x][12+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][12+ col];
                                result[block_x][13+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][13+ col];
                                result[block_x][14+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][14+ col];
                                result[block_x][15+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][15+ col];
                                break;
                            case 32:
                                result[block_x][col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][col];
                                result[block_x][1+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][1+ col];
                                result[block_x][2+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][2+ col];
                                result[block_x][3+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][3+ col];
                                result[block_x][4+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][4+ col];
                                result[block_x][5+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][5+ col];
                                result[block_x][6+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][6+ col];
                                result[block_x][7+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][7+ col];
                                result[block_x][8+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][8+ col];
                                result[block_x][9+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][9+ col];
                                result[block_x][10+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][10+ col];
                                result[block_x][11+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][11+ col];
                                result[block_x][12+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][12+ col];
                                result[block_x][13+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][13+ col];
                                result[block_x][14+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][14+ col];
                                result[block_x][15+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][15+ col];
                                result[block_x][16+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][16+ col];
                                result[block_x][17+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][17+ col];
                                result[block_x][18+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][18+ col];
                                result[block_x][19+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][19+ col];
                                result[block_x][20+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][20+ col];
                                result[block_x][21+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][21+ col];
                                result[block_x][22+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][22+ col];
                                result[block_x][23+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][23+ col];
                                result[block_x][24+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][24+ col];
                                result[block_x][25+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][25+ col];
                                result[block_x][26+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][26+ col];
                                result[block_x][27+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][27+ col];
                                result[block_x][28+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][28+ col];
                                result[block_x][29+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][29+ col];
                                result[block_x][30+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][30+ col];
                                result[block_x][31+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][31+ col];
                                break;
                            case 64:
                                result[block_x][col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][col];
                                result[block_x][1+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][1+ col];
                                result[block_x][2+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][2+ col];
                                result[block_x][3+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][3+ col];
                                result[block_x][4+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][4+ col];
                                result[block_x][5+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][5+ col];
                                result[block_x][6+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][6+ col];
                                result[block_x][7+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][7+ col];
                                result[block_x][8+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][8+ col];
                                result[block_x][9+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][9+ col];
                                result[block_x][10+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][10+ col];
                                result[block_x][11+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][11+ col];
                                result[block_x][12+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][12+ col];
                                result[block_x][13+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][13+ col];
                                result[block_x][14+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][14+ col];
                                result[block_x][15+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][15+ col];
                                result[block_x][16+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][16+ col];
                                result[block_x][17+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][17+ col];
                                result[block_x][18+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][18+ col];
                                result[block_x][19+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][19+ col];
                                result[block_x][20+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][20+ col];
                                result[block_x][21+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][21+ col];
                                result[block_x][22+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][22+ col];
                                result[block_x][23+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][23+ col];
                                result[block_x][24+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][24+ col];
                                result[block_x][25+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][25+ col];
                                result[block_x][26+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][26+ col];
                                result[block_x][27+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][27+ col];
                                result[block_x][28+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][28+ col];
                                result[block_x][29+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][29+ col];
                                result[block_x][30+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][30+ col];
                                result[block_x][31+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][31+ col];
                                result[block_x][32+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][32+ col];
                                result[block_x][33+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][33+ col];
                                result[block_x][34+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][34+ col];
                                result[block_x][35+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][35+ col];
                                result[block_x][36+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][36+ col];
                                result[block_x][37+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][37+ col];
                                result[block_x][38+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][38+ col];
                                result[block_x][39+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][39+ col];
                                result[block_x][40+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][40+ col];
                                result[block_x][41+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][41+ col];
                                result[block_x][42+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][42+ col];
                                result[block_x][43+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][43+ col];
                                result[block_x][44+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][44+ col];
                                result[block_x][45+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][45+ col];
                                result[block_x][46+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][46+ col];
                                result[block_x][47+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][47+ col];
                                result[block_x][48+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][48+ col];
                                result[block_x][49+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][49+ col];
                                result[block_x][50+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][50+ col];
                                result[block_x][51+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][51+ col];
                                result[block_x][52+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][52+ col];
                                result[block_x][53+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][53+ col];
                                result[block_x][54+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][54+ col];
                                result[block_x][55+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][55+ col];
                                result[block_x][56+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][56+ col];
                                result[block_x][57+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][57+ col];
                                result[block_x][58+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][58+ col];
                                result[block_x][59+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][59+ col];
                                result[block_x][60+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][60+ col];
                                result[block_x][61+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][61+ col];
                                result[block_x][62+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][62+ col];
                                result[block_x][63+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][63+ col];
                                break;
                        case 128:
                             result[block_x][0+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][0+ col];
                            result[block_x][1+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][1+ col];
                            result[block_x][2+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][2+ col];
                            result[block_x][3+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][3+ col];
                            result[block_x][4+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][4+ col];
                            result[block_x][5+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][5+ col];
                            result[block_x][6+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][6+ col];
                            result[block_x][7+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][7+ col];
                            result[block_x][8+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][8+ col];
                            result[block_x][9+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][9+ col];
                            result[block_x][10+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][10+ col];
                            result[block_x][11+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][11+ col];
                            result[block_x][12+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][12+ col];
                            result[block_x][13+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][13+ col];
                            result[block_x][14+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][14+ col];
                            result[block_x][15+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][15+ col];
                            result[block_x][16+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][16+ col];
                            result[block_x][17+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][17+ col];
                            result[block_x][18+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][18+ col];
                            result[block_x][19+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][19+ col];
                            result[block_x][20+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][20+ col];
                            result[block_x][21+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][21+ col];
                            result[block_x][22+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][22+ col];
                            result[block_x][23+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][23+ col];
                            result[block_x][24+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][24+ col];
                            result[block_x][25+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][25+ col];
                            result[block_x][26+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][26+ col];
                            result[block_x][27+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][27+ col];
                            result[block_x][28+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][28+ col];
                            result[block_x][29+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][29+ col];
                            result[block_x][30+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][30+ col];
                            result[block_x][31+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][31+ col];
                            result[block_x][32+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][32+ col];
                            result[block_x][33+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][33+ col];
                            result[block_x][34+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][34+ col];
                            result[block_x][35+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][35+ col];
                            result[block_x][36+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][36+ col];
                            result[block_x][37+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][37+ col];
                            result[block_x][38+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][38+ col];
                            result[block_x][39+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][39+ col];
                            result[block_x][40+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][40+ col];
                            result[block_x][41+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][41+ col];
                            result[block_x][42+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][42+ col];
                            result[block_x][43+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][43+ col];
                            result[block_x][44+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][44+ col];
                            result[block_x][45+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][45+ col];
                            result[block_x][46+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][46+ col];
                            result[block_x][47+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][47+ col];
                            result[block_x][48+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][48+ col];
                            result[block_x][49+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][49+ col];
                            result[block_x][50+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][50+ col];
                            result[block_x][51+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][51+ col];
                            result[block_x][52+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][52+ col];
                            result[block_x][53+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][53+ col];
                            result[block_x][54+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][54+ col];
                            result[block_x][55+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][55+ col];
                            result[block_x][56+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][56+ col];
                            result[block_x][57+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][57+ col];
                            result[block_x][58+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][58+ col];
                            result[block_x][59+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][59+ col];
                            result[block_x][60+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][60+ col];
                            result[block_x][61+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][61+ col];
                            result[block_x][62+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][62+ col];
                            result[block_x][63+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][63+ col];
                            result[block_x][64+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][64+ col];
                            result[block_x][65+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][65+ col];
                            result[block_x][66+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][66+ col];
                            result[block_x][67+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][67+ col];
                            result[block_x][68+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][68+ col];
                            result[block_x][69+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][69+ col];
                            result[block_x][70+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][70+ col];
                            result[block_x][71+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][71+ col];
                            result[block_x][72+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][72+ col];
                            result[block_x][73+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][73+ col];
                            result[block_x][74+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][74+ col];
                            result[block_x][75+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][75+ col];
                            result[block_x][76+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][76+ col];
                            result[block_x][77+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][77+ col];
                            result[block_x][78+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][78+ col];
                            result[block_x][79+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][79+ col];
                            result[block_x][80+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][80+ col];
                            result[block_x][81+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][81+ col];
                            result[block_x][82+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][82+ col];
                            result[block_x][83+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][83+ col];
                            result[block_x][84+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][84+ col];
                            result[block_x][85+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][85+ col];
                            result[block_x][86+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][86+ col];
                            result[block_x][87+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][87+ col];
                            result[block_x][88+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][88+ col];
                            result[block_x][89+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][89+ col];
                            result[block_x][90+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][90+ col];
                            result[block_x][91+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][91+ col];
                            result[block_x][92+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][92+ col];
                            result[block_x][93+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][93+ col];
                            result[block_x][94+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][94+ col];
                            result[block_x][95+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][95+ col];
                            result[block_x][96+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][96+ col];
                            result[block_x][97+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][97+ col];
                            result[block_x][98+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][98+ col];
                            result[block_x][99+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][99+ col];
                            result[block_x][100+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][100+ col];
                            result[block_x][101+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][101+ col];
                            result[block_x][102+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][102+ col];
                            result[block_x][103+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][103+ col];
                            result[block_x][104+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][104+ col];
                            result[block_x][105+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][105+ col];
                            result[block_x][106+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][106+ col];
                            result[block_x][107+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][107+ col];
                            result[block_x][108+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][108+ col];
                            result[block_x][109+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][109+ col];
                            result[block_x][110+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][110+ col];
                            result[block_x][111+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][111+ col];
                            result[block_x][112+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][112+ col];
                            result[block_x][113+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][113+ col];
                            result[block_x][114+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][114+ col];
                            result[block_x][115+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][115+ col];
                            result[block_x][116+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][116+ col];
                            result[block_x][117+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][117+ col];
                            result[block_x][118+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][118+ col];
                            result[block_x][119+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][119+ col];
                            result[block_x][120+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][120+ col];
                            result[block_x][121+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][121+ col];
                            result[block_x][122+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][122+ col];
                            result[block_x][123+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][123+ col];
                            result[block_x][124+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][124+ col];
                            result[block_x][125+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][125+ col];
                            result[block_x][126+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][126+ col];
                            result[block_x][127+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][127+ col];
                            break;
                        }
                        
                   }
                }
           }
        }
    }
    end = clock();

    cout << "分块矩阵乘法2 block_nums="<<block_nums << "    clock  ---  " << double(end-start)/CLOCKS_PER_SEC << endl; 

}


void tile_method3(ThreadParams parama,int block_nums){
    double** result = parama.result;
    double** matrix_1 = parama.matrix_1;
    double** matrix_2 = parama.matrix_2;
    // n left_row  p left_col    m right_col
    int n = parama.n,p = parama.p,m=parama.m;
    for(int a = 0;a < n;++a){
        for(int h = 0;h < p;++h)
            result[a][h] = 0.0;
    }
    clock_t start,end;//定义clock_t变量
    
    clock_t begin_1,end_1;
    double record_1 = 0;
    int times = n / block_nums;
    start = clock();//开始时间
    for(int row = 0; row < n; row+=block_nums){
        for(int col = 0;col < m;col+=block_nums){
            // 计算分块矩阵相乘
           for(int time = 0;time < times;++time){
                for(int block_row = 0;block_row < block_nums;++block_row){
                    int block_x = block_row + row;
                    for(int block_p = 0;block_p < block_nums;++block_p){
                        int block_bias = time * block_nums + block_p;
                        for(int block_col = 0;block_col < block_nums;++block_col){
                            result[block_x][block_col+col] += matrix_1[block_x][block_bias] * matrix_2[block_bias][block_col+col];
                        }

                    }
            }
           }
        }
    }
    end = clock();
    cout << "分块矩阵乘法2 block_nums="<<block_nums << "    clock  ---  " << double(end-start)/CLOCKS_PER_SEC << endl; 

}


// 分块的乘法
void tile_matrix_multiply(ThreadParams parama,int block){
    double** result = parama.result;
    double** matrix_1 = parama.matrix_1;
    double** matrix_2 = parama.matrix_2;
    // n left_row  p left_col    m right_col
    int n = parama.n,p = parama.p,m=parama.m;
    clock_t start,end;//定义clock_t变量
    start = clock();//开始时间
    int block_nums = block;
    for(int row = 0; row < n; row += block_nums){
       for(int k = 0;k<p;++k){
        for(int col = 0;col < m;col+=block_nums){
           
        }
       }
    }
    end = clock();
    cout << "分块乘法矩阵方法clock  ---  " << double(end-start)/CLOCKS_PER_SEC << endl; 

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