internal void
World_init(World *world, f32 side_in_meters_tile)
{
    world->tile_dim_in_meters = V3_init_uniform(side_in_meters_tile);
    
    world->chunk_dim_in_meters = (V3)
    {
        (f32)side_in_meters_tile * WORLD_TILES_PER_CHUNK,
        (f32)side_in_meters_tile * WORLD_TILES_PER_CHUNK,
        (f32)side_in_meters_tile
    };
    
    
    for(u32 worldchunk_index = 0;
        worldchunk_index < ARRAYCOUNT(world->chunk_hash); worldchunk_index++)
    {
        world->chunk_hash[worldchunk_index].x = WORLD_CHUNK_UNINITIALIZED;
        world->chunk_hash[worldchunk_index].first_block.entity_count = 0;
    }
    return;
}

inline b32
World_is_cannonical_f32(f32 chunk_side_in_meters, f32 tile_rel)
{
    f32 epsilon = 0.01f;
    // TODO(MIGUEL): fix floating point math
    b32 result = ((tile_rel >= -(0.5f * chunk_side_in_meters + epsilon)) &&
                  (tile_rel <=  (0.5f * chunk_side_in_meters + epsilon)));
    
    return result;
}

inline b32
World_is_cannonical_V3(World *world, V3 rel_pos)
{
    b32 result = (World_is_cannonical_f32(world->chunk_dim_in_meters.x, rel_pos.x) &&
                  World_is_cannonical_f32(world->chunk_dim_in_meters.y, rel_pos.y) &&
                  World_is_cannonical_f32(world->chunk_dim_in_meters.z, rel_pos.z));
    
    return result;
}

inline WorldCoord
World_null_position(void)
{
    WorldCoord result = { 0 };
    
    result.chunk_x = WORLD_CHUNK_UNINITIALIZED;
    
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
        
        if(arena && (chunk->x != WORLD_CHUNK_UNINITIALIZED) && (!chunk->next))
        {
            chunk->next = MEMORY_ARENA_PUSH_STRUCT(arena, WorldChunk);
            chunk       = chunk->next;
            chunk->x    = WORLD_CHUNK_UNINITIALIZED;
        }
        
        if(arena && (chunk->x == WORLD_CHUNK_UNINITIALIZED))
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
World_recanonicalize_coord(f32 chunk_side_in_meters, s32 *chunk, f32 *chunk_rel)
{
    //offset = displacement, where units are tiles
    s32 offset = round_f32_to_s32(*chunk_rel / chunk_side_in_meters);
    *chunk += offset;
    *chunk_rel -= offset * chunk_side_in_meters;
    
    ASSERT(World_is_cannonical_f32(chunk_side_in_meters, *chunk_rel));
    
    return;
}

inline WorldCoord
World_map_to_chunkspace(World *world, WorldCoord base_pos, V3 offset)
{
    WorldCoord result = base_pos;
    
    V3_add(result.rel_, offset, &result.rel_);
    
    World_recanonicalize_coord(world->chunk_dim_in_meters.x, &result.chunk_x, &result.rel_.x);
    World_recanonicalize_coord(world->chunk_dim_in_meters.y, &result.chunk_y, &result.rel_.y);
    World_recanonicalize_coord(world->chunk_dim_in_meters.z, &result.chunk_z, &result.rel_.z);
    
    return result;
}

inline b32
World_are_on_same_chunk(World *world, WorldCoord *a, WorldCoord *b)
{
    ASSERT(World_is_cannonical_V3(world, a->rel_));
    ASSERT(World_is_cannonical_V3(world, b->rel_));
    
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
            // NOTE(MIGUEL): debug
            if(new_pos->chunk_x == 0 &&
               new_pos->chunk_y == 0 &&
               new_pos->chunk_z == 0)
            {
                int i = 20;
                i += 50;
            }
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
    
    if(entity_low->sim.type == EntityType_sword)
    {
        int dbgint = 13;
        dbgint = 1408;
    }
    
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

inline V3
World_subtract(World *world, WorldCoord *a, WorldCoord *b)
{
    
    V3 chunk_delta = (V3)
    {
        (f32)a->chunk_x - (f32)b->chunk_x,
        (f32)a->chunk_y - (f32)b->chunk_y,
        (f32)a->chunk_z - (f32)b->chunk_z
    };
    
    V3 rel_delta = {0};
    V3_sub(a->rel_, b->rel_, &rel_delta);
    
    V3 result = {0};
    V3_hadamard(world->chunk_dim_in_meters, chunk_delta, &result);
    V3_add(result, (V3){rel_delta.x, rel_delta.y, 0}, &result);
    
    /*
        result.x = (world->chunk_dim_in_meters.x * chunk_delta.x) + (a->rel_.x - b->rel_.x);
        result.y = (world->chunk_dim_in_meters.y * chunk_delta.y) + (a->rel_.y - b->rel_.y);
        result.z = (world->chunk_dim_in_meters.z * chunk_delta.z);
        */
    return result;
}

inline WorldCoord
World_offset(World *tilemap, WorldCoord pos, V3 offset)
{
    V3_add(pos.rel_, offset, &pos.rel_);
    
    //pos = Tile_recanonicalize_position(tilemap, pos);
    
    return pos;
}

inline WorldCoord
World_worldcoord_from_tilecoord(World *world, s32 tile_abs_x, s32 tile_abs_y, s32 tile_abs_z)
{
    WorldCoord base_pos = { 0 };
    
    V3 tile_coord = 
    { 
        (f32)tile_abs_x,
        (f32)tile_abs_y,
        (f32)tile_abs_z
    };
    
    V3 offset = tile_coord;
    
    V3_scale(world->tile_dim_in_meters.x, &offset);
    
    WorldCoord result = World_map_to_chunkspace(world, base_pos, offset);
    
    ASSERT(World_is_cannonical_V3(world, result.rel_));
    
    return result;
}

