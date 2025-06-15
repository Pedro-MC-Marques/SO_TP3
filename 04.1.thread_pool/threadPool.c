#include "threadPool.h"

void *worker_thread(void *arg){
    ThreadPool *tp = (ThreadPool *)arg;

    while(1){
        WorkItem *wi = (WorkItem *)SharedBuffer_get(&tp->sb);
        
        if(wi == NULL){
            printf("[%ld] Received poison pill\n", pthread_self());
            break;
        }

        pthread_mutex_lock(&tp->lock);
        tp->runningThreads++;
        pthread_mutex_unlock(&tp->lock);
        
        if(wi->func != NULL){
            wi->func(wi->arg);
        }

        pthread_mutex_lock(&tp->lock);
        tp->runningThreads--;
        pthread_mutex_unlock(&tp->lock);
        
        free(wi);
    }

    //used to know when all threads finished
    //sem_post(&tp->sem);

    return NULL;
}

int threadpool_init(ThreadPool *tp, int queueDim, int nrOfThreads_min, int nrOfThreads_max){

    SharedBuffer_init(&tp->sb, queueDim);

    tp->nrOfThreads_min = nrOfThreads_min;
    tp->nrOfThreads_max = nrOfThreads_max;
    tp->runningThreads = 0;     //nr of active threads
    tp->shutdown = 0;           //active pool


    if(pthread_mutex_init(&tp->lock, NULL) != 0){
        SharedBuffer_destroy(&tp->sb);
        return -1;
    }


    tp->threads = malloc(sizeof(pthread_t) * nrOfThreads_min);
    if(tp->threads == NULL){
        SharedBuffer_destroy(&tp->sb);
        pthread_mutex_destroy(&tp->lock);
        return -1;
    }


    for(int i = 0; i < nrOfThreads_min; i++){
        if(pthread_create(&tp->threads[i], NULL, worker_thread, (void *)tp) != 0){
            perror("Error creating thread\n");
            pthread_mutex_lock(&tp->lock);
            tp->shutdown = 1;
            pthread_mutex_unlock(&tp->lock);

            for(int j = 0; j < i; j++){
                SharedBuffer_put(&tp->sb, NULL);
            }

            for(int j = 0; j < i + 1; j++){
                pthread_join(tp->threads[j], NULL);
            }

            SharedBuffer_destroy(&tp->sb);
            pthread_mutex_destroy(&tp->lock);
            free(tp->threads);
            return -1;
        }
    }

    printf("Thread pool created successfully with %d threads and queue size %d\n", nrOfThreads_min, queueDim);

    return 0;
}


int threadpool_submit(ThreadPool *tp, wi_function_t func, void *arg){

    //if threadpool is shuting down, don't submit new work
    pthread_mutex_lock(&tp->lock);
    if(tp->shutdown == 1){
        pthread_mutex_unlock(&tp->lock);
        return -1;
    }
    pthread_mutex_unlock(&tp->lock);


    WorkItem *wi = malloc(sizeof(WorkItem));
    if(wi == NULL){
        perror("Error allocating memory\n");
        return -1;
    }

    wi->func = func;
    wi->arg = arg;

    SharedBuffer_put(&tp->sb, wi);

    return 0;
}


/* Gemini 2.5 Flash -  Questionado quanto ao 


////Resposta////
Quando threadpool_destroy é chamado, ele define shutdown = 1 e, em seguida, envia os "poison pills" (NULLs) para o SharedBuffer.
As worker threads continuarão a obter itens do SharedBuffer. Se houver trabalhos "reais" na fila antes dos NULLs, eles serão processados primeiro. Somente quando os NULLs forem retirados do buffer é que as threads começarão a terminar.
Esta abordagem é funcional, mas é importante entender que ela depende da ordem de inserção no SharedBuffer.
*/

int threadpool_destroy(ThreadPool *tp){
    
    pthread_mutex_lock(&tp->lock);
    tp->shutdown = 1;
    pthread_mutex_unlock(&tp->lock);


    //use a "poison pill"
    for(int i = 0; i < tp->nrOfThreads_min; i++){
        printf("Sending poison pill\n");
        SharedBuffer_put(&tp->sb, NULL);
    }

    for(int i = 0; i < tp->nrOfThreads_min; i++){
        pthread_join(tp->threads[i], NULL);
    }

    pthread_mutex_destroy(&tp->lock);
    free(tp->threads);
    SharedBuffer_destroy(&tp->sb);

    return 0;
}


int threadpool_runningThreads(ThreadPool *tp){
    return tp->runningThreads;
}


