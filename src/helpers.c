#include "helpers.h"
#include <stdlib.h>

#include <stdio.h>
#include <string.h>

void DisplayMatrix(mat4 matrix, u8 *string)
{
    printf("Ya gotta write dis shit agaan");
}

// NOTE(MIGUEL): This is old hear just to hold things together until  ReadAShaderFile is done
void 
ReadAFile(u8 *Buffer, u32 BytesToRead, readonly u8* path)
{
    FILE *File;
    File = fopen(path, "r");
    ASSERT(File);
    
    fread(Buffer, sizeof(u8), BytesToRead, File);
    
    return;
}



void 
BuildPrefixSuffixTable(u32* table, u32 table_size,readonly u8 *SearchTerm)
{
    printf("Before: %p \n", table);
    //printf("Before: %p \n", table);
    u32* tableOG = table;
    u32 i = 1;
    u32 j = 0;
    
    for(u32 i = 0; i < table_size; i++)
    {
        *(table + i) = 0;
    }
    
    
    while(i < (table_size))
    {
        // LAST ELEMENT
        if(i == (table_size - 1))
        {
            if(*(SearchTerm + i) == *(SearchTerm + j) || j == 0)
            {
                break;
            }
            while(*(SearchTerm + i) != *(SearchTerm + j) && j != 0)
            {
                j = *(table + j - 1);
            }
            *(table + i) = ++j;
            i++;
        }
        // MISMATCH
        else if(*(SearchTerm + i) != *(SearchTerm + j))
        {
            *(table + i) = j = 0;
            i++;
            //printf("Searchi : %c  <- i : %d| Searchj : %c <- j : %d | PSST %d \n", *(SearchTerm + i), i, *(SearchTerm + j), j, *(table + i));
        }
        // MATCH
        else
        {
            //printf("Searchi : %c  <- i : %d| Searchj : %c <- j : %d | PSST %d \n", *(SearchTerm + i), i, *(SearchTerm + j), j, *(table + i));
            j++;
            *(table + i) = j;
            i++;
            
        }
    }
    
    ASSERT(table == tableOG);
    
    return;
}

// TODO(MIGUEL): Take out file and use u8 * instead
u32 StringMatchKMP(readonly u8 *Text, readonly u32 BytesToRead, readonly u8 *SearchTerm)
{
    // NOTE(MIGUEL): This implemenation only returns the first match!!
    readonly u32 SearchTermLength = strlen(SearchTerm);
    u32 result = 0;
    u32 i = 0;
    u32 j = 0;
    
    u32 *table = calloc(SearchTermLength, sizeof(u32)); 
    u32 *tableCopy = table;
    
    ASSERT(Text && table);
    
    
    BuildPrefixSuffixTable(table , SearchTermLength, SearchTerm);
    //for(u32 i = 0; i < SearchTermLength ; i++) { printf("%d ", *(table + i));}
    
    //printf("Before: %p \n", table);
    //printf(SearchTerm);
    
    while(i < BytesToRead)
    {
        //printf("i : %d | j : %d | Text : %c | | Pattern : %c \n",i, j, (u8)*(Text + i), (u8)*(SearchTerm + j));
        
        // CASE: Match
        
        if(*(SearchTerm + j) == *(Text + i))
        {
            i++;
            j++;
        }
        
        if(j == SearchTermLength)
        {
            result = (i - j);
            //printf("MATCH: i = %d - %d = j = %d \n", i , j, (i - j));
            //j = *(table + j - 1);
            break;
        }
        
        // CASE: Mis-match
        
        else if( (i < BytesToRead) && (*(SearchTerm + j) != *(Text + i)))
        {
            if(j != 0)
            {
                j = *(table + j - 1);
                
            }
            else
            {
                i++;
            }
        }
    }
    printf("MATCH: i = %d - %d = j = %d \n", i , j, (i -j));
    //printf("After : %p \n", table);
    
    ASSERT(table == tableCopy);
    
    free(table);
    
    
    return (i - j);
}


#define VERTSEC ("//~VERT SECTION")
#define FRAGSEC ("//~FRAG SECTION")

// TODO(MIGUEL): Remove VerDest and FragDest parameters. rename to processashaderfile
void 
ReadAShaderFile(u32 *ShaderProgram, readonly u8 *path)
{
    printf("\n\n************************\n  %s  \n ******************** \n", path);
    FILE *File = (void *)0x00;
    File = fopen(path, "r");
    ASSERT(File);
    
    //u32 j = 0;
    //u32 i = 1;
    
    u32 BytesToRead = GetFileSize(path);
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
        *(Shader + i) = fgetc(File);
    }
    
    printf("\n\n************************\n  %s \nsize: %d \n ******************** \n", path, BytesToRead);
    //printf("Shader\n %s \n\n", Shader);
    
    u32 VSpos = StringMatchKMP(Shader, BytesToRead, VERTSEC) + sizeof(VERTSEC);
    u32 FSpos = StringMatchKMP(Shader, BytesToRead, FRAGSEC) + sizeof(FRAGSEC);
    
    *(Shader + FSpos - sizeof(FRAGSEC) - 1) = '\0';
    //*(Shader + BytesToRead) = '\0';
    
    *ShaderProgram = CreateShaderProgram(Shader + VSpos, Shader + FSpos);
    
    printf("========== Vertex Shader\n%s \nDONE\n\n", Shader + VSpos);
    printf("========== Fragment Shader\n%s \nDONE\n\n", Shader + FSpos);
    
    //printf("String Match: %d \n", StringMatchKMP(Shader, BytesToRead, ENDSEC));
    //printf("String Match: %d \n", VSpos);
    //printf("String Match: %d \n", FSpos);
    
    fclose(File);
    free(Shader);
    
    return;
}

u32 
GetFileSize(readonly u8 *path) 
{
    FILE *File;
    File = fopen(path, "r");
    ASSERT(File);
    
    fseek(File, 0L, SEEK_END);
    u32 size_bytes = ftell(File);
    
    return size_bytes;
}

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


GLenum 
GL_CheckError_(readonly u8 *file, u32 line)
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

b32 GL_Log(readonly u8 *file, readonly u32 line, readonly u8* function)
{
    GLenum errorCode;
    while (errorCode = glGetError())
    {
        u8 error[100];
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  strcpy(error, "INVALID_ENUM" ); break;
            case GL_INVALID_VALUE:                 strcpy(error, "INVALID_VALUE"); break; 
            case GL_INVALID_OPERATION:             strcpy(error, "INVALID_OPERATION"); break;
            case GL_STACK_OVERFLOW:                strcpy(error, "STACK_OVERFLOW"); break;
            case GL_STACK_UNDERFLOW:               strcpy(error, "STACK_UNDERFLOW"); break;
            case GL_OUT_OF_MEMORY:                 strcpy(error, "OUT_OF_MEMORY"); break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: strcpy(error, "INVALID_FRAMEBUFFER_OPERATION"); break;
        }
        printf("%s | %s ( %d ) %s \r\n", error, file, line, function);
        
        return false;
    }
    
    return true;
}

void GL_ClearError(void)
{
    while(glGetError() != GL_NO_ERROR);
    
    return;
}
