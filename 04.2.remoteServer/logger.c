#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "logger.h"

//Create a global variable for pointer to the log file 
static FILE *log_file = NULL; 

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;


char *gettimedatestamp(){
    time_t timestamp;
    struct tm * timeinfo;

    time(&timestamp);
    timeinfo = localtime(&timestamp);

    char *time_str = malloc(32);
    if(time_str == NULL){
        perror("ERROR allocating memory for time string");
        return NULL;
    }
    strftime(time_str, 32, "%Y-%m-%d %H:%M:%S", timeinfo);

    return time_str;
}

int log_init(const char *pathname){
    pthread_mutex_lock(&log_mutex);


    log_file = fopen(pathname, "a+"); //create or append 
    if(log_file == NULL){
        perror("ERROR opening log file");
        pthread_mutex_unlock(&log_mutex);
        return -1;
    }

    pthread_mutex_unlock(&log_mutex);

    log_message(INFO, "Log file opened successfully");

    return 0;
}

int log_message(LOG_LEVEL level, const char *msg){
    if(log_file == NULL){
        perror("log file not initialized");
        return -1;
    }

    char *time = gettimedatestamp();
    if(time == NULL){
        return -1;
    }

    pthread_mutex_lock(&log_mutex);
    if(fprintf(log_file, "[%s] %s %s\n", (level == INFO) ? "INFO" : ((level == ERROR) ? "ERROR" : "DEBUG"), time, msg) < 0){
        perror("ERROR writing to log file");
        return -1;
    }
    fflush(log_file); //garante imediate log_message writing
    pthread_mutex_unlock(&log_mutex);
    free(time);
    return 0;
}

int log_message_width_end_point(LOG_LEVEL level, const char *msg, int sock){
    if (log_file == NULL) {
        perror("log file not initialized\n");
        return -1;
    }

    char *time = gettimedatestamp();
    if(time == NULL){
        return -1;
    }

    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    pthread_mutex_lock(&log_mutex);
    if (getpeername(sock, (struct sockaddr *)&addr, &addr_len) == 0) {
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(addr.sin_addr), ip_str, INET_ADDRSTRLEN);
        int port = ntohs(addr.sin_port);
        fprintf(log_file, "[%s] %s %s IP=%s Port=%d\n", (level == INFO) ? "INFO" : ((level == ERROR) ? "ERROR" : "DEBUG"), time, msg, ip_str, port);
    } else {
        fprintf(log_file, "[%s] %s %s (unable to get peer info)\n", (level == INFO) ? "INFO" : ((level == ERROR) ? "ERROR" : "DEBUG"), time, msg);
    }
    fflush(log_file);
    pthread_mutex_unlock(&log_mutex);

    free(time);
    return 0;
}

int log_close(){
    pthread_mutex_lock(&log_mutex);
    if(fclose(log_file) != 0){
        perror("ERROR closing log file");
        pthread_mutex_unlock(&log_mutex); 
        return -1;
    }
    log_file = NULL; 
    pthread_mutex_unlock(&log_mutex); 

    pthread_mutex_destroy(&log_mutex);
    return 0;
}