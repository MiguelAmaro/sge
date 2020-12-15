/* date = December 13th 2020 10:58 pm */

#ifndef INDEXBUFFER_H
#define INDEXBUFFER_H

typedef struct 
{
    u32 RendererId;
    u32 count;
}IndexBuffer;

void Create_IndexBuffer(IndexBuffer *buffer, readonly u32 *data, u32 count);
void Destroy_IndexBuffer(IndexBuffer *buffer);

void Bind_IndexBuffer(IndexBuffer *buffer);
void Unbind_IndexBuffer(void);

#endif //INDEXBUFFER_H
