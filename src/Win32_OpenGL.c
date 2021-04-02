#include "Win32_OpenGL.h"

void win32_OpenGL_draw_sprite(readonly Platform *platform, readonly OpenGL_Render_Info *render_info, readonly f32 *color)
{
    // NOTE(MIGUEL): Include way to specify gl texture?
    
    // NOTE(MIGUEL): Maybe take an array of uniforms 
    GL_Call(glUniformMatrix4fv(render_info->uniform_model, 1, GL_FALSE, (f32 *)render_info->matrix_model));
    GL_Call(glUniform3f       (render_info->uniform_color, color[0],color[1], color[2]));
    GL_Call(glUniform2f       (render_info->uniform_input, platform->mouse_x / platform->window_width, platform->mouse_y / platform->window_height));
    
    GL_Call(glActiveTexture   (GL_TEXTURE1));
    GL_Call(glBindTexture     (GL_TEXTURE_2D, render_info->texture));
    
    // Enables the alpha channel
    glEnable   (GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    
    GL_Call(glBindVertexArray(render_info->vertex_attributes_id));
    GL_Call(glDrawArrays     (GL_TRIANGLES, 0, 6));
    GL_Call(glBindVertexArray(0));
    
    
    return;
}


// TODO(MIGUEL): Change GL_ to OpenGL_
GLenum 
GL_CheckError_(readonly u8 *file, u32 line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        u8 error[100];
        
        switch (errorCode)
        {
            case GL_INVALID_ENUM                 : strcpy(error, "INVALID_ENUM"                 ); break;
            case GL_INVALID_VALUE                : strcpy(error, "INVALID_VALUE"                ); break; 
            case GL_INVALID_OPERATION            : strcpy(error, "INVALID_OPERATION"            ); break;
            case GL_STACK_OVERFLOW               : strcpy(error, "STACK_OVERFLOW"               ); break;
            case GL_STACK_UNDERFLOW              : strcpy(error, "STACK_UNDERFLOW"              ); break;
            case GL_OUT_OF_MEMORY                : strcpy(error, "OUT_OF_MEMORY"                ); break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: strcpy(error, "INVALID_FRAMEBUFFER_OPERATION"); break;
        }
        printf("%s | %s ( %d ) \r\n", error, file, line);
    }
    return errorCode;
}

b32 GL_Log(readonly u8 *file, readonly u32 line, readonly u8* function)
{
    GLenum errorCode;
    
    while (errorCode = glGetError())
    {
        u8 error[100];
        
        switch (errorCode)
        {
            case GL_INVALID_ENUM                 : strcpy(error, "INVALID_ENUM"                 ); break;
            case GL_INVALID_VALUE                : strcpy(error, "INVALID_VALUE"                ); break; 
            case GL_INVALID_OPERATION            : strcpy(error, "INVALID_OPERATION"            ); break;
            case GL_STACK_OVERFLOW               : strcpy(error, "STACK_OVERFLOW"               ); break;
            case GL_STACK_UNDERFLOW              : strcpy(error, "STACK_UNDERFLOW"              ); break;
            case GL_OUT_OF_MEMORY                : strcpy(error, "OUT_OF_MEMORY"                ); break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: strcpy(error, "INVALID_FRAMEBUFFER_OPERATION"); break;
        }
        
        printf("%s | %s ( %d ) | %s \r\n", error, file, line, function);
        
        return false;
    }
    
    return true;
}

void GL_ClearError(void)
{
    while(glGetError() != GL_NO_ERROR);
    
    return;
}

void OpenGL_VertexBuffer_Create(u32 *vertex_buffer_id, f32 *vertices, u32 size)
{
    GL_Call(glGenBuffers(1, vertex_buffer_id));
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, *vertex_buffer_id));
    GL_Call(glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW));
    
    return;
}

void OpenGL_VertexBuffer_Destroy(u32 vertex_buffer_id)
{
    GL_Call(glDeleteBuffers(1, &vertex_buffer_id));
    
    return;
}

void OpenGL_VertexBuffer_Bind(u32 vertex_buffer_id)
{
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id));
    
    return;
}

void OpenGL_VertexBuffer_Unbind(void)
{
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, 0));
    
    return;
}

// NOTE(MIGUEL): Mimic VertexBuffer functions for index buffers

void OpenGL_IndexBuffer_Create(u32 *index_buffer_id, f32 *index_buffer, u32 count)
{
    GL_Call(glGenBuffers(1, index_buffer_id));
    GL_Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *index_buffer_id));
    GL_Call(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(u32), index_buffer, GL_STATIC_DRAW));
    
    return;
}

void OpenGL_IndexBuffer_Destroy(u32 index_buffer_id)
{
    GL_Call(glDeleteBuffers(1, &index_buffer_id));
    
    return;
}

void OpenGL_IndexBuffer_Bind(u32 index_buffer_id)
{
    GL_Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id));
    
    return;
}

void OpenGL_IndexBuffer_Unbind(void)
{
    GL_Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    
    return;
}