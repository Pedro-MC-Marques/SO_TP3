#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/un.h>
#include <stdlib.h>
#include <errno.h>

#include "socket_utils.h"


#define BACKLOG 512


//Funções socket para TCP
//inicializa socket TCP (REF: IOHELPERS - tcp_socket_create)
int tcp_server_socket_init(int serverPort){
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    
    int flag = 1;
    if(setsockopt(socketfd,SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) == -1){
        log_message(ERROR, "ERROR setting socket options");
        perror("ERROR setting socket options");
        return -1;
    }
    
    if(socketfd < 0){
        log_message(ERROR, "ERROR opening socket");
        perror("ERROR opening socket");
        return -1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // use any available Network interface because we only have serverPort
    serv_addr.sin_port = htons(serverPort); //convert to network byte order


    if(bind(socketfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        log_message(ERROR, "ERROR on binding TCP serve socket");
		perror("bind failed");
        return -1;
    }
	
	
	if(listen(socketfd, BACKLOG) < 0){
        log_message(ERROR, "ERROR on listening TCP serve socket");
		perror("listen failed");
        return -1;
	}

    log_message(INFO, "TCP server socket initialized successfully");
	return socketfd;
}

//aceita socket TCP (REF: IOHELPERS - tcp_socket_accept)
int tcp_server_socket_accept(int serverSocket){

    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    
    int client_fd = accept(serverSocket, (struct sockaddr *) &client, &client_len);
    if(client_fd < 0){
        log_message(ERROR, "ERROR on accept TCP serve socket");
        perror("ERROR on accept");
        return -1;
    }


    
    return client_fd;
}

//inicializa socket TCP cliente (REF: IOHELPERS - tcp_socket_connect)
int tcp_client_socket_init(const char *host, int port){
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        log_message(ERROR, "ERROR opening socket");
        perror("ERROR opening socket");
        return -1;
    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(host); //converting IP number (string) to network byte order
    address.sin_port = htons(port);

    if(connect(sockfd, (struct sockaddr *) &address, sizeof(address)) < 0){
        log_message(ERROR, "ERROR connecting");
        perror("ERROR connecting");
        close(sockfd);
        return -1;
    }

    log_message(INFO, "TCP client socket initialized successfully");

    return sockfd;    
}




//Funções para UNIX
//inicializa socket UNIX (ref: SOt-13-Sockets.pdf)
int un_server_socket_init(const char *serverEndPoint){
    int serverSocket;
    

    //criar o socket
    if((serverSocket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){
        log_message(ERROR, "ERROR opening socket");
        perror("ERROR opening socket");
        return -1;
    }


    //configuraçã de endereço
    struct sockaddr_un serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, serverEndPoint, sizeof(serv_addr.sun_path) - 1);

    if(bind(serverSocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        log_message(ERROR, "ERROR on binding UNIX serve socket");
        perror("ERROR on binding");
        return -1;
    }

    //listen
    if(listen(serverSocket, BACKLOG) < 0){
        log_message(ERROR, "ERROR on listening UNIX serve socket");
        perror("ERROR on listening");
        return -1;
    }

    log_message(INFO, "UNIX server socket initialized successfully");

    return serverSocket;
}

//aceita conexão UNIX (ref: SOt-13-Sockets.pdf)
int un_server_socket_accept(int serverSocket){
    
    int client_fd;
    if((client_fd = accept(serverSocket, NULL, NULL)) < 0){
        log_message(ERROR, "ERROR on accept UNIX serve socket");
        perror("ERROR on accept");
        return -1;
    }

    return client_fd;
}

//inicializa socket UNIX cliente (ref: SOt-13-Sockets.pdf)
int un_client_socket_init(const char *serverEndPoint){
    int sockfd;
    struct sockaddr_un serv_addr;

    //cria socket
    if((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){
        log_message(ERROR, "ERROR opening socket");
        perror("ERROR opening socket");
        return -1;
    }

    //configura endereço
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, serverEndPoint, sizeof(serv_addr.sun_path) - 1);
    serv_addr.sun_path[sizeof(serv_addr.sun_path) - 1] = '\0';


    //connect
    if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        log_message(ERROR, "ERROR connecting");
        perror("ERROR connecting");
        return -1;
    }

    log_message(INFO, "UNIX client socket initialized successfully");

    return sockfd;
}
