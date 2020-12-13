/* date = December 3rd 2020 10:20 pm */

#ifndef HELPERS_H
#define HELPERS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <cglm/util.h>
#include <cglm/vec4.h>
#include <cglm/vec3.h>
#include <cglm/mat4.h>
#include <cglm/affine.h>
#include <stdio.h>
#include <LAL.h>


#define glCheckError() glCheckError_(__FILE__, __LINE__) 
GLenum glCheckError_(const char *file, int line);

// NOTE(MIGUEL): Change name of this function???
void InitElement(u32 EBO, u32 VertexBuffer, u32 VertexAttributes, f32* elementVertices, u32 sizeVertBuffer, u32 *indices, u32 sizeOfIndicies);

u32 GetFileSize(readonly u8 *path);
u32 StringMatchKMP(readonly u8 *Text, readonly u32 BytesToRead, readonly u8* SearchTerm);
void ReadAFile(u8 *Buffer, u32 BytesToRead, readonly u8* path);
void ReadAShaderFile(u32 *ShaderProgram, readonly u8 *path);
u32 CreateShaderProgram(readonly u8* vertexShaderSource, readonly u8* fragmentShaderSource);
void DisplayMatrix(mat4 matrix, u8 *string);


#endif //HELPERS_H
