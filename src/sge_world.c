internal void
World_init(World *world, f32 side_in_meters_tile)
{
    world->side_in_meters_tile  = side_in_meters_tile;
    world->side_in_meters_chunk = side_in_meters_tile * 16.0f;
    
    for(u32 worldchunk_index = 0;
        worldchunk_index < ARRAYCOUNT(world->chunk_hash); worldchunk_index++)
    {
        world->chunk_hash[worldchunk_index].x = WORLD_CHUNK_UNINITIALIZED;
        world->chunk_hash[worldchunk_index].first_block.entity_count = 0;
    }
    return;
}

inline b32
World_is_cannonical_f32(World *world, f32 tile_rel)
{
    f32 epsilon = 0.0001f;
    // TODO(MIGUEL): fix floating point math
    b32 result = ((tile_rel >= -(0.5f * world->side_in_meters_chunk + epsilon)) &&
                  (tile_rel <=  (0.5f * world->side_in_meters_chunk + epsilon)));
    
    return result;
}

inline b32
World_is_cannonical_V2(World *world, V2 rel_pos)
{
    b32 result = (World_is_cannonical_f32(world, rel_pos.x) &&
                  World_is_cannonical_f32(world, rel_pos.y));
    
    return result;
}

inline WorldCoord
World_null_position(void)
{
    WorldCoord result = { WORLD_CHUNK_UNINITIALIZED };
    
    return result;
}

inline b32
World_is_valid_position(WorldCoord position)
{
    b32 result = position.chunk_x != WORLD_CHUNK_UNINITIALIZED;
    
    return result;
}

internal WorldChunk *
World_get_worldchunk(World *world, s32 worldchunk_x, s32 worldchunk_y, s32 worldchunk_z, MemoryArena *arena)
{
    ASSERT(worldchunk_x > -WORLD_CHUNK_SAFE_MARGIN);
    ASSERT(worldchunk_y > -WORLD_CHUNK_SAFE_MARGIN);
    ASSERT(worldchunk_z > -WORLD_CHUNK_SAFE_MARGIN);
    
    ASSERT(worldchunk_x <  WORLD_CHUNK_SAFE_MARGIN);
    ASSERT(worldchunk_y <  WORLD_CHUNK_SAFE_MARGIN);
    ASSERT(worldchunk_z <  WORLD_CHUNK_SAFE_MARGIN);
    
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
            chunk->x    = WORLD_CHUNK_UNINITIALIZED;
        }
        
        if(arena && (worldchunk_x != WORLD_CHUNK_UNINITIALIZED))
        {
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

inline void
World_recanonicalize_coord(World *world, s32 *chunk, f32 *chunk_rel)
{
    //offset = displacement, where units are tiles
    s32 offset = round_f32_to_s32(*chunk_rel / world->side_in_meters_chunk );
    *chunk += offset;
    *chunk_rel -= offset * world->side_in_meters_chunk;
    
    ASSERT(World_is_cannonical_f32(world, *chunk_rel));
    
    return;
}

inline WorldCoord
World_map_to_chunkspace(World *world, WorldCoord base_pos, V2 offset)
{
    WorldCoord result = base_pos;
    
    V2_add(result.rel_, offset, &result.rel_);
    
    World_recanonicalize_coord(world, &result.chunk_x, &result.rel_.x);
    World_recanonicalize_coord(world, &result.chunk_y, &result.rel_.y);
    
    return result;
}

inline b32
World_are_on_same_chunk(World *world, WorldCoord *a, WorldCoord *b)
{
    ASSERT(World_is_cannonical_V2(world, a->rel_));
    ASSERT(World_is_cannonical_V2(world, b->rel_));
    
    b32 result = ((a->chunk_x == b->chunk_x) &&
                  (a->chunk_y == b->chunk_y) &&
                  (a->chunk_z == b->chunk_z));
    
    return result;
}


inline WorldCoord
World_centered_point(u32 chunk_x, s32 chunk_y, s32 chunk_z)
{
    WorldCoord result = { 0 };
    
    result.chunk_x = chunk_x;
    result.chunk_y = chunk_y;
    result.chunk_z = chunk_z;
    
    return result;
}


internal void
World_change_entity_location_raw(World * world, u32 index_low,
                                 WorldCoord *old_pos, WorldCoord *new_pos,
                                 MemoryArena *arena)
{
    ASSERT(!old_pos || World_is_valid_position(*old_pos));
    ASSERT(!new_pos || World_is_valid_position(*new_pos));
    
    if(old_pos && new_pos &&World_are_on_same_chunk(world, old_pos, new_pos))
    {
        // NOTE(MIGUEL): NOOP
    }
    else
    {
        if(old_pos)
        {
            // NOTE(MIGUEL): pull entity out of current block
            WorldChunk *chunk = World_get_worldchunk(world,
                                                     old_pos->chunk_x,
                                                     old_pos->chunk_y,
                                                     old_pos->chunk_z,
                                                     NULLPTR);
            ASSERT(chunk);
            if(chunk)
            {
                WorldEntityBlock *first_block = &chunk->first_block;
                b32 not_found = 1;
                
                for(WorldEntityBlock *block = first_block;
                    block && not_found; block = block->next)
                {
                    for(u32 index = 0; 
                        (index < block->entity_count) && not_found; index++)
                    {
                        if(block->entity_indices_low[index] == index_low)
                        {
                            ASSERT(first_block->entity_count > 0);
                            
                            block->entity_indices_low[index] = 
                                first_block->entity_indices_low[--first_block->entity_count];
                            
                            if(first_block->entity_count == 0)
                            {
                                if(first_block->next)
                                {
                                    WorldEntityBlock *next_block = first_block->next;
                                    *first_block = *next_block;
                                    
                                    next_block->next  = world->first_free;
                                    world->first_free = next_block;
                                }
                            }
                            not_found = 0;
                        }
                    }
                }
            }
        }
        
        
        if(new_pos)
        {
            // NOTE(MIGUEL): pull entity out of current block
            WorldChunk *chunk = World_get_worldchunk(world,
                                                     new_pos->chunk_x,
                                                     new_pos->chunk_y,
                                                     new_pos->chunk_z, arena);
            ASSERT(chunk);
            WorldEntityBlock *block = &chunk->first_block;
            
            if(block->entity_count == ARRAYCOUNT(block->entity_indices_low))
            {
                WorldEntityBlock *old_block = world->first_free;
                
                if(old_block)
                {
                    world->first_free  = old_block->next;
                }
                else
                {
                    old_block = MEMORY_ARENA_PUSH_STRUCT(arena, WorldEntityBlock);
                }
                
                *old_block = *block;
                block->next = old_block;
                block->entity_count = 0;
            }
            
            ASSERT(block->entity_count < ARRAYCOUNT(block->entity_indices_low));
            block->entity_indices_low[block->entity_count++] = index_low;
            
        }
    }
    
    return;
}


// NOTE(MIGUEL): thers a call b4 this definition. its located in end_sim at sge_sim_region.h
inline void
World_change_entity_location(World *world,
                             u32 index_low, EntityLow *entity_low,
                             WorldCoord new_pos_init,
                             MemoryArena *arena)
{
    WorldCoord *old_pos = NULLPTR;
    WorldCoord *new_pos = NULLPTR;
    
    if(!Entity_is_entity_sim_flags_set(&entity_low->sim, EntitySimFlag_nonspatial) &&
       World_is_valid_position(entity_low->position))
    {
        old_pos = &entity_low->position;
    }
    
    if(World_is_valid_position(new_pos_init) )
    {
        new_pos = &new_pos_init;
    }
    
    World_change_entity_location_raw(world, index_low,
                                     old_pos, new_pos,
                                     arena);
    
    if(new_pos)
    {
        entity_low->position = *new_pos;
        Entity_clear_entity_sim_flags(&entity_low->sim, EntitySimFlag_nonspatial);
    }
    else
    {
        entity_low->position = World_null_position();
        Entity_set_entity_sim_flags(&entity_low->sim, EntitySimFlag_nonspatial);
    }
    
    return;
}

inline WorldDifference
World_subtract(World *world, WorldCoord *a, WorldCoord *b)
{
    WorldDifference result;
    
    f32 dtilex = (f32)a->chunk_x - (f32)b->chunk_x;
    f32 dtiley = (f32)a->chunk_y - (f32)b->chunk_y;
    f32 dtilez = (f32)a->chunk_z - (f32)b->chunk_z;
    
    result.dxy.x = (world->side_in_meters_chunk * dtilex) + (a->rel_.x - b->rel_.x);
    result.dxy.y = (world->side_in_meters_chunk * dtiley) + (a->rel_.y - b->rel_.y);
    
    result.dz = world->side_in_meters_chunk * dtilez;
    
    return result;
}

inline WorldCoord
World_offset(World *tilemap, WorldCoord pos, V2 offset)
{
    V2_add(pos.rel_, offset, &pos.rel_);
    
    //pos = Tile_recanonicalize_position(tilemap, pos);
    
    return pos;
}

inline WorldCoord
World_worldcoord_from_tilecoord(World *world, s32 tile_abs_x, s32 tile_abs_y, s32 tile_abs_z)
{
    WorldCoord  result = { 0 };
    
    result.chunk_x = floor_f32_to_s32((f32)tile_abs_x / (f32)WORLD_TILES_PER_CHUNK);
    result.chunk_y = floor_f32_to_s32((f32)tile_abs_y / (f32)WORLD_TILES_PER_CHUNK);
    result.chunk_z = floor_f32_to_s32((f32)tile_abs_z / (f32)WORLD_TILES_PER_CHUNK);
    
    result.rel_.x = (f32)((tile_abs_x - WORLD_TILES_PER_CHUNK / 2) - (result.chunk_x * WORLD_TILES_PER_CHUNK)) * world->side_in_meters_tile;
    result.rel_.y = (f32)((tile_abs_y - WORLD_TILES_PER_CHUNK / 2) - (result.chunk_y * WORLD_TILES_PER_CHUNK)) * world->side_in_meters_tile;
    
    ASSERT(World_is_cannonical_V2(world, result.rel_));
    
    return result;
}

