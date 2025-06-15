#ifndef MENU_H
#define MENU_H


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



#include "socket_utils.h"
#include "client_handle.h"
#include "threadPool.h"




void *menu_thread(void *arg);

#endif