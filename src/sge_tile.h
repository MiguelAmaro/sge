/* date = May 15th 2021 1:53 pm */

#ifndef SGE_TILE_H
#define SGE_TILE_H

#include "sge_math.h"


#define TILE_DEFAULT_CHUNK_SHIFT (8)
#define TILE_DEFAULT_CHUNK_MASK  ((1 << 8) - 1)

typedef struct TileChunk TileChunk;
struct TileChunk
{
    u32 *tiles;
    
};

typedef struct TilemapDifference TilemapDifference;
struct TilemapDifference
{
    v2  dxy;
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


typedef struct TilemapPosition TilemapPosition;
struct TilemapPosition
{
    u32 tile_abs_x;
    u32 tile_abs_y;
    u32 tile_abs_z;
    
    v2  tile_rel_;
};

typedef struct Tilemap Tilemap;
struct Tilemap
{
    TileChunk *tilechunks;
    u32 chunk_dimensions;
    u32 chunk_shift;
    u32 chunk_mask;
    
    u32 tilechunk_count_x;
    u32 tilechunk_count_y;
    u32 tilechunk_count_z;
    
    f32 tile_side_in_meters;
    u32 padding;
};


#endif //SGE_TILE_H
