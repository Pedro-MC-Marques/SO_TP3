#include "menu_handle.h"
#include "remote_server.h"



void *menu_thread(void *arg){
    ServerData *server_data = (ServerData *)arg;


    log_message(INFO, "Menu thread created successfully");

    printf("=== Menu do Servidor ===\n");
    printf("Comandos disponíveis:\n");
    printf("  status - Mostrar conexões ativas\n");
    printf("  quit   - Encerrar o servidor\n");
    printf("=========================\n");


    //while the server is active
    char command[128];
    pthread_mutex_lock(&server_data->server_running_lock);
    int server_running = server_data->server_running;
    pthread_mutex_unlock(&server_data->server_running_lock);


    while (server_running == 0){
        printf("> ");
        if(!fgets(command, sizeof(command), stdin)){ //get user input in CLI
            log_message(ERROR, "ERROR reading from stdin, shuting down server\n");
            pthread_mutex_lock(&server_data->server_running_lock);
            server_data->server_running = 1;
            pthread_mutex_unlock(&server_data->server_running_lock);            
            break;
        }

        command[strcspn(command, "\n")] = '\0'; //remove break of line (\n) (https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input)

        if(strcmp(command, "status") == 0){
            printf("--- Status Servidor ---\n");
            pthread_mutex_lock(&server_data->client_count_lock);
            int TcpClient_Nr = server_data->TCPclient_Nr;
            int UnixClient_Nr = server_data->UNIXclient_Nr;
            printf("Conexões TCP Ativas: %d\n", TcpClient_Nr);
            printf("Conexões UNIX Ativas: %d\n", UnixClient_Nr);
            printf("Threads Ativas: %d\n", threadpool_runningThreads(server_data->tp));
            printf("---------------------\n");
            pthread_mutex_unlock(&server_data->client_count_lock);
            log_message(INFO, "Status command executed");
        } else if(strcmp(command, "quit") == 0){
            log_message(INFO, "Servidor encerrado pelo utilizador\n");
            pthread_mutex_lock(&server_data->server_running_lock);
            server_data->server_running = 1;
            pthread_mutex_unlock(&server_data->server_running_lock);
            threadpool_destroy(server_data->tp);
            return NULL;
        } else {
            printf("Comando inválido\n");
            log_message(DEBUG, "Invalid command");
        }
    }
    return NULL;
}