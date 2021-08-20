inline MoveSpec
default_movespec(void)
{
    MoveSpec result = { 0 };
    result.unitmaxaccel = 0;
    result.speed = 1.f;
    result.drag  = 0.f;
    
    return result;
}

internal void
Entity_update_friendly(SimRegion *sim_region, EntitySim *entity, f32 delta_t)
{
    EntitySim *closest_player = { 0 };
    f32    player_search_diametersq = square(10.0f);
    
    EntitySim *test_entity = sim_region->entities;
    for(u32 test_entity_index = 1;
        test_entity_index < sim_region->entity_count;
        test_entity_index++)
    {
        if(test_entity->type == EntityType_player)
        {
            //follow
            V2 position_delta;
            V2_sub(test_entity->position, entity->position, &position_delta);
            f32 test_dsq = V2_length_sq(position_delta);
            
            if(player_search_diametersq > test_dsq)
            {
                closest_player = test_entity;
                player_search_diametersq = test_dsq;
            }
            
        }
        
    }
    
    V2 acceleration = { 0 };
    if(closest_player && (player_search_diametersq > square(3.0f)))
    {
        f32 coef = 0.5f;
        f32 one_over_length = coef / square_root(player_search_diametersq);
        
        V2_sub(closest_player->position, entity->position, &acceleration);
        V2_scale(one_over_length, &acceleration);
        
        MoveSpec movespec = default_movespec();
        movespec.unitmaxaccel = 1;
        movespec.speed = 300.0f;
        movespec.drag  = 12.0f;
        
        SimRegion_move_entity(sim_region, entity, &movespec, delta_t, acceleration);
    }
    
    return;
}

internal void
Entity_update_hostile(SimRegion *sim_region, EntitySim *entity, f32 delta_t)
{
    
    return;
}

internal void
Entity_update_sword(SimRegion *sim_region, EntitySim *entity, f32 delta_t)
{
    
    // NOTE(MIGUEL): sword accerlerates even though theres no acceleration
    // TODO(MIGUEL): fix it
    MoveSpec movespec = default_movespec();
    movespec.unitmaxaccel = 1;
    movespec.speed = 0.0f;
    movespec.drag  = 0.0f;
    
    V2 old_pos = entity->position;
    SimRegion_move_entity(sim_region, entity, &movespec, delta_t, (V2){0,0});
    V2 result = { 0 };
    V2_sub(entity->position, old_pos , &result);
    
    f32 distance_travaled = V2_length(result);
    
    entity->distance_remaining -= distance_travaled;
    if(entity->distance_remaining < 0.0f)
    {
        ASSERT(!"NEED TO MAKE ENTITIES BE ABLE TO NOT BE THERE!");
    }
    
    return;
}
