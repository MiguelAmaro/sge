#include "RingBuffer.h"


// NOTE(MIGUEL): !!! Almost useless function !!!
void 
RingBuffer_Init(Ringbuffer *buffer, u8 *data, u32 capacity) 
{
    if(buffer)
    {
        buffer->data     = data;
        buffer->capacity = capacity;
        buffer->size     = 0;
        buffer->head     = 0;
        buffer->tail     = 0;
        
        if(buffer->data){
            for(u32 dataIndex = 0; dataIndex < capacity; dataIndex++)
            {
                buffer->data[dataIndex] = RING_BUFFER_DEFAULT_VALUE;
            }
        }
        else 
        {
            printf("Allocation For RingBuffer->Data(PTR) Failed");
        }
    }
    else{
        printf("Allocation For RingBuffer(PTR) Failed");
    }
    
    return;
}

b32 
RingBuffer_Full(RingBuffer* buffer) 
{
    
    return buffer->size == buffer->capacity;
}

b32 
RingBuffer_Empty(RingBuffer* buffer) 
{
    
    return buffer->size == 0;
}

// NOTE(MIGUEL): !!! Almost useless function !!!
void 
RingBuffer_Destroy(RingBuffer* buffer) 
{
    free(buffer->data);
    free(buffer);
    
    return;
}

void 
RingBuffer_Enqueue(RingBuffer *buffer, readonly u8 *input) 
{
    while(input)
    {        
        if(!RingBuffer_Full(buffer)){
            buffer->data[buffer->tail++] = *input;
            buffer->tail %= buffer->capacity;
            buffer->size++;
        }
    }
    
    return;
}

u8 
RingBuffer_Dequeue(RingBuffer* buffer) 
{
    u8 data = 0;
    
    if(!RingBuffer_Empty(buffer))
    {
        data = buffer->data[buffer->head];
        buffer->data[buffer->head++] = RING_BUFFER_DEFAULT_VALUE;
        buffer->head %= buffer->capacity;
        buffer->size--;
    }
    
    return data;   
}

