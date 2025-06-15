#include "client_handle.h"

int parse_header(char *header, HeaderInfo *info){
    
    char *token;
    token = strtok(header, "\n");
    while (1){
        if (token != NULL && strcmp(token, "") == 0){
            break;
        }

        if(strncmp(token, "RUN: ", 5) == 0){
            strcpy(info->program, token + 5);
        } else if(strncmp(token, "ARGS: ", 6) == 0){
            strcpy(info->args, token + 6);
        } else if(strncmp(token, "FILE: ", 6) == 0){
            strcpy(info->filename, token + 6);
        } else if(strncmp(token, "DIM: ", 6) == 0){
            info->size = atol(token + 6);
        }
        token = strtok(NULL, "\n");
    }
    return 0;
}


void *handle_client(void *client_data_ptr) {
    Client_data *client_data = (Client_data *)client_data_ptr;
    int client_socket = client_data->client_socket;
    int socket_type = client_data->socket_type;
    char header_buffer[MAX_BUFFER_SIZE];
    ssize_t bytes_received;
    HeaderInfo header_info;

    // Log de nova conexão
    pthread_mutex_lock(&client_data->server_data->client_count_lock);
    if (socket_type == 0) {
        log_message_width_end_point(INFO, "New connection TCP", client_socket);
        client_data->server_data->TCPclient_Nr++;
    } else {
        log_message_width_end_point(INFO, "New connection UNIX", client_socket);
        client_data->server_data->UNIXclient_Nr++;;
    }
    pthread_mutex_unlock(&client_data->server_data->client_count_lock);


    // 1. Receber o cabeçalho
    memset(header_buffer, 0, sizeof(header_buffer));
    bytes_received = recv(client_socket, header_buffer, sizeof(header_buffer) - 1, 0);
    if (bytes_received <= 0) {
        if (bytes_received == 0) {
            log_message_width_end_point(INFO, "Client lost connection", client_socket);
        } else {
            log_message_width_end_point(ERROR, "Error receiving header", client_socket);
        }
        close(client_socket);
        pthread_mutex_lock(&client_data->server_data->client_count_lock);
        if (socket_type == 0) {
            client_data->server_data->TCPclient_Nr--;
        } else {
            client_data->server_data->UNIXclient_Nr--;;
        }
        pthread_mutex_unlock(&client_data->server_data->client_count_lock);
        free(client_data);
        pthread_exit(NULL);
    }
    header_buffer[bytes_received] = '\0';
    log_message_width_end_point(DEBUG, "Header received", client_socket);
    log_message_width_end_point(DEBUG, header_buffer, client_socket);

    // 2. Parse do cabeçalho
    if (parse_header(header_buffer, &header_info) != 0) {
        log_message_width_end_point(ERROR, "Error parsing header", client_socket);
        // Enviar mensagem de erro para o cliente (a implementar)
        close(client_socket);

        pthread_mutex_lock(&client_data->server_data->client_count_lock);
        if (socket_type == 0) {
            client_data->server_data->TCPclient_Nr--;
        } else {
            client_data->server_data->UNIXclient_Nr--;;
        }
        pthread_mutex_unlock(&client_data->server_data->client_count_lock);
        free(client_data);
        pthread_exit(NULL);
    }
    log_message_width_end_point(DEBUG, "Header parsed", client_socket);
    log_message_width_end_point(DEBUG, header_info.program, client_socket);
    log_message_width_end_point(DEBUG, header_info.args, client_socket);
    log_message_width_end_point(DEBUG, header_info.filename, client_socket);
    char size_str[32];
    sprintf(size_str, "%ld", header_info.size);
    log_message_width_end_point(DEBUG, size_str, client_socket);


    // 3. Receber o ficheiro (se size > 0)
    char *file_content = NULL;
    if (header_info.size > 0) {
        file_content = malloc(header_info.size + 1);
        if (file_content == NULL) {
            log_message_width_end_point(ERROR, "Error allocating memory for file content", client_socket);
            // Enviar mensagem de erro para o cliente (a implementar)
            close(client_socket);
            pthread_mutex_lock(&client_data->server_data->client_count_lock);
            if (socket_type == 0) {
                client_data->server_data->TCPclient_Nr--;
            } else {
                client_data->server_data->UNIXclient_Nr--;;
            }
            pthread_mutex_unlock(&client_data->server_data->client_count_lock);
            free(client_data);
            pthread_exit(NULL);
        }
        ssize_t total_bytes_received = 0;
        ssize_t current_bytes_received;
        while (total_bytes_received < header_info.size) {
            current_bytes_received = recv(client_socket, file_content + total_bytes_received, header_info.size - total_bytes_received, 0);
            if (current_bytes_received <= 0) {
                log_message_width_end_point(ERROR, "Error receiving file content", client_socket);
                free(file_content);
                close(client_socket);
                pthread_mutex_lock(&client_data->server_data->client_count_lock);
                if (socket_type == 0) {
                    client_data->server_data->TCPclient_Nr--;
                } else {
                    client_data->server_data->UNIXclient_Nr--;;
                }
                pthread_mutex_unlock(&client_data->server_data->client_count_lock);
                free(client_data);
                pthread_exit(NULL);
            }
            total_bytes_received += current_bytes_received;
        }
        file_content[header_info.size] = '\0';
        log_message_width_end_point(DEBUG, "File content received", client_socket);
    }

    // 4. Executar o programa
    char command[1024];
    if (strlen(header_info.filename) > 0 && file_content != NULL) {
        // Salvar o ficheiro temporariamente
        char temp_filename[64];
        sprintf(temp_filename, "/tmp/server_temp_%d", client_socket);
        FILE *temp_file = fopen(temp_filename, "wb");
        if (temp_file) {
            fwrite(file_content, 1, header_info.size, temp_file);
            fclose(temp_file);
            snprintf(command, sizeof(command), "%s %s %s", header_info.program, header_info.args, temp_filename);
        } else {
            log_message_width_end_point(ERROR, "Error creating temporary file", client_socket);
            // Enviar mensagem de erro para o cliente (a implementar)
            if (file_content) free(file_content);
            close(client_socket);
            pthread_mutex_lock(&client_data->server_data->client_count_lock);
            if (socket_type == 0) {
                client_data->server_data->TCPclient_Nr--;
            } else {
                client_data->server_data->UNIXclient_Nr--;;
            }
            pthread_mutex_unlock(&client_data->server_data->client_count_lock);
            free(client_data);      
            pthread_exit(NULL);
        }
    } else {
        snprintf(command, sizeof(command), "%s %s", header_info.program, header_info.args);
    }

    log_message_width_end_point(INFO, "Executing command", client_socket);
    log_message_width_end_point(DEBUG, command, client_socket);

    FILE *process_output = popen(command, "r");
    if (process_output == NULL) {
        log_message_width_end_point(ERROR, "Error executing command", client_socket);
        // Enviar mensagem de erro para o cliente (a implementar)
    } else {
        char output_buffer[MAX_BUFFER_SIZE];
        size_t bytes_read;
        while ((bytes_read = fread(output_buffer, 1, sizeof(output_buffer) - 1, process_output)) > 0) {
            output_buffer[bytes_read] = '\0';
            send(client_socket, output_buffer, bytes_read, 0);
        }
        pclose(process_output);
        log_message_width_end_point(INFO, "Command executed, output sent", client_socket);
    }

    // Limpeza
    if (strlen(header_info.filename) > 0 && file_content != NULL) {
        char temp_filename[64];
        sprintf(temp_filename, "/tmp/server_temp_%d_%ld", client_socket, (long)pthread_self());
        remove(temp_filename);
        free(file_content);
    }
    close(client_socket);
        pthread_mutex_lock(&client_data->server_data->client_count_lock);
        if (socket_type == 0) {
            client_data->server_data->TCPclient_Nr--;
        } else {
            client_data->server_data->UNIXclient_Nr--;;
        }
        pthread_mutex_unlock(&client_data->server_data->client_count_lock);
    free(client_data);
    pthread_exit(NULL);
}

void *tcp_client_handling_thread(void *server_socket_ptr) {
    ServerData  *server_data = (ServerData *)server_socket_ptr;
    Client_data *client_data;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_socket;


    log_message(INFO, "Thread de handling de clientes TCP iniciada");

    while (1) {
        pthread_mutex_lock(&server_data->server_running_lock);
        if (server_data->server_running == 1){
            break;
        }
        pthread_mutex_unlock(&server_data->server_running_lock);


        client_socket = tcp_server_socket_accept(server_data->tcp_socket_fd); // Usando a função do socket_utils.c
        if (client_socket < 0) {
            perror("Erro ao aceitar conexão TCP");
            log_message(ERROR, "Erro ao aceitar conexão TCP");
            continue;
        }

        // Obter informações do cliente (endereço)
        if (getpeername(client_socket, (struct sockaddr *)&client_addr, &client_addr_len) == -1) {
            perror("Erro ao obter informações do cliente TCP");
            log_message(ERROR, "Erro ao obter informações do cliente TCP");
            close(client_socket);
            continue;
        }

        client_data = malloc(sizeof(Client_data));
        if (client_data == NULL) {
            perror("Erro ao alocar memória para dados do cliente TCP");
            log_message(ERROR, "Erro ao alocar memória para dados do cliente TCP");
            close(client_socket);
            continue;
        }

        client_data->client_socket = client_socket;
        client_data->client_addr = client_addr;
        client_data->socket_type = 0; // TCP

        if (threadpool_submit(server_data->tp, handle_client, (void *)client_data) != 0) {
            log_message(ERROR, "Error submitting task to thread pool");
            close(client_socket);
            pthread_mutex_lock(&client_data->server_data->client_count_lock);
            client_data->server_data->TCPclient_Nr--;
            pthread_mutex_unlock(&client_data->server_data->client_count_lock); 
            free(client_data);
        } else {
            pthread_mutex_lock(&client_data->server_data->client_count_lock);
            client_data->server_data->TCPclient_Nr++;
            pthread_mutex_unlock(&client_data->server_data->client_count_lock);   
        }
    }
    log_message(INFO,"Thread de handling de clientes TCP terminada");
    pthread_exit(NULL);
}

void *unix_client_handling_thread(void *server_socket_ptr) {
    ServerData  *server_data = (ServerData *)server_socket_ptr;
    int client_socket;
    Client_data *client_data;

    log_message(INFO, "Thread de handling de clientes UNIX iniciada");

    while (1) {
        pthread_mutex_lock(&server_data->server_running_lock);
        if (server_data->server_running == 1){
            break;
        }
        pthread_mutex_unlock(&server_data->server_running_lock);

        client_socket = un_server_socket_accept(server_data->unix_socket_fd); // Usando a função do socket_utils.c
        if (client_socket < 0) {
            perror("Erro ao aceitar conexão UNIX");
            log_message(ERROR, "Erro ao aceitar conexão UNIX");
            continue;
        }

        // Não é possível obter o endereço do cliente para sockets de domínio UNIX da mesma forma
        client_data = malloc(sizeof(Client_data));
        if (client_data == NULL) {
            perror("Erro ao alocar memória para dados do cliente UNIX");
            log_message(ERROR, "Erro ao alocar memória para dados do cliente UNIX");
            close(client_socket);
            continue;
        }

        client_data->client_socket = client_socket;
        client_data->socket_type = 1; // UNIX

        if (threadpool_submit(server_data->tp, handle_client, (void *)client_data) != 0) {
            perror("Erro ao enviar tarefa para thread pool");
            log_message(ERROR, "Erro ao enviar tarefa para thread pool");
            close(client_socket);
            pthread_mutex_lock(&client_data->server_data->client_count_lock);
            client_data->server_data->UNIXclient_Nr--;
            pthread_mutex_unlock(&client_data->server_data->client_count_lock); 
            free(client_data);
        } else {
            pthread_mutex_lock(&client_data->server_data->client_count_lock);
            client_data->server_data->UNIXclient_Nr++;
            pthread_mutex_unlock(&client_data->server_data->client_count_lock); 
        }
    }
    pthread_exit(NULL);
}
