#include "sge.h"
#include "sge_world.c"
#include "sge_random.h"
#include "sge_sim_region.c"
#include "sge_entity.c"
// TODO(MIGUEL): App cannnot Crash when stick is not connected
// TODO(MIGUEL): App cannnot Crash when MCU is not connected
// TODO(MIGUEL): App should give use an oppertunity to connect a device(stick, mcu) thoughout app life time
// TODO(MIGUEL): Integrate FreeType library
// TODO(MIGUEL): Implement a simple gui
// TODO(MIGUEL): GUI: No cursor, Buttons with key
// TODO(MIGUEL): Integrate Opencv once app has access to camera feed
// NOTE(MIGUEL): Check Network.h for Networking TODOS
//fuck git

SGE_GET_SOUND_SAMPLES(SGEGetSoundSamples)
{
    GameState *game_state = (GameState *)sge_memory->permanent_storage;
    
    Game_update_sound_buffer(game_state, sound_buffer, 400);
    
    return;
}

SGE_INIT(SGEInit)
{
    // NOTE(MIGUEL): get rid of this it makes no sense
    
    return;
}



//~ Entity Interface

typedef struct CreateEntitySimResult CreateEntitySimResult;
struct CreateEntitySimResult
{
    EntityLow *
        entity_low;
    u32         index_low;
};

internal CreateEntitySimResult
Entity_create_entity_low(GameState *game_state, EntityType type, WorldCoord position)
{
    ASSERT(game_state->entity_count_low < ARRAYCOUNT(game_state->entities_low));
    
    u32 index_low = game_state->entity_count_low++;
    
    EntityLow *entity_low = game_state->entities_low + index_low;
    
    memset(entity_low, 0, sizeof(EntityLow));
    entity_low->sim.type = type;
    entity_low->position = World_null_position();
    
    World_change_entity_location(game_state->world,
                                 index_low,
                                 entity_low,
                                 position,
                                 &game_state->world_arena);
    
    CreateEntitySimResult result;
    result.index_low  = index_low;
    result.entity_low = entity_low;
    
    return result;
}

internal void
init_hitpoint(EntityLow *entity_low, u32 hitpointcount)
{
    ASSERT(hitpointcount <= ARRAYCOUNT(entity_low->sim.hit_points));
    
    entity_low->sim.hit_point_max = hitpointcount;
    
    for(u32 hitpoint_index = 0; 
        hitpoint_index < entity_low->sim.hit_point_max; hitpoint_index++)
    {
        HitPoint *hitpoint = entity_low->sim.hit_points + hitpoint_index;
        hitpoint->flags = 0;
        hitpoint->filled_amount = HITPOINT_SUB_COUNT;
    }
    
    return;
}


internal CreateEntitySimResult
Game_add_sword(GameState *game_state)
{ 
    CreateEntitySimResult result = Entity_create_entity_low(game_state,
                                                            EntityType_sword,
                                                            World_null_position());
    
    result.entity_low->sim.dim.h          = 0.5f; //           UNITS: meters
    result.entity_low->sim.dim.w           = 1.0f; //           UNITS: meters
    
    return result;
}


internal CreateEntitySimResult
Game_add_player(GameState *game_state, u32 player_number)
{
    WorldCoord position = game_state->camera_position;
    position.rel_.x += player_number * 2;
    
    CreateEntitySimResult result = Entity_create_entity_low(game_state,
                                                            EntityType_player,
                                                            position);
    
    CreateEntitySimResult weapon = Game_add_sword (game_state);
    
    result.entity_low->sim.sword.index = weapon.index_low;
    
    init_hitpoint(result.entity_low, 3);
    
    add_collision_rule(game_state, weapon.index_low, result.index_low, 1);
    
    result.entity_low->sim.dim.h          = 0.5f; //           UNITS: meters
    result.entity_low->sim.dim.w         = 1.0f; //           UNITS: meters
    Entity_set_entity_sim_flags(&result.entity_low->sim, EntitySimFlag_collides);
    
    if(game_state->camera_following_entity_index == 0)
    {
        game_state->camera_following_entity_index = result.index_low;
    }
    
    return result;
}


internal CreateEntitySimResult
Game_add_friendly(GameState *game_state, s32 tile_abs_x, s32 tile_abs_y, s32 tile_abs_z)
{
    WorldCoord position = World_worldcoord_from_tilecoord(game_state->world,
                                                          tile_abs_x,
                                                          tile_abs_y,
                                                          tile_abs_z);
    
    CreateEntitySimResult result = Entity_create_entity_low(game_state,
                                                            EntityType_friendly,
                                                            position);
    
    //init_hitpoint(result.entity_low, 3); // NOTE(MIGUEL): these have no hitpoints yet
    
    result.entity_low->sim.dim.h          = 0.5f; // UNITS: meters
    result.entity_low->sim.dim.w           = 1.0f; // UNITS: meters
    Entity_set_entity_sim_flags(&result.entity_low->sim, EntitySimFlag_collides);
    
    return result;
}


internal CreateEntitySimResult
Game_add_hostile(GameState *game_state, s32 tile_abs_x, s32 tile_abs_y, s32 tile_abs_z)
{
    WorldCoord position = World_worldcoord_from_tilecoord(game_state->world,
                                                          tile_abs_x,
                                                          tile_abs_y,
                                                          tile_abs_z);
    
    CreateEntitySimResult result = Entity_create_entity_low(game_state,
                                                            EntityType_hostile,
                                                            position);
    
    
    init_hitpoint(result.entity_low, 3);
    
    
    result.entity_low->sim.dim.h          = 0.5f; //           UNITS: meters
    result.entity_low->sim.dim.w          = 1.0f; //           UNITS: meters
    Entity_set_entity_sim_flags(&result.entity_low->sim, EntitySimFlag_collides);
    
    return result;
}

internal CreateEntitySimResult
Game_add_wall(GameState *game_state, s32 tile_abs_x, s32 tile_abs_y, s32 tile_abs_z)
{
    
    WorldCoord position = World_worldcoord_from_tilecoord(game_state->world,
                                                          tile_abs_x,
                                                          tile_abs_y,
                                                          tile_abs_z);
    
    CreateEntitySimResult result = Entity_create_entity_low(game_state,
                                                            EntityType_wall,
                                                            position);
    
    result.entity_low->sim.dim.h   = game_state->world->tile_dim_in_meters.y; //UNITS: meters
    result.entity_low->sim.dim.w   = result.entity_low->sim.dim.h           ; //UNITS: meters
    Entity_set_entity_sim_flags (&result.entity_low->sim, EntitySimFlag_collides);
    
    return result;
}

internal void
clear_collision_all_rules(GameState *game_state, u32 storage_index)
{
    PairWiseCollisionRule *found = NULLPTR;
    
    for(u32 hash_bucket = 0;
        hash_bucket < ARRAYCOUNT(game_state->collision_rule_hash);
        hash_bucket++)
    {
        for(PairWiseCollisionRule **rule = &game_state->collision_rule_hash[hash_bucket];
            *rule; 
            )
        {   
            if(((*rule)->storage_index_a == storage_index) ||
               ((*rule)->storage_index_b == storage_index))
            {
                PairWiseCollisionRule *removed_rule = *rule;
                *rule = (*rule)->next_in_hash;
                
                removed_rule->next_in_hash = game_state->first_free_collision_rule_node;
                game_state->first_free_collision_rule_node = removed_rule;
            }
            else
            {
                rule = &(*rule)->next_in_hash;
            }
        }
    }
    
    return;
}

internal void
add_collision_rule(GameState *game_state, u32 storage_index_a, u32 storage_index_b, b32 should_collide)
{
    if(storage_index_a > storage_index_b)
    {
        u32 temp = storage_index_a;
        storage_index_a = storage_index_b;
        storage_index_b = temp;
    }
    
    PairWiseCollisionRule *found = NULLPTR;
    
    u32 hash_bucket = storage_index_a & (ARRAYCOUNT(game_state->collision_rule_hash) - 1);
    
    for(PairWiseCollisionRule *rule = game_state->collision_rule_hash[hash_bucket];
        rule;
        rule = rule->next_in_hash)
    {
        if((rule->storage_index_a == storage_index_a) &&
           (rule->storage_index_b == storage_index_b))
        {
            found = rule;
            break;
        }
    }
    
    if(!found)
    {
        found = game_state->first_free_collision_rule_node;
        
        if(found)
        {
            game_state->first_free_collision_rule_node = found->next_in_hash; 
        }
        else
        {
            found = MEMORY_ARENA_PUSH_STRUCT(&game_state->world_arena, PairWiseCollisionRule);
        }
        
        //found = MEMORY_ARENA_PUSH_STRUCT(&game_state->world_arena, PairWiseCollisionRule);
        found->next_in_hash = game_state->collision_rule_hash[hash_bucket];
        game_state->collision_rule_hash[hash_bucket] = found;
    }
    
    if(found)
    { 
        found->storage_index_a = storage_index_a;
        found->storage_index_b = storage_index_b;
        found->should_collide = should_collide;
    }
    
    return;
}

void 
Game_draw_bounding_box(GameBackBuffer *back_buffer,
                       V2              origin,
                       RectV2          bounds,
                       f32             scale,
                       V4              color,
                       b32             debug)
{
    V4 red    = {1.0f, 0.0f, 0.0f, 0.5};
    V4 green  = {0.0f, 1.0f, 0.0f, 0.5};
    V4 blue   = {0.0f, 0.0f, 1.0f, 0.5};
    V4 yellow = {1.0f, 1.0f, 0.0f, 0.5};
    V4 purple = {1.0f, 0.0f, 1.0f, 0.5};
    V4 grey   = {0.6f, 0.6f, 0.6f, 0.5};
    V4 white  = {1.0f, 1.0f, 1.0f, 0.5};
    V4 black  = {0.0f, 0.0f, 0.0f, 0.5};
    V4 dark_gray = {0.3f, 0.3f, 0.3f, 0.5};
    
    
    //RectV2 bounds_in_minimap_space = bounds;
    // TODO(MIGUEL): fix the coordinate bullshit with point whatever
    //V2_scale(-1.0f, &bounds_in_minimap_space.min);
    //V2_scale(-1.0f, &bounds_in_minimap_space.max);
    
    V2_scale(scale, &bounds.min);
    V2_scale(scale, &bounds.max);
    V2_add(bounds.min, origin, &bounds.min);
    V2_add(bounds.max, origin, &bounds.max);
    
    RectV2 bounds_top    = bounds;
    RectV2 bounds_bottom = bounds;
    RectV2 bounds_left   = bounds;
    RectV2 bounds_right  = bounds;
    
    V4 colors[4];
    
    if(debug)
    {
        colors[0] = red;
        colors[1] = blue;
        colors[2] = purple;
        colors[3] = yellow;
    }
    else
    {
        colors[0] = color;
        colors[1] = color;
        colors[2] = color;
        colors[3] = color;
    }
    
    bounds_top.min.y = bounds.max.y - 2;
    Game_draw_rectangle(back_buffer,
                        bounds_top.min,
                        bounds_top.max,
                        colors[0],
                        0);
    
    
    bounds_bottom.max.y = bounds.min.y + 2;
    Game_draw_rectangle(back_buffer,
                        bounds_bottom.min,
                        bounds_bottom.max,
                        colors[1],
                        0);
    
    bounds_left.min.x = bounds.max.x - 2;
    Game_draw_rectangle(back_buffer,
                        bounds_left.min,
                        bounds_left.max,
                        colors[2],
                        0);
    
    
    bounds_right.max.x = bounds.min.x + 2;
    Game_draw_rectangle(back_buffer,
                        bounds_right.min,
                        bounds_right.max,
                        colors[3],
                        0);
    
    
    return;
}

internal void
Game_draw_mini_map(GameState      *game_state,
                   GameBackBuffer *back_buffer,
                   GameInput      *input,
                   SimRegion      *sim_region,
                   s32             tile_side_in_pixels,
                   V2              camera_range_in_meters)
{
    World *world = game_state->world;
    
    f32 mouse_wheel = input->mouse_wheel_integral * 0.01f;
    
    if(mouse_wheel > 1.0f)
    {
        mouse_wheel = 1.0f;
    }
    if(mouse_wheel < -0.25f)
    {
        mouse_wheel = -0.25f;
    }
    
    f32 normalized_zoom = mouse_wheel + 0.25f;
    
    s32 x = (s32)(back_buffer->width  / -2.0f) + input->mouse_x;
    s32 y = (s32)(back_buffer->height / -2.0f) + input->mouse_y;
    
    // NOTE(MIGUEL): This is only good for positioning of the camera
    EntityLow *camera_following_entity = Entity_get_entity_low(game_state,
                                                               game_state->camera_following_entity_index);
    
    
    // NOTE(MIGUEL): VERY HIGH RISK CODE !!!!
    //               tile_side_in_pixels depend on the tile sides to be uniform.
    //               If in the future that canges then the following code wont draw
    //               correctly on y. Maybe meters_to_pixels vector.
    f32 meters_to_pixels = ((f32)tile_side_in_pixels / (f32)world->tile_dim_in_meters.x);
    
    /// CAMERA BASIS CHANGE
    V2 screen_center =
    {
        0.5f * (f32)back_buffer->width,
        0.5f * (f32)back_buffer->height
    };
    
    V2 minimap_center = 
    {
        (f32)screen_center.x + x,
        (f32)screen_center.y + y,
    };
    
    V2_scale((f32)meters_to_pixels, &camera_range_in_meters);
    
    V2 draw_pos = { 0 };
    
    if(camera_following_entity)
    {
        draw_pos.x = camera_following_entity->sim.position.x * meters_to_pixels;
        draw_pos.y = camera_following_entity->sim.position.y * meters_to_pixels;
    }
    
    V2 draw_size_in_meters =
    {
        .x = 1.3f * meters_to_pixels,
        .y = 1.3f * meters_to_pixels,
    };
    
    
    
    RectV2 minimap_draw_area = RectV2_center_dim(minimap_center, camera_range_in_meters);
    
    // TODO(MIGUEL): calculate the which chunks are in in the minimap view space.
    //               Grab low entities from those chunks to render their status.
    s32 view_scale = 1;
    camera_range_in_meters.x *= view_scale;
    camera_range_in_meters.y *= view_scale;
    
    /// PALLETE
    V4 color     = { 0 };
    V4 red       = {1.0f, 0.0f, 0.0f, 0.5};
    V4 green     = {0.0f, 1.0f, 0.0f, 0.5};
    V4 blue      = {0.0f, 0.0f, 1.0f, 0.5};
    V4 yellow    = {1.0f, 1.0f, 0.0f, 0.5};
    V4 cyan      = {0.0f, 1.0f, 1.0f, 0.5};
    V4 purple    = {1.0f, 0.0f, 1.0f, 0.5};
    V4 grey      = {0.6f, 0.6f, 0.6f, 0.5};
    V4 white     = {1.0f, 1.0f, 1.0f, 0.5};
    V4 black     = {0.0f, 0.0f, 0.0f, 0.5};
    V4 dark_gray = {0.3f, 0.3f, 0.3f, 0.5};
    
    /// MINIMAP BACKGROUND
    Game_draw_rectangle(back_buffer,
                        (V2){minimap_draw_area.min.x,minimap_draw_area.min.y},
                        (V2){minimap_draw_area.max.x,minimap_draw_area.max.y},
                        green,
                        0);
    
    // NOTE(MIGUEL): APPLY THIS TO EVERYTHING!!!!
    f32 meters_to_pixels_with_zoom_comp = (f32)meters_to_pixels * ((10.0f * normalized_zoom));
    EntityLow *entity_low = game_state->entities_low;
    
    // DRAW CAMERA BOUNDS
    
    V2 view_tile_span = { 17, 9 };
    V2_scale(world->tile_dim_in_meters.x, &view_tile_span);
    V2_scale(0.5f, &view_tile_span);
    
    RectV2 camera_bounds = RectV2_center_half_dim((V2){0 , 0},
                                                  view_tile_span);
    
    Game_draw_bounding_box(back_buffer,
                           minimap_center,
                           camera_bounds,
                           meters_to_pixels_with_zoom_comp,
                           red,
                           0);
    
    
    // DRAW UPDATABLE BOUNDS
    
    RectV2 updatable_bounds = RectV2_min_max(sim_region->updatable_bounds.min.xy,
                                             sim_region->updatable_bounds.max.xy);
    
    Game_draw_bounding_box(back_buffer,
                           minimap_center,
                           updatable_bounds,
                           meters_to_pixels_with_zoom_comp,
                           blue,
                           0);
    
    
    // DRAW SIMREGION BOUNDS
    RectV2 sim_region_bounds = RectV2_min_max(sim_region->bounds.min.xy,
                                              sim_region->bounds.max.xy);
    
    Game_draw_bounding_box(back_buffer,
                           minimap_center,
                           sim_region_bounds,
                           meters_to_pixels_with_zoom_comp,
                           yellow,
                           0);
    
    
    V3 sim_bounds = V3_init_2f32(sim_bounds.x = sim_region->bounds.min.x,
                                 sim_bounds.y = sim_region->bounds.min.y);
    
    WorldCoord min = World_map_to_chunkspace(world,
                                             game_state->camera_position,
                                             sim_bounds);
    
    sim_bounds = V3_init_2f32(sim_region->bounds.max.x,
                              sim_region->bounds.max.y);
    
    WorldCoord max = World_map_to_chunkspace(world,
                                             game_state->camera_position,
                                             sim_bounds);
    
    for(s32 chunk_y = min.chunk_y;
        chunk_y <= max.chunk_y;
        chunk_y++)
    {
        for(s32 chunk_x = min.chunk_x;
            chunk_x <= max.chunk_x;
            chunk_x++)
        {
            WorldChunk *chunk = World_get_worldchunk(world,
                                                     chunk_x,
                                                     chunk_y,
                                                     sim_region->origin.chunk_z,
                                                     NULLPTR);
            if(chunk)
            {
                V2 chunk_center = { 0 };
                RectV2 chunk_bounds = { 0 };
                Game_draw_bounding_box(back_buffer,
                                       minimap_center,
                                       chunk_bounds,
                                       meters_to_pixels_with_zoom_comp,
                                       cyan,
                                       0);
                
                for(WorldEntityBlock *block = &chunk->first_block;
                    block; block = block->next)
                {
                    for(u32 entityindexindex = 0; 
                        entityindexindex < block->entity_count;
                        entityindexindex++)
                    {
                        u32 entity_low_index = block->entity_indices_low[entityindexindex];
                        EntityLow *entity_low = game_state->entities_low + entity_low_index;
                        
                        
                        V4 draw_color = red;
                        
                        draw_pos.x = entity_low->sim.position.x * meters_to_pixels_with_zoom_comp;
                        draw_pos.y = entity_low->sim.position.y * meters_to_pixels_with_zoom_comp;
                        
                        draw_size_in_meters.x = 1.3f * meters_to_pixels_with_zoom_comp;
                        draw_size_in_meters.y = 1.3f * meters_to_pixels_with_zoom_comp;
                        
                        EntitySimHash *entry = SimRegion_get_hash_from_storage_index(sim_region, entity_low_index);
                        
                        switch(entity_low->sim.type)
                        {
                            case EntityType_player:
                            {
                                draw_color = blue;
                            } break;
                            
                            case EntityType_friendly:
                            {
                                draw_color = yellow;
                            } break;
                            
                            case EntityType_hostile:
                            {
                                draw_color = red;
                            } break;
                            
                            case EntityType_wall:
                            {
                                draw_color = purple;
                            } break;
                            
                            case EntityType_sword:
                            {
                                draw_color = cyan;
                            } break;
                            
                            case EntityType_null:
                            {
                                draw_color = black;
                            } break;
                            
                        }
                        
                        if(entry->ptr == NULLPTR)
                        {
                            draw_color = (V4){1.0f, 1.0f, 1.0f, 1.0f};
                            
                        }
                        
                        Game_draw_rectangle(back_buffer,
                                            (V2)
                                            {
                                                (minimap_center.x + draw_pos.x) - (draw_size_in_meters.x * 0.5f),
                                                (minimap_center.y - draw_pos.y) - (draw_size_in_meters.y * 0.5f)
                                            },
                                            (V2)
                                            {
                                                (minimap_center.x + draw_pos.x) + (draw_size_in_meters.x * 0.5f),
                                                (minimap_center.y - draw_pos.y) + (draw_size_in_meters.y * 0.5f),
                                            },
                                            draw_color,
                                            0);
                        
                        
                        //validate_sim_entities(sim_region);
                    }
                }
            }
        }
    }
    
    return;
}

internal void
Game_draw_array(GameState *game_state,
                GameBackBuffer *back_buffer,
                SimRegion *sim_region)
{
    //UX1
    //SimRegion DEBUG
    V4 color  = { 0 };
    V4 black  = {0.0f, 0.0f, 0.0f, 1.0f};
    V4 white  = {1.0f, 1.0f, 1.0f, 1.0f};
    V4 red    = {1.0f, 0.0f, 0.0f, 0.5f};
    V4 green  = {0.0f, 1.0f, 0.0f, 0.5f};
    V4 blue   = {0.0f, 0.0f, 1.0f, 0.5f};
    V4 yellow = {1.0f, 1.0f, 0.0f, 0.5f};
    V4 cyan   = {0.0f, 1.0f, 1.0f, 0.5f};
    V4 purple = {1.0f, 0.0f, 1.0f, 0.5f};
    V4 grey   = {0.6f, 0.6f, 0.6f, 0.5f};
    V4 dark_gray = {0.3f, 0.3f, 0.3f, 0.5f};
    
    f32 array_debug_zoom = 79.0f;
    f32 padding = 10.0f;
    V2  offset  = {padding, padding};
    // NOTE(MIGUEL): wtf is thisa
    f32 height  = (f32)floor_f32_to_s32((back_buffer->height - (padding * 2)) /
                                        (f32)(sim_region->max_entity_count * 0.001f * (100.1f - array_debug_zoom)));
    f32 width   = 8.0f;
    u32 max_array_rows_for_height = (u32)((back_buffer->height - padding * 2.0f) / height);
    
    u32 array_row = 0;
    u32 array_col  = 1;
    f32 array_col_padding = 1.0f;
    
    EntitySim *entity = sim_region->entities;
    
    for(u32 entity_index = 0;
        entity_index < sim_region->max_entity_count;
        entity_index++, entity++ )
    {
        V4 divider_color = black;
        V4 empty_color   = green;
        
        array_row = entity_index % max_array_rows_for_height;
        array_col = (u32)floor_f32_to_s32(entity_index / (f32)max_array_rows_for_height) + 1;
        
        f32 x = (array_col * (width + array_col_padding));
        f32 y = (f32)height * (f32)array_row + padding;
        
        if(entity_index <= sim_region->entity_count)
        {
            switch(entity->type)
            {
                case EntityType_player:
                {
                    color = blue;
                } break;
                
                case EntityType_friendly:
                {
                    color = yellow;
                } break;
                
                case EntityType_hostile:
                {
                    color = red;
                } break;
                
                case EntityType_wall:
                {
                    color = purple;
                } break;
                
                case EntityType_sword:
                {
                    color = cyan;
                } break;
                
                case EntityType_null:
                {
                    color = black;
                } break;
                
                default:
                {
                    color = white;
                } break;
                
            }
            
            Game_draw_rectangle(back_buffer,
                                (V2){x +  0.0f, y},
                                (V2){x + width, height * ((f32)array_row + 1.0f) + padding},
                                color, 0);
            
            Game_draw_rectangle(back_buffer,
                                (V2){x +  0.0f, y + (height * 0.90f)},
                                (V2){x + width, height * ((f32)array_row + 1.0f) + padding},
                                divider_color, 0);
            
            
            continue;
        }
        
        /// EMPTY SLOT
        Game_draw_rectangle(back_buffer,
                            (V2){x +  0.0f, y + 0.0f},
                            (V2){x + width, height * ((f32)array_row + 1.0f) + padding},
                            empty_color, 0);
        
        /// DIVIDER LINE
        Game_draw_rectangle(back_buffer,
                            (V2){x +  0.0f, y + (height * 0.90f)},
                            (V2){x + width,  height * ((f32)array_row + 1.0f) + padding},
                            divider_color, 0);
        
        //validate_sim_entities(sim_region);
    }
    
    return;
}

internal void
Game_draw_bitmap(GameBackBuffer *buffer, BitmapData *bitmap,
                 f32 real_x, f32 real_y,
                 f32 c_alpha)
{
    /// rounding / ruling
    s32 min_x = round_f32_to_s32(real_x);
    s32 min_y = round_f32_to_s32(real_y);
    
    s32 max_x = min_x + bitmap->width ;
    s32 max_y = min_y + bitmap->height;
    
    /// clipping
    // NOTE(MIGUEL): will right to but not including the final row 
    s32 src_offset_x = 0;
    if(min_x < 0)
    {
        src_offset_x = -min_x;
        min_x = 0;
    }
    s32 src_offset_y = 0;
    if(min_y < 0)
    {
        src_offset_y = -min_y;
        min_y = 0;
    }
    if(max_x > buffer->width)
    {
        max_x = buffer->width;
    }
    if(max_y > buffer->height)
    {
        max_y = buffer->height;
    }
    
    // NOTE(MIGUEL): why is it supposed to be pixel_height - 1
    u32 *src_row  = bitmap->pixels + (bitmap->width * (bitmap->height - 1));
    
    src_row += -bitmap->width * src_offset_y + src_offset_x;
    
    u8  *dest_row = ((u8 *)  (buffer->data)            + 
                     (min_x * buffer->bytes_per_pixel) +
                     (min_y * buffer->pitch));
    
    for(s32 y = min_y; y < max_y; y++)
    {
        u32 *dest = (u32 *)dest_row;
        u32 *src  = (u32 *)src_row;
        
        for(s32 x = min_x; x < max_x; x++)
        {
            f32 a  = (f32)((*src >> 24) & 0xFF) / 255.0f;
            a *= c_alpha;
            
            f32 sr = (f32)((*src >> 16) & 0xFF);
            f32 sg = (f32)((*src >>  8) & 0xFF);
            f32 sb = (f32)((*src >>  0) & 0xFF);
            
            f32 dr = (f32)((*dest >> 16) & 0xFF);
            f32 dg = (f32)((*dest >>  8) & 0xFF);
            f32 db = (f32)((*dest >>  0) & 0xFF);
            
            f32 r = dr * (1.0f - a) + sr * a;
            f32 g = dg * (1.0f - a) + sg * a;
            f32 b = db * (1.0f - a) + sb * a;
            
            *dest = (
                     ((u32)(r + 0.5f) << 16) |
                     ((u32)(g + 0.5f) <<  8) |
                     ((u32)(b + 0.5f) <<  0));
            
            dest++;
            src++;
        }
        
        dest_row += buffer->pitch;
        src_row  -= bitmap->width;
    }
    
    
    return;
}

internal BitmapData
Game_debug_load_bmp(ThreadContext *thread, DEBUG_PlatformReadEntireFile *read_entire_file, u8 *file_name)
{
    BitmapData result = { 0 };
    
    DebugReadFileResult read_result = read_entire_file(thread, file_name);
    
    //ASSERT(read_result.contents);
    if(read_result.contents_size > 0)
    {
        BitmapHeader *header = (BitmapHeader *)read_result.contents;
        
        u32 *pixels = (u32 *)((u8 *)read_result.contents + header->bitmap_offset);
        
        result.pixels = pixels;
        result.width  = header->width;
        result.height = header->height;
        
        if(header->compression > 0)
        {
            u32 red_mask   = header->red_mask;
            u32 green_mask = header->green_mask;
            u32 blue_mask  = header->blue_mask;
            u32 alpha_mask = ~(red_mask | green_mask | blue_mask);
            
            bit_scan_result red_shift   = find_least_significant_set_bit(red_mask  );
            bit_scan_result green_shift = find_least_significant_set_bit(green_mask);
            bit_scan_result blue_shift  = find_least_significant_set_bit(blue_mask );
            bit_scan_result alpha_shift = find_least_significant_set_bit(alpha_mask);
            
            
            ASSERT(  red_shift.found);
            ASSERT(green_shift.found);
            ASSERT( blue_shift.found);
            ASSERT(alpha_shift.found);
            
            u32 *src_dest = pixels;
            
            for(    s32 y = 0; y < header->height; y++)
            {
                for(s32 x = 0; x < header->width; x++)
                {
                    u32 c = *src_dest;
                    *src_dest  = ((((c >> alpha_shift.index) & 0xFF) << 24)|
                                  (((c >>   red_shift.index) & 0xFF) << 16)|
                                  (((c >> green_shift.index) & 0xFF) <<  8)|
                                  (((c >>  blue_shift.index) & 0xFF) <<  0));
                    src_dest++;
                }
            }
        }
    }
    
    return result;
}

internal void
push_piece(EntityVisiblePieceGroup *group,
           BitmapData              *bitmap,
           V2  dim,
           V3  offset,
           V2  align,
           f32 entity_zc,
           V4  color)
{
    ASSERT(group->piece_count < ARRAYCOUNT(group->pieces));
    EntityVisiblePiece *piece = group->pieces + group->piece_count++;
    piece->bitmap = bitmap;
    
    V3_scale(group->game_state->meters_to_pixels, &offset);
    V2_sub((V2){offset.x, -offset.y}, align, &offset.xy);
    piece->offset = offset;
    
    piece->entity_zc = entity_zc;
    piece->color = color;
    piece->dim = dim;
    
    return;
} 


internal void
push_bitmap(EntityVisiblePieceGroup *group, BitmapData *bitmap,
            V3 offset, f32 entity_zc, V2 align, f32 alpha)
{
    offset.z *= -1;
    
    push_piece(group, bitmap, (V2){0,0}, offset, align, entity_zc, (V4){1.0f, 1.0f, 1.0f, alpha});
    
    return;
} 


//f32 r, f32 g, f32 b, f32 a
internal void
push_rect(EntityVisiblePieceGroup *group,
          V2 dim, V3 offset, f32 entity_zc,
          V4 color)
{
    V2_scale(group->game_state->meters_to_pixels, &dim);
    
    push_piece(group, NULLPTR, dim, offset, (V2){0.0f,0.0f}, entity_zc, color);
    
    return;
} 


internal void
draw_hitpoint(EntitySim *entity, EntityVisiblePieceGroup *piece_group)
{
    
    if(entity->hit_point_max >= 1)
    {
        V2 health_dim = { 0.2f, 0.2f};
        f32 spacing_x = 1.5f * health_dim.x;
        f32 first_x = 0.5f *  (entity->hit_point_max - 1) * spacing_x;
        // NOTE(MIGUEL): pixel space. y axis upside down
        V3 hit_position = {-0.5f * (entity->hit_point_max - 1) * spacing_x, 0.5f, 0};
        V2 hit_position_delta = {spacing_x, 0.0f};
        
        for(u32 health_index = 0; health_index < entity->hit_point_max;
            health_index++)
        {
            HitPoint *hitpoint = entity->hit_points + health_index;
            V4 color = { 1.0f, 0.0f, 0.0f, 1.0f };
            
            if(hitpoint->filled_amount == 0)
            {
                color = (V4){ 0.2f, 0.2f, 0.2f, 1.0f };
            }
            
            push_rect(piece_group, health_dim, hit_position, 1.0f, color);
            hit_position.x += hit_position_delta.x;
        }
    }
    
    return;
}

SGE_UPDATE(SGEUpdate)
{
    ASSERT((&input->controllers[0].terminator - &input->controllers[0].buttons[0]) ==
           (ARRAYCOUNT(input->controllers[0].buttons)));
    
    ASSERT(sizeof(GameState) <= sge_memory->permanent_storage_size);
    
    
    GameState *game_state = (GameState *)sge_memory->permanent_storage;
    if(!sge_memory->is_initialized)
    {   
        // NOTE(MIGUEL): Entity at slot 0 reserved as null
        Entity_create_entity_low(game_state, EntityType_null, World_null_position());
        
        //~ BITMAP LOADING
        game_state->back_drop  = Game_debug_load_bmp(thread,
                                                     sge_memory->debug_platform_read_entire_file,
                                                     "../res/images/test_background.bmp");
        game_state->shadow     = Game_debug_load_bmp(thread,
                                                     sge_memory->debug_platform_read_entire_file,
                                                     "../res/images/test_hero_shadow.bmp");
        
        game_state->tree       = Game_debug_load_bmp(thread,
                                                     sge_memory->debug_platform_read_entire_file,
                                                     "../res/images/tree00.bmp");
        
        game_state->stairwell  = Game_debug_load_bmp(thread,
                                                     sge_memory->debug_platform_read_entire_file,
                                                     "../res/images/rock01.bmp");
        
        game_state->sword      = Game_debug_load_bmp(thread,
                                                     sge_memory->debug_platform_read_entire_file,
                                                     "../res/images/rock03.bmp");
        
        DEBUG_PlatformReadEntireFile *read_file_callback = sge_memory->debug_platform_read_entire_file;
        PlayerBitmaps               *bitmap             = game_state->playerbitmaps;
        
        //RIGHT
        bitmap->head  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/shadow_right_head.bmp");
        bitmap->torso = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_right_torso.bmp");
        bitmap->cape  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_right_cape.bmp");
        bitmap->align = (V2){72, 182};
        bitmap++;
        
        //BACK
        bitmap->head  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/shadow_front_head.bmp");
        bitmap->torso = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_back_torso.bmp");
        bitmap->cape  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_back_cape.bmp");
        bitmap->align = (V2){72, 182};
        bitmap++;
        
        //LEFT
        bitmap->head  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/shadow_left_head.bmp");
        bitmap->torso = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_left_torso.bmp");
        bitmap->cape  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_left_cape.bmp");
        bitmap->align = (V2){72, 182};
        bitmap++;
        
        //FRONT
        bitmap->head  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/shadow_front_head.bmp");
        bitmap->torso = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_front_torso.bmp");
        bitmap->cape  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_front_cape.bmp");
        bitmap->align = (V2){72, 182};
        
        //~ WORLD GENERATION
        
        MemoryArena_init(&game_state->world_arena,
                         sge_memory->permanent_storage_size  - sizeof(GameState),
                         (u8 *)sge_memory->permanent_storage + sizeof(GameState));
        
        
        game_state->world = MEMORY_ARENA_PUSH_STRUCT(&game_state->world_arena, World); 
        World *world  = game_state->world;
        World_init(world, 1.4f); 
        
        s32 tile_side_in_pixels      = 60;
        game_state->meters_to_pixels = ((f32)tile_side_in_pixels /
                                        (f32)world->tile_dim_in_meters.x);
        
        u32 tiles_per_chunk_width  = 17;
        u32 tiles_per_chunk_height =  9;
        
        
        u32 random_number_index = 0;
        
        u16 screen_base_x = 0;
        u16 screen_base_y = 0;
        u16 screen_base_z = 0;
        
        
        u32 screenx = screen_base_x;
        u32 screeny = screen_base_y;
        u32 screenz = screen_base_z;
        
        b32 door_left   = 0;
        b32 door_top    = 0;
        b32 door_right  = 0;
        b32 door_bottom = 0;
        b32 door_up     = 0;
        b32 door_down   = 0;
        
        for(u32 screen_index = 0; screen_index < 2000; screen_index++)
        {
            ASSERT(random_number_index < ARRAYCOUNT(random_number_table));
            u32 random_choice;
            if(door_up || door_down)
            {
                random_choice = random_number_table[random_number_index++] % 2;
                
            }
            else
            {
                random_choice = random_number_table[random_number_index++] % 3;
            }
            
            b32 created_z_door = 0;
            
            if(random_choice == 2)
            {
                created_z_door = 1;
                if(screenz  == 0)
                {
                    door_up = 1;
                }
                else
                {
                    door_down = 1;
                    
                }
            }
            else if(random_choice == 1)
            {
                door_right = 1;
            }
            else
            {
                door_top = 1;
            }
            
            for(u32 tiley = 0 ; tiley < tiles_per_chunk_height; tiley++ )
            {
                for(u32 tilex = 0; tilex < tiles_per_chunk_width; tilex++ )
                {
                    u32 tile_abs_x = screenx * tiles_per_chunk_width  + tilex;
                    u32 tile_abs_y = screeny * tiles_per_chunk_height + tiley;
                    
                    u32 tile_value = 1;
                    // MAKE LEFT WALL AND DOOR
                    if((tilex == 0) && (!door_left || (tiley != (tiles_per_chunk_height / 2))))
                    {
                        tile_value = 2;
                    }
                    
                    // MAKE RIGHT WALL AND DOOR
                    if((tilex == (tiles_per_chunk_width - 1)) && (!door_right || (tiley != (tiles_per_chunk_height / 2))))
                    {
                        tile_value = 2;
                    }
                    
                    if ((tiley == 0) && (!door_bottom || (tilex != (tiles_per_chunk_width / 2))))
                    {
                        tile_value = 2;
                    }
                    
                    if((tiley == (tiles_per_chunk_height - 1)) && (!door_top || (tilex != (tiles_per_chunk_width / 2))))
                    {
                        tile_value = 2;
                    }
                    
                    if((tilex == 10) && (tiley == 6))
                    {
                        if(door_up)
                        {
                            tile_value = 3;
                        }
                        if(door_down)
                        {
                            tile_value = 4;
                        }
                    }
                    
                    //debug
                    //tile_value = 1;
                    
                    if(tile_value == 2) 
                    {
                        // WALL
                        Game_add_wall(game_state, tile_abs_x, tile_abs_y, screenz);
                        
                    }
                }
            }
            
            door_left   = door_right;
            door_bottom = door_top;
            
            if(created_z_door)
            {
                door_up   = !door_up;
                door_down = !door_down;
                
            }
            else
            {
                door_up   = 0;
                door_down = 0;
            }
            
            door_right    = 0;
            door_top      = 0;
            
            if(random_choice == 2)
            {
                if(screenz == screen_base_z)
                {
                    screenz = screen_base_z + 1;
                }
                else
                {
                    screenz = screen_base_z;
                }
            }
            else if(random_choice == 1)
            {
                screenx += 1;
            }
            else
            {
                screeny += 1;
            }
        }
        
        WorldCoord initial_camera_position = { 0 };
        
        u32 camera_tile_x = screen_base_x * tiles_per_chunk_width  + 17 / 2;
        u32 camera_tile_y = screen_base_y * tiles_per_chunk_height +  9 / 2;
        u32 camera_tile_z = screen_base_z;
        
        initial_camera_position = World_worldcoord_from_tilecoord(world,
                                                                  camera_tile_x,
                                                                  camera_tile_y,
                                                                  camera_tile_z);
        
        game_state->camera_position = initial_camera_position;
        
        Game_add_hostile(game_state,
                         camera_tile_x + 2,
                         camera_tile_y + 2,
                         camera_tile_z);
        
        for(f32 friendly_index = 0; friendly_index < 20; friendly_index++)
        {
            s32 friendly_offset_x = (random_number_table[random_number_index++] % 10) - 7;
            s32 friendly_offset_y = (random_number_table[random_number_index++] % 10) - 3;
            
            if((friendly_offset_x != 0) &&
               (friendly_offset_y != 0))
            {
                if(friendly_index < (20.0f / 2.0f))
                {
                    Game_add_friendly(game_state,
                                      camera_tile_x + friendly_offset_x,
                                      camera_tile_y + friendly_offset_y,
                                      camera_tile_z);
                }
                else
                {
                    Game_add_hostile(game_state,
                                     camera_tile_x + friendly_offset_x,
                                     camera_tile_y + friendly_offset_y,
                                     camera_tile_z);
                }
            }
        }
        
        sge_memory->is_initialized = 1; 
    }
    
    World *world  = game_state->world;
    f32 meters_to_pixels = game_state->meters_to_pixels;
    
    // ************************************************
    // INPUT RESPONSE
    //*************************************************
    for(u32 controller_index = 0; controller_index < 5; controller_index++)
    {
        GameControllerInput *controller = &input->controllers[controller_index];
        
        ControlledPlayer *player = game_state->controlled_players + controller_index;
        
        if(player->entity_index == 0)
        {
            if(controller->button_start.ended_down)
            { 
                memset(player, 0, sizeof(player));
                player->entity_index = Game_add_player(game_state, controller_index).index_low; 
            }
        }
        else
        {
            player->delta_z = 0.0f;
            player->delta_sword  = (V2){ 0.0f, 0.0f };
            player->acceleration = (V2){ 0.0f, 0.0f };
            
            if(controller->is_analog)
            {
                // NOTE(MIGUEL): use analog mov tunning
                player->acceleration = (V2){ controller->stick_avg_x, controller->stick_avg_y };
                
            }
            else
            {
                if(controller->button_y.ended_down)
                {
                    // up
                    player->acceleration.y = 1.0f;
                }
                if(controller->button_a.ended_down)
                {
                    // down
                    player->acceleration.y = -1.0f;
                }
                
                if(controller->button_x.ended_down)
                {
                    // left
                    player->acceleration.x = -1.0f;
                }
                
                if(controller->button_b.ended_down)
                {
                    // right
                    player->acceleration.x = 1.0f;
                }
            }
            
            player->delta_sword = (V2){ 0.0f, 0.0f };
            if(controller->button_start.ended_down)
            {
                player->delta_z = 3.0f;
            }
            
            if(controller->action_up.ended_down)
            {
                player->delta_sword.y =  1.0f;
            }
            if(controller->action_down.ended_down)
            {
                player->delta_sword.y = -1.0f;
            }
            if(controller->action_left.ended_down)
            {
                player->delta_sword.x = -1.0f;
            }
            
            if(controller->action_right.ended_down)
            {
                player->delta_sword.x =  1.0f;
            }
            
            if(controller->shoulder_left.ended_down)
            {
                game_state->camera_following_entity_index = player->entity_index;
            }
        }
        
    } /// END OF INPUT LOOP
    
    V3 view_tile_span = V3_init_3f32(17.0f, 9.0f, 1.0f);
    V3_scale(3.0f, &view_tile_span); // NOTE(MIGUEL): broaden view span for simspace
    V3_scale(world->tile_dim_in_meters.x, &view_tile_span);
    V3_scale(0.5f, &view_tile_span);
    
    RectV3 high_frequency_bounds = RectV3_center_half_dim(V3_init_uniform(0.0f),
                                                          view_tile_span);
    MemoryArena sim_arena = { 0 };
    
    MemoryArena_init(&sim_arena,
                     sge_memory->transient_storage_size,
                     (u8 *)sge_memory->transient_storage);
    
    SimRegion *sim_region = SimRegion_begin_sim(&sim_arena,
                                                game_state,
                                                game_state->world,
                                                game_state->camera_position,
                                                high_frequency_bounds,
                                                back_buffer,
                                                input->delta_t);
    
    ASSERT(validate_sim_entities(sim_region));
    
    
    /// debug purp background clear
    Game_draw_rectangle(back_buffer,
                        (V2){0.0f, 0.0f} ,
                        (V2){(f32)back_buffer->width,(f32)back_buffer->height},
                        (V4){0.4f, 0.8f, 1.0f, 0.5},
                        0);
    
    
    game_state->clock += input->delta_t * 0.1f;
    // Game_render_weird_shit(back_buffer, 10, 40, game_state->clock);
    
    //~DRAW ALL HIGH ENTITIES
    
    V2 screen_center = 
    {
        0.5f * (f32)back_buffer->width,
        0.5f * (f32)back_buffer->height,
    };
    
    EntityVisiblePieceGroup piece_group = { 0 };
    piece_group.game_state = game_state;
    
    EntitySim *entity = sim_region->entities;
    for(u32 entity_index = 0;
        entity_index < sim_region->entity_count;
        entity_index++, entity++)
    {
        
        if(entity->type == EntityType_sword)
        {
            int dbgint = 13;
            
            if(!Entity_is_entity_sim_flags_set(entity, EntitySimFlag_nonspatial))
            {
                dbgint = 1408;
            }
        }
        
        if(entity->updatable)
        {
            piece_group.piece_count = 0;
            f32 delta_time = input->delta_t;
            
            // JUMP CODE
            f32 shadow_alpha = 1.0f - 0.5f * entity->position.z;
            
            if(shadow_alpha < 0.0f)
            {
                shadow_alpha = 0.0f;
            }
            // END OF JUMP CODE
            
            MoveSpec movespec = default_movespec();
            V3 acceleration = { 0 };
            
            PlayerBitmaps *playerbitmaps = &game_state->playerbitmaps[entity->facing_direction];
            switch(entity->type )
            {
                case EntityType_player:
                {
                    for(u32 player_index = 0;
                        player_index < ARRAYCOUNT(game_state->controlled_players);
                        player_index++)
                    {
                        ControlledPlayer *player = game_state->controlled_players + player_index;
                        
                        // NOTE(MIGUEL): these index naming differences are fucked!!!
                        //                what array do they refer to ????
                        if(entity->index_storage == player->entity_index)
                        {
                            if(player->delta_z != 0.0f)
                            {
                                entity->velocity.z += player->delta_z;
                            }
                            
                            movespec.unitmaxaccel = 1;
                            movespec.speed = 80.0f;
                            movespec.drag  = 4.0f;
                            acceleration   = V3_init_v2(player->acceleration, 0.0f);
                            
                            if((player->delta_sword.x != 0.0f) ||
                               (player->delta_sword.y != 0.0f))
                            {
                                EntitySim *sword = entity->sword.ptr;
                                
                                if(sword && Entity_is_entity_sim_flags_set(sword, EntitySimFlag_nonspatial))
                                {
                                    sword->distance_limit = 5.0f; //UNITS: meters
                                    
                                    V3 sword_velocity = V3_init_v2(player->delta_sword, 0.0f);
                                    V3_scale(5.0f, &sword_velocity);
                                    V3_add(entity->velocity, sword_velocity, &sword_velocity);
                                    
                                    Entity_make_spatial(sword,
                                                        entity->position,
                                                        sword_velocity);
                                    
                                    add_collision_rule(game_state, sword->index_storage, entity->index_storage, 0);
                                }
                            }
                        }
                    }
                    
                    
                    push_bitmap(&piece_group,
                                &game_state->shadow,
                                V3_init_uniform( 0.0f),
                                0.0f,
                                playerbitmaps->align,
                                shadow_alpha);
                    
                    push_bitmap(&piece_group,
                                &playerbitmaps->torso,
                                V3_init_uniform( 0.0f),
                                0.0f,
                                playerbitmaps->align,
                                1.0f);
                    
                    push_bitmap(&piece_group,
                                &playerbitmaps->cape,
                                V3_init_uniform( 0.0f),
                                0.0f,
                                playerbitmaps->align,
                                1.0f); 
                    
                    push_bitmap(&piece_group,
                                &playerbitmaps->head,
                                V3_init_uniform( 0.0f),
                                0.0f,
                                playerbitmaps->align,
                                1.0f);
                    
                    draw_hitpoint(entity, &piece_group);
                } break;
                
                case EntityType_wall:
                {
                    push_bitmap(&piece_group,
                                &game_state->tree,
                                (V3){0, 0, 0},
                                0.0f,
                                (V2){40, 80},
                                1.0f);
                    
                } break;
                
                case EntityType_sword:
                {
                    // NOTE(MIGUEL): sword accerlerates even though theres no acceleration 
                    // TODO(MIGUEL): fix it
                    movespec.unitmaxaccel = 0;
                    movespec.speed = 0.0f;
                    movespec.drag  = 0.0f;
                    
                    if(entity->distance_limit == 0.0f)
                    {
                        clear_collision_all_rules(game_state, entity->index_storage);
                        Entity_make_nonspatial(entity);
                        
                    }
                    
                    push_bitmap(&piece_group,
                                &game_state->shadow,
                                (V3){0, 0, 0},
                                0.0f,
                                (V2){72, 182},
                                1.0f);
                    
                    push_bitmap(&piece_group,
                                &game_state->sword,
                                (V3){0, 0, 0},
                                0.0f,
                                (V2){29, 10},
                                1.0f);
                } break;
                
                case EntityType_friendly: 
                {
                    EntitySim *closest_player = NULLPTR;
                    f32    player_search_diametersq = square(30.0f);
                    
                    EntitySim *test_entity = sim_region->entities;
                    for(u32 test_entity_index = 0;
                        test_entity_index < sim_region->entity_count;
                        test_entity++, test_entity_index++)
                    {
                        if(test_entity->type == EntityType_player)
                        {
                            //follow
                            V2 position_delta;
                            V2_sub((V2){test_entity->position.x, test_entity->position.y},
                                   (V2){     entity->position.x,      entity->position.y},
                                   &position_delta);
                            
                            f32 test_dsq = V2_length_sq(position_delta);
                            
                            if(player_search_diametersq > test_dsq)
                            {
                                closest_player = test_entity;
                                player_search_diametersq = test_dsq;
                            }
                            
                        }
                        
                    }
                    
                    if(closest_player && (player_search_diametersq > square(5.0f)))
                    {
                        f32 coef = 0.5f;
                        f32 one_over_length = coef / square_root(player_search_diametersq);
                        
                        V2 scratch_delta = { 0 };
                        
                        V2_sub((V2){closest_player->position.x, closest_player->position.y},
                               (V2){        entity->position.x,         entity->position.y},
                               &scratch_delta);
                        
                        V2_scale(one_over_length, &scratch_delta);
                        
                        acceleration = V3_init_v2(scratch_delta, 0.0f);
                    }
                    
                    movespec.unitmaxaccel = 1;
                    movespec.speed = 80.0f;
                    movespec.drag  = 8.0f;
                    
                    
                    entity->bob_t += delta_time;
                    
                    if(entity->bob_t > (2.0f * PI_32BIT))
                    {
                        entity->bob_t -= 2.0f * PI_32BIT;
                    }
                    
                    f32 bobsin = math_sin(2.0f * entity->bob_t);
                    
                    push_bitmap(&piece_group,
                                &game_state->shadow,
                                (V3){0, 0, 0},
                                0.0f,
                                playerbitmaps->align,
                                (0.5f * shadow_alpha) - (0.4f * bobsin));
                    
                    push_bitmap(&piece_group,
                                &playerbitmaps->head,
                                (V3){0, 0, 1.0f * bobsin},
                                0.0f,
                                playerbitmaps->align,
                                1.0f);
                    
                } break;
                
                case EntityType_hostile: 
                {
                    EntitySim *closest_player = NULLPTR;
                    f32    player_search_diametersq = square(30.0f);
                    
                    EntitySim *test_entity = sim_region->entities;
                    for(u32 test_entity_index = 0;
                        test_entity_index < sim_region->entity_count;
                        test_entity++, test_entity_index++)
                    {
                        if(test_entity->type == EntityType_player)
                        {
                            //follow
                            V2 position_delta;
                            V2_sub((V2){test_entity->position.x, test_entity->position.y},
                                   (V2){     entity->position.x,      entity->position.y},
                                   &position_delta);
                            
                            f32 test_dsq = V2_length_sq(position_delta);
                            
                            if(player_search_diametersq > test_dsq)
                            {
                                closest_player = test_entity;
                                player_search_diametersq = test_dsq;
                            }
                            
                        }
                        
                    }
                    
                    if(closest_player && (player_search_diametersq > square(0.0f)))
                    {
                        f32 coef = 0.5f;
                        f32 one_over_length = coef / square_root(player_search_diametersq);
                        
                        V2 scratch_delta = { 0 };
                        
                        V2_sub((V2){closest_player->position.x, closest_player->position.y},
                               (V2){        entity->position.x,         entity->position.y},
                               &scratch_delta);
                        
                        V2_scale(one_over_length, &scratch_delta);
                        
                        acceleration = V3_init_v2(scratch_delta, 0.0f);
                    }
                    
                    movespec.unitmaxaccel = 1;
                    movespec.speed = 90.0f;
                    movespec.drag  = 4.0f;
                    
                    
                    PlayerBitmaps *playerbitmaps = &game_state->playerbitmaps[entity->facing_direction];
                    
                    push_bitmap(&piece_group,
                                &game_state->shadow,
                                (V3){0, 0, 0},
                                0.0f,
                                playerbitmaps->align,
                                shadow_alpha);
                    
                    push_bitmap(&piece_group,
                                &playerbitmaps->torso,
                                (V3){0, 0, 0},
                                0.0f,
                                playerbitmaps->align,
                                1.0f);
                    
                    draw_hitpoint(entity, &piece_group);
                } break;
                
                default:
                {
                    INVALID_CODE_PATH;
                } break;
            }
            
            if(!Entity_is_entity_sim_flags_set(entity, EntitySimFlag_nonspatial))
            {
                SimRegion_move_entity(sim_region,
                                      game_state,
                                      entity,
                                      &movespec,
                                      delta_time,
                                      acceleration);
            }
            
            f32 entity_ground_point_x = screen_center.x + meters_to_pixels * entity->position.x;
            f32 entity_ground_point_y = screen_center.y - meters_to_pixels * entity->position.y;
            f32 entity_z = -meters_to_pixels * entity->position.z;
            
            for(u32 piece_index = 0; piece_index < piece_group.piece_count; piece_index++)
            {
                EntityVisiblePiece *piece = piece_group.pieces + piece_index;
                
                V2 center =
                {
                    entity_ground_point_x + piece->offset.x,
                    entity_ground_point_y + piece->offset.y + piece->offset.z + piece->entity_zc * entity_z
                }; 
                
                if(piece->bitmap)
                {
                    
                    Game_draw_bitmap(back_buffer,
                                     piece->bitmap,
                                     center.x,
                                     center.y,
                                     piece->color.a);
                }
                else
                {
                    V2 halfdim = piece->dim ;
                    V2_scale(0.5f, &halfdim);
                    RectV2 piece_rect = { 0 };
                    V2_sub(center, halfdim, &piece_rect.min);
                    V2_add(center, halfdim, &piece_rect.max);
                    
                    Game_draw_rectangle(back_buffer,
                                        piece_rect.min,
                                        piece_rect.max,
                                        piece->color,
                                        1);
                }
            }
        }
    }
    
    Game_draw_array(game_state,
                    back_buffer,
                    sim_region);
    
    
    Game_draw_mini_map(game_state,
                       back_buffer,
                       input,
                       sim_region,
                       4,
                       (V2){120.0f, 80.0f});
    
    SimRegion_end_sim  (sim_region, game_state);
    
    return;
}


internal void Game_update_sound_buffer(GameState *game_state, GameSoundOutputBuffer *sound_buffer, u32 tone_hz)
{
    local_persist f32 t_sin;
    
    u16 tone_volume = 100;
    u16 wave_period = (u16)(sound_buffer->samples_per_second / tone_hz);
    
    u16 *sample_out = sound_buffer->samples;
    
    for(u32 sample_index = 0; sample_index < sound_buffer->sample_count; sample_index++)
    {
#if 0
        f32 sine_value   = sin(t_sin);
        u16 sample_value = (u16)(sine_value * tone_volume);
#else
#endif
        u16 sample_value = 0;
        
        *sample_out++ = sample_value;
        *sample_out++ = sample_value;
        
#if 0
        t_sin += 2.0f * PI_32BIT * 1.0f / (f32)wave_period;
#endif
    }
    
    return;
} 


internal void
Game_draw_rectangle(GameBackBuffer *buffer,
                    V2 min, V2 max,
                    V4 color_vec, b32 grid)
{
    f32 r = color_vec.r; 
    f32 g = color_vec.g;
    f32 b = color_vec.b;
    f32 a = color_vec.a;
    
    /// rounding / ruling
    s32 min_x = round_f32_to_s32(min.x);
    s32 min_y = round_f32_to_s32(min.y);
    
    s32 max_x = round_f32_to_s32(max.x);
    s32 max_y = round_f32_to_s32(max.y);
    
    /// clipping
    // NOTE(MIGUEL): will right to but not including the final row 
    if(min_x < 0)
    {
        min_x = 0;
    }
    if(min_y < 0)
    {
        min_y = 0;
    }
    if(max_x > buffer->width)
    {
        max_x = buffer->width;
    }
    if(max_y > buffer->height)
    {
        max_y = buffer->height;
    }
    /*
    /// bit pattern 0x AA RR GG BB
    u32 color = ((round_f32_to_u32(r * 255.0f) << 16) |
                 (round_f32_to_u32(g * 255.0f) <<  8) |
                 (round_f32_to_u32(b * 255.0f) <<  0));
    */
    
    /// drawing
    u8 *line =  ((u8 *)  (buffer->data)            + 
                 (min_x * buffer->bytes_per_pixel) +
                 (min_y * buffer->pitch));
    
    for(s32 y = min_y; y < max_y; y++ )
    {
        u32 *pixel = (u32 *)line;
        
        for(s32 x = min_x; x < max_x; x++)
        {
            u32 fragment      = 0;
            f32 alpha_channel = a;
            f32 red_channel   = r;
            f32 green_channel = g;
            f32 blue_channel  = b;
            
            u8 frag_red   = (u8)(255.0f *   red_channel);
            u8 frag_green = (u8)(255.0f * green_channel);
            u8 frag_blue  = (u8)(255.0f *  blue_channel);
            
            u8 pixel_red   = (u8)((*pixel & 0x00FF0000) >> 16);
            u8 pixel_green = (u8)((*pixel & 0x0000FF00) >>  8);
            u8 pixel_blue  = (u8)((*pixel & 0x000000FF) >>  0);
            
            fragment = ((u32)                                        ((u32)(    255.0f * alpha_channel)  << 24) |
                        ((u32)(pixel_red   * (1.0f - alpha_channel) + (u32)(  frag_red * alpha_channel)) << 16) |
                        ((u32)(pixel_green * (1.0f - alpha_channel) + (u32)(frag_green * alpha_channel)) <<  8) |
                        ((u32)(pixel_blue  * (1.0f - alpha_channel) + (u32)( frag_blue * alpha_channel)) <<  0));
            
            
            
            *pixel = fragment;
            
            //debug
            if(grid)
            {
                if(x > (max_x - 2))
                {
                    *pixel = 0xAAaaaaaa;
                }
                if(y > (max_y - 2))
                {
                    *pixel = 0xAAaaaaaa;
                }
            }
            
            pixel++;
        }
        
        line += buffer->pitch;
    }
    
    return;
}

#if 0
internal void Game_render_weird_shit(GameBackBuffer *buffer, s32 x_offset, s32 y_offset, f32 time)
{
    u8 *line = ((u8*)buffer->data);
    
    for(s32 y = 0; y < buffer->height; y++) 
    {
        
        u32 *pixel  = (u32 *)line;
        
        for(s32 x = 0; x < buffer->width; x++) 
        {
            u32 fragment = 2;
            
            f32 alpha_channel = 0.5f;
            f32 red_channel   = 0.0f;
            f32 green_channel = 0.0f;
            f32 blue_channel  = math_cos(time );
            
#if 1
            f32 amplitude = 0.0001f;
            s32 crest = 20;
            local_persist u64 move = 0;
            move += 5000;
            RectV2 bounds = { {0 ,0}, {(f32)buffer->width, (f32)buffer->height} };
            
            //printf("N-Y: %f | X-CMP: %f  \n", y_comp, x_comp);
            if(((floor_f32_to_s32(math_cos(time * 2.0f * ((move + x) * 0.04f) * amplitude) * (f32)buffer->height)) == y))
            {
                //alpha_channel = 220;
                red_channel   = math_cos(time);
                green_channel += 0.9f;
                blue_channel  += 1.0f;
            } 
            
            
            if((floor_f32_to_s32(math_sin(time * 1.0f * (x * 0.001f)) * (f32)buffer->height) == y))
            {
                alpha_channel = 1.0f;
                red_channel   = 0.4f;
                green_channel = 0.3f;
                blue_channel  = 0.9f;
                
            }
#endif
            //ALPHA BLENDING
            u8 frag_red   = (u8)(255.0f *   red_channel);
            u8 frag_green = (u8)(255.0f * green_channel);
            u8 frag_blue  = (u8)(255.0f *  blue_channel);
            
            u8 pixel_red   = (u8)((*pixel & 0x00FF0000) >> 16);
            u8 pixel_green = (u8)((*pixel & 0x0000FF00) >>  8);
            u8 pixel_blue  = (u8)((*pixel & 0x000000FF) >>  0);
            
            fragment = ((u32)                                        ((u32)(    255.0f * alpha_channel)  << 24) |
                        ((u32)(pixel_red   * (1.0f - alpha_channel) + (u32)(  frag_red * alpha_channel)) << 16) |
                        ((u32)(pixel_green * (1.0f - alpha_channel) + (u32)(frag_green * alpha_channel)) <<  8) |
                        ((u32)(pixel_blue  * (1.0f - alpha_channel) + (u32)( frag_blue * alpha_channel)) <<  0));
            
            *pixel = fragment;
            
            pixel++;
        }
        
        line += buffer->pitch;
    }
    
    return;
}
#endif
