/* date = January 29th 2021 7:36 pm */

#ifndef WIN32_SGE_H
#define WIN32_SGE_H

#include <GLAD/glad.h>
#include "Win32_OpenGL.h"
#include "SGE.h"
#include "SGE_Program_Options.h"
#include "SGE_Platform.h"
#include "SGE_Shader.h"


typedef struct
{
    HMODULE SGE_DLL   ;
    SGE_Update *Update;
    SGE_Init   *Init  ;
    b32 is_valid      ;
    FILETIME dll_last_write_time;
} win32_game_code;

typedef struct
{
    BITMAPINFO info      ;
    void *data           ;
    u32   width          ;
    u32   height         ;
    u32   pitch          ;
    u32   bytes_per_pixel;
} win32_back_buffer;

typedef struct
{
    HANDLE record_handle;
    s32 input_record_index;
    
    HANDLE playback_handle;
    s32 input_playback_index; 
} win32_state;

typedef struct
{
    u32 *tiles;
} Tile_map;

typedef struct World
{
    u32 tilemap_count_x;
    u32 tilemap_count_y;
    
    Tile_map *tile_maps;
} World;


internal LRESULT CALLBACK 
win32_Main_Window_Procedure (HWND Window, UINT Message , WPARAM w_param, LPARAM l_param);

internal void 
win32_resize_DIB_Section    (int Width, int Height);

internal void 
win32_update_Window         (HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height);

internal void
win32_Init_OpenGL_Extensions(void);

internal HGLRC
win32_Init_OpenGL           (HDC real_dc);

//~ LIVE CODE EDITING

internal win32_game_code 
win32_load_game_code        (void);

internal void 
win32_unload_game_code      (win32_game_code *game);

//~ LOOPED CODE EDITTING

internal FILETIME
win32_get_last_write_time   (u8 *file_name);


#endif //WIN32_SGE_H
