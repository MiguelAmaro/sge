inline TileChunk *
Tile_get_TileChunk(Tilemap *tilemap, u32 tilechunk_x, u32 tilechunk_y, u32 tilechunk_z)
{
    TileChunk *tilechunk = NULLPTR;
    
    if((tilechunk_x >= 0) && (tilechunk_x < tilemap->tilechunk_count_x) &&
       (tilechunk_y >= 0) && (tilechunk_y < tilemap->tilechunk_count_y) &&
       (tilechunk_z >= 0) && (tilechunk_z < tilemap->tilechunk_count_z))
    {
        tilechunk = &tilemap->tilechunks[tilechunk_z * (tilemap->tilechunk_count_x * tilemap->tilechunk_count_y) +
                                         tilechunk_y * (tilemap->tilechunk_count_x) +
                                         tilechunk_x];
    }
    
    return tilechunk;
}


//~ GET ACCESSORS
inline TileChunkPosition
Tile_get_TileChunkPosition(Tilemap *tilemap, u32 tile_abs_x, u32 tile_abs_y, u32 tile_abs_z)
{
    TileChunkPosition result;
    
    result.tile_chunk_x = tile_abs_x >> tilemap->chunk_shift;
    result.tile_chunk_y = tile_abs_y >> tilemap->chunk_shift;
    result.tile_chunk_z = tile_abs_z;
    
    result.tile_rel_x   = (tile_abs_x  & tilemap->chunk_mask);
    result.tile_rel_y   = (tile_abs_y  & tilemap->chunk_mask);
    
    return result;
}

inline u32
Tile_get_tile_value_unchecked(Tilemap *tilemap, TileChunk *tilechunk, u32 tile_x, u32 tile_y)
{
    ASSERT(tilechunk);
    ASSERT(tile_x < tilemap->chunk_dimensions);
    ASSERT(tile_y < tilemap->chunk_dimensions);
    
    u32 tile_value = tilechunk->tiles[tile_y * tilemap->chunk_dimensions + tile_x];
    
    return(tile_value);
}

internal u32
Tile_get_tile_value_if_valid_chunk(Tilemap *tilemap, TileChunk *tilechunk, u32 test_x, u32 test_y)
{
    u32 tilechunk_value = 0;
    
    if(tilechunk && tilechunk->tiles)
    {
        tilechunk_value = Tile_get_tile_value_unchecked(tilemap, tilechunk, test_x, test_y);
    }
    
    return tilechunk_value;
}

internal u32
Tile_get_tile_value(Tilemap *tilemap, u32 tile_abs_x, u32 tile_abs_y, u32 tile_abs_z)
{
    TileChunkPosition chunk_pos = Tile_get_TileChunkPosition(tilemap,
                                                             tile_abs_x,
                                                             tile_abs_y,
                                                             tile_abs_z);
    
    TileChunk         *tilechunk = Tile_get_TileChunk(tilemap,
                                                      chunk_pos.tile_chunk_x,
                                                      chunk_pos.tile_chunk_y,
                                                      chunk_pos.tile_chunk_z);
    
    u32           tilechunk_value = Tile_get_tile_value_if_valid_chunk(tilemap,
                                                                       tilechunk,
                                                                       chunk_pos.tile_rel_x,
                                                                       chunk_pos.tile_rel_y);
    
    
    return tilechunk_value;
}

internal u32
Tile_get_tile_value_tilemap_pos(Tilemap *tilemap, TilemapCoord pos)
{
    u32 tilechunk_value = Tile_get_tile_value(tilemap, pos.tile_abs_x, pos.tile_abs_y, pos.tile_abs_z);
    
    return tilechunk_value;
}

//~ SET ACCESSORS
inline void
Tile_set_tile_value_unchecked(Tilemap *tilemap, TileChunk *tilechunk, u32 tile_x, u32 tile_y, u32 tile_value)
{
    ASSERT(tilechunk);
    ASSERT(tile_x < tilemap->chunk_dimensions);
    ASSERT(tile_y < tilemap->chunk_dimensions);
    
    tilechunk->tiles[tile_y * tilemap->chunk_dimensions + tile_x] = tile_value;
    
    return;
}

internal void
Tile_set_tile_value_if_valid_chunk(Tilemap *tilemap, TileChunk *tilechunk, u32 test_x, u32 test_y, u32 tile_value)
{
    if(tilechunk && tilechunk->tiles)
    {
        Tile_set_tile_value_unchecked(tilemap, tilechunk, test_x, test_y, tile_value);
    }
    
    return;
}

internal void
Tile_set_tile_value(MemoryArena *arena, Tilemap *tilemap, u32 tile_abs_x, u32 tile_abs_y, u32 tile_abs_z, u32 tile_value)
{
    TileChunkPosition chunk_pos = Tile_get_TileChunkPosition(tilemap,
                                                             tile_abs_x,
                                                             tile_abs_y,
                                                             tile_abs_z)
        ;
    TileChunk         *tilechunk = Tile_get_TileChunk(tilemap,
                                                      chunk_pos.tile_chunk_x,
                                                      chunk_pos.tile_chunk_y,
                                                      chunk_pos.tile_chunk_z);
    
    ASSERT(tilechunk);
    
    if(!tilechunk->tiles)
    {
        u32 tile_count = tilemap->chunk_dimensions * tilemap->chunk_dimensions;
        tilechunk->tiles = MEMORY_ARENA_PUSH_ARRAY(arena, tile_count, u32);
        
        for(u32 tile_index  = 0; tile_index < tile_count; tile_index++)
        {
            tilechunk->tiles[tile_index] = 1;
        }
    }
    
    Tile_set_tile_value_if_valid_chunk(tilemap,
                                       tilechunk,
                                       chunk_pos.tile_rel_x,
                                       chunk_pos.tile_rel_y,
                                       tile_value);
    
    return;
}


inline b32
Tile_is_tile_value_empty(u32 tile_value)
{
    b32 is_empty = ((tile_value == 1) ||
                    (tile_value == 3) ||
                    (tile_value == 4));
    
    return is_empty;
}

internal b32
Tile_is_point_empty(Tilemap *tilemap, TilemapCoord can_pos)
{
    b32 is_empty = 1;
    
    // NOTE(MIGUEL): tile system query when rendering to screen
    u32 tilechunk_value = Tile_get_tile_value(tilemap, can_pos.tile_abs_x, can_pos.tile_abs_y, can_pos.tile_abs_z);
    is_empty = Tile_is_tile_value_empty(tilechunk_value);
    
    return is_empty;
}

inline void
Tile_recanonicalize_coord(Tilemap *tilemap, s32 *tile, f32 *tile_rel)
{
    //offset = displacement, where units are tiles
    s32 offset = round_f32_to_s32(*tile_rel / tilemap->tile_side_in_meters );
    *tile += offset;
    *tile_rel -= offset * tilemap->tile_side_in_meters;
    
    // TODO(MIGUEL): fix floating point math
    ASSERT(*tile_rel >= -0.5f * tilemap->tile_side_in_meters);
    ASSERT(*tile_rel <=  0.5f * tilemap->tile_side_in_meters);
    
    return;
}

inline TilemapCoord
Tile_map_to_tilespace(Tilemap *tilemap, TilemapCoord base_pos, V2 offset)
{
    TilemapCoord result = base_pos;
    V2_add(result.tile_rel_, offset, &result.tile_rel_);
    
    Tile_recanonicalize_coord(tilemap, &result.tile_abs_x, &result.tile_rel_.x);
    Tile_recanonicalize_coord(tilemap, &result.tile_abs_y, &result.tile_rel_.y);
    
    return result;
}

inline b32
Tile_is_on_same_tile(TilemapCoord *a, TilemapCoord *b)
{
    b32 result = ((a->tile_abs_x == b->tile_abs_x) &&
                  (a->tile_abs_y == b->tile_abs_y) &&
                  (a->tile_abs_z == b->tile_abs_z));
    
    return result;
}


inline TilemapCoord
Tile_centered_tile_point(u32 tile_abs_x, u32 tile_abs_y, u32 tile_abs_z)
{
    TilemapCoord result = { 0 };
    
    result.tile_abs_x = tile_abs_x;
    result.tile_abs_y = tile_abs_y;
    result.tile_abs_z = tile_abs_z;
    
    return result;
}

inline TilemapDifference
Tile_subtract(Tilemap *tilemap, TilemapCoord *a, TilemapCoord *b)
{
    TilemapDifference result;
    
    f32 dtilex = (f32)a->tile_abs_x - (f32)b->tile_abs_x;
    f32 dtiley = (f32)a->tile_abs_y - (f32)b->tile_abs_y;
    f32 dtilez = (f32)a->tile_abs_z - (f32)b->tile_abs_z;
    
    result.dxy.x = (tilemap->tile_side_in_meters * dtilex) + (a->tile_rel_.x - b->tile_rel_.x);
    result.dxy.y = (tilemap->tile_side_in_meters * dtiley) + (a->tile_rel_.y - b->tile_rel_.y);
    
    result.dz = tilemap->tile_side_in_meters * dtilez;
    
    return result;
}

inline TilemapCoord
Tile_offset(Tilemap *tilemap, TilemapCoord pos, V2 offset)
{
    V2_add(pos.tile_rel_, offset, &pos.tile_rel_);
    
    //pos = Tile_recanonicalize_position(tilemap, pos);
    
    return pos;
}