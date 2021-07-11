internal void
World_init(World *world, f32 tile_side_in_meters)
{
    world->chunk_shift = 4;
    world->chunk_mask  = (1 << world->chunk_shift) - 1;
    world->chunk_dimensions = (1 << world->chunk_shift);
    world->tile_side_in_meters = tile_side_in_meters;
    
    for(u32 worldchunk_index = 0;
        worldchunk_index < ARRAYCOUNT(world->chunk_hash); worldchunk_index++)
    {
        world->chunk_hash[worldchunk_index].x = TILE_CHUNK_UNINITIALIZED;
    }
    return;
}

internal WorldChunk *
World_get_worldchunk(World *world, s32 worldchunk_x, s32 worldchunk_y, s32 worldchunk_z, MemoryArena *arena)
{
    ASSERT(worldchunk_x > -TILE_CHUNK_SAFE_MARGIN);
    ASSERT(worldchunk_y > -TILE_CHUNK_SAFE_MARGIN);
    ASSERT(worldchunk_z > -TILE_CHUNK_SAFE_MARGIN);
    
    ASSERT(worldchunk_x <  TILE_CHUNK_SAFE_MARGIN);
    ASSERT(worldchunk_y <  TILE_CHUNK_SAFE_MARGIN);
    ASSERT(worldchunk_z <  TILE_CHUNK_SAFE_MARGIN);
    
    // TODO(MIGUEL): BETTER HASH FUNCTION!!
    u32 hash_value   = 19 * worldchunk_x + 7 * worldchunk_y + 3 * worldchunk_z;
    u32 hash_slot    = hash_value & (ARRAYCOUNT(world->chunk_hash) - 1);
    ASSERT(hash_slot < ARRAYCOUNT(world->chunk_hash));
    
    WorldChunk *chunk = world->chunk_hash + hash_slot;
    do
    {
        if((worldchunk_x == chunk->x) &&
           (worldchunk_y == chunk->y) &&
           (worldchunk_z == chunk->z))
        {
            break;
        }
        
        if(arena && (!chunk->next))
        {
            chunk->next = MEMORY_ARENA_PUSH_STRUCT(arena, WorldChunk);
            chunk       = chunk->next;
            chunk->x    = TILE_CHUNK_UNINITIALIZED;
        }
        
        if(arena && (worldchunk_x != TILE_CHUNK_UNINITIALIZED))
        {
            u32 tile_count = world->chunk_dimensions * world->chunk_dimensions;
            
            chunk->x = worldchunk_x;
            chunk->y = worldchunk_y;
            chunk->z = worldchunk_z;
            
            chunk->next = NULLPTR;
            
            break;
        }
        
        chunk = chunk->next;
    }while(chunk);
    
    return chunk;
}

#if 0
inline WorldChunkPosition
Tile_get_worldchunkcoord(World *world, s32 tile_abs_x, s32 tile_abs_y, s32 tile_abs_z)
{
    WorldChunkPosition result;
    
    result.tile_chunk_x = tile_abs_x >> world->chunk_shift;
    result.tile_chunk_y = tile_abs_y >> world->chunk_shift;
    result.tile_chunk_z = tile_abs_z;
    
    result.tile_rel_x   = (tile_abs_x  & world->chunk_mask);
    result.tile_rel_y   = (tile_abs_y  & world->chunk_mask);
    
    return result;
}
#endif

inline void
World_recanonicalize_coord(World *tilemap, s32 *tile, f32 *tile_rel)
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

inline WorldCoord
World_map_to_tilespace(World *world, WorldCoord base_pos, V2 offset)
{
    WorldCoord result = base_pos;
    V2_add(result.tile_rel_, offset, &result.tile_rel_);
    
    World_recanonicalize_coord(world, &result.tile_abs_x, &result.tile_rel_.x);
    World_recanonicalize_coord(world, &result.tile_abs_y, &result.tile_rel_.y);
    
    return result;
}

inline b32
World_is_on_same_tile(WorldCoord *a, WorldCoord *b)
{
    b32 result = ((a->tile_abs_x == b->tile_abs_x) &&
                  (a->tile_abs_y == b->tile_abs_y) &&
                  (a->tile_abs_z == b->tile_abs_z));
    
    return result;
}


inline WorldCoord
World_centered_tile_point(u32 tile_abs_x, s32 tile_abs_y, s32 tile_abs_z)
{
    WorldCoord result = { 0 };
    
    result.tile_abs_x = tile_abs_x;
    result.tile_abs_y = tile_abs_y;
    result.tile_abs_z = tile_abs_z;
    
    return result;
}

inline WorldDifference
World_subtract(World *world, WorldCoord *a, WorldCoord *b)
{
    WorldDifference result;
    
    f32 dtilex = (f32)a->tile_abs_x - (f32)b->tile_abs_x;
    f32 dtiley = (f32)a->tile_abs_y - (f32)b->tile_abs_y;
    f32 dtilez = (f32)a->tile_abs_z - (f32)b->tile_abs_z;
    
    result.dxy.x = (world->tile_side_in_meters * dtilex) + (a->tile_rel_.x - b->tile_rel_.x);
    result.dxy.y = (world->tile_side_in_meters * dtiley) + (a->tile_rel_.y - b->tile_rel_.y);
    
    result.dz = world->tile_side_in_meters * dtilez;
    
    return result;
}

inline WorldCoord
World_offset(World *tilemap, WorldCoord pos, V2 offset)
{
    V2_add(pos.tile_rel_, offset, &pos.tile_rel_);
    
    //pos = Tile_recanonicalize_position(tilemap, pos);
    
    return pos;
}
