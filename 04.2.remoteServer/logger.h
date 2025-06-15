#ifndef LOGGER_H
#define LOGGER_H

typedef enum {
    INFO, 
    ERROR, 
    DEBUG
} LOG_LEVEL;

int log_init(const char *pathname);

int log_message(LOG_LEVEL level, const char *msg);

int log_message_width_end_point(LOG_LEVEL level, const char *msg, int sock);

int log_close();

#endif 