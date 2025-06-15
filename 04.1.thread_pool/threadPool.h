#ifndef M_THREADPOOL
#define M_THREADPOOL

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sharedBuffer.h"

typedef void *(*wi_function_t)(void *);

typedef struct{
	wi_function_t func;
	void* arg;	
}WorkItem;

typedef struct _mtp{
    pthread_t *threads;
    SharedBuffer sb;
    int nrOfThreads_min;
    int nrOfThreads_max;
    int runningThreads;
    int shutdown;
    pthread_mutex_t lock;
}ThreadPool;

int threadpool_init(ThreadPool* tp, int queueDim, int nrOfThreads_min, int nrOfThreads_max);
int threadpool_submit(ThreadPool* tp, wi_function_t func, void *arg);
int threadpool_destroy(ThreadPool* tp);

int threadpool_runningThreads(ThreadPool* tp);

#endif //M_THREADPOOL