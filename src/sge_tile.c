
inline tile_chunk * 
tile_get_tile_chunk(tile_map *tilemap, u32 tilechunk_x, u32 tilechunk_y)
{
    tile_chunk *tilechunk = NULLPTR;
    
    if((tilechunk_x >= 0) && (tilechunk_x < tilemap->tilechunk_count_x) &&
       (tilechunk_y >= 0) && (tilechunk_y < tilemap->tilechunk_count_y))
    {
        tilechunk = &tilemap->tilechunks[tilechunk_y * tilemap->tilechunk_count_x + tilechunk_x];
    }
    
    return tilechunk;
}

inline u32
tile_get_tile_value_unchecked(tile_map *tilemap, tile_chunk *tilechunk, u32 tile_x, u32 tile_y)
{
    ASSERT(tilechunk);
    ASSERT(tile_x < tilemap->chunk_dimensions);
    ASSERT(tile_y < tilemap->chunk_dimensions);
    
    u32 tile_value = tilechunk->tiles[tile_y * tilemap->chunk_dimensions + tile_x];
    
    return(tile_value);
}

inline void
tile_set_tile_value_unchecked(tile_map *tilemap, tile_chunk *tilechunk, u32 tile_x, u32 tile_y, u32 tile_value)
{
    ASSERT(tilechunk);
    ASSERT(tile_x < tilemap->chunk_dimensions);
    ASSERT(tile_y < tilemap->chunk_dimensions);
    
    tilechunk->tiles[tile_y * tilemap->chunk_dimensions + tile_x] = tile_value;
    
    return;
}

internal u32
tile_get_tile_value_if_valid_chunk(tile_map *tilemap, tile_chunk *tilechunk, u32 test_x, u32 test_y)
{
    u32 tilechunk_value = 0;
    
    if(tilechunk)
    {
        tilechunk_value = tile_get_tile_value_unchecked(tilemap, tilechunk, test_x, test_y);
    }
    
    return tilechunk_value;
}


internal u32
tile_set_tile_value_if_valid_chunk(tile_map *tilemap, tile_chunk *tilechunk, u32 test_x, u32 test_y, u32 tile_value)
{
    u32 tilechunk_value = 0;
    
    if(tilechunk)
    {
        tile_set_tile_value_unchecked(tilemap, tilechunk, test_x, test_y, tile_value);
    }
    
    return tilechunk_value;
}


inline tile_chunk_position
tile_get_tile_chunk_position(tile_map *tilemap, u32 tile_abs_x, u32 tile_abs_y)
{
    tile_chunk_position result;
    
    result.tile_chunk_x = tile_abs_x >> tilemap->chunk_shift;
    result.tile_chunk_y = tile_abs_y >> tilemap->chunk_shift;
    
    result.tile_rel_x   = tile_abs_x  & tilemap->chunk_mask;
    result.tile_rel_y   = tile_abs_y  & tilemap->chunk_mask;
    
    return result;
}

internal u32
tile_get_tile_value(tile_map *tilemap, u32 tile_abs_x, u32 tile_abs_y)
{
    tile_chunk_position chunk_pos = tile_get_tile_chunk_position(tilemap, tile_abs_x, tile_abs_y);
    tile_chunk         *tilechunk = tile_get_tile_chunk(tilemap, chunk_pos.tile_chunk_x, chunk_pos.tile_chunk_y);
    //tile_chunk         *tilechunk = tile_map_get_tile_chunk(tilemap, 0, 0);
    u32           tilechunk_value = tile_get_tile_value_if_valid_chunk(tilemap,
                                                                       tilechunk,
                                                                       chunk_pos.tile_rel_x,
                                                                       chunk_pos.tile_rel_y);
    
    
    return tilechunk_value;
}

internal b32
tile_is_point_empty(tile_map *tilemap, tile_map_position can_pos)
{
    b32 is_empty = 0;
    
    // NOTE(MIGUEL): tile system query when rendering to screen
    u32 tilechunk_value = tile_get_tile_value(tilemap, can_pos.tile_abs_x, can_pos.tile_abs_y);
    is_empty = (tilechunk_value == 0);
    
    return is_empty;
}

internal void
tile_set_tile_value(memory_arena *arena, tile_map *tilemap, u32 tile_abs_x, u32 tile_abs_y, u32 tile_value)
{
    tile_chunk_position chunk_pos = tile_get_tile_chunk_position(tilemap, tile_abs_x, tile_abs_y);
    tile_chunk         *tilechunk = tile_get_tile_chunk(tilemap, chunk_pos.tile_chunk_x, chunk_pos.tile_chunk_y);
    
    ASSERT(tilechunk);
    
    u32           tilechunk_value = tile_set_tile_value_if_valid_chunk(tilemap,
                                                                       tilechunk,
                                                                       chunk_pos.tile_rel_x,
                                                                       chunk_pos.tile_rel_y,
                                                                       tile_value);
    
    return;
}

inline void
tile_recanonicalize_coord(tile_map *tilemap, s32 *tile, f32 *tile_rel)
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

inline tile_map_position
tile_recanonicalize_position(tile_map *tilemap, tile_map_position pos)
{
    tile_map_position result = pos;
    
    tile_recanonicalize_coord(tilemap, &result.tile_abs_x, &result.tile_rel_x);
    tile_recanonicalize_coord(tilemap, &result.tile_abs_y, &result.tile_rel_y);
    
    return result;
}
