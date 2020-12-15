#include "vertexbuffer.h"

void Create_VertexBuffer(VertexBuffer *buffer, readonly void *data, u32 size)
{
    glGenBuffers(1, &buffer.RendererId);
    
    glBindBuffer(GL_ARRAY_BUFFER, buffer.RendererId);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    
    return;
}


void Destroy_VertexBuffer(VertexBuffer *buffer)
{
    glDeleteBuffers(1, &buffer.RendererId);
    
    return;
}

void Bind_VertexBuffer(VertexBuffer *buffer)
{
    glBindBuffer(GL_ARRAY_BUFFER, buffer.RendererId);
    
    return;
}

void Unbind_VertexBuffer(void)
{
    void Bind(GL_ARRAY_BUFFER, 0);
    
    return;
}
