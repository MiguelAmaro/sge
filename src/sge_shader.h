/* date = January 5th 2021 7:33 pm */
#ifndef SGE_SHADER_H
#define SGE_SHADER_H
// NOTE(MIGUEL): Why don't i have to include FC_OpenGL.h
#include "SGE_FileIO.h"
#include "SGE_Renderer.h"
#include "Win32_OpenGL.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "LAL.h"

#define VERTSEC ("//~VERT SHADER")
#define FRAGSEC ("//~FRAG SHADER")


u32 
CreateShaderProgram(readonly u8* vertexShaderSource, readonly u8* fragmentShaderSource)
{
    
    u32 vertexShader   = glCreateShader(GL_VERTEX_SHADER);
    u32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    
    s32 success;
    u8 infoLog[512];
    
    // CREATING VERTEX SHADER
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    
    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED | %s \r\n", infoLog);
    }
    
    
    // CREATING FRAGMENT SHADER
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    // Set Debug log Buffer to Zero
    for(u32 byte = 0; byte < 512; ++byte){
        infoLog[byte] = 0;
    }
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    
    if(!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED | %s \r\n", infoLog);
    }
    
    //printf("FFFFFFFUUUUUUUUUUUCCKKKKKKKKKKKKKKKKKKK!!!!!");
    // CREATING A SHADER PROGRAM
    // AND LINKING SHADERS TO IT
    u32 shaderProgram;
    shaderProgram = glCreateProgram();
    
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    
    glLinkProgram(shaderProgram);
    
    //  Set Debug log Buffer to Zero
    for(u32 byte = 0; byte < 512; ++byte){
        infoLog[byte] = 0;
    }
    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER_PROGRAM::LINKING_FAILED | %s \r\n", infoLog);
    }
    
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader); 
    
    return shaderProgram;
}


void 
ReadAShaderFile(u32 *ShaderProgram, readonly u8 *path)
{
    //printf("\n\n************************\n  %s  \n ******************** \n", path);
    FILE *File = (void *)0x00;
    File = fopen(path, "r");
    ASSERT(File);
    
    //u32 j = 0;
    //u32 i = 1;
    
    u32 BytesToRead = FileIO_GetFileSize(path);
    u8 *Shader = calloc( ( BytesToRead + 10 ),  sizeof(u8));
    
    ASSERT(Shader);
    
    // ZERO INITIALIZATION
    for(u32 i = 0; i < ( BytesToRead + 10 ); i++)
    {
        *(Shader + i) = 0x00;
    }
    
    // WRITE FILE INTO BUFFER
    for(u32 i = 0; i < BytesToRead || *(Shader + i) == EOF; i++)
    {
        *(Shader + i) = (u8)fgetc(File);
    }
    
    //printf("\n\n************************\n  %s \nsize: %d \n ******************** \n", path, BytesToRead);
    //printf("Shader\n %s \n\n", Shader);
    
    u32 VSpos = StringMatchKMP(Shader, BytesToRead, VERTSEC) + sizeof(VERTSEC);
    u32 FSpos = StringMatchKMP(Shader, BytesToRead, FRAGSEC) + sizeof(FRAGSEC);
    
    *(Shader + FSpos - sizeof(FRAGSEC) - 1) = '\0';
    //*(Shader + BytesToRead) = '\0';
    
    *ShaderProgram = CreateShaderProgram(Shader + VSpos, Shader + FSpos);
    
    //printf("========== Vertex Shader\n%s \nDONE\n\n", Shader + VSpos);
    //printf("========== Fragment Shader\n%s \nDONE\n\n", Shader + FSpos);
    
    //printf("String Match: %d \n", StringMatchKMP(Shader, BytesToRead, ENDSEC));
    //printf("String Match: %d \n", VSpos);
    //printf("String Match: %d \n", FSpos);
    
    fclose(File);
    free(Shader);
    
    return;
}


void Shader_Load_Texture(OpenGL_Render_Info *render_info, b32 should_flip)
{
    // THE AFFECTS OF THIS MIGHT NOT BE APPARENT UNSLESS THERE ARE CERTAIN CONDITIONS
    GL_Call(glGenTextures(1, &render_info->texture));
    GL_Call(glBindTexture(GL_TEXTURE_2D, render_info->texture));
    // CONFIGUE OPENGL WRAPPING OPTIONS
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));
    // CONFIGURE OPENGL FILTERING OPTIONS
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    
    //GL_Call(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, texture_border_color)); 
    
    // LOAD TEXTURE
    s32 sprite_tex_width, sprite_tex_height, sprite_nrChannels;
    stbi_set_flip_vertically_on_load(should_flip);  
    u8 *sprite_tex_data = stbi_load("../res/images/geo.png", &sprite_tex_width, &sprite_tex_height, &sprite_nrChannels, STBI_rgb_alpha); 
    if(sprite_tex_data)
    {
        //printf("Tex Data: \n %d | %d | %s  \n", sprite_tex_width, sprite_tex_height, sprite_tex_data);
        //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        // NOTE(MIGUEL): NO AFFECT
        GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sprite_tex_width, sprite_tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, sprite_tex_data));
        GL_Call(glGenerateMipmap(GL_TEXTURE_2D));
    }
    else
    {
        printf("Failed to load texture");
    }
    
    stbi_image_free(sprite_tex_data);
    
    
    return;
}

#endif //SGE_SHADER_H
