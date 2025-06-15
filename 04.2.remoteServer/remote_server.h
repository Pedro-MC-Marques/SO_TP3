#ifndef R_SERVER_H
#define R_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> // Para sleep


// Inclui os cabeçalhos dos módulos necessários
#include "logger.h"
#include "socket_utils.h"
#include "threadPool.h"



// Definições de macros para configurações do servidor
#define TCP_PORT 8080                   // Porta TCP para o servidor
#define UNIX_SOCKET_PATH "/tmp/server_socket_so" // Caminho para o socket de domínio UNIX
#define LOG_FILE_PATH "server.log"      // Caminho para o ficheiro de log
#define THREAD_POOL_QUEUE_DIM 100       // Dimensão da fila da thread pool
#define THREAD_POOL_MIN_THREADS 5       // Número mínimo de threads na pool
#define THREAD_POOL_MAX_THREADS 100     // Número máximo de threads na pool (para escalabilidade)

// Estrutura para armazenar dados globais do servidor, passados entre as threads
typedef struct{
    ThreadPool *tp;                     // Ponteiro para a thread pool
    int TCPclient_Nr;                   // Contador de clientes TCP ativos
    int UNIXclient_Nr;                  // Contador de clientes UNIX ativos
    pthread_mutex_t client_count_lock;  // Mutex para proteger os contadores de clientes
    int server_running;                 // Flag para controlar o estado de execução do servidor (0 = ativo, 1 = a encerrar)
    pthread_mutex_t server_running_lock;// Mutex para proteger a flag server_running
    int tcp_socket_fd;                  // File descriptor do socket TCP do servidor
    int unix_socket_fd;                 // File descriptor do socket UNIX do servidor
}ServerData;


#endif // R_SERVER_H