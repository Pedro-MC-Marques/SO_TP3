#include "ringBuffer.h"
#include <stdlib.h>
#include <stdio.h>


void* ringbuffer_get(RingBuffer* rb){
	if(ringbuffer_isempty(rb))
		return NULL;
	
	void* ret = rb->arr[rb->get];
	
	rb->get= (rb->get + 1) % rb->length;
	rb->count--;
	
	return ret;
}

int ringbuffer_put(RingBuffer* rb, void* data){
	if(ringbuffer_isfull(rb))
		return 0;
	
	rb->arr[rb->put] = data;
	
	rb->put++;
	
	//if(rb->put >= length)
	//	rb->put = 0;
	// equals to below
	rb->put = rb->put % rb->length;
	
	rb->count++;
	return 1;
}



RingBuffer* ringbuffer_create(int length){
	RingBuffer* rb = malloc(sizeof(RingBuffer*));
	rb->count = 0;
	rb->length = length;
	rb->put = 0;
	rb->get = 0;
	rb->arr = malloc(length * sizeof(void*));
	return rb;
}


RingBuffer* ringbuffer_destroy(RingBuffer* rb){
	free(rb->arr);
	free(rb);
}



int ringbuffer_isfull(RingBuffer* rb){
	return rb->count == rb->length;
}

int ringbuffer_isempty(RingBuffer* rb){
	return rb->count == 0;
}
