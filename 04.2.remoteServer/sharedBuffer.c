#include <semaphore.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "sharedBuffer.h"

#define BUFFER_CAPACITY  100
#define MAX_THREADS (100)
#define MAX_READS_PER_THREAD (100)
#define MAX_WRITERS_PER_THREAD (100)


void SharedBuffer_init (SharedBuffer *b, int capacity){
	pthread_mutex_init(&b->lock, NULL);
	pthread_cond_init(&b->waitReaders, NULL);
	pthread_cond_init(&b->waitWriters, NULL);
	b->rb = ringbuffer_create(capacity);
}

void SharedBuffer_destroy(SharedBuffer *b){
	pthread_cond_destroy(&b->waitReaders);
	pthread_cond_destroy(&b->waitWriters);
	pthread_mutex_destroy(&b->lock);
	ringbuffer_destroy(b->rb);
}


void SharedBuffer_put(SharedBuffer *b, void* data){
	
	pthread_mutex_lock(&b->lock);
	
	while(ringbuffer_isfull(b->rb))
		pthread_cond_wait(&b->waitWriters, &b->lock);
	
	ringbuffer_put(b->rb, data);
	
	pthread_mutex_unlock(&b->lock);
	
	pthread_cond_signal(&b->waitReaders);
}

void* SharedBuffer_get (SharedBuffer *b){
	pthread_mutex_lock(&b->lock);
	
	while(ringbuffer_isempty(b->rb))
		pthread_cond_wait(&b->waitReaders, &b->lock);
	
	void* ret = ringbuffer_get(b->rb);
	
	pthread_mutex_unlock(&b->lock);
	
	pthread_cond_signal(&b->waitWriters);
	
	return ret;
}