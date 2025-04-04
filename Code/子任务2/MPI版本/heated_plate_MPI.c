# include <stdlib.h>
# include <string.h>
# include <stdio.h>
# include <math.h>
# include <mpi.h>
# define M 500
# define N 500
// # define M 16
// # define N 16
#define max(a,b) ((a) > (b) ? (a) : (b))
double epsilon = 0.001;
double diff = 0.0;
int i;
int iterations = 0;
int iterations_print = 1;
int j;
double mean = 0.0;
double my_diff;
double wtime;
double u[M][N];
double w[M][N];


int main ( int argc, char *argv[] ){
    int rank,comm_sz;
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    if(rank == 0){
        printf ( "\n" );
        printf ( "HEATED_PLATE_MPI\n" );
        printf ( "  C/MPI version\n" );
        printf ( "  A program to solve for the steady state temperature distribution\n" );
        printf ( "  over a rectangular plate.\n" );
        printf ( "\n" );
        printf ( "  Spatial grid of %d by %d points.\n", M, N );
        printf ( "  The iteration will be repeated until the change is <= %e\n", epsilon ); 
        printf ( "  Number of processors available = %d\n", comm_sz );
        printf ( "  Number of threads =              %d\n", comm_sz );
        // Initialize boundary values
        for (i = 1; i < M - 1; i++) {
            w[i][0] = 100.0;
            w[i][N-1] = 100.0;
        }
        for (j = 0; j < N; j++) {
            w[M-1][j] = 100.0;
            w[0][j] = 0.0;
        }
        // Compute mean
        for (i = 1; i < M - 1; i++) {
            mean += w[i][0] + w[i][N-1];
        }
        for (j = 0; j < N; j++) {
            mean += w[M-1][j] + w[0][j];
        }
        mean /= (2 * M + 2 * N - 4);
        printf("\n  MEAN = %f\n", mean);
        for (i = 1; i < M - 1; i++) {
            for (j = 1; j < N - 1; j++) {
                w[i][j] = mean;
            }
        }
        printf("\n Iteration  Change\n\n");
    }
    int range = M / comm_sz;
    int start = rank * range;
    int end = (rank != comm_sz-1) ? (rank + 1) * range - 1 : M - 1;
    int row_nums = end - start + 1; 
    //printf("rank: %d,start:%d,end:%d，row_num: %d\n",rank,start,end,row_nums); 
    //比所需要的行数多两行，用来缓冲存储上下相邻
    double* local_u = (double*)malloc((row_nums + 2) * N * sizeof(double));
    double* local_w = (double*)malloc((row_nums + 2) * N * sizeof(double));
    memset(local_u,0,(row_nums + 2) * N * sizeof(double));
    memset(local_w,0,(row_nums + 2) * N * sizeof(double));
    MPI_Scatter(
        w,                
        row_nums * N,      
        MPI_DOUBLE,             
        &local_w[N],          
        row_nums * N,       
        MPI_DOUBLE,             
        0,                       
        MPI_COMM_WORLD           
    );
    MPI_Scatter(
        u,                
        row_nums * N,       
        MPI_DOUBLE,             
        &local_u[N],       
        row_nums * N,      
        MPI_DOUBLE,             
        0,                      
        MPI_COMM_WORLD           
    );
    double start_time = MPI_Wtime();
    diff = epsilon;
    while(diff >= epsilon)

    {    
        //复制旧解 
        memcpy(&local_u[N], &local_w[N], row_nums * N * sizeof(double));
        if (rank > 0) {
            MPI_Sendrecv(&local_u[N], N,MPI_DOUBLE,rank - 1,0,
            &local_u[0],N,MPI_DOUBLE,rank - 1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        }
        if (rank < comm_sz - 1) {
            MPI_Sendrecv(&local_u[N * row_nums], N,MPI_DOUBLE,rank + 1,0,
            &local_u[N * (row_nums + 1)],N,MPI_DOUBLE,rank + 1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        }
        double local_diff = 0.0;
        for(int i = 1;i < row_nums+1;i++){
            for(int j = 1;j < N-1;j++){
                if(rank == 0 && i == 1) continue;
                if(rank == comm_sz-1 && i == row_nums) continue; 
                local_w[i*N+j] = (local_u[(i-1)*N+j]+local_u[(i+1)*N+j]+local_u[i*N+j-1]+local_u[i*N+j+1]) / 4.0;
                local_diff = max(local_diff,fabs(local_w[i*N+j]-local_u[i*N+j]));
            }
        }
        // if(rank == 0){
        //     printf("iterations:%d\n",iterations);
        //     for(int i = 0;i < row_nums+2;i++ ){
        //         for(int j = 0; j < N;j++ ){
        //             printf("%f ",local_w[i*N+j]);
        //         }
        //         printf("\n");
        //     }
        //     printf("\n");
        // }
        MPI_Allreduce(&local_diff, &diff, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
        //printf("rank:%d,diff:%f\n",rank,diff);
        iterations++;
        if(rank == 0 && iterations == iterations_print){
            printf ( "  %8d  %f\n", iterations, diff );
            iterations_print = 2 * iterations_print;
        }
    }
    double end_time = MPI_Wtime();
    double local_time = end_time - start_time;
    MPI_Allreduce(&local_time, &wtime, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
    if(rank == 0){
        printf ( "\n" );
        printf ( "  %8d  %f\n", iterations, diff );
        printf ( "\n" );
        printf ( "  Error tolerance achieved.\n" );
        printf ( "  Wallclock time = %f\n", wtime );
        printf ( "\n" );
        printf ( "HEATED_PLATE_OPENMP:\n" );
        printf ( "  Normal end of execution.\n" );
    }
    free(local_u);
    free(local_w);
    MPI_Finalize();
}
