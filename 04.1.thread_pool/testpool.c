#include <stdio.h>
#include <stdlib.h>

#include "threadPool.h"


void *work(void *arg){
    printf("[%ld]work\n", pthread_self());
    sleep(1);
    printf("[%ld]work done\n", pthread_self());

    return NULL;
}



int main(){

    ThreadPool tp;
    threadpool_init(&tp, 5, 100, 200);

    for(int i = 0; i < 100; i++){
        threadpool_submit(&tp, work, NULL);
    }
    printf("All work submitted\n");
    fflush(stdout);

    threadpool_destroy(&tp);
    printf("All done\n");

    return 0;
}