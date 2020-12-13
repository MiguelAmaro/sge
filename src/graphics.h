/* date = October 2nd 2020 9:32 pm */

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <cglm/util.h>
#include <cglm/vec4.h>
#include <cglm/vec3.h>
#include <cglm/mat4.h>
#include <cglm/affine.h>

#include <LAL.h>

internal u32 CubeTransformMatrix;
internal u32 TriangleColorLocation;
internal u32 TrianglePosOffset;

internal f64 loopCounter;
internal f32 offsetX;
internal f32 offsetY;
internal f32 timeValue;
internal f32 greenValue;

internal u32 TriangleShader;
internal u32 ElementShader;
internal u32 CubeShader;

internal u32 CubeVAO;
internal u32 CubeVBO;
internal u32 CubeEBO;


internal u32 VertexArrayObject;
internal u32 VertexBufferElement;
internal u32 EBO;

internal u32 VertexAttributes;

// CREATING A VERTEX BUFFER OBJECT
// AND BINDING VERTEX BUFFER TO IT

internal u32 VertexBuffer;
internal f32 TriVerts[18];

// DEFINING VERTEX BUFFER
// OF VERTICIES OF THE TRIANGLE


internal mat4 trans;

internal u32 TextureShader;
internal u32 textureVBO;
internal u32 textureVAO;
internal u32 textureEBO;
internal u32 texture_texture;

internal u32 spriteVBO;
internal u32 spriteVAO;
internal u32 SpriteShader;
internal u32 Sprite_texture;

void GraphicsSetup();
void UpdateAndRender(GLFWwindow *window);
void DrawElement(GLFWwindow *window, u32 VertexAttributes, u32 shaderProgram, u32 VBO, u32 numIndices);
void DrawTriangle(GLFWwindow *window, u32 VertexAttributeObject,
                  u32 shaderProgram, u32 VBO, f32 *vertices);

void InitElement(u32 EBO, u32 VertexBuffer, u32 VertexAttributes, f32* elementVertices, u32 sizeVertBuffer, u32 *indices, u32 sizeOfIndicies);
#endif //GRAPHICS_H
