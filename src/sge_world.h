/* date = May 15th 2021 1:53 pm */

#ifndef SGE_TILE_H
#define SGE_TILE_H

#include "sge_math.h"
#include "sge.h"


#define WORLD_CHUNK_DEFAULT_SHIFT (8)
#define WORLD_CHUNK_DEFAULT_MASK  ((1 << 8) - 1)
#define WORLD_CHUNK_SAFE_MARGIN   (INT32_MAX / 64)
#define WORLD_CHUNK_UNINITIALIZED (INT32_MAX)
#define WORLD_TILES_PER_CHUNK     (16)

typedef struct WorldEntityBlock WorldEntityBlock;
struct  WorldEntityBlock
{
    u32 entity_count;
    u32 entity_indices_low[16];
    WorldEntityBlock *next;
};

typedef struct WorldChunk WorldChunk;
struct WorldChunk
{
    s32 x;
    s32 y;
    s32 z;
    
    WorldEntityBlock first_block;
    
    WorldChunk *next; ///External chaining
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
    s32 chunk_x;
    s32 chunk_y;
    s32 chunk_z;
    
    V2  rel_;
};

typedef struct World World;
struct World
{
    f32 side_in_meters_tile ;
    f32 side_in_meters_chunk;
    
    WorldChunk chunk_hash[4096];
    
    // NOTE(MIGUEL): Entity blocks that are allocated at some point
    //               for storage but then "freed" cause nothing is stored
    //               anymore are chained here for reuse. 
    WorldEntityBlock *first_free;
};


#endif //SGE_TILE_H
