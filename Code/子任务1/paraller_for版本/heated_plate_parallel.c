#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "parallel.h" // 引入自定义的并行库
#include <pthread.h>
#include <time.h>
#define M 500 // 矩阵的行数
#define N 500 // 矩阵的列数
#define max(a,b) ((a) > (b) ? (a) : (b)) // 定义求最大值的宏
// 自定义结构体，用于传递参数到线程函数
typedef struct {
    double(*u)[N]; // 原始矩阵指针
    double(*w)[N]; // 更新后的矩阵指针
    double diff;   // 当前的最大差值
} Args;
// 获取当前时间的函数（高精度）
double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts); // 使用高精度时钟获取时间
    return ts.tv_sec + ts.tv_nsec * 1e-9; // 返回秒数（包括纳秒部分）
}
// 全局变量
int num_threads = 8; // 线程数
double u[M][N];      // 原始矩阵
double w[M][N];      // 更新后的矩阵
double diff;         // 当前最大差值
double epsilon = 0.001; // 收敛条件
double mean = 0.0;   // 边界值平均值
int iterations = 0;  // 迭代次数
int iterations_print = 1; // 控制输出的迭代次数
pthread_mutex_t diff_mutex = PTHREAD_MUTEX_INITIALIZER; // 互斥锁，用于保护 diff
// 线程函数，用于计算每一行的更新和差值
void* interation_operation(void* args, int i) {
    Args* a = (Args*)args;
    double(*u)[N] = a->u;
    double(*w)[N] = a->w;
    double local_diff = 0.0; // 每个线程的局部最大差值
    // 更新矩阵的第 i 行（不包括边界）
    for (int j = 1; j < N - 1; j++) {
        w[i][j] = (u[i-1][j] + u[i+1][j] + u[i][j-1] + u[i][j+1]) / 4.0; // 计算新值
        local_diff = max(local_diff, fabs(w[i][j] - u[i][j])); // 计算差值的绝对值
    }
    // 使用互斥锁保护全局变量 diff 的更新
    pthread_mutex_lock(&diff_mutex);
    a->diff = max(a->diff, local_diff); // 更新全局最大差值
    pthread_mutex_unlock(&diff_mutex);
    return NULL;
}
int main(int argc, char* argv[]) {
    int i, j;
    // 输出程序信息
    printf("\nHEATED_PLATE_PARALLEL_FOR\n");
    printf("  C/PARALLEL_FOR version\n");
    printf("  A program to solve for the steady state temperature distribution\n");
    printf("  over a rectangular plate.\n");
    printf("\n");
    printf("  Spatial grid of %d by %d points.\n", M, N);
    printf("  The iteration will be repeated until the change is <= %e\n", epsilon);
    printf("  Number of processors available = %d\n", num_threads);
     printf("  Number of threads = %d\n", num_threads);
    // 初始化边界值
    for (i = 1; i < M - 1; i++) {
        w[i][0] = 100.0;   // 左边界
        w[i][N-1] = 100.0; // 右边界
    }
    for (j = 0; j < N; j++) {
        w[M-1][j] = 100.0; // 下边界
        w[0][j] = 0.0;     // 上边界
    }
    // 计算边界值的平均值
    for (i = 1; i < M - 1; i++) {
        mean += w[i][0] + w[i][N-1];
    }
    for (j = 0; j < N; j++) {
        mean += w[M-1][j] + w[0][j];
    }
    mean /= (2 * M + 2 * N - 4); // 平均值
    //printf("\n  MEAN = %f\n", mean);
    // 初始化内部值为边界值的平均值
    for (i = 1; i < M - 1; i++) {
        for (j = 1; j < N - 1; j++) {
            w[i][j] = mean;
        }
    }
    // 输出迭代头信息
    //printf("\n Iteration  Change\n\n");
    diff = epsilon; // 初始化最大差值
    double start_time = get_time(); // 开始计时
    // 主迭代循环，直到差值小于等于 epsilon
    while (epsilon <= diff) {
        // 将 w 的值复制到 u 中
        for (i = 0; i < M; i++) {
            for (j = 0; j < N; j++) {
                u[i][j] = w[i][j];
            }
        }
        // 创建参数结构体，并初始化 diff 为 0
        Args args = { .u = u, .w = w, .diff = 0.0 };
        parallel_for(1, M - 1, 1, interation_operation, &args, num_threads); // 并行执行每一行的更新
        // 更新全局 diff
        diff = args.diff;
        iterations++; // 增加迭代次数
        if (iterations == iterations_print) {
            // 输出当前迭代的信息
            printf("  %8d  %f\n", iterations, diff);
            iterations_print *= 2; // 下一次输出间隔加倍
        }
    }
    double end_time = get_time(); // 结束计时
    // 输出最终结果
    printf("\n  %8d  %f\n", iterations, diff);
    printf("\n  Error tolerance achieved.\n");
    printf("  Wallclock time = %f\n", end_time - start_time);
    printf("\nHEATED_PLATE_PARALLEL_FOR:\n");
    printf("  Normal end of execution.\n");
    return 0;
}
