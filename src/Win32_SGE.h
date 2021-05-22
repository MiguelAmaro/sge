/* date = January 29th 2021 7:36 pm */

#ifndef WIN32_SGE_H
#define WIN32_SGE_H

#include <GLAD/glad.h>
#include "Win32_OpenGL.h"
#include "SGE.h"
#include "SGE_Program_Options.h"
#include "SGE_Platform.h"
#include "SGE_Shader.h"

#define WIN32_STATE_FILE_NAME_COUNT (MAX_PATH)

typedef enum
{
    stream_state = 0,
    stream_input = 1,
} stream_type;

typedef struct
{
    HANDLE file_handle;
    HANDLE memory_map;
    u8 file_name[WIN32_STATE_FILE_NAME_COUNT];
    void *memory_block;
} win32_replay_buffer;

typedef struct
{
    u32 width;
    u32 height;
} win32_window_dimensions;

typedef struct
{
    HMODULE SGE_DLL   ;
    SGE_Update          *update;
    SGE_Init            *init  ;
    SGE_GetSoundSamples *get_sound_samples;
    b32                 is_valid      ;
    FILETIME            dll_last_write_time;
} win32_game_code;

typedef struct
{
    u64    main_memory_block_size;
    void  *main_memory_block     ;
    
    HANDLE input_record_handle  ;
    HANDLE input_playback_handle;
    
    s32                 input_record_index     ;
    s32                 input_playback_index   ;
    win32_replay_buffer input_replay_buffers[4];
    
    u8     exe_file_name[WIN32_STATE_FILE_NAME_COUNT];
    u8    *one_past_last_exe_file_name_slash;
} win32_state;


//~ GRAPHICS 

// NOTE(MIGUEL): consider naming to _offscreen_buffer;
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
    u32 *tiles;
} Tile_map;


//~ AUDIO INTERFACE

typedef struct
{
    b32 is_playing           ;
    u32 samples_per_second   ;
    u32 tone_hz              ;
    s32 tone_volume          ;
    u32 wave_period          ;
    u32 running_sample_index ;
    u32 bytes_per_sample     ;
    u32 secondary_buffer_size;
    f32 t_sin                ;
    u32 latency_sample_count ;
    u32 fill_pos_tolerance   ;
} win32_sound_output;

typedef struct
{
    u32 output_play_cursor;
    u32 output_write_cursor;
    
    u32 output_location  ;
    u32 output_byte_count;
    
    u32 flip_play_cursor ;
    u32 flip_write_cursor;
} win32_debug_sound_time_marker;


internal LRESULT CALLBACK 
win32_Main_Window_Procedure   (HWND Window, UINT Message , WPARAM w_param, LPARAM l_param);

internal void 
win32_resize_DIB_Section      (int Width, int Height);

internal void 
win32_update_Window           (HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height);

internal void
win32_Init_OpenGL_Extensions  (void);

internal HGLRC
win32_Init_OpenGL             (HDC real_dc);



//~ WINDOW INTERFACE
internal win32_window_dimensions
win32_window_get_dimensions(HWND window);

internal void
win32_window_display(win32_back_buffer *buffer, HDC device_context, u32 window_width, u32 window_height);


//~ LIVE CODE EDITING INTERFACE
internal win32_game_code 
win32_load_game_code          (u8 *source_dll_name, u8 *temp_dll_name, u8 *lock_file_name);

internal void 
win32_unload_game_code        (win32_game_code *game);


//~ FILE MANAGING INTERFACE
internal FILETIME 
win32_get_last_write_time     (u8 *file_name);

internal void
win32_get_exe_file_name       (win32_state *state);

internal void
win32_build_exe_path_file_name(win32_state *state, u8 *file_name,
                               int dest_count, u8 *dest);


//~ INPUT RECORDING INTERFACE
internal void
win32_input_get_file_location (win32_state *state, b32 input_stream,
                               u32 slot_index, u32 dest_count, u8 *dest);

internal win32_replay_buffer *
win32_input_get_replay_buffer (win32_state *state, u32 index);

internal void
win32_input_begin_recording   (win32_state *state, s32 input_record_index);

internal void
win32_input_end_recording     (win32_state *state);

internal void
win32_input_begin_playback    (win32_state *state, u32 input_playback_index);

internal void
win32_input_end_playback      (win32_state *state);

internal void
win32_input_record            (win32_state *state, game_input *new_input);

internal void
win32_input_playback          (win32_state *state, game_input *new_input);


//~ STRINGS
internal void
string_concat                 (size_t source_a_count, u8 *source_a,
                               size_t source_b_count, u8 *source_b,
                               size_t dest_count    , u8 *dest    );

internal u32
string_get_length             (u8 *string);



#endif //WIN32_SGE_H
