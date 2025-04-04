# Parallelize-heat-conduction-iterations
高性能热传导模拟并行化实现 基于 C 语言的热传导模拟并行化实验，使用 Pthreads、OpenMP 和 MPI 实现迭代计算的并行化，对比不同规模下的执行时间、内存消耗与加速比。实验包含理论分析、代码实现与性能优化，适合学习并行计算与科学计算优化。

## 项目简介
本项目通过 **Pthreads、OpenMP 和 MPI** 三种并行技术实现热传导模拟的迭代计算，对比不同并行策略在大规模矩阵计算中的性能差异。实验涵盖任务分解、线程/进程通信、负载均衡等核心问题，并提供完整的性能分析数据。
- Code文件夹中存储该项目的代码.其中子任务1为利用Pthread来实现并行热传导,子任务2为利用MPI来实现并行热传导
- Result文件夹中存储着本次实验的所有实验结果
- Report.pdf文件为详细的实验报告
---

## 实验内容  
1. **Pthreads 并行化**  
   - 使用自定义 `parallel_for` 函数替换 OpenMP 的并行循环，实现线程级并行。  
   - 通过互斥锁（Mutex）同步全局最大温差（`diff`）的更新。  

2. **MPI 并行化**  
   - 将矩阵按行划分，每个进程独立计算局部数据。  
   - 使用 `MPI_Sendrecv` 实现进程间边界数据通信，避免死锁。  
   - 动态分配局部网格，额外增加缓冲区存储相邻进程数据。  

3. **性能分析**  
   - 对比不同矩阵规模（128x128 至 4096x4096）与线程/进程数（1-8）的执行时间。  
   - 使用 **Valgrind Massif** 工具分析内存消耗峰值与分配模式。  

---
## 快速开始
### 环境要求
- C 编译器（GCC 推荐）
- MPI 环境（如 OpenMPI）
- Valgrind（可选，用于性能分析）

如果需要修改或编译parallel动态链接库,可以使用以下命令编译链接运行:
```bash
gcc-shared-fPIC-o libparallel.so parallel.c-lpthread
gcc-O3-o Parallel heated_plate_parallel.c -L.-lparallel-lpthread
LD_LIBRSRY_PATH=. ./Parallel
```
如果需要修改或编译MPI版本的代码,可以使用以下命令编译运行:
```bash
mpicc-O3-o Mpi heated_plate_MPI.c
mpirun-np 4 Mpi
```
如果仅需要运行可执行文件查看结果,则只需要索引到对应目录后运行即可:
```
cd Parallelize-heat-conduction-iterations\Code\子任务1\paraller_for版本
./ Parallel
cd Parallelize-heat-conduction-iterations\Code\子任务2\MPI版本
mpirun -n 进程数 Mpi
```
如果需要查看内存消耗,可以使用以下命令:
```
gcc-O3-o Parallel heated_plate_parallel.c -L.-lpthread-lparallel
LD_LIBRARY_PATH=.
valgrind--tool=massif --time-unit=B--stacks=yes ./Parallel
ms_print massif.out.18052
```
 
