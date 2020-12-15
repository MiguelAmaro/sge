/* date = December 14th 2020 0:32 am */

#ifndef VERTEXBUFFERLAYOUT_H
#define VERTEXBUFFERLAYOUT_H

typedef VertexBufferElement
{
    u32 count;
    u32 type;
    b32 normalized;
}

// TODO(MIGUEL): is private Move to .c file
VertexBufferElement *Elements;

void Create_VertexBufferLayout();

void Push(u32 count)
{
    //static assert false...
    return;
}

void
#endif //VERTEXBUFFERLAYOUT_H
