/* date = January 25th 2021 0:25 pm */
#ifndef SGE_H
#define SGE_H

#include "sge_platform.h"

//#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))
#define ARRAYCOUNT(array) (sizeof(array) /  sizeof(array[0]))
#define PI_32BIT 3.14159265359

#define WORLD_CHUNK_SHIFT (8)
#define WORLD_CHUNK_MASK  ((1 << 8) - 1)

inline u32
safe_truncate_u64(u64 value)
{
    ASSERT(value <= 0xffffffff);
    
    u32 result = (u32)value;
    
    return result;
}

typedef struct
{
    u32 *tiles;
    
} tile_chunk;

typedef struct
{
    u32 tile_chunk_x;
    u32 tile_chunk_y;
    
    u32 tile_rel_x;
    u32 tile_rel_y;
} tile_chunk_position;

typedef struct
{
    u32 tile_abs_x;
    u32 tile_abs_y;
    
    f32 tile_rel_x;
    f32 tile_rel_y;
} world_position;

typedef struct
{
    f32 tile_side_in_meters;
    s32 tile_side_in_pixels;
    f32 meters_to_pixels;
    
    u32 chunk_dimensions;
    
    s32 tilechunk_count_x;
    s32 tilechunk_count_y;
    
    tile_chunk *tilechunks;
} world;


typedef struct
{
    world_position player_pos;
    
    f32 accelx;
    f32 accely;
} game_state;

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


