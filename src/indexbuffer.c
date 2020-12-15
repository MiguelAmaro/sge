#include "indexbuffer.h"

void Create_IndexBuffer(IndexBuffer *buffer, readonly u32 *data, u32 count)
{
    glGenBuffers(1, &buffer.RendererId);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.RendererId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(u32) , data, GL_STATIC_DRAW);
    
    return;
}


void Destroy_IndexBuffer(IndexBuffer *buffer)
{
    glDeleteBuffers(1, &buffer.RendererId);
    
    return;
}

void Bind_IndexBuffer(IndexBuffer *buffer)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.RendererId);
    
    return;
}

void Unbind_IndexBuffer(void)
{
    void Bind(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    return;
}
