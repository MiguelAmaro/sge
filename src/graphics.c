#include "graphics.h"
#include "helpers.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

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
    glGenVertexArrays(1, &VertexAttributes);
    glBindVertexArray(VertexAttributes);
    
    
    glGenBuffers(1, &VertexBuffer);  
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(TriVerts), TriVerts, GL_DYNAMIC_DRAW);
    
    //VERTEX ATTRIBUTE: POSITION
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(f32), (void*)0);
    glEnableVertexAttribArray(0);
    //VERTEX ATTRIBUTE: COLOR
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1);
    
    
    // UNBIND FROM VERTEX ARRAY OBJECT
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);
    
    
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
    
    glGenVertexArrays(1, &VertexArrayObject);
    glGenBuffers(1, &VertexBufferElement);
    glGenBuffers(1, &EBO);
    
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
    
    
    glGenVertexArrays( 1, &CubeVAO );
    glGenBuffers     ( 1, &CubeVBO );
    glGenBuffers     ( 1, &CubeEBO );
    
    glBindVertexArray(CubeVAO);
    glBindBuffer( GL_ARRAY_BUFFER, CubeVBO );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, CubeEBO );
    
    glBufferData( GL_ARRAY_BUFFER        , sizeof(CubeVerts)   , CubeVerts   , GL_DYNAMIC_DRAW );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(CubeIndicies), CubeIndicies, GL_DYNAMIC_DRAW );
    
    //VERTEX ATTRIBUTE: POSITION
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(f32), (void *)0);
    glEnableVertexAttribArray(0);
    //VERTEX ATTRIBUTE: COLOR
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1);
    
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
    glGenVertexArrays(1, &textureVAO);
    glGenBuffers(1, &textureVBO);
    glGenBuffers(1, &textureEBO);
    
    
    
    glBindVertexArray(textureVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textureVAO);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(texture_verts), texture_verts, GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(texture_indices), texture_indices, GL_DYNAMIC_DRAW);
    
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*)(3 * sizeof(f32)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*)(6 * sizeof(f32)));
    glEnableVertexAttribArray(2);
    
    // THE AFFECTS OF THIS MIGHT NOT BE APPARENT UNSLESS THERE ARE CERTAIN CONDITIONS
    glGenTextures(1, &texture_texture);
    glBindTexture(GL_TEXTURE_2D, texture_texture);
    // CONFIGUE OPENGL WRAPPING OPTIONS
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    // CONFIGURE OPENGL FILTERING OPTIONS
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, texture_border_color); 
    
    
    // LOAD TEXTURE
    s32 width, height, nrChannels;
    u8 *data = stbi_load("../res/images/container.jpg", &width, &height, &nrChannels, 0); 
    if(data)
    {
        // NOTE(MIGUEL): NO AFFECT
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        printf("Failed to load texture");
    }
    
    stbi_image_free(data);
    
    // THIS SHADER MAyBE FUCKED UP
    ReadAShaderFile(&TextureShader, "../res/shaders/TextureShader.glsl");
    
    // UNBIND BUFFER
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);
    
    // NOTE(MIGUEL): ISSUES HERE !!!! 
    glUseProgram(TextureShader);
    glUniform1i(glGetUniformLocation(TextureShader, "texture_texture"), 0);
    
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
    
    glGenVertexArrays(1, &spriteVAO);
    glGenBuffers(1, &spriteVBO);
    
    glBindBuffer(GL_ARRAY_BUFFER, spriteVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sprite_vertices), sprite_vertices, GL_STATIC_DRAW);
    
    glBindVertexArray(spriteVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void *)0x00);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    glCheckError();
    
    // LOAD TEXTURE
    s32 sprite_tex_width, sprite_tex_height, sprite_nrChannels;
    u8 *sprite_tex_data = stbi_load("../res/images/cross.png", &width, &height, &nrChannels, 0); 
    if(data)
    {
        // NOTE(MIGUEL): NO AFFECT
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sprite_tex_width, sprite_tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, sprite_tex_data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        printf("Failed to load texture");
    }
    
    stbi_image_free(sprite_tex_data);
    
    // THIS SHADER MAyBE FUCKED UP
    ReadAShaderFile(&SpriteShader, "../res/shaders/SpriteShader.glsl");
    
    // UNBIND BUFFER
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);
    
    // NOTE(MIGUEL): ISSUES HERE !!!! 
    glUseProgram(SpriteShader);
    glUniform1i(glGetUniformLocation(SpriteShader, "sprite_texture"), 0);
    
    // NOTE(MIGUEL): deleting the folowing 2 lines changes it from black to brown
    
    // FOR LATER
    //mat4 projection = glm_ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);
    
    //~UNIFORMS
    
    
    CubeTransformMatrix = glGetUniformLocation(CubeShader, "transform");
    TriangleColorLocation = glGetUniformLocation(TriangleShader, "triangleColorOffset");
    TrianglePosOffset = glGetUniformLocation(TriangleShader, "trianglePosOffset");
    
    return;
}

// TODO(MIGUEL): Finish writing this func after rendering a single sprite
void SpirteRender_DrawSprite(u32 Texture,  vec2 position, vec2 size, f32 rotate, vec3 color)
{
    
    return;
}

void DrawElement(GLFWwindow *window, u32 VertexAttributes, u32 shaderProgram, u32 VBO, u32 numIndices)
{
    glUseProgram(shaderProgram);
    
    glBindVertexArray(VertexAttributes);
    
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
    
    //glCheckError();
    
    return;
}


void UpdateAndRender(GLFWwindow *window)
{
    //PHYSICS / ANIM???
    
    glCheckError();
    
    // TODO(MIGUEL): CREATE A RENDERER
    // TODO(MIGUEL): SEPERATE RENDING COMPONENTS INTO MODULES - WATCH CHERNO OPENGL SERIES
    //
    
    //~ RENDERING
    
    // SETTING A COLOR TO THE WINDOW
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    
    // TODO(MIGUEL): Find out why this shader doesnt appear
    //~CUBE
    glUseProgram(CubeShader);
    
    // NOTE(MIGUEL): CAUSES INVALID OP on every loop
    glUniformMatrix4fv(CubeTransformMatrix, 1, GL_FALSE, (f32 *)trans);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    DrawElement(window,  CubeVAO, TriangleShader, CubeVBO, 36);
    
    
    
    //~QUAD WITH TEXTURE
    
    //SPRITE QUAD
    // NOTE(MIGUEL): Will assume this is correct and look at init code
    glUseProgram(TextureShader);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_texture);
    
    glBindVertexArray(textureVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    // NOTE(MIGUEL): THE FOLLOING FUCNTION DOESNT WORK WITH TEXTURES
    DrawElement(window, textureVAO, SpriteShader, textureVBO, 6 );
    glBindVertexArray(0);
    
    
    //~QUAD
    glUseProgram(ElementShader);
    //TransformVecCoords(elementVertices, &offsetX, &offsetY);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    DrawElement(window,  VertexArrayObject, ElementShader, VertexBufferElement, 6);
    
    //~TRIANGLE
    glUseProgram(TriangleShader);
    
    // NOTE(MIGUEL): FOLLOWIN 2 FUNC CALLS CAUSE INVALID OP ON FIRST LOOP
    glUniform4f(TrianglePosOffset, cos(loopCounter), sin(loopCounter), 0.0f, 1.0f);
    glUniform4f(TriangleColorLocation, cos(loopCounter), sin(loopCounter), tan(loopCounter), 1.0f);
    
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    DrawTriangle(window, VertexAttributes, TriangleShader, VertexBuffer, TriVerts);
    
    
    glfwSwapBuffers(window);
    
    
    timeValue = glfwGetTime();
    greenValue = (sin(timeValue) / 2.0f) + 0.5f;
    
    
    offsetX += 0.002;
    loopCounter += 0.01134f;
    
    return;
}


void DrawTriangle(GLFWwindow *window, u32 VertexAttributeObject, u32 shaderProgram, u32 VBO, f32 *vertices)
{
    glUseProgram(shaderProgram);
    
    glBindVertexArray(VertexAttributeObject);
    
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    glUseProgram(shaderProgram);
    
    //glCheckError();
    
    return;
}


void InitElement(u32 EBO, u32 VertexBuffer, u32 VertexAttributes, f32* elementVertices, u32 sizeVertBuffer, u32 *indices, u32 sizeOfIndicies)
{
    glBindVertexArray(VertexAttributes);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    
    glBufferData(GL_ARRAY_BUFFER, sizeVertBuffer, elementVertices, GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeOfIndicies, indices, GL_DYNAMIC_DRAW);
    
    // NOTE(MIGUEL): THIS DOESNT COVER ALL POSSIBLE WAYS THAT VERTEX ATTRIBUTES CAN BE SETUP
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void*)0);
    glEnableVertexAttribArray(0);
    
    // NOTE(MIGUEL): UNBINDING AT THIS POINT FUCKS YOU IF U NEED TO CONFIGURE VERTEX ATTRIBUTES AFTER THIS FUNCTION CALL
    // UNBIND VERTEX BUFFER AND VERTEX ATTRIBUTES
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);
    
    
    
    glCheckError();
    
    return;
}
