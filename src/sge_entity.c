
internal void
Entity_update_friendly(GameState *game_state, Entity entity, f32 delta_t)
{
    Entity closest_player = { 0 };
    f32    player_search_diametersq = square(10.0f);
    
    for(u32 index_high = 1; index_high < game_state->entity_count_high; index_high++)
    {
        Entity test_entity = Entity_from_high_index(game_state, index_high);
        
        if(test_entity.low->type == EntityType_player)
        {
            //follow
            V2 position_delta;
            V2_sub(test_entity.high->position, entity.high->position, &position_delta);
            f32 test_dsq = V2_length_sq(position_delta);
            
            if(player_search_diametersq > test_dsq)
            {
                closest_player = test_entity;
                player_search_diametersq = test_dsq;
            }
            
        }
        
    }
    
    V2 acceleration = { 0 };
    if(closest_player.high && (player_search_diametersq > square(3.0f)))
    {
        f32 coef = 0.5f;
        f32 one_over_length = coef / square_root(player_search_diametersq);
        
        V2_sub(closest_player.high->position, entity.high->position, &acceleration);
        V2_scale(one_over_length, &acceleration);
        
        
        
        MoveSpec movespec = default_movespec();
        movespec.unitmaxaccel = 1;
        movespec.speed = 300.0f;
        movespec.drag  = 12.0f;
        
        Entity_move(game_state, entity, &movespec, delta_t, acceleration);
    }
    
    return;
}

internal void
Entity_update_sword(GameState *game_state, Entity entity, f32 delta_t)
{
    
    // NOTE(MIGUEL): sword accerlerates even though theres no acceleration
    // TODO(MIGUEL): fix it
    MoveSpec movespec = default_movespec();
    movespec.unitmaxaccel = 1;
    movespec.speed = 0.0f;
    movespec.drag  = 0.0f;
    
    V2 old_pos = entity.high->position;
    Entity_move(game_state, entity, &movespec, delta_t, (V2){0,0});
    V2 result = { 0 };
    V2_sub(entity.high->position, old_pos , &result);
    
    f32 distance_travaled = V2_length(result);
    
    entity.low->distance_remaining -= distance_travaled;
    if(entity.low->distance_remaining < 0.0f)
    {
        //&entity.low->position
        Entity_change_entity_location(game_state->world,
                                      entity.index_low, entity.low,
                                      NULLPTR, NULLPTR,
                                      NULLPTR);
        
    }
    
    return;
}

internal void
Entity_update_hostile(EntityState *game_state, Entity entity, f32 delta_t)
{
    
    return;
}
