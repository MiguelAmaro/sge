/* date = January 25th 2021 0:25 pm */
#ifndef SGE_H
#define SGE_H

#include "sge_platform.h"

//#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))
#define ARRAYCOUNT(array) (sizeof(array) /  sizeof(array[0]))
#define PI_32BIT 3.14159265359f

#define MINIMUM(a, b) ((a < b) ? (a) : (b))
#define MAXIMUM(a, b) ((a > b) ? (a) : (b))

inline u32
safe_truncate_u64(u64 value)
{
    ASSERT(value <= 0xffffffff);
    
    u32 result = (u32)value;
    
    return result;
}


typedef struct MemoryArena MemoryArena;
struct MemoryArena
{
    memory_index size;
    memory_index used;
    u8 *base_ptr;
};



//~ MEMORY INTERFACE

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
#include "sge_tile.h"
#include "sge_tile.c"

typedef struct World World;
struct World
{
    Tilemap *tilemap;
};


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

typedef struct
{
    u32 align_x;
    u32 align_y;
    BitmapData head;
    BitmapData cape;
    BitmapData torso;
} player_bitmaps;

typedef enum EntityResidence EntityResidence;
enum EntityResidence
{
    EntityResidence_nonexistent,
    EntityResidence_dormant,
    EntityResidence_low,
    EntityResidence_high,
};

typedef enum EntityType EntityType;
enum EntityType
{
    EntityType_null,
    EntityType_player,
    EntityType_wall,
    EntityType_floor,
    EntityType_ladder_up,
    EntityType_ladder_down,
};

typedef struct HighEntity HighEntity;
struct HighEntity
{
    b32 exists;
    V2  position; // NOTE(MIGUEL): relative to camera
    V2  velocity;
    u32 facing_direction;
    s32 tile_abs_z;
    
    f32 z;
    f32 delta_z;
};

typedef struct LowEntity LowEntity;
struct LowEntity
{
    int placeholder;
};

typedef struct DormantEntity DormantEntity;
struct DormantEntity
{
    f32 width, height;
    TilemapCoord position;
    b32 collides;
    s32 delta_tile_abs_z;
    EntityType type;
};

typedef struct Entity Entity;
struct Entity
{
    EntityResidence  residence;
    HighEntity      *high;
    LowEntity       *low;
    DormantEntity   *dormant;
};

typedef struct GameState GameState;
struct GameState 
{
    MemoryArena world_arena;
    World       *world      ;
    
    u32 camera_following_entity_index;
    TilemapCoord camera_position;
    
    BitmapData  back_drop;
    BitmapData  player_head;
    BitmapData  player_torso;
    BitmapData  player_cape;
    BitmapData  shadow;
    BitmapData  debug_bmp;
    
    player_bitmaps playerbitmaps[4];
    
    u32 player_controller_entity_index[ARRAYCOUNT(((game_input *)0)->controllers)];
    u32 entity_count;   //256
    //Entity entities[256];
    EntityResidence entity_residence [256 * 4];
    HighEntity      high_entities    [256 * 4];
    LowEntity       low_entities     [256 * 4];
    DormantEntity   dormant_entities [256 * 4];
    
    // NOTE(MIGUEL): temp shit
    u32 *pixel_ptr;
    
    f32 accely;
};

//~ FUNCTION DECLERATIONS

#define SGE_INIT(    name) void name(thread_context *thread, game_memory *sge_memory)
typedef SGE_INIT(SGE_Init);
SGE_INIT(SGEInitStub)
{ return; } 
// NOTE(MIGUEL): what should the return value be??? Any value that matches the function signiture. The stub is just a place holder/ fallback if we cant load the real thing

#define SGE_UPDATE( name) void name(thread_context *thread, game_memory *sge_memory, game_input *input, game_back_buffer *back_buffer)
typedef SGE_UPDATE(SGE_Update);
SGE_UPDATE(SGEUpdateStub)
{ return; }

// NOTE(MIGUEL): game_memroy doesnt exist
#define SGE_GET_SOUND_SAMPLES(name) void name(thread_context *thread, game_memory *sge_memory, game_sound_output_buffer *sound_buffer)
typedef SGE_GET_SOUND_SAMPLES(SGE_GetSoundSamples);
// NOTE(MIGUEL): no stub cause game should crash if core fucntions are missing

internal void game_render_weird_gradient(game_back_buffer *buffer, s32 x_offset, s32 y_offset, f32 *delta_t);


internal void game_draw_rectangle(game_back_buffer *buffer,
                                  V2 min, V2 max, f32 r, f32 g, f32 b);


internal void game_update_sound_buffer  (GameState *state, game_sound_output_buffer *sound_buffer, u32 tone_hz);

inline game_controller_input *get_controller(game_input *input, u32 controller_index)
{
    ASSERT(controller_index < (u32)ARRAYCOUNT(input->controllers));
    game_controller_input *result = &input->controllers[controller_index];
    
    return result;
}

#endif //SGE_H


