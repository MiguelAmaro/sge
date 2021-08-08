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

#define MEMORY_ARENA_PUSH_STRUCT(arena,        type) (type *)MemoryArena_push_data_structure(arena, sizeof(type))
#define MEMORY_ARENA_PUSH_ARRAY( arena, count, type) (type *)MemoryArena_push_data_structure(arena, (count) * sizeof(type))
internal void *
MemoryArena_push_data_structure(MemoryArena *arena, memory_index size)
{
    ASSERT((arena->used + size) <= arena->size);
    
    void *new_arena_partition_adress  = arena->base_ptr + arena->used;
    arena->used  += size;
    
    return new_arena_partition_adress;
}


#include "sge_intrinsics.h"
#include "sge_world.h"
#include "sge_world.c"

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
    EntityType_wall,
    EntityType_sword,
    EntityType_floor,
    EntityType_ladder_up,
    EntityType_ladder_down,
    EntityType_friendly,
    EntityType_hostile,
};

typedef struct EntityHigh EntityHigh;
struct EntityHigh
{
    b32 exists;
    V2  position; // NOTE(MIGUEL): relative to camera
    V2  velocity;
    u32 facing_direction;
    s32 tile_abs_z; // NOTE(MIGUEL): should this be chunk z??
    
    f32 z;
    f32 bob_t;
    f32 delta_z;
    
    u32 index_low;
};

#define HITPOINT_SUB_COUNT (4)
typedef struct HitPoint HitPoint;
struct HitPoint
{
    u8 flags;
    u8 filled_amount;
}; 

typedef struct EntityLow EntityLow;
struct EntityLow
{
    EntityType type;
    
    f32 width, height;
    WorldCoord position;
    
    // NOTE(MIGUEL): for stairs
    s32 delta_tile_abs_z;
    b32 collides;
    
    u32 index_high;
    
    u32 hit_point_max;
    HitPoint hit_points[16];
    
    u32 index_low_sword;
    f32 distance_remaining; // sword
};  

typedef struct Entity Entity;
struct Entity
{
    u32 index_low;
    EntityHigh *high;
    EntityLow  *low;
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
    
    u32 player_controller_entity_index[ARRAYCOUNT(((GameInput *)0)->controllers)];
    
    u32        entity_count_high;
    u32        entity_count_low;
    EntityHigh entities_high_[256];
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
;



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
                                  V2 min, V2 max, f32 r, f32 g, f32 b, b32 grid);


internal void Game_update_sound_buffer  (GameState *game_state, GameSoundOutputBuffer *sound_buffer, u32 tone_hz);

inline GameControllerInput *get_controller(GameInput *input, u32 controller_index)
{
    ASSERT(controller_index < (u32)ARRAYCOUNT(input->controllers));
    GameControllerInput *result = &input->controllers[controller_index];
    
    return result;
}

#endif //SGE_H


