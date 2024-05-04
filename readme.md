### 计算强度
引入计算强度的目的就是评价这个算法是计算密集还是内存密集，我的想法是……\
variable: m = 要访问的内存数目,将内存的元素放到寄存器中 \
$t_{m}$  = 一次mov cost time   f=number of arithmetic operations \
$t_{f}$ = time per arithmetic operation   CI= f / m
Actual time = $f * t_{f} + m * t_{m} = f * t_{f} *(1 + t_{m} / t_{f}*1/CI)$\
$CI$:key to alrogithm effiecncy         $t_m / t_f$:key to matchine efficency\
(General Matrix Multiply)

$m$ = $n^3$ to read each colume of B n times\
    + $n^2$ to read each colume of A once\
    + $2n^2$ to read and write each element of C once\
    = $n^3+3n^2$\
q = f/m = $2n^3$ / ($n^3+3n^2$)

```python
# implements C = C + A*B
for i in range(N):
    # read row i of A into fast memory
    for j in range(N):
        # read block C[i,j] into fast memory
        # read column j of B into fast memory
        for k in range(N):
            C[i,j] = C[i,j] + A[i,k] * B[k,j]
```

### roofline model
roofline model 它表明的是一个应用程序具有不同算力强度时在这个算力平台的各种表现。\
首先在考虑优化系统性能时一定要注意CI(计算强度)的计算。F:单位时间内的浮点数运算次数。M:单位时间内的带宽\
而模型它本身是由属于自己的F和M的，而我们的算力平台的F和M可能不是一致的，所以模型在这个算力平台的极限计算强度E就值得研究了
* $x$-axis:代表着计算强度
* $y$-axis:代表着算力(Flop/s)

```
我的电脑的CPU是AMD R7 7840H,它的基准频率是3.8GHZ,内核数量是8，单个CPU周期里进行双精度浮点数运算的次数是16(32)
我的电脑内存基准频率5600MHZ
经过计算得到计算强度为86.85(针对flop数据)
```


我的电脑的CPU是AMD R7 7840H,它的基准频率是3.8GHZ,内核数量是8，单个CPU周期里进行双精度浮点数运算的次数是16(32)，而我的电脑内存基准频率是5600MHZ。
本地的计算强度为:$\frac{f}m=\frac{3.8*10^9*32*8}{5600 * 10^6 * 2}=86.85$

![alt text](https://github.com/ctn12345/Matmultiply/blob/master/picture/intensity.png)

在本次两者的实验中我发现所有都是内存访问速度限制了运算速度。
### 实验一 tile 实验
#### 算法原理
它确定了block_size然后，再依次相乘。\
首先设置 block 是 $b * b$ 而矩阵维度 $N * N$,被分割后矩阵块得到的维度是 $M * M$\
理由：因为我们的fast memory是有缓存，也就是cache line,所以我们的fast memory读取相比于没有分block的是它的$\frac{1}{b}$。\
$m = n^2$ to read each colume of A times once \
   +$M^3 * b^2$ to read each colume of B times $M^3$ \
   +$2 * n^2$ to read and write C once

```C
for(int i = 0;i < N;i += b){
    for(int j = 0;j < N; j += b){
        for(int k = 0;k < N;j+=b)
        {
            // block A[i:i+b,k:k+b]  B[k:k+b,j:j+b] C[i:i+b,j:j+b]
            block_Multiply(A,B,C);
        }
    }
}

void block_Multiply(A,B,C){
    for(int i = 0;i<b;++i){

    for(int j = 0;j<b;++j){
            for(int k=0;k<b;++k){
                C[i,j] = A[i,k] * B[k,j]; 
            }
        }
    }
}
```

本地寄存器的优化:它实质上是编译器的优化，将常使用的数据放在寄存器里，而不是L1 cache，它实质上是优化了访存(可以看出它是内存访问的瓶颈)\
我觉得有必要定义一下以下这些公式成立的条件，首先我们将result数组内容视而不见先，因为它在cache中只需要占据很小一部分内容(一个cacheline),而A数组中的一行数据是需要长期占据在cacheline里的，则我们计算一下n的最小值是多少,我们假设cacheline为64B $n+64n >= 512KB$求得n约等于8066。\
暴力矩阵求法:$f = 2*n^3 m > 2*n^2 + n^2 / p(重复访问了p次) + n^3$(可能存在L1cache满了的问题)  它的计算强度如此计算 intensity = $f / m = \frac{2*n^3}{2*n^2+n^2/p+n^3}$\
block_nums=16:$f=2*n^3 m= 2*n^2 + 2*N^3*b^2 intensity=\frac{f}{m}=b$\
block_nums=32:$f=2*n^3 m= 2*n^2 + 2*N^3*b^2 intensity=\frac{f}{m}=b$\
block_nums=64:$f=2*n^3 m= 2*n^2 + 2*N^3*b^2 intensity=\frac{f}{m}=b$\
block_nums=128:$f=2*n^3 m > 2*n^2 + 2*N^3*b^2 intensity=\frac{f}{m}<b$
#### 代码步骤

1、定义了结构体Threadparams,在此次实验中我将有关线程的参数传到这个结构体里面。
```C
// 矩阵相乘 C=AB
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
        this->matrix_1 = matrix_1;  // 相乘矩阵A
        this->matrix_2 = matrix_2;  // 相乘矩阵B
        this->result = result;      // 矩阵结果C
        this->row = row;            // 分块的row号(vec里用的上)
        this->col = col;            // 分块的col号(vec里用的上)
        this->thread_times = thread_times;   // 线程数目
    }
};
```
2、定义矩阵相乘的函数，在此次矩阵相乘中我使用了四种办法，分别是逐个元素计算结果、一行一行计算矩阵结果、寄存器优化、分块矩阵。\
以下是逐个元素计算结果的代码，它的f是$2n^3$,m是$2n^2+n^2+n^3$,计算强度为$\frac{f}{m} \approx 2$
```C
for(int i = 0;i < n;++i){
        for(int j = 0;j < p;j++){
            for(int k = 0;k<m;++k){
                result[i][j] += matrix_1[i][k] * matrix_2[k][j];
            }
        }
    }
```
以下一行一行计算矩阵结果的代码,它的$f$是$2n^3$,$m$是$2n^2+n^2+K(K < n^3)$,计算强度大概为3,这是我做实验得到的。(至于这个办法为什么会快，这是因为访问matrix_2的数组是按行访问的，而按行访问是有cacheline的，它会将旁边的内存也存放在cache里，所以会稍微快一点)
```C
for(int i = 0;i < n;++i){
        for(int j = 0;j < m;j++){
            for(int k = 0;k< p;++k){
                result[i][k] += matrix_1[i][j] * matrix_2[j][k];
            }
        }
    }
```
以下是寄存器优化的函数，它的$f$是$2*n^3$,$m$是$2*n^2+n^2+K(K<n^3)$,而它的计算强度也是3，但是访问temp是通过寄存器访问，访问速度比cache更大，所以访问速度更快
```C
 for(int i = 0;i < n;++i){
        for(int j = 0;j < m;j++){
             // 本地寄存器变量
            double temp = matrix_1[i][j];
            for(int k = 0;k< p;++k){
                result[i][k] += temp * matrix_2[j][k];
            }
        }
    }
```
接下来就是重中之重，tile矩阵分块算法上了。它的$f$仍然是$2*n^3$,而它的m是$2*n^2+n^2+N^2*b$,所以它的计算强度是$\frac{f}{m} \approx b$。\
注意接下来的代码可能有一点冗长…………………………
```C
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
```
#### 算法实践


在本次实验中给予我最大困惑的就是一开始，分块矩阵乘法计算得到的浮点数运算速率是明显慢于暴力矩阵乘法的速率的，这显然是跟现实不相符合，然后我通过在网上搜索和查询资料知道了可能是for循环控制的开销太大，因此需要适当的增加代码冗余程度，具体如下图所示。

![alt text](https://github.com/ctn12345/Matmultiply/blob/master/picture/冗余程度.png)

我觉得自己需要明确一个问题，那就是我需要计算的应该是此方法的访存次数是多少和所需要的计算量多少\
如下图，计算次数$f = 2 * n^3$   访存次数$m=n^3+3*n^2$\
$intensity=\frac{f}{m}$

![alt text](https://github.com/ctn12345/Matmultiply/blob/master/picture/不同方法优化.png)

如下图，$f=2*n^3\thickspace$$m=n^3/{cacheline}+3*n^2\thickspace$
$intensity=\frac{f}{m}$

![alt text](https://github.com/ctn12345/Matmultiply/blob/master/picture/violent_2.png)

而根据网课课程里讲的 $3*blocknums^2$<=size of cache,我的L1 cache 大小为512KB,而一个float占据四个字节，估算上来block_nums大小应该在100左右，因为我们希望自己的blcok_nums能被512整除，所以此次采用了16，32，64，128，256的size来进行运算。\

在本次实验中我对比了五种矩阵相乘的办法，最终根据实验跑得的数据可知block_nums=64的效果应该是最好的。此次的flop计算公式$f=2 * n^{3}$ ①\
$time = end() - start() ②$  然后由①和②式得到最终的浮点数运算速率为 $v = f / time$
![alt text](https://github.com/ctn12345/Matmultiply/blob/master/picture/method.png)

### 实验二 多线程实验

#### 实验步骤
在此次实验中我定义了vector<Threadparams>的向量来存储参数，而之前所说的Threadparams中的row、col、threadtimes也在此时此刻派上用场了。此次实验中线程个数必须是完全平方数，因为我是这样设置的呵呵🙂。\
子线程代码如下图所示\
```C
 // 分成x个子任务
    int x_times = 0;
    for(;x_times*x_times<times;++x_times); // 得到分块大小
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
```
#### 实验结果分析
经过估计单个线程的内存访问次数大约是当n>=1024时，它的内存访问次数$2N^2+n*N^2+n*N$ 计算得到Intensity=$\frac{2N^2*n}{2N^2+2n*N^2}\approx 2$ 而当n<=768时，它的内存访存次数$2N^2+2*n*N$ 计算得到Intensity=$\frac{2N^2*n}{2N^2+2n*N}\approx N$ 当矩阵维度是512或者768时，矩阵运算速率最快。这应该时综合线程切换开销时间跟访存时间的得到的结果\

另外在此次实验中，我尝试了多线程加速的速度比,经过实验得到16线程的速度是最快的，而根据多次实验观察,multi_threads=16效果是最好的，而在此次实验中矩阵维度是从256到3582,flops在维度为768时差不多达到最大。从数学上也可以解释，$\sqrt{\sqrt{multithreads}*cachesize}\approx 724$ 这的确较为符合实验结果！\
多线程的优点:
* 多核的参与可以实现并行计算
* 多核的参与使得L2 cache的size变大，内存访问上更有利

实验结果通过如下图所示

![alt text](https://github.com/ctn12345/Matmultiply/blob/master/picture/multi_threads.png)
