#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <LAL.h>
#include <string.h>
#include <math.h>

#define ASSERT(Expression) if(!(Expression)) { *(int*)0 = 0; }

#define true  (1)
#define false (0)

readonly u32 SCR_WIDTH  = 800;
readonly u32 SCR_HEIGHT = 600;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);

GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        u8 error[100];
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  strcpy(error, "INVALID_ENUM\0" ); break;
            case GL_INVALID_VALUE:                 strcpy(error, "INVALID_VALUE\0"); break;
            case GL_INVALID_OPERATION:             strcpy(error, "INVALID_OPERATION\0"); break;
            case GL_STACK_OVERFLOW:                strcpy(error, "STACK_OVERFLOW\0"); break;
            case GL_STACK_UNDERFLOW:               strcpy(error, "STACK_UNDERFLOW\0"); break;
            case GL_OUT_OF_MEMORY:                 strcpy(error, "OUT_OF_MEMORY\0"); break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: strcpy(error, "INVALID_FRAMEBUFFER_OPERATION\0"); break;
        }
        printf("%s | %s ( %d ) \r\n", error, file, line);
    }
    return errorCode;
}

#define glCheckError() glCheckError_(__FILE__, __LINE__) 

void DrawTriangle(GLFWwindow *window, u32 VertexAttributeObject, u32 shaderProgram, u32 VBO, f32 *vertices)
{
    glUseProgram(shaderProgram);
    
    glBindVertexArray(VertexAttributeObject);
    
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
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
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void*)0);
    glEnableVertexAttribArray(0);
    
    
    // UNBIND VERTEX BUFFER AND VERTEX ATTRIBUTES
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);
    
    glCheckError();
    
    return;
}

void ReadFile(u8 *Buffer, u32 BytesToRead, readonly u8* path)
{
    FILE *File;
    File = fopen(path, "r");
    ASSERT(File);
    
    fread(Buffer, sizeof(u8), BytesToRead, File);
    
    return;
}

u32 CreateShaderProgram(readonly u8* vertexShaderSource, readonly u8* fragmentShaderSource)
{
    
    u32 vertexShader   = glCreateShader(GL_VERTEX_SHADER);
    u32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    
    // CREATING VERTEX SHADER
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    s32 success;
    u8 infoLog[512];
    
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    
    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED | %s \r\n", infoLog);
    }
    
    
    // CREATING FRAGMENT SHADER
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    for(u32 byte = 0; byte < 512; ++byte){
        infoLog[byte] = 0;
    }
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    
    if(!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED | %s \r\n", infoLog);
    }
    
    
    // CREATING A SHADER PROGRAM
    // AND LINKING SHADERS TO IT
    u32 shaderProgram;
    shaderProgram = glCreateProgram();
    
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    
    glLinkProgram(shaderProgram);
    
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

// NOTE: NOT GENERIC AT ALLLLL DONT THINK THIS HOW I SHOULE MOVE THINGS
void TransformVecCoords(f32 vertices[], f32 *offsetX, f32 *offsetY)
{
    
    if(vertices[ 0] <= 1.00 &
       vertices[ 1] <= 1.00 &
       
       vertices[ 3] <=  1.00 &
       vertices[ 4] >= -1.00 &
       
       vertices[ 6] >= -1.00 &
       vertices[ 7] >= -1.00 &
       
       vertices[ 9] >= -1.00 &
       vertices[10] <= 1.00 )
    {
        vertices[ 0] += *offsetX;
        vertices[ 1] += *offsetY;
        vertices[ 2] += 0.00;
        vertices[ 3] += *offsetX;
        vertices[ 4] += *offsetY;
        vertices[ 5] += 0.00;
        vertices[ 6] += *offsetX;
        vertices[ 7] += *offsetY;
        vertices[ 8] += 0.00;
        vertices[ 9] += *offsetX;
        vertices[10] += *offsetY;
        vertices[11] += 0.00;
        
    }
    else
    {
        *offsetX *= -1;
        *offsetY *= -1;
        vertices[ 0] += *offsetX - (0.0001);
        vertices[ 1] += *offsetY - (0.0001);
        
        vertices[ 3] += *offsetX - (0.0001);
        vertices[ 4] += *offsetY + (0.0001);
        
        vertices[ 6] += *offsetX + (0.0001);
        vertices[ 7] += *offsetY + (0.0001);
        
        vertices[ 9] += *offsetX + (0.0001);
        vertices[10] += *offsetY - (0.0001);
        
    }
    
    return;
}

void DrawElement(GLFWwindow *window, u32 EBO, u32 VertexAttributes, u32 shaderProgram, u32 VBO, f32 *vertices)
{
    glUseProgram(shaderProgram);
    
    glBindVertexArray(VertexAttributes);
    
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    //glCheckError();
    
    return;
}


void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Learn OpenGL", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create GLFW window \r\n");
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD \r\n");
        return -1;
    }    
    
    
    // DEFINING VERTEX BUFFER
    // OF VERTICIES OF THE TRIANGLE
    f32 vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f , -0.5f, 0.0f,
        0.0f ,  0.5f, 0.0f,
    };
    
    // CREATING VERTEX ARRAY OBJECT
    // TO SAVE STATE OF THE VERTEX 
    // ATTRIBUTES POINTER
    u32 VertexAttributes;
    glGenVertexArrays(1, &VertexAttributes);
    glBindVertexArray(VertexAttributes);
    
    
    // CREATING A VERTEX BUFFER OBJECT
    // AND BINDING VERTEX BUFFER TO IT
    u32 VertexBuffer = { 0 };
    glGenBuffers(1, &VertexBuffer);  
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer); 
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void*)0);
    glEnableVertexAttribArray(0);  
    
    
    // UNBIND FROM VERTEX ARRAY OBJECT
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);
    
    
    //CREATE A SHADER FOR TRIANGLE
    u8 TriangleVertexShaderSource[512] = { 0 };
    u8 TriangleFragmentShaderSource[512] = { 0 };
    
    ReadFile(TriangleVertexShaderSource, 1024, "../TriangleVertexShader.vert");
    ReadFile(TriangleFragmentShaderSource, 1024, "../TriangleFragmentShader.frag");
    u32 TriangleShader = CreateShaderProgram(TriangleVertexShaderSource, TriangleFragmentShaderSource);
    
    f32 elementVertices[] = {
        0.3f,  0.3f, 0.0f,  // top right
        0.3f, -0.3f, 0.0f,  // bottom right
        -0.3f, -0.3f, 0.0f,  // bottom left
        -0.3f,  0.3f, 0.0f   // top left 
    };
    
    u32 indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    }; 
    
    u32 VertexArrayObject;
    u32 VertexBufferElement;
    u32 EBO;
    glGenVertexArrays(1, &VertexArrayObject);
    glGenBuffers(1, &VertexBufferElement);
    glGenBuffers(1, &EBO);
    
    // INIT ALL THE BUFFERS
    InitElement(EBO, VertexBufferElement, VertexArrayObject, elementVertices,  sizeof(elementVertices), indices, sizeof(indices));
    
    
    
    //CREATE A SHADER FOR ELEMENT
    u8 ElementVertexShaderSource[512] = { 0 };
    u8 ElementFragmentShaderSource[512] = { 0 };
    
    ReadFile(ElementVertexShaderSource, 1024, "../ElementVertexShader.vert");
    ReadFile(ElementFragmentShaderSource, 1024, "../ElementFragmentShader.frag");
    u32 ElementShader = CreateShaderProgram(ElementVertexShaderSource, ElementFragmentShaderSource);
    
    
    u32 CubeVAO;
    u32 CubeVBO;
    u32 CubeEBO;
    
    float CubeVerts[] = 
    {
        // positions         // colors
        -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, 0.0f, // 0 left  top    front
        0.5f , 0.5f,  0.5f,  0.0f, 1.0f, 0.0f, // 1 right top    front
        0.5f ,-0.5f,  0.5f,  0.0f, 0.0f, 1.0f, // 2 left  bottom front
        0.5f ,-0.5f,  0.5f,  0.0f, 0.0f, 1.0f, // 3 right bottom front
        -0.5f, 0.5f, -0.5f,  1.0f, 0.0f, 0.0f, // 4 left  top    back
        0.5f , 0.5f, -0.5f,  0.0f, 1.0f, 0.0f, // 5 right top    back 
        -0.5f,-0.5f, -0.5f,  0.0f, 0.0f, 1.0f, // 6 left  bottom back
        0.5f ,-0.5f, -0.5f,  0.0f, 0.0f, 1.0f, // 7 right bottom back
    };
    
    f32 CubeIndicies[] = 
    {
        0, 1, 2, //front
        2, 3, 0, //front
        4, 5, 6, //back
        4, 5, 6, //back
        0, 1, 4, //top
        0, 1, 4, //top
        2, 3, 6, //bottom
        2, 3, 6, //bottom
        0, 2, 4, //left
        0, 2, 4, //left
        1, 3, 5, //right
        1, 3, 5, //right
    };
    
    
    glGenVertexArrays( 1, &CubeVAO );
    glGenBuffers     ( 1, &CubeVBO );
    glGenBuffers     ( 1, &CubeEBO );
    
    glBindVertexArray(CubeVAO);
    glBindBuffer( GL_ARRAY_BUFFER, CubeVBO );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, CubeEBO );
    
    glBufferData( GL_ARRAY_BUFFER        , sizeof(CubeVerts)   , CubeVerts   , GL_DYNAMIC_DRAW );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(CubeIndicies), CubeIndicies, GL_DYNAMIC_DRAW );
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void *)0);
    glEnableVertexAttribArray(0);
    
    glCheckError();
    
    f64 loopCounter = .000000f;
    f32 offsetX = 0.04; 
    f32 offsetY = 0.04; 
    f32 timeValue = 0;
    f32 greenValue = 0;
    u32 vertexColorLocation = glGetUniformLocation(TriangleShader, "ourColor");
    u32 TrianglePosOffset = glGetUniformLocation(TriangleShader, "trianglePosOffset");
    //****************** 
    //   RENDER LOOP
    //******************
    while(!glfwWindowShouldClose(window))
    {
        // PLATFORM
        glfwPollEvents(); 
        
        // INPUT
        processInput(window);
        
        //PHYSICS / ANIM???
        //TransformVecCoords(elementVertices, &offsetX, &offsetY);
        
        
        
        glUniform4f(TrianglePosOffset, cos(loopCounter), sin(loopCounter), 0.0f, 1.0f);
        
        timeValue = glfwGetTime();
        greenValue = (sin(timeValue) / 2.0f) + 0.5f;
        //offsetX = cos(timeValue);
        //offsetY = (sin(timeValue));
        //printf("%f \n", loopCounter);
        
        
        
        greenValue = (sin(timeValue) / 2.0f) + 0.5f;
        
        glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);
        glUseProgram(TriangleShader);
        
        // RENDERING
        // SETTING A COLOR TO THE WINDOW
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        DrawElement(window, CubeEBO, CubeVAO, ElementShader, CubeVBO, CubeVerts);
        
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        DrawElement(window, EBO, VertexArrayObject, TriangleShader, VertexBufferElement, elementVertices);
        /*
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        DrawTriangle(window, VertexAttributes, TriangleShader, VertexBuffer, vertices);
        */
        
        glfwSwapBuffers(window);
        
        offsetX += 0.002;
        loopCounter += 0.01134f;
    }
    
    glfwTerminate();
    
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

