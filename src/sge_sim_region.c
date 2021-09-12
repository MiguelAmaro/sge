inline b32 validate_sim_entities(SimRegion *sim_region)
{
    EntitySim *sim_entity = sim_region->entities;
    
    b32 result = 1;
    
    for(u32 sim_entity_index = 0; sim_entity_index < sim_region->entity_count; sim_entity_index++, sim_entity++)
    {
#if 0
        if((sim_entity->index_storage == 0) ||
           (sim_entity->flags & EntitySimFlag_nonspatial)) result = 0;
#else
        ASSERT(!(sim_entity->index_storage == 0));
        ASSERT(!(sim_entity->flags & EntitySimFlag_nonspatial));
#endif
    }
    
    return result;
}

internal EntitySimHash *
SimRegion_get_hash_from_storage_index(SimRegion *sim_region, u32 index_storage)
{
    ASSERT(index_storage);
    
    EntitySimHash *result = NULLPTR;
    
    u32 hash_value = index_storage;
    
    for(u32 offset = 0; 
        offset < ARRAYCOUNT(sim_region->hash); offset++)
    {
        u32 hash_mask  = (ARRAYCOUNT(sim_region->hash) - 1);
        u32 hash_index = (hash_value + offset) & hash_mask;
        
        EntitySimHash *entry = sim_region->hash + hash_index;
        
        if((entry->index == 0) || (entry->index == index_storage))
        {
            result = entry;
            break;
        }
        
    }
    
    
    return result;
}

internal void
SimRegion_map_storage_index_to_entity(SimRegion *sim_region, u32 index_storage, EntitySim *entity)
{
    EntitySimHash *entry = SimRegion_get_hash_from_storage_index(sim_region, index_storage);
    ASSERT((entry->index == 0) || (entry->index == index_storage));
    entry->index = index_storage;
    entry->ptr   = entity;
    
    return;
}

inline EntitySim *
SimRegion_get_entity_by_storage_index(SimRegion *sim_region, u32 index_storage)
{
    EntitySimHash *entry  = SimRegion_get_hash_from_storage_index(sim_region, index_storage);
    EntitySim     *result = entry->ptr;
    
    return result;
}

// NOTE(MIGUEL): forward decleration
internal EntitySim *
SimRegion_add_entity_raw(GameState *game_state, SimRegion *sim_region, u32 index_storage, EntityLow *source);

inline void
SimRegion_load_entity_reference(GameState *game_state, SimRegion *sim_region, EntityReference *ref)
{
    if(ref->index)
    {
        EntitySimHash *entry = SimRegion_get_hash_from_storage_index(sim_region, ref->index);
        
        if(entry->ptr == NULLPTR)
        {
            entry->index = ref->index;
            SimRegion_add_entity_raw(game_state,
                                     sim_region,
                                     ref->index,
                                     Entity_get_entity_low(game_state, ref->index));
        }
        
        ref->ptr = entry->ptr;
    }
    
    return;
}

internal EntitySim *
SimRegion_add_entity_raw(GameState *game_state, SimRegion *sim_region, u32 index_storage, EntityLow *source)
{
    ASSERT(index_storage);
    
    if(index_storage == 92034)
    {
        int i = 0;
        i += 2093;
    }
    
    EntitySim *entity = NULLPTR;
    
    EntitySimHash *entry = SimRegion_get_hash_from_storage_index(sim_region, index_storage);
    if(entry->ptr == NULLPTR)
    {
        if(sim_region->entity_count < sim_region->max_entity_count)
        {
            entity = sim_region->entities + sim_region->entity_count++;
            SimRegion_map_storage_index_to_entity(sim_region, index_storage, entity);
            
            if(source)
            {
                *entity = source->sim;
                SimRegion_load_entity_reference(game_state, sim_region, &entity->sword);
                
                ASSERT(!Entity_is_entity_sim_flags_set(&source->sim, EntitySimFlag_simming));
                Entity_set_entity_sim_flags(&source->sim, EntitySimFlag_simming);
                
            }
            
            entity->index_storage = index_storage; // NOTE(MIGUEL): why is this out here???
        }
        else
        {
            INVALID_CODE_PATH;
        }
    }
    
    return entity;
}

//EntityStored *entity_stored)
inline V2 SimRegion_get_sim_space_position(SimRegion *sim_region, EntityLow *entity_stored)
{
    V2 result = ENTITY_INVALID_POSITION;
    
    if(!Entity_is_entity_sim_flags_set(&entity_stored->sim, EntitySimFlag_nonspatial))
    {
        WorldDifference diff = World_subtract(sim_region->world,
                                              &entity_stored->position,
                                              &sim_region->origin);
        result = diff.dxy;
    }
    
    return result;
}

internal EntitySim *
SimRegion_add_entity(GameState *game_state, SimRegion *sim_region, u32 index_storage, EntityLow *source, V2 *sim_position)
{
    EntitySim *dest = SimRegion_add_entity_raw(game_state, sim_region, index_storage, source);
    
    if(dest)
    {
        if(sim_position)
        {
            dest->position = *sim_position;
        }
        else
        {
            dest->position = SimRegion_get_sim_space_position(sim_region, source);
        }
    }
    
    return dest;
}

internal SimRegion *
SimRegion_begin_sim(MemoryArena *sim_arena, GameState *game_state, World *world, WorldCoord region_origin, RectV2 region_bounds, GameBackBuffer *back_buffer)
{
    SimRegion *sim_region = MEMORY_ARENA_PUSH_STRUCT(sim_arena, SimRegion);
    MEMORY_ARENA_ZERO_STRUCT(sim_region->hash);
    
    //ASSERT(validate_sim_entities(sim_region));
    
    sim_region->world  = world;
    sim_region->origin = region_origin;
    sim_region->bounds = region_bounds;
    
    sim_region->max_entity_count = 4096;
    sim_region->entity_count     = 0;
    sim_region->entities   = MEMORY_ARENA_PUSH_ARRAY(sim_arena, sim_region->max_entity_count, EntitySim);
    
    // TODO(MIGUEL): 07/12/2021 | HMH DAY 058  | TIME 35:57
    WorldCoord minchunk_pos = World_map_to_chunkspace(world, sim_region->origin, RectV2_min_corner(sim_region->bounds));
    WorldCoord maxchunk_pos = World_map_to_chunkspace(world, sim_region->origin, RectV2_max_corner(sim_region->bounds));
    
    for(s32 chunk_y = minchunk_pos.chunk_y; chunk_y <= maxchunk_pos.chunk_y; chunk_y++)
    {
        for(s32 chunk_x = minchunk_pos.chunk_x; chunk_x <= maxchunk_pos.chunk_x; chunk_x++)
        {
            WorldChunk *chunk = World_get_worldchunk(world, chunk_x, chunk_y, sim_region->origin.chunk_z, NULLPTR);
            
            if(chunk)
            {
                for(WorldEntityBlock *block = &chunk->first_block;
                    block; block = block->next)
                {
                    for(u32 entityindexindex = 0; 
                        entityindexindex < block->entity_count; entityindexindex++)
                    {
                        u32 index_low = block->entity_indices_low[entityindexindex];
                        EntityLow *entity_low = game_state->entities_low + index_low;
                        
                        V2 sim_space_pos = SimRegion_get_sim_space_position(sim_region, entity_low);
                        
                        ASSERT(index_low);
                        ASSERT(index_low != 92034);
                        
                        if(!Entity_is_entity_sim_flags_set(&entity_low->sim, EntitySimFlag_nonspatial))
                        {
                            
                            if(RectV2_is_in_rect(region_bounds, sim_space_pos))
                            {
                                SimRegion_add_entity(game_state, sim_region, index_low, entity_low, &sim_space_pos);
                            }
                        }
                    }
                }
            }
        }
    }
    
    
    return sim_region;
}

inline void
SimRegion_store_entity_reference(EntityReference *ref)
{
    if(ref->ptr != 0)
    {
        ref->index = ref->ptr->index_storage;
    }
    
    return;
}

internal void
SimRegion_end_sim(SimRegion *sim_region, GameState *game_state)
{
    EntitySim *entity = sim_region->entities;
    
    for(u32 sim_entity_index = 0;
        sim_entity_index < sim_region->entity_count; sim_entity_index++, entity++)
    {
        EntityLow *entity_stored = game_state->entities_low + entity->index_storage;
        
        ASSERT(Entity_is_entity_sim_flags_set(&entity_stored->sim, EntitySimFlag_simming));
        entity_stored->sim = *entity;
        ASSERT(!Entity_is_entity_sim_flags_set(&entity_stored->sim, EntitySimFlag_simming));
        
        SimRegion_store_entity_reference(&entity_stored->sim.sword);
        
        WorldCoord new_position =
            (Entity_is_entity_sim_flags_set(&entity_stored->sim,
                                            EntitySimFlag_nonspatial)
             ?
             World_null_position()
             :
             World_map_to_chunkspace(sim_region->world,
                                     sim_region->origin,
                                     entity->position));
        
        ASSERT(entity->index_storage);
        World_change_entity_location(game_state->world,
                                     entity->index_storage,
                                     entity_stored,
                                     new_position,
                                     &game_state->world_arena);
        
        
        
        /// CAMRERA MOVEMENT LOGIC
        // TODO(MIGUEL): ENTITY MAPPOING HASH TABLES
        
        EntityLow *camera_following_entity = Entity_get_entity_low(game_state,
                                                                   game_state->camera_following_entity_index);
        
        if(entity->index_storage == game_state->camera_following_entity_index)
        {
            
            WorldCoord new_camera_position = game_state->camera_position;
            
            game_state->camera_position.chunk_z = entity_stored->position.chunk_z;
            
            b32 room_based_camera_movement = 0;
            
            if(room_based_camera_movement)
            {
                if(camera_following_entity->sim.position.x > (9.0f * sim_region->world->side_in_meters_tile))
                {
                    new_camera_position.rel_.x += 17.0f * (sim_region->world->side_in_meters_tile);
                    World_recanonicalize_coord(sim_region->world,
                                               &new_camera_position.chunk_x,
                                               &new_camera_position.rel_.x); 
                }
                if(camera_following_entity->sim.position.x < -(9.0f * sim_region->world->side_in_meters_tile))
                {
                    new_camera_position.rel_.x -= 17.0f * (sim_region->world->side_in_meters_tile);
                    World_recanonicalize_coord(sim_region->world,
                                               &new_camera_position.chunk_x,
                                               &new_camera_position.rel_.x); 
                }
                if(camera_following_entity->sim.position.y > (5.0f * sim_region->world->side_in_meters_tile))
                {
                    new_camera_position.rel_.y += 9.0f * (sim_region->world->side_in_meters_tile);
                    World_recanonicalize_coord(sim_region->world,
                                               &new_camera_position.chunk_y,
                                               &new_camera_position.rel_.y); 
                }
                if(camera_following_entity->sim.position.y < -(5.0f * sim_region->world->side_in_meters_tile))
                {
                    new_camera_position.rel_.y -= 9.0f * (sim_region->world->side_in_meters_tile);
                    World_recanonicalize_coord(sim_region->world,
                                               &new_camera_position.chunk_y,
                                               &new_camera_position.rel_.y); 
                }
            }
            else
            {
                f32 follow_speed             = 2.0f;
                V2 entity_offset_for_frame   = { 0 };
                V2 camera_half_field_of_view =
                {
                    (17.0f * 0.5f) * game_state->world->side_in_meters_tile,
                    ( 9.0f * 0.5f) * game_state->world->side_in_meters_tile
                };
                
                entity_offset_for_frame = (V2)
                {
                    follow_speed * (camera_following_entity->sim.position.x / camera_half_field_of_view.x),
                    follow_speed * (camera_following_entity->sim.position.y / camera_half_field_of_view.y),
                };
                
                new_camera_position = World_map_to_chunkspace(game_state->world,
                                                              new_camera_position,
                                                              entity_offset_for_frame);
            }
            
            game_state->camera_position = new_camera_position;
        }
        
    }
    
    return;
}


inline b32
Game_get_normalized_time_at_collision(f32 *normalized_time_at_closest_possible_collision,
                                      f32 wall_a,
                                      f32 rel_a, f32 rel_b,
                                      f32 position_delta_a, f32 position_delta_b,
                                      f32 min_b, f32 max_b)
{
    // NOTE(MIGUEL): a & b = generic coord components
    // NOTE(MIGUEL): t_min = time at closet collision
    // NOTE(MIGUEL): rel a & b = position of the colliding entity relative to the collidable entity being tested
    // NOTE(MIGUEL): position_delta a & b = vector representing the player's direciton of travel
    // NOTE(MIGUEL): min_b and max_b = 
    
    b32 hit = 0;
    f32 t_epsilon = 0.001f; // floating point tolerance
    
    if(position_delta_a != 0.0f)
    {
        f32 time_at_collision = (wall_a - rel_a) / position_delta_a;
        f32 b        = rel_b + time_at_collision * position_delta_b;
        
        //time_at_collision will be normalized if valid
        if((time_at_collision >= 0.0f) && (*normalized_time_at_closest_possible_collision > time_at_collision))
        {
            if((b >= min_b) && (b <= max_b))
            {
                *normalized_time_at_closest_possible_collision = MAXIMUM(0.0f, time_at_collision - t_epsilon);
                hit = 1;
            }
        }
    }
    
    return hit;
}

internal void
SimRegion_move_entity(SimRegion *sim_region,  EntitySim *entity, MoveSpec *movespec, f32 delta_t, V2 acceleration)
{
    ASSERT(!Entity_is_entity_sim_flags_set(entity, EntitySimFlag_nonspatial));
    
    World *world = sim_region->world;
    
    if(movespec->unitmaxaccel)
    {
        f32 accel_magnitude = V2_dot(acceleration, acceleration);
        
        if(accel_magnitude > 1.0f)
        {
            V2_scale(1.0f / square_root(accel_magnitude), &acceleration);
        }
    }
    
    V2 v = entity->velocity;
    V2 a = acceleration;
    //V2 old_pos = entity->position;
    
    V2_scale(movespec->speed, &acceleration); // Tune the accleration with speed
    V2_scale(-movespec->drag, &v);            // Apply friction to acceleration
    V2_add  (acceleration, v, &acceleration);
    
    V2 position_delta = { 0 };
    a = acceleration;
    v = entity->velocity;
    // ACCELRATION COMPONENT
    V2_scale(0.5f , &a);
    V2_scale(square(delta_t), &a);
    // VELOCITY COMPONENT
    V2_scale(delta_t , &v);
    // JOIN ACCEL & VELOCITY COMPONENT
    V2_add  (a, v, &position_delta); // NOTE(MIGUEL): do not alter value! used a bit lower in the function
    
    // STORE VELOCITY EQUATION
    a = acceleration;
    v = entity->velocity;
    V2_add(a, v, &a);
    V2_scale(0.5f , &a);
    V2_scale(delta_t, &a);
    V2_add  (a, entity->velocity, &entity->velocity);
    
    
    for(u32 collision_resolve_attempt = 0; 
        collision_resolve_attempt < 4;
        collision_resolve_attempt++)
    {
        V2 wall_normal = { 0 };
        // NORMALIZED SACALAR THAT REPS THE TIME STEP! NOT .033MS (MS PER FRAME)
        f32 normalized_time_of_pos_delta = 1.0f; 
        EntitySim *hit_entity = NULLPTR;
        
        V2 desired_position;
        V2_add(entity->position, position_delta, &desired_position);
        
        if(Entity_is_entity_sim_flags_set(entity, EntitySimFlag_collides))
        {
            for(u32 test_entity_sim_index = 0;
                test_entity_sim_index < sim_region->entity_count;
                test_entity_sim_index++)
            {
                EntitySim *test_entity = sim_region->entities + test_entity_sim_index;
                // NOTE(MIGUEL): to avoid colision test of an entity with itself (reference compararison)
                if(entity != test_entity)
                { 
                    if(Entity_is_entity_sim_flags_set(test_entity, EntitySimFlag_collides) &&
                       !Entity_is_entity_sim_flags_set(test_entity, EntitySimFlag_nonspatial))
                    {
                        // NOTE(MIGUEL): Minkowski sum
                        f32 diameter_w = test_entity->width  + entity->width;
                        f32 diameter_h = test_entity->height + entity->height;
                        V2 min_corner = { diameter_w, diameter_h };
                        V2 max_corner = { diameter_w, diameter_h };
                        
                        // NOTE(MIGUEL): distance away from tile center
                        V2_scale(-0.5f, &min_corner);
                        V2_scale( 0.5f, &max_corner);
                        
                        // NOTE(MIGUEL): old pos's Distance away from test tile in meters in x & y respectively
                        // NOTE(MIGUEL): Tile_subtract operand order maybe wrong...
                        V2 rel = { 0.0f };
                        V2_sub(entity->position, test_entity->position, &rel);
                        
                        f32 MinCowSkied_wall_left_x   = min_corner.x;
                        f32 MinCowSkied_wall_right_x  = max_corner.x;
                        f32 MinCowSkied_wall_top_y    = max_corner.y;
                        f32 MinCowSkied_wall_bottom_y = min_corner.y;
                        
                        // VERTICAL WALL RIGHT FACE
                        if(Game_get_normalized_time_at_collision(&normalized_time_of_pos_delta, MinCowSkied_wall_right_x,
                                                                 rel.x, rel.y,
                                                                 position_delta.x, position_delta.y,
                                                                 min_corner.y, max_corner.y))
                        {
                            wall_normal = (V2){1.0, 0.0};
                            hit_entity =  test_entity;
                        }
                        
                        // VERTICAL WALL LEFT FACE
                        if(Game_get_normalized_time_at_collision(&normalized_time_of_pos_delta, MinCowSkied_wall_left_x,
                                                                 rel.x, rel.y,
                                                                 position_delta.x, position_delta.y,
                                                                 min_corner.y, max_corner.y))
                        {
                            wall_normal = (V2){-1.0, 0.0};
                            hit_entity = test_entity;
                        }
                        
                        // HORIZONTAL BOTTOM WALL
                        if(Game_get_normalized_time_at_collision(&normalized_time_of_pos_delta, MinCowSkied_wall_bottom_y,
                                                                 rel.y, rel.x,
                                                                 position_delta.y, position_delta.x,
                                                                 min_corner.x, max_corner.x))
                        {
                            wall_normal = (V2){0.0, -1.0};
                            hit_entity = test_entity;
                        }
                        
                        // HORIZONTAL TOP WALL
                        if(Game_get_normalized_time_at_collision(&normalized_time_of_pos_delta, MinCowSkied_wall_top_y,
                                                                 rel.y, rel.x,
                                                                 position_delta.y, position_delta.x,
                                                                 min_corner.x, max_corner.x))
                        {
                            wall_normal = (V2){0.0, 1.0};
                            hit_entity = test_entity;
                        }
                    }
                }
            }
        }
        
        // UPDATED PLAYER POSITION & STORE NEW PLAYER POSITION
        V2 scratch_position_delta = position_delta;
        V2_scale(normalized_time_of_pos_delta, &scratch_position_delta);
        V2_add(entity->position, scratch_position_delta ,&entity->position);
        
        if(hit_entity)
        {
            V2 scratch_wall_normal = wall_normal;
            // UPDATE VELOCITY VECTOR 
            V2_scale(V2_dot(entity->velocity, scratch_wall_normal), &scratch_wall_normal);
            V2_scale(1.0f, &scratch_wall_normal);
            V2_sub  (entity->velocity, scratch_wall_normal, &entity->velocity);
            
            // NEW POSITION FOR COLLISION TEST
            V2_sub(desired_position, entity->position, &position_delta);
            
            scratch_wall_normal = wall_normal;
            // UPDATE MOVEMENT VECTOR
            V2_scale(V2_dot(position_delta, scratch_wall_normal), &scratch_wall_normal);
            V2_scale(1.0f, &scratch_wall_normal);
            V2_sub  (position_delta, scratch_wall_normal, &position_delta);
            
            // TODO(MIGUEL): STAIRS
        }
        else
        {
            break;
        }
        
    }
    
    if((entity->velocity.x == 0.0f) &&
       (entity->velocity.y == 0.0f))
    {
    }
    else if(absolute_value(entity->velocity.x) > absolute_value(entity->velocity.y))
    {
        if(entity->velocity.x > 0)
        {
            entity->facing_direction = 0;
        }
        else
        {
            entity->facing_direction = 2;
        }
        
    }
    else if(absolute_value(entity->velocity.x) < absolute_value(entity->velocity.y))
    {
        if(entity->velocity.y > 0)
        {
            entity->facing_direction = 1;
        }
        else
        {
            entity->facing_direction = 3;
        }
    }
    
    return;
}
