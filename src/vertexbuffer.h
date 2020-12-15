/* date = December 13th 2020 8:11 pm */

#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

typedef struct 
{
    u32 RendererId;
}VertexBuffer;

void Create_VertexBuffer(VertexBuffer *buffer, readonly void *data, u32 size);
void Destroy_VertexBuffer(VertexBuffer *buffer);

void Bind_VertexBuffer(VertexBuffer *buffer);
void Unbind_VertexBuffer(void);

#endif //VERTEXBUFFER_H
