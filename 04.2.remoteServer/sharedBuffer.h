#ifndef M_SB
#define M_SB

#include "ringBuffer.h"
#include <pthread.h>

typedef struct
{
	pthread_cond_t waitReaders;
	pthread_cond_t waitWriters;
	pthread_mutex_t lock;
	RingBuffer* rb;
} SharedBuffer;


void SharedBuffer_init (SharedBuffer *b, int capacity);

void SharedBuffer_destroy(SharedBuffer *b);


void SharedBuffer_put(SharedBuffer *b, void* data);

void* SharedBuffer_get (SharedBuffer *b);


#endif