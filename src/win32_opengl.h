/* date = January 29th 2021 11:34 pm */

#ifndef WIN32_OPENGL_H
#define WIN32_OPENGL_H

#include <Windows.h>
#include <GLAD/glad.h>  // NOTE(MIGUEL): Causes SATUS_ACCESS_VIOLATION! FUCK USING GLAD
#include "SGE_Platform.h"
//#include <WGL/wglext.h>
//#include <GL/gl.h> 
#include <cglm/cglm.h>
#include <cglm/util.h>
#include <cglm/vec4.h>
#include <cglm/vec3.h>
#include <cglm/mat4.h>
#include <cglm/affine.h>
//#include <stdbool.h>

#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092

#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_COLOR_BITS_ARB                        0x2014
#define WGL_DEPTH_BITS_ARB                        0x2022
#define WGL_STENCIL_BITS_ARB                      0x2023

#define WGL_FULL_ACCELERATION_ARB                 0x2027
#define WGL_TYPE_RGBA_ARB                         0x202B
#define GL_INVALID_FRAMEBUFFER_OPERATION          0x0506

#define GL_CheckError() GL_CheckError_(__FILE__, __LINE__)

GLenum  GL_CheckError_(readonly u8 *file, u32 line);

#define GL_Call(function) GL_ClearError();\
function;\
GL_Log(__FILE__, __LINE__, #function);


b32  GL_Log(readonly u8 *file, readonly u32 line, readonly u8 *function);
void GL_ClearError(void);


typedef struct
{
    u32  vertex_buffer_id;
    u32  vertex_attributes_id;
    u32  index_buffer_id;
    u32  shader;
    u32  texture;
    f32 *matrix_model;
    u32  uniform_model;
    u32  uniform_projection;
    u32  uniform_color;
    u32  uniform_input;
} OpenGL_Render_Info;

#endif //WIN32_OPENGL_H
