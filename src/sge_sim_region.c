inline b32 validate_sim_entities(SimRegion *sim_region)
{
    EntitySim *entity = sim_region->entities;
    
    b32 result = 1;
    
    for(u32 entity_index = 0; entity_index < sim_region->entity_count; entity_index++, entity++)
    {
        /// ALLOWED CONDITIONS
        // NOTE(MIGUEL): Non spatial sim entities are allowed into the sim_region entity set
        //               if it is the players sword ...
        //               but they are only rendered / updated if they are updatable
        //               i think. Also non updatable sim entities are allowed into the
        //               sim_region entity set. That flag only affect wether or not it gets
        //               updated or rendered.
        //ASSERT(!Entity_is_entity_sim_flags_set(entity, EntitySimFlag_nonspatial)); ALLOWED!!!
        
        // NOTE(MIGUEL): Non updatable entities are alowed in the simregion.
        //               Their movement are not updatable but they exist at the
        //               outer bounds of the simregion as static collidable objects
        //               for entites in the updatable region.
        //ASSERT(!(entity->updatable == 0));
        
        // NOTE(MIGUEL): The sword is brought into the sim region along with the player.
        //               Therefore it always will be in the sim region if the player is
        //               in the simregion even if it is not spatial.
        //ASSERT(entity->type != EntityType_sword);
        
        // NOTE(MIGUEL): Entities are allowed to exist in the sim region with out any 
        //               sim flags set. This is the case when the player fires the
        //               sword and the swords only sim flag, nonspacial is cleared. 
        //ASSERT(entity->flags != 0);
        
        /// NOTALLOWED CONDITIONS
        // NOTE(MIGUEL): There should not be any null types in the simregion.
        //               The only null entity is the enitiy at index 0 in the
        //               stored entity array.
        ASSERT(entity->type != EntityType_null);
        
        // NOTE(MIGUEL): This is the same as the test above againt null entities being 
        //               present in the sim region.
        ASSERT(!(entity->index_storage == 0));
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
SimRegion_add_entity(GameState *game_state,
                     SimRegion *sim_region,
                     u32 index_storage,
                     EntityLow *source,
                     V3 *sim_position);

inline V3 SimRegion_get_sim_space_position(SimRegion *sim_region, EntityLow *entity_stored)
{
    V3 result = ENTITY_INVALID_POSITION;
    
    if(!Entity_is_entity_sim_flags_set(&entity_stored->sim, EntitySimFlag_nonspatial))
    {
        V3 diff = World_subtract(sim_region->world,
                                 &entity_stored->position,
                                 &sim_region->origin);
        result = diff;
    }
    
    return result;
}

inline void
SimRegion_load_entity_reference(GameState *game_state, SimRegion *sim_region, EntityReference *ref)
{
    if(ref->index)
    {
        EntitySimHash *entry = SimRegion_get_hash_from_storage_index(sim_region, ref->index);
        
        if(entry->ptr == NULLPTR)
        {
            entry->index = ref->index;
            
            EntityLow *entity_low = Entity_get_entity_low(game_state, ref->index);
            
            V3 sim_pos = SimRegion_get_sim_space_position(sim_region, entity_low);
            
            entry->ptr = SimRegion_add_entity(game_state,
                                              sim_region,
                                              ref->index,
                                              entity_low,
                                              &sim_pos);
        }
        
        ref->ptr = entry->ptr;
    }
    
    return;
}

internal EntitySim *
SimRegion_add_entity_raw(GameState *game_state, SimRegion *sim_region, u32 index_storage, EntityLow *source)
{
    ASSERT(index_storage);
    
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
            entity->updatable = 0;
        }  
        else
        {
            INVALID_CODE_PATH;
        }
    }
    
    return entity;
}


internal EntitySim *
SimRegion_add_entity(GameState *game_state,
                     SimRegion *sim_region,
                     u32 index_storage,
                     EntityLow *source, V3 *sim_position)
{
    EntitySim *dest = SimRegion_add_entity_raw(game_state, sim_region, index_storage, source);
    
    if(dest)
    {
        if(sim_position)
        {
            dest->position  = *sim_position;
            dest->updatable = RectV3_is_in_rect(sim_region->updatable_bounds,
                                                dest->position);
        }
        else
        {
            dest->position = SimRegion_get_sim_space_position(sim_region, source);
        }
    }
    
    return dest;
}

internal SimRegion *
SimRegion_begin_sim(MemoryArena    *sim_arena,
                    GameState      *game_state,
                    World          *world,
                    WorldCoord      region_origin,
                    RectV3          region_bounds,
                    GameBackBuffer *back_buffer)
{
    SimRegion *sim_region = MEMORY_ARENA_PUSH_STRUCT(sim_arena, SimRegion);
    MEMORY_ARENA_ZERO_STRUCT(sim_region->hash);
    
    //ASSERT(validate_sim_entities(sim_region));
    
    f32 update_safety_margin = 1.0f; //UNIT: meters
    
    sim_region->world  = world;
    sim_region->origin = region_origin;
    sim_region->bounds = region_bounds;
    
    sim_region->updatable_bounds = region_bounds;
    sim_region->updatable_bounds = RectV3_add_radius_to(sim_region->updatable_bounds,
                                                        V3_init_uniform(update_safety_margin));
    
    sim_region->max_entity_count = 4096;
    sim_region->entity_count     = 0;
    sim_region->entities   = MEMORY_ARENA_PUSH_ARRAY(sim_arena, sim_region->max_entity_count, EntitySim);
    
    // TODO(MIGUEL): 07/12/2021 | HMH DAY 058  | TIME 35:57
    
    V3 corner = {0};
    
    corner = RectV3_min_corner(sim_region->bounds);
    
    WorldCoord minchunk_pos = World_map_to_chunkspace(world,
                                                      sim_region->origin,
                                                      corner);
    
    corner = RectV3_max_corner(sim_region->bounds);
    
    WorldCoord maxchunk_pos = World_map_to_chunkspace(world,
                                                      sim_region->origin,
                                                      corner);
    
    
    
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
                        
                        if((entity_low->sim.type == EntityType_sword) ||
                           (entity_low->sim.type == EntityType_player))
                        {
                            int dbgint = 13;
                            dbgint = 1408;
                        }
                        
                        V3 sim_space_pos = SimRegion_get_sim_space_position(sim_region, entity_low);
                        
                        if(!Entity_is_entity_sim_flags_set(&entity_low->sim, EntitySimFlag_nonspatial))
                        {
                            ASSERT(!Entity_is_entity_sim_flags_set(&entity_low->sim, EntitySimFlag_nonspatial));
                            
                            if(RectV3_is_in_rect(region_bounds, sim_space_pos))
                            {
                                SimRegion_add_entity(game_state, sim_region, index_low, entity_low, &sim_space_pos);
                            }
                        }
                        
                        validate_sim_entities(sim_region);
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
        
        
        if(entity->type == EntityType_sword)
        {
            u32 dbgint = 13;
            dbgint = 1408;
        }
        
        WorldCoord new_position = { 0 };
        if(Entity_is_entity_sim_flags_set(entity,EntitySimFlag_nonspatial))
        {
            new_position = World_null_position();
        }
        else
        {
            new_position = World_map_to_chunkspace(sim_region->world,
                                                   sim_region->origin,
                                                   entity->position);
        }
        
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
                if(camera_following_entity->sim.position.x > (9.0f * sim_region->world->tile_dim_in_meters.x))
                {
                    new_camera_position.rel_.x += 17.0f * (sim_region->world->tile_dim_in_meters.x);
                    World_recanonicalize_coord(sim_region->world->chunk_dim_in_meters.x,
                                               &new_camera_position.chunk_x,
                                               &new_camera_position.rel_.x); 
                }
                if(camera_following_entity->sim.position.x < -(9.0f * sim_region->world->tile_dim_in_meters.x))
                {
                    new_camera_position.rel_.x -= 17.0f * (sim_region->world->tile_dim_in_meters.x);
                    World_recanonicalize_coord(sim_region->world->chunk_dim_in_meters.x,
                                               &new_camera_position.chunk_x,
                                               &new_camera_position.rel_.x); 
                }
                if(camera_following_entity->sim.position.y > (5.0f * sim_region->world->tile_dim_in_meters.y))
                {
                    new_camera_position.rel_.y += 9.0f * (sim_region->world->tile_dim_in_meters.y);
                    World_recanonicalize_coord(sim_region->world->chunk_dim_in_meters.y,
                                               &new_camera_position.chunk_y,
                                               &new_camera_position.rel_.y); 
                }
                if(camera_following_entity->sim.position.y < -(5.0f * sim_region->world->tile_dim_in_meters.y))
                {
                    new_camera_position.rel_.y -= 9.0f * (sim_region->world->tile_dim_in_meters.y);
                    World_recanonicalize_coord(sim_region->world->chunk_dim_in_meters.y,
                                               &new_camera_position.chunk_y,
                                               &new_camera_position.rel_.y); 
                }
            }
            else
            {
                f32 follow_speed             = 2.0f;
                V3 entity_offset_for_frame   = { 0 };
                V2 camera_half_field_of_view =
                {
                    (17.0f * 0.5f) * game_state->world->tile_dim_in_meters.x,
                    ( 9.0f * 0.5f) * game_state->world->tile_dim_in_meters.y
                };
                
                entity_offset_for_frame = (V3)
                {
                    follow_speed * (camera_following_entity->sim.position.x / camera_half_field_of_view.x),
                    follow_speed * (camera_following_entity->sim.position.y / camera_half_field_of_view.y),
                    0.0f
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

internal b32
should_collide(GameState *game_state, EntitySim *a, EntitySim *b)
{
    b32 result = 0;
    
    if( a->index_storage > b->index_storage)
    {
        EntitySim *temp = a;
        a = b;
        b = temp;
    }
    
    if(!Entity_is_entity_sim_flags_set(a, EntitySimFlag_nonspatial) &&
       !Entity_is_entity_sim_flags_set(b, EntitySimFlag_nonspatial))
    {
        result = 1;
    }
    
    /// COLLSION HASH GETTER FUNCTION
    u32 hash_bucket = a->index_storage & (ARRAYCOUNT(game_state->collision_rule_hash) - 1);
    
    for(PairWiseCollisionRule *rule = game_state->collision_rule_hash[hash_bucket];
        rule;
        rule = rule->next_in_hash)
    {
        if((rule->storage_index_a == a->index_storage) &&
           (rule->storage_index_b == b->index_storage))
        {
            result = rule->should_collide;
            break;
        }
    }
    
    return result;
}

internal b32
handle_collision(EntitySim *a, EntitySim *b)
{
    b32 stops_on_collision = 0;
    
    if(a->type == EntityType_sword)
    {
        stops_on_collision = 0;
    }
    else
    {
        stops_on_collision = 1;
    }
    
    // NOTE(MIGUEL): a is a lower enum value than b
    if(a->type > b->type)
    {
        EntitySim *temp = a;
        a = b;
        b = temp;
    }
    
    if((a->type == EntityType_hostile) &&
       (b->type == EntityType_sword))
    {
        --a->hit_point_max;
        Entity_make_nonspatial(b);
    }
    
    return stops_on_collision;
}

/* // NOTE(MIGUEL): SYMPTOMS
GETTING STUCK ON COLLIDED ENTITY - throwing away the eccess vector after first collisionf
PASSING INTO - im not redirecting the vector
UNREASONABLE VELOCITY AFTER COLLISION 
*/

internal void
SimRegion_move_entity(SimRegion *sim_region,
                      GameState *game_state,
                      EntitySim *entity,
                      MoveSpec  *movespec,
                      f32        delta_t,
                      V3         acceleration)
{
    ASSERT(!Entity_is_entity_sim_flags_set(entity, EntitySimFlag_nonspatial));
    
    World *world = sim_region->world;
    
    
    ///DEBUG
    if(entity->type == EntityType_player)
    {
        int dbg_int = 0;
        dbg_int += 13;
    }
    
    
    if(movespec->unitmaxaccel)
    {
        f32 accel_magnitude = V2_inner(acceleration.xy, acceleration.xy);
        
        if(accel_magnitude > 1.0f)
        {
            V2_scale(1.0f / square_root(accel_magnitude), &acceleration.xy);
        }
    }
    
    
    V3 gravity        = V3_init_3f32(0.0f, 0.0f, -9.8f);
    V3 position_delta = { 0 };
    
    V3 a = acceleration; 
    V3 v = entity->velocity;
    
    V3_scale(movespec->speed, &acceleration); // Tune the accleration with speed
    V3_scale(-movespec->drag, &v);            // Apply friction to acceleration
    V3_add  (v, gravity, &v);
    V3_add  (acceleration, v, &acceleration);
    
    a = acceleration;
    v = entity->velocity;
    // ACCELRATION COMPONENT 
    V3_scale(0.5f , &a);
    V3_scale(square(delta_t), &a);
    
    // VELOCITY COMPONENT 
    V3_scale(delta_t , &v);
    // JOIN ACCEL & VELOCITY COMPONENT [ (accel / 2) * t^2 + (vel * t) ]
    V3_add  (a, v, &position_delta); // NOTE(MIGUEL): do not alter value! used a bit lower in the function
    
    // STORE VELOCITY EQUATION
    a = acceleration;
    v = entity->velocity;
    V3_add  (a, v, &a);
    V3_scale(0.5f , &a);
    V3_scale(delta_t, &a);
    V3_add  (a, entity->velocity, &entity->velocity);
    
    f32 distance_remaining = entity->distance_limit;
    if(distance_remaining == 0.0f)
    {
        distance_remaining = 10000.0f;
    }
    
    for(u32 collision_resolve_attempt = 0; 
        collision_resolve_attempt < 4;
        collision_resolve_attempt++)
    {
        f32 normalized_time_of_pos_delta  = 1.0f; 
        f32 player_movement_vector_length = V2_length(position_delta.xy);
        
        if(player_movement_vector_length > 0.0f)
        {
            if(player_movement_vector_length > distance_remaining)
            {
                normalized_time_of_pos_delta = (distance_remaining / player_movement_vector_length);
            }
            
            V3 wall_normal = { 0 };
            // NORMALIZED SACALAR THAT REPS THE TIME STEP! NOT .033MS (MS PER FRAME)
            EntitySim *hit_entity = NULLPTR;
            
            V3 desired_position = {0};
            V3_add(entity->position, position_delta, &desired_position);
            
            //b32 stops_on_collision = Entity_is_entity_sim_flags_set(entity, EntitySimFlag_collides);
            
            if(!Entity_is_entity_sim_flags_set(entity, EntitySimFlag_nonspatial))
            {
                for(u32 test_entity_sim_index = 0;
                    test_entity_sim_index < sim_region->entity_count;
                    test_entity_sim_index++)
                {
                    EntitySim *test_entity = sim_region->entities + test_entity_sim_index;
                    // NOTE(MIGUEL): to avoid colision test of an entity with itself (reference compararison)
                    
                    
                    /// DEBUG
                    if(entity->type == EntityType_player)
                    {
                        int dbg_int = 0;
                        dbg_int += 123;
                    }
                    
                    if(should_collide(game_state, entity, test_entity) &&
                       (entity != test_entity))
                    { 
                        
                        // NOTE(MIGUEL): Minkowski sum
                        V3 minkowski_diameter = V3_init_3f32(test_entity->width  + entity->width,
                                                             test_entity->height + entity->height,
                                                             2.0f * world->tile_dim_in_meters.z);
                        
                        V3 min_corner = minkowski_diameter;
                        V3 max_corner = minkowski_diameter;
                        
                        // NOTE(MIGUEL): distance away from tile center
                        V3_scale(-0.5f, &min_corner);
                        V3_scale( 0.5f, &max_corner);
                        
                        // NOTE(MIGUEL): old pos's Distance away from test tile in meters in x & y respectively
                        // NOTE(MIGUEL): Tile_subtract operand order maybe wrong...
                        V2 rel_old_pos = { 0.0f };
                        V2_sub((V2){     entity->position.x,      entity->position.y},
                               (V2){test_entity->position.x, test_entity->position.y},
                               &rel_old_pos);
                        
                        f32 MinCowSkied_wall_left_x   = min_corner.x;
                        f32 MinCowSkied_wall_right_x  = max_corner.x;
                        f32 MinCowSkied_wall_top_y    = max_corner.y;
                        f32 MinCowSkied_wall_bottom_y = min_corner.y;
                        
                        // VERTICAL WALL RIGHT FACE
                        if(Game_get_normalized_time_at_collision(&normalized_time_of_pos_delta, MinCowSkied_wall_right_x,
                                                                 rel_old_pos.x, rel_old_pos.y,
                                                                 position_delta.x, position_delta.y,
                                                                 min_corner.y, max_corner.y))
                        {
                            wall_normal = V3_init_3f32(1.0f, 0.0f, 0.0f);
                            hit_entity =  test_entity;
                        }
                        
                        // VERTICAL WALL LEFT FACE
                        if(Game_get_normalized_time_at_collision(&normalized_time_of_pos_delta, MinCowSkied_wall_left_x,
                                                                 rel_old_pos.x, rel_old_pos.y,
                                                                 position_delta.x, position_delta.y,
                                                                 min_corner.y, max_corner.y))
                        {
                            wall_normal = V3_init_3f32(-1.0f, 0.0f, 0.0f);
                            hit_entity = test_entity;
                        }
                        
                        // HORIZONTAL BOTTOM WALL
                        if(Game_get_normalized_time_at_collision(&normalized_time_of_pos_delta, MinCowSkied_wall_bottom_y,
                                                                 rel_old_pos.y, rel_old_pos.x,
                                                                 position_delta.y, position_delta.x,
                                                                 min_corner.x, max_corner.x))
                        {
                            wall_normal = V3_init_3f32(0.0f, -1.0f, 0.0f);
                            hit_entity = test_entity;
                        }
                        
                        // HORIZONTAL TOP WALL
                        if(Game_get_normalized_time_at_collision(&normalized_time_of_pos_delta, MinCowSkied_wall_top_y,
                                                                 rel_old_pos.y, rel_old_pos.x,
                                                                 position_delta.y, position_delta.x,
                                                                 min_corner.x, max_corner.x))
                        {
                            wall_normal = V3_init_3f32(0.0f, 1.0f, 0.0f);
                            hit_entity = test_entity;
                        }
                    }
                }
            }
            
            ///DEBUG
            if(entity->type == EntityType_player)
            {
                int dbg_int = 0;
                dbg_int += 13;
            }
            
            /// MOVE PLAYER TO THE COLLISION POINT
            V3 scratch_position_delta = position_delta;
            V3_scale(normalized_time_of_pos_delta, &scratch_position_delta);
            V3_add(entity->position, scratch_position_delta, &entity->position);
            
            distance_remaining -= (player_movement_vector_length *
                                   normalized_time_of_pos_delta);
            
            
            if(hit_entity)
            {
                /// NEW POSITION FOR COLLISION TEST
                V3_sub(desired_position, entity->position, &position_delta);
                
                b32 stops_on_collision = handle_collision(entity, hit_entity);
                if(stops_on_collision)
                {
                    V3 new_velocity = wall_normal;
                    
                    // UPDATE VELOCITY VECTOR 
                    f32 velocity_to_normal_axis_projection_scalar =
                        V2_inner(entity->velocity.xy, wall_normal.xy);
                    
                    V2_scale(velocity_to_normal_axis_projection_scalar, &new_velocity.xy);
                    V2_scale(1.0f, &new_velocity.xy);
                    V2_sub  (entity->velocity.xy, new_velocity.xy, &entity->velocity.xy);
                    
                    /*
    */
                    V3 new_position_delta = wall_normal;
                    f32 position_delta_to_normal_axis_projection_scalar =
                        V2_inner(position_delta.xy, wall_normal.xy);
                    // UPDATE MOVEMENT VECTOR
                    V2_scale(position_delta_to_normal_axis_projection_scalar, &new_position_delta.xy);
                    V2_scale(1.0f, &new_position_delta.xy);
                    V2_sub  (position_delta.xy, new_position_delta.xy, &position_delta.xy);
                }
                else
                {
                    add_collision_rule(game_state, entity->index_storage, hit_entity->index_storage, 0);
                }
                
                
                ///DEBUG
                if(entity->type == EntityType_player)
                {
                    int dbg_int = 0;
                    dbg_int += 13;
                    
                    if((entity->velocity.x == 0.0f) ||
                       (entity->velocity.y == 0.0f))
                    {
                        int dbgint = 13;
                        dbgint = 1408;
                    }
                }
                
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
    
    if(entity->position.z < 0.0f)
    {
        entity->position.z = 0.0f;
    }
    
    if(entity->distance_limit != 0.0f)
    {
        entity->distance_limit = distance_remaining;
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
