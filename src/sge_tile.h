/* date = May 15th 2021 1:53 pm */

#ifndef SGE_TILE_H
#define SGE_TILE_H

#include "sge_math.h"


#define TILE_CHUNK_DEFAULT_SHIFT (8)
#define TILE_CHUNK_DEFAULT_MASK  ((1 << 8) - 1)
#define TILE_CHUNK_SAFE_MARGIN   (INT32_MAX / 64)
#define TILE_CHUNK_UNINITIALIZED (INT32_MAX)

typedef struct TileChunk TileChunk;
struct TileChunk
{
    s32 x;
    s32 y;
    s32 z;
    
    u32 *tiles;
    
    TileChunk *next;
};

typedef struct TilemapDifference TilemapDifference;
struct TilemapDifference
{
    V2  dxy;
    f32 dz;
};

typedef struct TileChunkPosition TileChunkPosition;
struct TileChunkPosition
{
    s32 tile_chunk_x;
    s32 tile_chunk_y;
    s32 tile_chunk_z;
    
    s32 tile_rel_x;
    s32 tile_rel_y;
};


typedef struct TilemapCoord TilemapCoord;
struct TilemapCoord
{
    s32 tile_abs_x;
    s32 tile_abs_y;
    s32 tile_abs_z;
    
    V2  tile_rel_;
};

typedef struct Tilemap Tilemap;
struct Tilemap
{
    s32 chunk_shift;
    s32 chunk_mask;
    s32 chunk_dimensions;
    
    f32 tile_side_in_meters;
    
    TileChunk tilechunk_hash[4096];
};


#endif //SGE_TILE_H
