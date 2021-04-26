/* date = January 25th 2021 0:25 pm */
#ifndef SGE_H
#define SGE_H

#include "sge_platform.h"

//#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))
#define ARRAYCOUNT(array) (sizeof(array) /  sizeof(array[0]))
#define PI_32BIT 3.14159265359

inline u32
safe_truncate_u64(u64 value)
{
    ASSERT(value <= 0xffffffff);
    
    u32 result = (u32)value;
    
    return result;
}

typedef struct
{
    s32 player_tilemap_x;
    s32 player_tilemap_y;
    f32 player_x;
    f32 player_y;
    
    f32 accelx;
    f32 accely;
} game_state;

typedef struct
{
    u32 *tiles;
    
} tile_map;

typedef struct
{
    u32 count_x;
    u32 count_y;
    
    f32 upper_left_x;
    f32 upper_left_y;
    f32 tile_height ;
    f32 tile_width  ;
    
    u32 tilemap_count_x;
    u32 tilemap_count_y;
    tile_map *tilemaps;
} world;

typedef struct
{
    s32 tilemap_x;
    s32 tilemap_y;
    s32 tile_x;
    s32 tile_y;
    f32 x;
    f32 y;
} canonical_position;

typedef struct
{
    s32 tilemap_x;
    s32 tilemap_y;
    f32 x;
    f32 y;
} raw_position;


//~ FUNCTION DECLERATIONS

#define SGE_INIT(    name) void name(game_memory *sge_memory)
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


internal void game_draw_rectangle(game_back_buffer *buffer,
                                  f32 real_min_x, f32 real_max_x,
                                  f32 real_min_y, f32 real_max_y,
                                  f32 r, f32 g, f32 b);


internal void game_update_sound_buffer  (game_state *state, game_sound_output_buffer *sound_buffer, u32 tone_hz);

inline game_controller_input *get_controller(game_input *input, u32 controller_index)
{
    ASSERT(controller_index < (u32)ARRAYCOUNT(input->controllers));
    game_controller_input *result = &input->controllers[controller_index];
    
    return result;
}

#endif //SGE_H


