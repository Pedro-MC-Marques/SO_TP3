#ifndef M_RINGBUFFER
#define M_RINGBUFFER

typedef struct m_rb
{
	int count;
	int put;
	int get;
	int length;
	void** arr;
}RingBuffer;

RingBuffer* ringbuffer_create(int length);
RingBuffer* ringbuffer_destroy(RingBuffer* rb);
int ringbuffer_put(RingBuffer* rb, void* data);
void* ringbuffer_get(RingBuffer* rb);
int ringbuffer_isfull(RingBuffer* rb);
int ringbuffer_isempty(RingBuffer* rb);
#endif