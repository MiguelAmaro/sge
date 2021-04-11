/* date = January 25th 2021 0:25 pm */

#ifndef SGE_H
#define SGE_H

#include "windows.h"
#include "SGE_Platform.h"
#include "cglm/mat4.h" // NOTE(MIGUEL): kinda wanna do my own thing isstead

//#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))
#define ARRAYCOUNT(array) (sizeof(array) /  sizeof(array[0]))

typedef struct
{
    int place_holder;
} thread_context;

typedef struct
{
    u32 contents_size;
    void  *contents;
} debug_read_file_result;

#if SGE_INTERNAL
#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(void *memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUG_PlatformFreeFileMemory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(u8 *file_name)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUG_PlatformReadEntireFile);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) b32 name(u8 *file_name, u32 memory_size, void *memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUG_PlatformWriteEntireFile);
#endif


inline u32
safe_truncate_u64(u64 value)
{
    ASSERT(value <= 0xffffffff);
    
    u32 result = (u32)value;
    
    return result;
}

typedef struct
{
    b32 is_initialized   ;
    void *permanent_storage;
    u64   permanent_storage_size;
    void *transient_storage;
    u64   transient_storage_size;
    
    DEBUG_PlatformFreeFileMemory  *debug_platform_free_file_memory ;
    DEBUG_PlatformReadEntireFile  *debug_platform_read_entire_file ;
    DEBUG_PlatformWriteEntireFile *debug_platform_write_entire_file;
} game_memory;

typedef struct
{
    s32 tone_hz;
    // TODO(MIGUEL): fix t_sin to be like HMH 022
    f32 t_sin;
    s32 offset_blue ;
    s32 offset_green;
    
    s32 player_x;
    s32 player_y;
    
    f32 t_jump;
} game_state;

// NOTE(MIGUEL): not imporntant right now
typedef struct
{
    void *data     ;
    s32   width    ;
    s32   height   ;
    s32   pitch    ;
    u32   bytes_per_pixel;
} game_back_buffer;


typedef struct
{
    u16 *samples          ;
    u32 sample_count      ;
    u32 samples_per_second;
    
} game_sound_output_buffer;

////////// THESE STRUCTS ARE ALL SHIT FOR XBOX CONTROLLER A DAKLFJD;AKDJF;ADKFJ

// NOTE(MIGUEL): imporntant ones
typedef struct
{
    u32 half_trans_count; // button half transition
    b32 ended_down;
} game_button_state;

/// Example of a Discriminated Union
typedef struct
{
    b32 is_connected;
    
    /// Stick Input
    b32 is_analog   ;
    f32 stick_avg_x ;
    f32 stick_avg_y ;
    
    /// Buttons Input
    union
    {
        game_button_state buttons[12];
        struct
        {
            // NOTE(MIGUEL): THIS WASNT MEANT TO REPRESENT THE SPECIFIC BUTTON ON A CONTROLLER
            //               JUST THE BEHAVIOR THAT IS SUPPOSED TO BE INFLUENCED BY A BUTTONS
            game_button_state button_a; //Move Down
            game_button_state button_b; //Move Right
            game_button_state button_x; //Move Left
            game_button_state button_y; //Move Up
            
            game_button_state action_up   ;
            game_button_state action_down ;
            game_button_state action_right;
            game_button_state action_left ;
            
            game_button_state shoulder_left ;
            game_button_state shoulder_right;
            
            game_button_state button_start;
            game_button_state button_back ;
            
            game_button_state terminator;
        };
    };
} game_controller_input;

typedef struct
{
    game_button_state mouse_buttons[5];
    
    s32 mouse_x;
    s32 mouse_y;
    s32 mouse_z;
    
    game_controller_input controllers[5];
} game_input;

typedef struct App {
    f32 delta_time;
    
    s32 player_x;
    s32 player_y;
    
    vec3 player_translate;
} App;


//~ FUNCTION DECLERATIONS

#define SGE_INIT(    name) void name(Platform *platform_)
typedef SGE_INIT(SGE_Init);
SGE_INIT(SGEInitStub)
{ return; } 
// NOTE(MIGUEL): what should the return value be??? Any value that matches the function signiture. The stub is just a place holder/ fallback if we cant load the real thing

#define SGE_UPDATE( name) void name(game_memory *sge_memory, game_input *input, game_back_buffer *back_buffer)
typedef SGE_UPDATE(SGE_Update);
SGE_UPDATE(SGEUpdateStub)
{ return; }

// NOTE(MIGUEL): game_memroy doesnt exist
#define SGE_GET_SOUND_SAMPLES(name) void name(game_memory *sge_memory, game_sound_output_buffer *sound_buffer)
typedef SGE_GET_SOUND_SAMPLES(SGE_GetSoundSamples);
// NOTE(MIGUEL): no stub cause game should crash if core fucntions are missing

internal void game_render_weird_gradient(game_back_buffer *buffer, s32 x_offset, s32 y_offset, f32 *delta_t);


inline game_controller_input *get_controller(game_input *input, u32 controller_index)
{
    ASSERT(controller_index < (u32)ARRAYCOUNT(input->controllers));
    game_controller_input *result = &input->controllers[controller_index];
    
    return result;
}

#endif //SGE_H


