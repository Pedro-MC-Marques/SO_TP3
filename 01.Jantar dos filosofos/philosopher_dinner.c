/*https://blog.pantuza.com/artigos/o-jantar-dos-filosofos-problema-de-sincronizacao-em-sistemas-operacionais
Cinco filósofos estão sentados em uma mesa redonda para jantar. Cada filósofo tem um prato com espaguete à sua frente. Cada prato possui um garfo para pegar o espaguete. O espaguete está muito escorregadio e, para que um filósofo consiga comer, será necessário utilizar dois garfos. 

Lembre-se que é apenas uma analogia. Nesse sentido, cada filósofo alterna entre duas tarefas: comer ou pensar. Quando um filósofo fica com fome, ele tenta pegar os garfos à sua esquerda e à sua direita; um de cada vez, independente da ordem. Caso ele consiga pegar dois garfos, ele come durante um determinado tempo e depois recoloca os garfos na mesa. Em seguida ele volta a pensar.*/

/*https://www.geeksforgeeks.org/dining-philosopher-problem-using-semaphores/*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>


#define N_PHILOSOPHERS 5
#define THINKING 2                     //define value for thinking
#define HUNGRY 1                       //define value for hungry
#define EATING 0                       //define value for eating
#define LEFT (id + N_PHILOSOPHERS - 1) % N_PHILOSOPHERS //calculate de fork id to the left
#define RIGHT (id) % N_PHILOSOPHERS    //calculate de fork id to the right



int state[N_PHILOSOPHERS];   //sate of the philosofpher (thinking, hungry or eating)
int phil_id[N_PHILOSOPHERS]; //id of each philosopher

sem_t mutex;                 //semaphore
sem_t S[N_PHILOSOPHERS];     //semaphore array


void test(int id){
    if(state[id] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING){ //confirm nobody next to me is eating
        state[id] = EATING;
        sleep(2);
        printf("Philosopher %d is eating\n", id);
        sem_post(&S[id]);
    }
}


void take_fork(int id){
    sem_wait(&mutex);

    state[id] = HUNGRY;
    printf("Philosopher %d is hungry\n", id);

    test(id);

    sem_post(&mutex);

    sem_wait(&S[id]);
    sleep(1);
}

void put_fork(int id){
    sem_wait(&mutex);

    state[id] = THINKING;
    printf("Philosopher %d is thinking\n", id);

    test(LEFT);
    test(RIGHT);

    sem_post(&mutex);
}

void *philosopher(void *arg){
    int *phil_id = (int *)arg;

    printf("Philosopher %d is thinking\n", *phil_id);

    while(1){
        sleep(1);    
        take_fork(*phil_id);
        sleep(0);
        put_fork(*phil_id);
    }
}


int main(){

    for (int i = 0; i < N_PHILOSOPHERS; i++){
        phil_id[i] = i;
        state[i] = THINKING;
    }

    pthread_t philosophers_threads[N_PHILOSOPHERS];

    //start semaphores
    sem_init(&mutex, 0, 1);

    for (int i = 0; i < N_PHILOSOPHERS; i++){
        sem_init(&S[i], 0, 0);
        pthread_create(&philosophers_threads[i], NULL, philosopher,&phil_id[i]);
    }


    for(int i = 0; i < N_PHILOSOPHERS; i++){
        pthread_join(philosophers_threads[i], NULL);
    }

    return 0;
}