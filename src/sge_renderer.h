/* date = January 7th 2021 11:41 pm */

#ifndef SGE_RENDERER_H
#define SGE_RENDERER_H

#include "SGE_Platform.h"
#include "LAL.h"

// NOTE(MIGUEL): Maybe jumping the gun with a struct
// NOTE(MIGUEL): Shader also in Entity struct

/*
typedef struct
{
    u32 vertex_buffer_id;
    u32 vertex_attributes_id;
    u32 index_buffer_id;
    u32  shader;
    u32  texture;
    f32 *matrix_model;
    u32  uniform_model;
    u32  uniform_projection;
    u32  uniform_color;
    u32  uniform_input;
} Render_Info;

void Renderer_Draw_Sprite(readonly Platform *platform, readonly Render_Info *render_info, readonly f32 *color )
{
    // NOTE(MIGUEL): Include way to specify gl texture?
    
    // NOTE(MIGUEL): Maybe take an array of uniforms 
    GL_Call(glUniformMatrix4fv(render_info->uniform_model, 1, GL_FALSE, (f32 *)render_info->matrix_model));
    GL_Call(glUniform3f       (render_info->uniform_color, color[0],color[1], color[2]));
    GL_Call(glUniform2f       (render_info->uniform_input, platform->mouse_x / platform->window_width, platform->mouse_y / platform->window_height));
    
    GL_Call(glActiveTexture(GL_TEXTURE1));
    GL_Call(glBindTexture(GL_TEXTURE_2D, render_info->texture));
    
    // Enables the alpha channel
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    
    GL_Call(glBindVertexArray(render_info->vertex_attributes_id));
    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));
    GL_Call(glBindVertexArray(0));
    
    
    return;
}

*/
#endif //SGE_RENDERER_H
