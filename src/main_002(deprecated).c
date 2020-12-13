#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <LAL.h>
#include <string.h>

#define true  (1)
#define false (0)

readonly u32 SCR_WIDTH  = 800;
readonly u32 SCR_HEIGHT = 600;

readonly u8 *vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}";

readonly u8 *fragmentShaderSource =
"#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}";



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
    
    glUseProgram(shaderProgram);
    
    //glCheckError();
    
    return;
}

void UpdateElement(u32 EBO, u32 VertexBuffer, u32 VertexAttributes, f32* elementVertices, u32 sizeVertBuffer, u32 *indices, u32 sizeOfIndicies)
{
    glBindVertexArray(VertexAttributes);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeOfIndicies, indices, GL_DYNAMIC_DRAW);
    
    glBufferData(GL_ARRAY_BUFFER, sizeVertBuffer, elementVertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void*)0);
    
    glEnableVertexAttribArray(0);
    
    
    // UNBIND VERTEX BUFFER AND VERTEX ATTRIBUTES
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);
    
    glCheckError();
    
    return;
}

// NOTE: NOT GENERIC AT ALLLLL
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
    
    glBindVertexArray(0);
    
    glCheckError();
    
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
    u32 VertexBuffer;
    glGenBuffers(1, &VertexBuffer);  
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer); 
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void*)0);
    
    
    // UNBIND FROM VERTEX ARRAY OBJECT
    glEnableVertexAttribArray(0);  
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);
    
    
    
    /* WRONG!!!!!!!!!!!
    float vertices1WrongWay!!!![] = {
        // first triangle
        0.5f,  0.5f, 0.0f,  // top right
        0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f,  0.5f, 0.0f,  // top left 
        // second triangle
        0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left
    }; 
    */
    
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
    
    u32 EBO;
    glGenBuffers(1, &EBO);
    
    /*
    // UNBIND VERTEX BUFFER AND VERTEX ATTRIBUTES
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);
    */
    
    glCheckError();
    
    
    f32 offsetX = 0.01; 
    f32 offsetY = 0.01; 
    
    //****************** 
    //   RENDER LOOP
    //******************
    while(!glfwWindowShouldClose(window))
    {
        // PLATFORM
        glfwPollEvents(); 
        
        // INPUT
        processInput(window);
        
        // RENDERING
        // SETTING A COLOR TO THE WINDOW
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        TransformVecCoords(elementVertices, &offsetX, &offsetY);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        UpdateElement(EBO, VertexBuffer, VertexAttributes, elementVertices,  sizeof(elementVertices), indices, sizeof(indices));
        DrawElement(window, EBO, VertexAttributes, shaderProgram, VertexBuffer, elementVertices);
        
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        UpdateElement(EBO, VertexBuffer, VertexAttributes, vertices,  sizeof(vertices), indices, sizeof(indices));
        DrawTriangle(window, VertexAttributes, shaderProgram, VertexBuffer, vertices);
        
        glfwSwapBuffers(window);
        
        offsetX += 0.002;
    }
    
    glfwTerminate();
    
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

