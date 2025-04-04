#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "parallel.h"
typedef struct {
    int start;
    int end;
    int increment;
    int thread_id;
    void*(*functor)(void*,int);
    int num_threads;
    void* arg;
} ThreadArgs;
void* thread_func(void* args) {
    ThreadArgs* t = (ThreadArgs*)args;
    for (int i = t->start + t -> thread_id * t -> increment; i < t->end; i += t->increment * t -> num_threads) {
        t->functor(t->arg,i); // 增加一个参数传进去对应的行号
    }
    return NULL;
}
void parallel_for(int start, int end, int increment, void*(*functor)(void*,int), void* arg, int num_threads) {
    pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t*));
    ThreadArgs* thread_args = (ThreadArgs*)malloc(num_threads * sizeof(ThreadArgs));
    for (int i = 0; i < num_threads; i++) {
        thread_args[i].start = start;
        thread_args[i].end = end;
        thread_args[i].increment = increment;
        thread_args[i].functor = functor;
        thread_args[i].thread_id = i;
        thread_args[i].num_threads = num_threads;
        thread_args[i].arg = arg;
        pthread_create(&threads[i], NULL, thread_func, &thread_args[i]);
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    free(threads);
    free(thread_args);
}
