#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#define ACCOUNT_BALANCE (1000)
#define ACCOUNTS_MAX (100)
#define N_THREADS (100)
#define NUM_MAX_PER_THREAD (100)

typedef struct{
    int id;
    double balance;
    pthread_mutex_t lock;
}account_t;

typedef struct{
    account_t *client_db;
    int num_transactions;
}thread_args;



double total_ammount(account_t *db){
    double total = 0;
    for(int i = 0; i < ACCOUNTS_MAX; i++){
        total += db[i].balance;
    }
    return total;
}


account_t *create_accounts(int num_accounts){
    account_t *db = malloc(sizeof(account_t) * num_accounts);
    if (db == NULL) return NULL;

    for(int i = 0; i < num_accounts; i++){
        db[i].id = i;
        db[i].balance = ACCOUNT_BALANCE;
        if(pthread_mutex_init(&db[i].lock, NULL) != 0){
            free(db);
            return NULL;
        }
    }

    return db;
}

thread_args *create_thread_args(account_t *db){
    thread_args *args = malloc(sizeof(thread_args));
    if (args == NULL) return NULL;

    args->client_db = db;
    args->num_transactions = NUM_MAX_PER_THREAD;

    return args;
}



void *transfer_thread(void *arg){
    thread_args *args = (thread_args *)arg;
    account_t *clients_info_arr = args->client_db;
    int num_transactions = args->num_transactions;


    for (int i = 0; i < num_transactions; i++){

        //Create client ID 
        int src_client_id = rand() % ACCOUNTS_MAX;
        int dest_client_id = rand() % ACCOUNTS_MAX;
        
        //Avoid auto-transactions
        while(dest_client_id == src_client_id){
            dest_client_id = rand() % ACCOUNTS_MAX;
        }    

        //randmom ammount too transfer between clients
        double transfer_ammount = rand() % 100;

        //get client info 
        account_t *src_client = &clients_info_arr[src_client_id];
        account_t *dest_client = &clients_info_arr[dest_client_id];

        //lock clients
        if(src_client_id > dest_client_id){ 
            pthread_mutex_lock(&src_client->lock);
            pthread_mutex_lock(&dest_client->lock);
        }else{
            pthread_mutex_lock(&dest_client->lock);
            pthread_mutex_lock(&src_client->lock);
        }

        printf("Transfer %.2f from client %d to client %d\n", transfer_ammount, src_client->id, dest_client->id);

        if(src_client->balance >= transfer_ammount){
            src_client->balance -= transfer_ammount;
            dest_client->balance += transfer_ammount;
        }

        //printf("New balance of client %d: %.2f\n", src_client->id, src_client->balance);
        //printf("New balance of client %d: %.2f\n", dest_client->id, dest_client->balance);
        //printf("\n");

        //sleep(1);

        pthread_mutex_unlock(&src_client->lock);
        pthread_mutex_unlock(&dest_client->lock);
    }

    return NULL;
}



int main(){
    srand(time(NULL));
    account_t *db = create_accounts(ACCOUNTS_MAX);
    if (db == NULL){
        perror("Failed to create database");
        return -1;
    }

    printf("Total ammount: %.2f -> at begining.\n", total_ammount(db));

    pthread_t threads[N_THREADS];

    thread_args *args = create_thread_args(db);
    if (args == NULL){
        perror("Failed to create thread arguments");
        for(int i = 0; i < ACCOUNTS_MAX; i++){
            pthread_mutex_destroy(&db[i].lock);
        }
        free(db);
        return -1;
    }

    for(int i = 0; i < N_THREADS; i++){
        pthread_create(&threads[i], NULL, transfer_thread, (void *)args);
    }

    for(int i = 0; i < N_THREADS; i++){
        pthread_join(threads[i], NULL);
    }

    printf("Total ammount: %.2f -> at end.\n", total_ammount(db));

    for(int i = 0; i < ACCOUNTS_MAX; i++){
        pthread_mutex_destroy(&db[i].lock);
    }
    
    free(args);
    free(db);

    return 0;
}