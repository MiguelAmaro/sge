#include "graphics.h"
#include "helpers.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "vertexbuffer.h"
#include "indexbuffer.h"

f64 loopCounter = .000000f;
f32 offsetX = 0.04; 
f32 offsetY = 0.04; 
f32 timeValue = 0;
f32 greenValue = 0;

f32 TriVerts[18] = {
    // positions         // colors
    0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
    -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
    0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // top 
};    

internal drawcounter = 0;

void DrawSprite()
{
    
    return;
}

void GraphicsSetup()
{
    // CREATING VERTEX ARRAY OBJECT
    // TO SAVE STATE OF THE VERTEX 
    // ATTRIBUTES POINTER
    //~TRIANGLE
    GL_Call(glGenVertexArrays(1, &VertexAttributes));
    GL_Call(glBindVertexArray(VertexAttributes));
    
    
    GL_Call(glGenBuffers(1, &TriVertexBuffer));  
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, TriVertexBuffer)); 
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeof(TriVerts), TriVerts, GL_DYNAMIC_DRAW));
    
    //VERTEX ATTRIBUTE: POSITION
    GL_Call(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(f32), (void*)0));
    GL_Call(glEnableVertexAttribArray(0));
    //VERTEX ATTRIBUTE: COLOR
    GL_Call(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float))));
    GL_Call(glEnableVertexAttribArray(1));
    
    
    // UNBIND FROM VERTEX ARRAY OBJECT
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, 0)); 
    GL_Call(glBindVertexArray(0));
    
    
    //CREATE A SHADER FOR TRIANGLE
    ReadAShaderFile(&TriangleShader, "../res/shaders/TriangleShader.glsl");
    
    //~ELEMENT/QUAD
    // IS A GRAPH
    f32 elementVertices[] = {
        0.3f,  0.3f, 0.0f,  // top right
        0.3f, -0.3f, 0.0f,  // bottom right
        -0.3f, -0.3f, 0.0f,  // bottom left
        -0.3f,  0.3f, 0.0f   // top left 
    };
    
    u32 indices[6] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    }; 
    
    GL_Call(glGenVertexArrays(1, &VertexArrayObject));
    GL_Call(glGenBuffers(1, &VertexBufferElement));
    GL_Call(glGenBuffers(1, &EBO));
    
    // INIT ALL THE BUFFERS
    InitElement(EBO, VertexBufferElement, VertexArrayObject, elementVertices,  sizeof(elementVertices), indices, sizeof(indices));
    
    
    
    //CREATE A SHADER FOR ELEMENT
    ReadAShaderFile(&ElementShader, "../res/shaders/ElementShader.glsl");
    
    //~CUBE
    float CubeVerts[] = 
    {
        // positions         // colors
        0.5f , 0.5f,  0.5f,  0.0f, 1.0f, 0.0f, // 0 right top    front
        0.5f ,-0.5f,  0.5f,  0.0f, 0.0f, 1.0f, // 1 right bottom front
        -0.5f,-0.5f,  0.5f,  0.0f, 0.0f, 1.0f, // 2 left  bottom front
        -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, 0.0f, // 3 left  top    front
        
        -0.5f, 0.5f, -0.5f,  1.0f, 0.0f, 0.0f, // 4 right top    back
        -0.5f,-0.5f, -0.5f,  0.0f, 0.0f, 1.0f, // 5 right bottom back
        0.5f ,-0.5f, -0.5f,  0.0f, 0.0f, 1.0f, // 6 left  bottom back
        0.5f , 0.5f, -0.5f,  0.0f, 1.0f, 0.0f, // 7 left  top    back 
        
    };
    
    u32 CubeIndicies[36] = 
    {
        0, 1, 3, //front
        1, 2, 3, //front
        4, 5, 7, //back
        5, 6, 7, //back
        7, 0, 4, //top
        0, 3, 4, //top
        1, 6, 2, //bottom
        6, 5, 2, //bottom
        3, 2, 4, //left
        2, 5, 4, //left
        7, 6, 0, //right
        6, 1, 0, //right
    };
    
    
    GL_Call(glGenVertexArrays( 1, &CubeVAO ));
    GL_Call(glGenBuffers     ( 1, &CubeVBO ));
    GL_Call(glGenBuffers     ( 1, &CubeEBO ));
    
    GL_Call(glBindVertexArray(CubeVAO));
    GL_Call(glBindBuffer( GL_ARRAY_BUFFER, CubeVBO ));
    GL_Call(glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, CubeEBO ));
    
    GL_Call(glBufferData( GL_ARRAY_BUFFER        , sizeof(CubeVerts)   , CubeVerts   , GL_DYNAMIC_DRAW ));
    GL_Call(glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(CubeIndicies), CubeIndicies, GL_DYNAMIC_DRAW ));
    
    //VERTEX ATTRIBUTE: POSITION
    GL_Call(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(f32), (void *)0));
    GL_Call(glEnableVertexAttribArray(0));
    //VERTEX ATTRIBUTE: COLOR
    GL_Call(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float))));
    GL_Call(glEnableVertexAttribArray(1));
    
    //CREATE A SHADER CUBE
    ReadAShaderFile(&CubeShader, "../res/shaders/CubeShader.glsl");
    
    
    //~ TRYING CGLM STUFF
    vec4 vector = {1.0f, 0.0f, 0.0f, 1.0f};
    mat4 trans = { 0 }; // Identiy Matrix
    glm_mat4_identity(trans);
    
    
    printf(" %f \r\n", vector[0]);
    printf(" %f \r\n", vector[1]);
    printf(" %f \r\n", vector[2]);
    
    
    printf("\r\n");
    
    vec3 translationVec = { 1.0f, 1.0f, 0.0f };
    vec3 rotationVec = { 0.0f, 0.0f, 1.0f };
    vec3 scaleVec = { 0.5f, 0.5f, 0.5f };
    DisplayMatrix(trans, "Trans Matrix");
    
    glm_translate(trans, translationVec);
    glm_rotate(trans, glm_rad(90.0f), rotationVec);
    glm_scale(trans, scaleVec);
    DisplayMatrix(trans, "Trans Matrix");
    
    glm_mat4_mulv(trans, vector, vector);
    
    printf(" %f \r\n", vector[0]);
    printf(" %f \r\n", vector[1]);
    printf(" %f \r\n", vector[2]);
    
    
    
    //~TEXTURE
    
    // TODO(MIGUEL): MAKE A QUAD WITH A TEXTURE
    // TODO(MIGUEL): MAKE ON SPRITE USING LEARNINGOPENGL
    // ISSUE: Other bound buffers are being affected in a way that their position arent being updated or drawn 
    
    float texture_verts[] = {
        // positions          // colors           // texture coords
        0.2f,  0.2f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
        0.2f, -0.2f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -0.2f, -0.2f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -0.2f,  0.2f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
    };
    
    // NOTE(MIGUEL): I MIGHT NOT NEED VAO AND INDICIES
    u32 texture_indices[6] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    }; 
    
    float texture_border_color[] = { 1.0f, 1.0f, 0.0f, 1.0f };
    // NOTE(MIGUEL): Do I DO USE and EBO
    GL_Call(glGenVertexArrays(1, &textureVAO));
    GL_Call(glGenBuffers(1, &textureVBO));
    GL_Call(glGenBuffers(1, &textureEBO));
    
    
    
    GL_Call(glBindVertexArray(textureVAO));
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, textureVBO));
    GL_Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textureVAO));
    
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeof(texture_verts), texture_verts, GL_DYNAMIC_DRAW));
    GL_Call(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(texture_indices), texture_indices, GL_DYNAMIC_DRAW));
    
    // position attribute
    GL_Call(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*)0));
    GL_Call(glEnableVertexAttribArray(0));
    // color attribute
    GL_Call(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*)(3 * sizeof(f32))));
    GL_Call(glEnableVertexAttribArray(1));
    // texture coord attribute
    GL_Call(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*)(6 * sizeof(f32))));
    GL_Call(glEnableVertexAttribArray(2));
    
    // THE AFFECTS OF THIS MIGHT NOT BE APPARENT UNSLESS THERE ARE CERTAIN CONDITIONS
    GL_Call(glGenTextures(1, &texture_texture));
    GL_Call(glBindTexture(GL_TEXTURE_2D, texture_texture));
    // CONFIGUE OPENGL WRAPPING OPTIONS
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));
    // CONFIGURE OPENGL FILTERING OPTIONS
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    
    GL_Call(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, texture_border_color)); 
    
    
    // LOAD TEXTURE
    s32 width, height, nrChannels;
    u8 *data = stbi_load("../res/images/container.jpg", &width, &height, &nrChannels, STBI_rgb); 
    if(data)
    {
        //printf("Tex Data: \n %s \n", data);
        // NOTE(MIGUEL): NO AFFECT
        GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
        GL_Call(glGenerateMipmap(GL_TEXTURE_2D));
    }
    else
    {
        printf("Failed to load texture");
    }
    
    stbi_image_free(data);
    
    // THIS SHADER MAyBE FUCKED UP
    ReadAShaderFile(&TextureShader, "../res/shaders/TextureShader.glsl");
    
    // UNBIND BUFFER
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, 0)); 
    GL_Call(glBindVertexArray(0));
    
    // NOTE(MIGUEL): ISSUES HERE !!!! 
    GL_Call(glUseProgram(TextureShader));
    GL_Call(glUniform1i(glGetUniformLocation(TextureShader, "texture_texture"), 0));
    
    //~GRAPH-> SPRITE.INIT
    
    f32 sprite_vertices[] = { 
        // pos      // tex
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 
        
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };
    
    
    GL_Call(glGenVertexArrays(1, &spriteVAO));
    GL_Call(glGenBuffers(1, &spriteVBO));
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, spriteVBO));
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeof(sprite_vertices), sprite_vertices, GL_DYNAMIC_DRAW));
    
    GL_Call(glBindVertexArray(spriteVAO));
    GL_Call(glEnableVertexAttribArray(0));
    GL_Call(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void *)0x00));
    
    
    // THE AFFECTS OF THIS MIGHT NOT BE APPARENT UNSLESS THERE ARE CERTAIN CONDITIONS
    GL_Call(glGenTextures(1, &sprite_texture));
    GL_Call(glBindTexture(GL_TEXTURE_2D, sprite_texture));
    // CONFIGUE OPENGL WRAPPING OPTIONS
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));
    // CONFIGURE OPENGL FILTERING OPTIONS
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    
    GL_Call(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, texture_border_color)); 
    
    // LOAD TEXTURE
    s32 sprite_tex_width, sprite_tex_height, sprite_nrChannels;
    stbi_set_flip_vertically_on_load(true);  
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
    
    
    // THIS SHADER MAyBE FUCKED UP
    ReadAShaderFile(&SpriteShader, "../res/shaders/SpriteShader.glsl");
    
    
    // NOTE(MIGUEL): ISSUES HERE !!!! 
    GL_Call(glUseProgram(SpriteShader));
    GL_Call(glUniform1i(glGetUniformLocation(SpriteShader, "sprite_texture"), 1));
    
    // NOTE(MIGUEL): deleting the folowing 2 lines changes it from black to brown
    
    // FOR LATER
    mat4 projection;
    GL_Call(glm_ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f, projection));
    GL_Call(glUniformMatrix4fv(glGetUniformLocation(SpriteShader, "projection"), 1, GL_FALSE, (f32 *)projection));
    
    // UNBIND BUFFER
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, 0)); 
    GL_Call(glBindVertexArray(0));
    
    
    //~ CREATE JUST A TEXTURE
    
    
    // THE AFFECTS OF THIS MIGHT NOT BE APPARENT UNSLESS THERE ARE CERTAIN CONDITIONS
    GL_Call(glGenTextures(1, &nick_texture));
    GL_Call(glBindTexture(GL_TEXTURE_2D, nick_texture));
    // CONFIGUE OPENGL WRAPPING OPTIONS
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));
    // CONFIGURE OPENGL FILTERING OPTIONS
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    
    //GL_Call(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, texture_border_color)); 
    
    // LOAD TEXTURE
    s32 nick_tex_width, nick_tex_height, nick_nrChannels;
    u8 *nick_tex_data = stbi_load("../res/images/nick.png", &nick_tex_width, &nick_tex_height, &nick_nrChannels, STBI_rgb_alpha); 
    if(nick_tex_data)
    {
        //printf("Tex Data: \n %d | %d | %s  \n", sprite_tex_width, sprite_tex_height, sprite_tex_data);
        //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        // NOTE(MIGUEL): NO AFFECT
        GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nick_tex_width, nick_tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nick_tex_data));
        GL_Call(glGenerateMipmap(GL_TEXTURE_2D));
    }
    else
    {
        printf("Failed to load texture");
    }
    
    stbi_image_free(nick_tex_data);
    
    
    
    //~UNIFORMS
    
    
    CubeTransformMatrix    = glGetUniformLocation(CubeShader, "transform");
    TriangleColorLocation  = glGetUniformLocation(TriangleShader, "triangleColorOffset");
    TrianglePosOffset      = glGetUniformLocation(TriangleShader, "trianglePosOffset");
    ufrm_sprite_model      = glGetUniformLocation(SpriteShader, "model");
    ufrm_sprite_color      = glGetUniformLocation(SpriteShader, "spriteColor");
    ufrm_sprite_projection = glGetUniformLocation(SpriteShader, "projection");
    
    
    return;
}

// TODO(MIGUEL): Finish writing this func after rendering a single sprite
void SpirteRender_DrawSprite(u32 Texture,  vec2 position, vec2 size, f32 rotate, vec3 color)
{
    
    return;
}

void DrawElement(GLFWwindow *window, u32 VertexAttributes, u32 shaderProgram, u32 VBO, u32 numIndices)
{
    GL_Call(glUseProgram(shaderProgram));
    
    GL_Call(glBindVertexArray(VertexAttributes));
    
    GL_Call(glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0));
    
    
    GL_CheckError();
    
    return;
}


void UpdateAndRender(GLFWwindow *window)
{
    //PHYSICS / ANIM???
    
    
    // TODO(MIGUEL): CREATE A RENDERER
    // TODO(MIGUEL): SEPERATE RENDING COMPONENTS INTO MODULES - WATCH CHERNO OPENGL SERIES <- FUCK THAT 
    //
    
    //~ RENDERING
    GL_CheckError();
    
    // SETTING A COLOR TO THE WINDOW
    GL_Call(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
    GL_Call(glClear(GL_COLOR_BUFFER_BIT));
    
    
    // TODO(MIGUEL): Find out why this shader doesnt appear
    //~CUBE
    GL_Call(glUseProgram(CubeShader));
    
    // NOTE(MIGUEL): CAUSES INVALID OP on every loop
    GL_Call(glUniformMatrix4fv(CubeTransformMatrix, 1, GL_FALSE, (f32 *)trans));
    
    GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    GL_Call(DrawElement(window,  CubeVAO, TriangleShader, CubeVBO, 36));
    GL_Call(glBindVertexArray(0));
    
    //~QUAD WITH TEXTURE
    
    //SPRITE QUAD
    // NOTE(MIGUEL): Will assume this is correct and look at init code
    GL_Call(glUseProgram(TextureShader));
    
    GL_Call(glActiveTexture(GL_TEXTURE0));
    GL_Call(glBindTexture(GL_TEXTURE_2D, texture_texture));
    
    GL_Call(glBindVertexArray(textureVAO));
    GL_Call(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
    // NOTE(MIGUEL): THE FOLLOING FUCNTION DOESNT WORK WITH TEXTURES
    GL_Call(DrawElement(window, textureVAO, TextureShader, textureVBO, 6 ));
    GL_Call(glBindVertexArray(0));
    
    
    //~QUAD
    GL_Call(glUseProgram(ElementShader));
    //TransformVecCoords(elementVertices, &offsetX, &offsetY);
    GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    GL_Call(DrawElement(window,  VertexArrayObject, ElementShader, VertexBufferElement, 6));
    GL_Call(glBindVertexArray(0));
    
    //~TRIANGLE
    GL_Call(glUseProgram(TriangleShader));
    
    // NOTE(MIGUEL): FOLLOWIN 2 FUNC CALLS CAUSE INVALID OP ON FIRST LOOP
    GL_Call(glUniform4f(TrianglePosOffset, cos(loopCounter), sin(loopCounter), 0.0f, 1.0f));
    GL_Call(glUniform4f(TriangleColorLocation, cos(loopCounter), sin(loopCounter), tan(loopCounter), 1.0f));
    
    
    GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    GL_Call(DrawTriangle(window, VertexAttributes, TriangleShader, TriVertexBuffer, TriVerts));
    
    
    //~SPRITE RENDERER_01
    
    static vec3 position = { 0.0f, 0.0f, 0.0f };
    static vec3 size = { 1.0f, 1.0f, 0.0f };
    static vec3 color = { 1.0f, 1.0f, 1.0f };
    static vec2 size2 = { 1.0f, 1.0f };
    static f32 rotate = 10;
    static vec3 scalefactor = { 0 };
    
    
    scalefactor[0] = size[0];
    scalefactor[1] = size[1];
    scalefactor[2] = 1.0f;
    
    rotate++;
    
    GL_Call(glUseProgram(SpriteShader));
    
    mat4 model = GLM_MAT4_IDENTITY_INIT; // constructor
    glm_translate(model, position );
    
    size[0] += 0.5f;
    size[1] += 1.0f;
    glm_translate(model, size); 
    glm_rotate(model, glm_rad(rotate),(vec3){0.0f, 0.0f, 1.0f}); 
    //size[0] += -0.5f;
    //size[1] += -0.5f;
    
    //glm_translate(model, size);
    
    glm_scale(model, scalefactor); 
    
    GL_Call(glUniform3f(ufrm_sprite_color, color[0],color[1], color[2]));
    GL_Call(glUniformMatrix4fv(ufrm_sprite_model, 1, GL_FALSE, (f32 *)model));
    
    
    GL_Call(glActiveTexture(GL_TEXTURE1));
    GL_Call(glBindTexture(GL_TEXTURE_2D, sprite_texture));
    
    // Enables the alpha channel
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    
    GL_Call(glBindVertexArray(spriteVAO));
    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));
    GL_Call(glBindVertexArray(0));
    
    
    
    //~SPRITE RENDERER_02
    
    static vec3 nposition = { 0.0f, 1.0f, 0.0f };
    static vec3 nsize = { 1.0f, 1.0f, 0.0f };
    static vec3 ncolor = { 1.0f, 1.0f, 1.0f };
    static vec2 nsize2 = { 1.0f, 1.0f };
    static f32 nrotate = 10;
    static vec3 nscalefactor = { 0 };
    
    
    nscalefactor[0] = cos(timeValue) * nsize[0];
    nscalefactor[1] = sin(timeValue) * nsize[1];
    nscalefactor[2] = 1.0f;
    
    nposition[0] *= cos(timeValue);
    nposition[1] *= sin(timeValue);
    
    nrotate++;
    
    GL_Call(glUseProgram(SpriteShader));
    
    mat4 nmodel = GLM_MAT4_IDENTITY_INIT; // constructor
    glm_translate(nmodel, nposition );
    
    nsize[0] += 0.5f;
    nsize[1] += 1.0f;
    //glm_translate(nmodel, nsize); 
    //glm_rotate(nmodel, glm_rad(rotate),(vec3){0.0f, 0.0f, 1.0f}); 
    //size[0] += -0.5f;
    //size[1] += -0.5f;
    
    //glm_translate(nmodel, nsize);
    
    glm_scale(nmodel, nscalefactor); 
    
    GL_Call(glUniform3f(ufrm_sprite_color, ncolor[0],ncolor[1], ncolor[2]));
    GL_Call(glUniformMatrix4fv(ufrm_sprite_model, 1, GL_FALSE, (f32 *)nmodel));
    
    
    GL_Call(glActiveTexture(GL_TEXTURE1));
    GL_Call(glBindTexture(GL_TEXTURE_2D, nick_texture));
    
    // Enables the alpha channel
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    
    GL_Call(glBindVertexArray(spriteVAO));
    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));
    GL_Call(glBindVertexArray(0));
    
    
    //~UPDATE STATE
    glfwSwapBuffers(window);
    
    timeValue = glfwGetTime();
    greenValue = (sin(timeValue) / 2.0f) + 0.5f;
    
    
    offsetX += 0.002;
    loopCounter += 0.01134f;
    
    return;
}


void DrawTriangle(GLFWwindow *window, u32 VertexAttributeObject, u32 shaderProgram, u32 VBO, f32 *vertices)
{
    GL_Call(glUseProgram(shaderProgram));
    
    GL_Call(glBindVertexArray(VertexAttributeObject));
    
    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 3));
    
    GL_Call(glUseProgram(shaderProgram));
    
    //GL_CheckError();
    
    return;
}


void InitElement(u32 EBO, u32 TriVertexBuffer, u32 VertexAttributes, f32* elementVertices, u32 sizeVertBuffer, u32 *indices, u32 sizeOfIndicies)
{
    GL_Call(glBindVertexArray(VertexAttributes));
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, TriVertexBuffer));
    GL_Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
    
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeVertBuffer, elementVertices, GL_DYNAMIC_DRAW));
    GL_Call(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeOfIndicies, indices, GL_DYNAMIC_DRAW));
    
    // NOTE(MIGUEL): THIS DOESNT COVER ALL POSSIBLE WAYS THAT VERTEX ATTRIBUTES CAN BE SETUP
    GL_Call(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void*)0));
    GL_Call(glEnableVertexAttribArray(0));
    
    // NOTE(MIGUEL): UNBINDING AT THIS POINT FUCKS YOU IF U NEED TO CONFIGURE VERTEX ATTRIBUTES AFTER THIS FUNCTION CALL
    // UNBIND VERTEX BUFFER AND VERTEX ATTRIBUTES
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, 0)); 
    GL_Call(glBindVertexArray(0));
    
    GL_CheckError();
    
    return;
}
