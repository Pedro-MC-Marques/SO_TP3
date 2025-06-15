#ifndef CLIENT_HANDLE_H
#define CLIENT_HANDLE_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/un.h>
#include <errno.h>

#include "remote_server.h"
#include "socket_utils.h"
#include "logger.h"
#include "threadPool.h"



#define MAX_BUFFER_SIZE 4096
#define CLIENT_TYPE_TCP 0
#define CLIENT_TYPE_UNIX 1

typedef struct {
    int client_socket;
    struct sockaddr_in client_addr;
    int socket_type; //define type o connection used (0 - TCP, 1 - UNIX)
    ServerData *server_data;
}Client_data;

typedef struct{
    char program[256];
    char args[256];
    char filename[256];
    long size;
}HeaderInfo;

void *handle_client(void *client_data_ptr);

void *tcp_client_handling_thread(void *server_socket_ptr);

void *unix_client_handling_thread(void *server_socket_ptr);


#endif