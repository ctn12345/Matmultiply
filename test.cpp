#include "test.h"

// 最简单的先算完C矩阵的一项，然后再算另一项，但是它的空间范围较广
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
    int x_times = 0;
    for(;x_times*x_times<times;++x_times);
    int start_x = parama->m * row / x_times;
    int start_y = parama->n * col / x_times;
    int x= 0,y=0;

    for(int left_row = 0;left_row < m / x_times;++left_row){
        x = left_row + start_x;
        for(int right_col = 0;right_col < p / x_times;right_col++){
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


// 行矩阵先确定一个元素，将这个元素在C矩阵中的所有运算操作都结束，就访问下一个元素
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
    int start_x = parama->m * row / x_times;
    int start_y = parama->n * col / x_times;
    int x= 0,y=0;

    for(int left_row = 0;left_row < m / x_times; ++left_row){
        x = left_row + start_x;
        for(int left_col = 0; left_col < n; ++left_col){
            int temp = matrix_1[x][left_col];
            for(int right_col = 0;right_col < p / x_times;++right_col ){
                y = right_col + start_y;
                result[x][y] += temp * matrix_2[left_col][y];
            }
        }
    }
    return 0;
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
    for(int a = 0;a < vec[0].col;++a){
        for(int h = 0;h < vec[0].col;++h)
            vec[0].result[a][h] = 0.0;
    }
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

// 给每个块标号，但是这有个致命的破绽，那就是他要分为一个可以被正开根号的数
void init_vector(vector<ThreadParams>& vec,int nums_threads,int rows,int cols,int right_cols,double** matrix_1,double** matrix_2,double** result){
    for(int i = 0; i * i < nums_threads;++i){
        for(int j = 0;j * j < nums_threads;++j){
            ThreadParams threadid(rows,cols,right_cols,matrix_1,matrix_2,result,i,j,nums_threads);
            vec.push_back(threadid);
            // cout << "i,j" << "  " << i<< "  " << j << endl;
        }
    }
    for(int i = 0; i<rows;i++){
        for(int j = 0;j < cols;++j){
            result[i][j] = 0.0;
        }
    }
}