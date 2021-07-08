/* date = May 15th 2021 1:53 pm */

#ifndef SGE_TILE_H
#define SGE_TILE_H

#include "sge_math.h"


#define TILE_DEFAULT_CHUNK_SHIFT (8)
#define TILE_DEFAULT_CHUNK_MASK  ((1 << 8) - 1)
#define TILE_SAFE_MARGIN (256)

typedef struct TileChunk TileChunk;
struct TileChunk
{
    u32 x;
    u32 y;
    u32 z;
    
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
    u32 tile_chunk_x;
    u32 tile_chunk_y;
    u32 tile_chunk_z;
    
    u32 tile_rel_x;
    u32 tile_rel_y;
};


typedef struct TilemapCoord TilemapCoord;
struct TilemapCoord
{
    u32 tile_abs_x;
    u32 tile_abs_y;
    u32 tile_abs_z;
    
    V2  tile_rel_;
};

typedef struct Tilemap Tilemap;
struct Tilemap
{
    u32 chunk_shift;
    u32 chunk_mask;
    u32 chunk_dimensions;
    
    f32 tile_side_in_meters;
    
    TileChunk tilechunk_hash[4096];
};


#endif //SGE_TILE_H
