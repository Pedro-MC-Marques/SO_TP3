#include "remote_server.h"
#include "logger.h"
#include "socket_utils.h"
#include "threadPool.h"
#include "client_handle.h" 
#include "menu_handle.h"   

int main() {
    pthread_t tcp_accept_thread;
    //pthread_t unix_accept_thread;
    pthread_t menu_thread_id;

    int queue = THREAD_POOL_QUEUE_DIM;
    int min_threads = THREAD_POOL_MIN_THREADS;
    int max_threads = THREAD_POOL_MAX_THREADS;

    ServerData *server_data = malloc(sizeof(ServerData));
    if(server_data == NULL) return -1;

    server_data->tp = malloc(sizeof(ThreadPool));
    if(server_data->tp == NULL){ 
        free(server_data);
        return -1;
    }

    server_data->TCPclient_Nr = 0;
    server_data->UNIXclient_Nr = 0;
    server_data->server_running = 0; //server is running
    pthread_mutex_init(&server_data->client_count_lock, NULL);
    pthread_mutex_init(&server_data->server_running_lock, NULL);
    server_data->tcp_socket_fd = -1;
    server_data->unix_socket_fd = -1;


    // Inicializar o logger
    if (log_init(LOG_FILE_PATH) != 0) {
        perror("Error startign logger");
        free(server_data->tp);
        free(server_data);
        return -1;
    }
    log_message(INFO, "Server starting...");

    // Inicializar o thread pool
    if (threadpool_init(server_data->tp ,queue, min_threads, max_threads) != 0){
        perror("Error starting thread pool");
        log_message(ERROR, "Erro ao inicializar o thread pool");
        log_close();
        pthread_mutex_destroy(&server_data->client_count_lock);
        pthread_mutex_destroy(&server_data->server_running_lock);
        free(server_data->tp);
        free(server_data);
        return -1;
    }
    log_message(INFO, "Thread pool inicializada com sucesso");

    //Start TCP and UNIX sockets on server side
    server_data->tcp_socket_fd = tcp_server_socket_init(TCP_PORT);
    if(server_data->tcp_socket_fd < 0){
        log_message(ERROR, "Erro ao iniciar o socket TCP");
        perror("Error starting TCP socket");
        threadpool_destroy(server_data->tp);
        log_close();
        pthread_mutex_destroy(&server_data->client_count_lock);
        pthread_mutex_destroy(&server_data->server_running_lock);
        free(server_data->tp);
        free(server_data);
        return -1;
    }
    log_message(INFO, "Socket TCP inicializado com sucesso");

    /*
    server_data->unix_socket_fd = un_client_socket_init(UNIX_SOCKET_PATH);
    if(server_data->unix_socket_fd < 0){
        log_message(ERROR, "Erro ao iniciar o socket UNIX");
        perror("Error starting UNIX socket");
        close(server_data->tcp_socket_fd);
        threadpool_destroy(server_data->tp);
        log_close();
        pthread_mutex_destroy(&server_data->client_count_lock);
        pthread_mutex_destroy(&server_data->server_running_lock);
        free(server_data->tp);
        free(server_data);
        return -1;
    }
    log_message(INFO, "Socket UNIX inicializado com sucesso");
*/

    //Create threads for wich socket and menu_handler
    if(pthread_create(&tcp_accept_thread, NULL, tcp_client_handling_thread, (void *)server_data) != 0){
        log_message(ERROR, "Erro ao criar a thread para aceitar conexões TCP");
        perror("Error creating TCP accept thread");
        close(server_data->tcp_socket_fd);
        close(server_data->unix_socket_fd);
        threadpool_destroy(server_data->tp);
        log_close();
        pthread_mutex_destroy(&server_data->client_count_lock);
        pthread_mutex_destroy(&server_data->server_running_lock);
        free(server_data->tp);
        free(server_data);
        return -1;
    }
    log_message(INFO, "Thread para aceitar conexões TCP criada com sucesso");

/*
    if(pthread_create(&unix_accept_thread, NULL, unix_client_handling_thread, (void *)server_data) != 0){
        log_message(ERROR, "Erro ao criar a thread para aceitar conexões TCP");
        perror("Error creating TCP accept thread");
        pthread_join(tcp_accept_thread, NULL);
        close(server_data->tcp_socket_fd);
        close(server_data->unix_socket_fd);
        threadpool_destroy(server_data->tp);
        log_close();
        pthread_mutex_destroy(&server_data->client_count_lock);
        pthread_mutex_destroy(&server_data->server_running_lock);
        free(server_data->tp);
        free(server_data);
        return -1;
    }
    log_message(INFO, "Thread para aceitar conexões TCP criada com sucesso");
*/

    if(pthread_create(&menu_thread_id, NULL, menu_thread, (void *)server_data) != 0){
        log_message(ERROR, "Erro ao criar a thread para aceitar conexões TCP");
        perror("Error creating TCP accept thread");
        pthread_join(tcp_accept_thread, NULL);
//        pthread_join(unix_accept_thread, NULL);
        close(server_data->tcp_socket_fd);
        close(server_data->unix_socket_fd);
        threadpool_destroy(server_data->tp);
        log_close();
        pthread_mutex_destroy(&server_data->client_count_lock);
        pthread_mutex_destroy(&server_data->server_running_lock);
        free(server_data->tp);
        free(server_data);
        return -1;
    }        
    


    log_message(INFO, "Server a correr...");

    //Busy-wait for server shutdown
    pthread_mutex_lock(&server_data->server_running_lock);
    while (server_data->server_running == 0) {
        pthread_mutex_unlock(&server_data->server_running_lock);
        sleep(1); 
        pthread_mutex_lock(&server_data->server_running_lock);
    }
    pthread_mutex_unlock(&server_data->server_running_lock);


    //closing tasks
    log_message(INFO, "Server encerrado");
    pthread_join(tcp_accept_thread, NULL);
//    pthread_join(unix_accept_thread, NULL);
    pthread_join(menu_thread_id, NULL);
    threadpool_destroy(server_data->tp);
    close(server_data->tcp_socket_fd);
    close(server_data->unix_socket_fd);
    pthread_mutex_destroy(&server_data->client_count_lock);
    pthread_mutex_destroy(&server_data->server_running_lock);
    free(server_data->tp);
    free(server_data);    
    log_close(); 

    return 0;
}