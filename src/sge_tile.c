inline tile_chunk * 
tile_get_tile_chunk(tile_map *tilemap, u32 tilechunk_x, u32 tilechunk_y, u32 tilechunk_z)
{
    tile_chunk *tilechunk = NULLPTR;
    
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
inline tile_chunk_position
tile_get_tile_chunk_position(tile_map *tilemap, u32 tile_abs_x, u32 tile_abs_y, u32 tile_abs_z)
{
    tile_chunk_position result;
    
    result.tile_chunk_x = tile_abs_x >> tilemap->chunk_shift;
    result.tile_chunk_y = tile_abs_y >> tilemap->chunk_shift;
    result.tile_chunk_z = tile_abs_z;
    
    result.tile_rel_x   = tile_abs_x  & tilemap->chunk_mask;
    result.tile_rel_y   = tile_abs_y  & tilemap->chunk_mask;
    
    return result;
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

internal u32
tile_get_tile_value_if_valid_chunk(tile_map *tilemap, tile_chunk *tilechunk, u32 test_x, u32 test_y)
{
    u32 tilechunk_value = 0;
    
    if(tilechunk && tilechunk->tiles)
    {
        tilechunk_value = tile_get_tile_value_unchecked(tilemap, tilechunk, test_x, test_y);
    }
    
    return tilechunk_value;
}

internal u32
tile_get_tile_value(tile_map *tilemap, u32 tile_abs_x, u32 tile_abs_y, u32 tile_abs_z)
{
    tile_chunk_position chunk_pos = tile_get_tile_chunk_position(tilemap,
                                                                 tile_abs_x,
                                                                 tile_abs_y,
                                                                 tile_abs_z);
    
    tile_chunk         *tilechunk = tile_get_tile_chunk(tilemap,
                                                        chunk_pos.tile_chunk_x,
                                                        chunk_pos.tile_chunk_y,
                                                        chunk_pos.tile_chunk_z);
    
    u32           tilechunk_value = tile_get_tile_value_if_valid_chunk(tilemap,
                                                                       tilechunk,
                                                                       chunk_pos.tile_rel_x,
                                                                       chunk_pos.tile_rel_y);
    
    
    return tilechunk_value;
}


//~ SET ACCESSORS
inline void
tile_set_tile_value_unchecked(tile_map *tilemap, tile_chunk *tilechunk, u32 tile_x, u32 tile_y, u32 tile_value)
{
    ASSERT(tilechunk);
    ASSERT(tile_x < tilemap->chunk_dimensions);
    ASSERT(tile_y < tilemap->chunk_dimensions);
    
    tilechunk->tiles[tile_y * tilemap->chunk_dimensions + tile_x] = tile_value;
    
    return;
}

internal void
tile_set_tile_value_if_valid_chunk(tile_map *tilemap, tile_chunk *tilechunk, u32 test_x, u32 test_y, u32 tile_value)
{
    if(tilechunk && tilechunk->tiles)
    {
        tile_set_tile_value_unchecked(tilemap, tilechunk, test_x, test_y, tile_value);
    }
    
    return;
}

internal void
tile_set_tile_value(memory_arena *arena, tile_map *tilemap, u32 tile_abs_x, u32 tile_abs_y, u32 tile_abs_z, u32 tile_value)
{
    tile_chunk_position chunk_pos = tile_get_tile_chunk_position(tilemap,
                                                                 tile_abs_x,
                                                                 tile_abs_y,
                                                                 tile_abs_z)
        ;
    tile_chunk         *tilechunk = tile_get_tile_chunk(tilemap,
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
    
    tile_set_tile_value_if_valid_chunk(tilemap,
                                       tilechunk,
                                       chunk_pos.tile_rel_x,
                                       chunk_pos.tile_rel_y,
                                       tile_value);
    
    return;
}

internal b32
tile_is_point_empty(tile_map *tilemap, tile_map_position can_pos)
{
    b32 is_empty = 1;
    
    // NOTE(MIGUEL): tile system query when rendering to screen
    u32 tilechunk_value = tile_get_tile_value(tilemap, can_pos.tile_abs_x, can_pos.tile_abs_y, can_pos.tile_abs_z);
    is_empty = (tilechunk_value == 1);
    
    return is_empty;
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
