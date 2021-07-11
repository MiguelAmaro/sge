/* date = May 15th 2021 1:53 pm */

#ifndef SGE_TILE_H
#define SGE_TILE_H

#include "sge_math.h"
#include "sge.h"


#define TILE_CHUNK_DEFAULT_SHIFT (8)
#define TILE_CHUNK_DEFAULT_MASK  ((1 << 8) - 1)
#define TILE_CHUNK_SAFE_MARGIN   (INT32_MAX / 64)
#define TILE_CHUNK_UNINITIALIZED (INT32_MAX)

typedef struct WorldEntityBlock WorldEntityBlock;
struct  WorldEntityBlock
{
    u32     entity_count;
    u32 low_entity_index[16];
    WorldEntityBlock *next;
};

typedef struct WorldChunk WorldChunk;
struct WorldChunk
{
    s32 x;
    s32 y;
    s32 z;
    
    WorldEntityBlock first_block;
    
    WorldChunk *next;
};

typedef struct WorldDifference WorldDifference;
struct WorldDifference
{
    V2  dxy;
    f32 dz;
};

typedef struct WorldCoord WorldCoord;
struct WorldCoord
{
    s32 tile_abs_x;
    s32 tile_abs_y;
    s32 tile_abs_z;
    
    V2  tile_rel_;
};

typedef struct World World;
struct World
{
    s32 chunk_shift;
    s32 chunk_mask;
    s32 chunk_dimensions;
    
    f32 tile_side_in_meters;
    
    WorldChunk chunk_hash[4096];
};


#endif //SGE_TILE_H
