#ifndef RINGBUFFER_H
#define RINGBUFFER_H
#include "LAL.h"

#define RING_BUFFER_DEFAULT_VALUE (0)

typedef struct
{
    u8* data;
    u32 capacity;
    u16 size;
    u16 head;
    u16 tail;
    u16 padding;
} RingBuffer;

// NOTE(MIGUEL): !!! Almost useless function !!!
void 
RingBuffer_Init(RingBuffer *buffer, u8 *data, u32 size);

b32 
RingBuffer_Full(RingBuffer* buffer);

b32 
RingBuffer_Empty(RingBuffer* buffer); 

void 
RingBuffer_Destroy(RingBuffer* buffer);

void 
RingBuffer_Enqueue(RingBuffer *buffer, readonly u8 *input);

u8 
RingBuffer_Dequeue(RingBuffer* buffer);


#endif // RINGBUFFER_H