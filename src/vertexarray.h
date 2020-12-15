/* date = December 14th 2020 0:06 am */

#ifndef VERTEXARRAY_H
#define VERTEXARRAY_H

#include "vertexbuffer.h"

void Create_VertexArray(void);
void Destroy_VertexArray(void);

void AddBuffer(readonly VertexBuffer &buffer, readonly VertexBufferLayout &layout);
#endif //VERTEXARRAY_H
