#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <limits.h> // Para INT_MIN e INT_MAX

#define NTHREADS 10


typedef struct {
    int *v;             
    size_t v_sz;        
    int nThreads;       
    int global_min;
    int global_max;
    long long global_sum;   
    pthread_mutex_t mutex;  
    pthread_cond_t cond;    
    int barrier_counter;    // Contador para a barreira
    int generation;         // Geração da barreira para evitar o problema de "thundering herd"
} thread_shared_data;


typedef struct {
    int thread_id;          // ID da thread
    size_t start_index;     // Índice de início da porção do vetor da thread
    size_t end_index;       // Índice de fim da porção do vetor da thread
    thread_shared_data *shared_data; // Ponteiro para os dados partilhados
} thread_local_data;

/*
Gemini 2.5 Flash - como resolvo o acesso à info partilhada
*/
// Função da barreira utilizando pthread_cond_t para sincronizar threads.
// Esta função garante que todas as threads atinjam este ponto antes que qualquer uma prossiga.
// É robusta e lida com o problema de "thundering herd" e múltiplos usos da barreira.
void pthread_barrier_wait_custom(thread_shared_data *data) {
    pthread_mutex_lock(&data->mutex); // Adquire o lock para proteger o contador e a geração
    int current_generation = data->generation; // Captura a geração atual da barreira
    data->barrier_counter++; // Incrementa o contador de threads que chegaram à barreira

    // Verifica se todas as threads chegaram à barreira
    if (data->barrier_counter == data->nThreads) {
        data->barrier_counter = 0; // Resetar o contador para a próxima utilização da barreira
        data->generation++;        // Avançar a geração para evitar que threads que já passaram entrem novamente
        pthread_cond_broadcast(&data->cond); // Sinaliza todas as threads que estão à espera
    } else {
        // Se ainda não todas as threads chegaram, espera (aguarda por um broadcast ou sinal)
        // O loop 'while' é importante para proteger contra "spurious wakeups" (despertar falso)
        // e para garantir que a thread só avança quando a geração muda (ou seja, todas as threads passaram).
        while (current_generation == data->generation) {
            pthread_cond_wait(&data->cond, &data->mutex);
        }
    }
    pthread_mutex_unlock(&data->mutex); 
}
/**/


void *thread_Handling(void *arg){
    thread_local_data *local_data = (thread_local_data*)arg;
    thread_shared_data *shared_data = local_data->shared_data;

    // Fase 1: Determinação dos valores mínimo e máximo locais e colaboração para os globais
    int local_min = INT_MAX; //Min value of int in local vector
    int local_max = INT_MIN; //Max value of int in local vector

    for(size_t i = local_data->start_index; i < local_data->end_index; i++){
        if(shared_data->v[i] < local_min)
            local_min = shared_data->v[i];
        if(shared_data->v[i] > local_max)
            local_max = shared_data->v[i];
    }

    // Colaborar na determinação dos valores mínimo e máximo globais
    pthread_mutex_lock(&shared_data->mutex);
    if(local_min < shared_data->global_min)
        shared_data->global_min = local_min;
    if(local_max > shared_data->global_max)
        shared_data->global_max = local_max;
    pthread_mutex_unlock(&shared_data->mutex);

    // Fase 1 sincronização
    pthread_barrier_wait_custom(shared_data);


    // Fase 2: Normalizar valores
    long long local_sum = 0;
    int range = shared_data->global_max - shared_data->global_min;

    if (range == 0) {
        for(size_t i = local_data->start_index; i < local_data->end_index; i++){
            shared_data->v[i] = 50; // Normaliza para um valor médio, por exemplo
            local_sum += shared_data->v[i];
        }
    } else {
        for(size_t i = local_data->start_index; i < local_data->end_index; i++){
            shared_data->v[i] = (int)(((long long)(shared_data->v[i] - shared_data->global_min) * 100) / range);
            local_sum += shared_data->v[i];
        }
    }
    
    pthread_mutex_lock(&shared_data->mutex);
    shared_data->global_sum += local_sum;
    pthread_mutex_unlock(&shared_data->mutex);

    // Fase 2 sincronização
    pthread_barrier_wait_custom(shared_data);


    // Fase 3: Classificar valores
    // A média global é calculada APÓS a sincronização da Fase 2 para garantir que global_sum está completo
    double global_average = (double)shared_data->global_sum / shared_data->v_sz;

    // Loop com size_t para consistência e evitar overflows
    for(size_t i = local_data->start_index; i < local_data->end_index; i++){
        if(shared_data->v[i] >= global_average)
            shared_data->v[i] = 1;
        else
            shared_data->v[i] = 0;
    }

    free(local_data); 
    return NULL;
}



void norm_min_max_and_classify_parallel(int v[], size_t v_sz, int nThreads) {
    // Validação dos parâmetros de entrada
    if(v == NULL || v_sz == 0 || nThreads <= 0) {
        fprintf(stderr, "Invalid input parameters: v is NULL, v_sz is 0, or nThreads is <= 0\n");
        return;
    }
    
    // Criar e preencher estrutura de dados partilhados
    thread_shared_data *shared_data = (thread_shared_data*)malloc(sizeof(thread_shared_data));
    if(shared_data == NULL) {
        perror("malloc shared_data"); 
        return; 
    }

    shared_data->v = v;
    shared_data->v_sz = v_sz;
    shared_data->nThreads = nThreads;
    shared_data->global_min = INT_MAX; // Min value of int
    shared_data->global_max = INT_MIN; // Max value of int
    shared_data->global_sum = 0;
    shared_data->barrier_counter = 0;
    shared_data->generation = 0; 


    if(pthread_mutex_init(&shared_data->mutex, NULL) != 0) {
        perror("pthread_mutex_init");
        free(shared_data); 
        return;
    }
    if(pthread_cond_init(&shared_data->cond, NULL) != 0) {
        perror("pthread_cond_init");
        pthread_mutex_destroy(&shared_data->mutex); 
        free(shared_data);
        return;
    }

    // Criar array de IDs das threads
    pthread_t *threads = (pthread_t*)malloc(sizeof(pthread_t) * nThreads);
    if(threads == NULL){
        perror("malloc threads array");
        // Limpar recursos já inicializados
        pthread_mutex_destroy(&shared_data->mutex);
        pthread_cond_destroy(&shared_data->cond);
        free(shared_data);
        return;
    }


    size_t chunk_size = v_sz / nThreads;
    int remaining_elements = v_sz % nThreads; 

    size_t current_start_index = 0; 


    for(int i = 0; i < nThreads; i++){ 
        thread_local_data *local_data = (thread_local_data*)malloc(sizeof(thread_local_data));
        if(local_data == NULL){
            perror("malloc local_data");
            free(threads);
            pthread_mutex_destroy(&shared_data->mutex);
            pthread_cond_destroy(&shared_data->cond);
            free(shared_data);
            return;
        }
        
        local_data->thread_id = i;
        local_data->shared_data = shared_data; 
        local_data->start_index = current_start_index;
        local_data->end_index = current_start_index + chunk_size + (i < remaining_elements ? 1 : 0);

        current_start_index = local_data->end_index; 

        if(pthread_create(&threads[i], NULL, thread_Handling, (void*)local_data) != 0){
            perror("pthread_create");
            free(local_data); 
            free(threads);
            pthread_mutex_destroy(&shared_data->mutex);
            pthread_cond_destroy(&shared_data->cond);
            free(shared_data);
            return;
        }
    }


    for (int i = 0; i < nThreads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
        }
    }

    // Limpar os recursos alocados: mutex, variável de condição, e a memória das estruturas
    pthread_mutex_destroy(&shared_data->mutex);
    pthread_cond_destroy(&shared_data->cond);
    free(threads);
    free(shared_data);
}


int main() {
    srand(time(NULL));


    size_t vz_size = 100; 
    int *v = (int *)malloc(sizeof(int) * vz_size);
    if (v == NULL) {
        return -1;
    }

    for (size_t i = 0; i < vz_size; i++) {
        v[i] = rand() % 1000; 
    }

    int nThreads_large = NTHREADS; 


    printf("Original Vector:\n");
    for (size_t i = 0; i < vz_size; i++) {
        printf("%d ", v[i]);
    }
    printf("\n");

    norm_min_max_and_classify_parallel(v, vz_size, nThreads_large);

    printf("Normalized Vector:\n");
    for (size_t i = 0; i < vz_size; i++) {
        printf("%d ", v[i]);
    }
    printf("\n\n");

    free(v); 


    return 0;
}
