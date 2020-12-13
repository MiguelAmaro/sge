#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <LAL.h>
#include <string.h>
#include <math.h>
#include <graphics.h>

//DONE!!! TODO(MIGUEL): Make a fucntion that can read one file with vertex and fragment shader code
// TODO(MIGUEL): Make a shader that outputs a sinewave
// TODO(MIGUEL): Make a cube using the wiki opengl tutorial
// TODO(MIGUEL): Create a movable object
// TODO(MIGUEL): Make a tile map and implement some basic colision detection
// TODO(MIGUEL): Apply some kenemtics to movement  
// TODO(MIGUEL): Add some simple collisin detection learingin opengl
// TODO(MIGUEL): Create a networking layer
// TODO(MIGUEL): LOOK OVER GAME ENGINE ARCH BOOK FOR NECCESARY ENGINE SYSTEMS AND UPDATE TODOS
// ************************
// OPENGL ABSTRACTIONS
// -----------------------
// VERTEX BUFFER SETUP
// INDEX BUFFER SETUP
// VERTEX ARRAYS
// ***********************

/*
#define true  (1)
#define false (0)
*/
readonly u32 SCR_WIDTH  = 1280;
readonly u32 SCR_HEIGHT = 720;


void processInput(GLFWwindow *window);
GLFWwindow *CreateWindow(void);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

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

int main()
{
    GLFWwindow *window  = CreateWindow();
    
    GraphicsSetup();
    
    
    //TIMING STUFF
    f64 LastCounter = glfwGetTime();
    
    
    
    //****************** 
    //   RENDER LOOP
    //******************
    while(!glfwWindowShouldClose(window))
    {
        
        // PLATFORM
        glfwPollEvents(); 
        
        // INPUT
        processInput(window);
        
        UpdateAndRender(window);
        
        // TIMING
        f64 EndCounter = glfwGetTime();
        
        f64 CounterElapsed = LastCounter - EndCounter;
        //printf("Micros Elapsed: %f \n", CounterElapsed);
        LastCounter = EndCounter;
    }
    
    glfwTerminate();
    
    
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    
    return;
}


void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    
    return;
}

GLFWwindow *CreateWindow(void)
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
        ASSERT(window);
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    ASSERT(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress));
    
    return window;
}