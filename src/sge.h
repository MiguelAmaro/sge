/* date = January 25th 2021 0:25 pm */
#ifndef SGE_H
#define SGE_H

#include "sge_platform.h"

inline u32
safe_truncate_u64(u64 value)
{
    ASSERT(value <= 0xffffffff);
    
    u32 result = (u32)value;
    
    return result;
}

//~ MEMORY INTERFACE

typedef struct MemoryArena MemoryArena;
struct MemoryArena
{
    memory_index size;
    memory_index used;
    u8 *base_ptr;
};

internal void
MemoryArena_init(MemoryArena *arena, memory_index size, u8 *base_ptr)
{
    arena->base_ptr = base_ptr;
    arena->size     = size;
    arena->used     = 0;
    
    return;
}

#define MEMORY_ARENA_PUSH_STRUCT(arena,        type) (type *)MemoryArena_push_block(arena, sizeof(type))
#define MEMORY_ARENA_PUSH_ARRAY( arena, count, type) (type *)MemoryArena_push_block(arena, (count) * sizeof(type))
#define MEMORY_ARENA_ZERO_STRUCT(instance          )         MemoryArena_zero_block(sizeof(instance), &(instance))
inline void *
MemoryArena_push_block(MemoryArena *arena, memory_index size)
{
    ASSERT((arena->used + size) <= arena->size);
    
    void *new_arena_partition_adress  = arena->base_ptr + arena->used;
    arena->used  += size;
    
    return new_arena_partition_adress;
}
inline void
MemoryArena_zero_block(memory_index size, void *address)
{
    u8 *byte = (u8 *)address;
    
    while(size--)
    {
        *byte++ = 0;
    }
    
    return;
}
typedef enum EntityType EntityType;

#include "sge_intrinsics.h"
#include "sge_math.h"
#include "sge_world.h"
#include "sge_sim_region.h"
#include "sge_entity.h"

#pragma pack(push, 1)
typedef struct BitmapHeader BitmapHeader;
struct BitmapHeader
{
    u16 file_type;
    u32 file_size;
    u16 reserved_1;
    u16 reserved_2;
    u32 bitmap_offset;
    u32 size;
    s32 width;
    s32 height;
    u16 planes;
    u16 bits_per_pixel;
    u32 compression;
    u32 sizeofbitmap;
    s32 horz_resolution;
    s32 vert_resolution;
    u32 colors_used;
    u32 colors_important;
    
    u32 red_mask;
    u32 green_mask;
    u32 blue_mask;
};
#pragma pack(pop)

typedef struct BitmapData BitmapData;
struct BitmapData
{
    s32  width;
    s32  height;
    u32 *pixels;
};

typedef struct PlayerBitmaps PlayerBitmaps;
struct PlayerBitmaps
{
    V2 align;
    BitmapData head;
    BitmapData cape;
    BitmapData torso;
};

typedef enum EntityType EntityType;
enum EntityType
{
    EntityType_null,
    EntityType_player,
    EntityType_friendly,
    EntityType_hostile,
    EntityType_wall,
    EntityType_sword,
};

typedef struct EntityLow EntityLow;
struct EntityLow
{
    WorldCoord position;
    EntitySim  sim;
};  

typedef struct ControlledPlayer ControlledPlayer;
struct ControlledPlayer
{
    u32 entity_index; // NOTE(MIGUEL): index_low
    // NOTE(MIGUEL): controller request for simulation
    V2 acceleration;
    V2 delta_sword;
    f32 delta_z;
};

typedef struct GameState GameState;
struct GameState
{
    MemoryArena world_arena;
    World       *world    ;
    
    u32 camera_following_entity_index;
    WorldCoord camera_position;
    
    BitmapData  back_drop;
    BitmapData  player_head;
    BitmapData  player_torso;
    BitmapData  player_cape;
    BitmapData  shadow;
    BitmapData  sword;
    
    
    BitmapData  debug_bmp;
    BitmapData  tree;
    
    PlayerBitmaps playerbitmaps[4];
    
    ControlledPlayer controlled_players[ARRAYCOUNT(((GameInput *)0)->controllers)];
    
    u32        entity_count_low;
    //EntityHigh entities_high_[256];
    EntityLow  entities_low  [100000];
    
    u32 *pixel_ptr; // NOTE(MIGUEL): temp shit
    
    f32 accely;
    f32 clock;
    
    f32 meters_to_pixels;
};



typedef struct EntityVisiblePiece EntityVisiblePiece;
struct EntityVisiblePiece
{
    BitmapData *bitmap;
    V3 offset;
    f32 entity_zc;
    
    V4 color;
    V2 dim;
};

typedef struct EntityVisiblePieceGroup EntityVisiblePieceGroup;
struct EntityVisiblePieceGroup
{
    u32 piece_count;
    EntityVisiblePiece pieces[32];
    GameState *game_state;
};


inline EntityLow *
Entity_get_entity_low(GameState *game_state,  u32 index_low)
{
    EntityLow *result = NULLPTR;
    
    if((index_low > 0) && (index_low < game_state->entity_count_low))
    {
        result = game_state->entities_low + index_low;
    }
    
    return result;
}



//~ FUNCTION DECLERATIONS

#define SGE_INIT(    name) void name(ThreadContext *thread, GameMemory *sge_memory)
typedef SGE_INIT(SGE_Init);
SGE_INIT(SGEInitStub)
{ return; } 
// NOTE(MIGUEL): what should the return value be??? Any value that matches the function signiture. The stub is just a place holder/ fallback if we cant load the real thing

#define SGE_UPDATE( name) void name(ThreadContext *thread, GameMemory *sge_memory, GameInput *input, GameBackBuffer *back_buffer)
typedef SGE_UPDATE(SGE_Update);
SGE_UPDATE(SGEUpdateStub)
{ return; }

// NOTE(MIGUEL): game_memroy doesnt exist
#define SGE_GET_SOUND_SAMPLES(name) void name(ThreadContext *thread, GameMemory *sge_memory, GameSoundOutputBuffer *sound_buffer)
typedef SGE_GET_SOUND_SAMPLES(SGE_GetSoundSamples);
// NOTE(MIGUEL): no stub cause game should crash if core fucntions are missing

internal void Game_render_weird_shit(GameBackBuffer *buffer, s32 x_offset, s32 y_offset, f32 delta_t);


internal void Game_draw_rectangle(GameBackBuffer *buffer,
                                  V2 min, V2 max, V4 color, b32 grid);


internal void Game_update_sound_buffer  (GameState *game_state, GameSoundOutputBuffer *sound_buffer, u32 tone_hz);

inline GameControllerInput *get_controller(GameInput *input, u32 controller_index)
{
    ASSERT(controller_index < (u32)ARRAYCOUNT(input->controllers));
    GameControllerInput *result = &input->controllers[controller_index];
    
    return result;
}

#endif //SGE_H


