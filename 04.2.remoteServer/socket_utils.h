#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <stddef.h>
#include "logger.h"

int tcp_server_socket_init(int serverPort);
int tcp_server_socket_accept(int serverSocket);
int tcp_client_socket_init(const char *host, int port);

int un_server_socket_init(const char *serverEndPoint);
int un_server_socket_accept(int serverSocket);
int un_client_socket_init(const char *serverEndPoint);

#endif