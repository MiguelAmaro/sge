/* date = May 15th 2021 1:53 pm */

#ifndef SGE_TILE_H
#define SGE_TILE_H


#define TILE_DEFAULT_CHUNK_SHIFT (8)
#define TILE_DEFAULT_CHUNK_MASK  ((1 << 8) - 1)

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
} tile_map_position;

typedef struct
{
    f32 tile_side_in_meters;
    s32 tile_side_in_pixels;
    f32 meters_to_pixels;
    
    u32 chunk_dimensions;
    u32 chunk_shift;
    u32 chunk_mask;
    
    u32 tilechunk_count_x;
    u32 tilechunk_count_y;
    
    tile_chunk *tilechunks;
} tile_map;


#endif //SGE_TILE_H
