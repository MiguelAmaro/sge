#include "sge.h"
#include "sge_random.h"
#include "sge_math.h"
#include "stdio.h" // for debug tilemap out to file
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
inline V2 Entity_get_camera_space_position(GameState *game_state, EntityLow *entity_low)
{
    
    WorldDifference diff = World_subtract(game_state->world,
                                          &entity_low->position,
                                          &game_state->camera_position);
    V2 result = diff.dxy;
    
    return result;
}

internal EntityHigh *
Entity_set_frequency_high_(GameState *game_state, EntityLow *entity_low, u32 index_low, V2 camara_space_position)
{
    EntityHigh *entity_high = NULLPTR;
    
    ASSERT(entity_low->index_high == 0 );
    
    if(entity_low->index_high == 0)
    {
        if(game_state->entity_count_high < ARRAYCOUNT(game_state->entities_high_))
        {   
            
            u32         index_high  =  game_state->entity_count_high++;
            EntityHigh *entity_high = &game_state->entities_high_[index_high];
            
            entity_high->position   = camara_space_position;
            entity_high->velocity   = (V2){0, 0};
            entity_high->tile_abs_z = entity_low->position.chunk_z;
            entity_high->facing_direction = 0;
            entity_high->index_low  = index_low;
            
            entity_low->index_high  = index_high;
        }
        else
        {
            INVALID_CODE_PATH;
        }
    }
    
    return entity_high;
}

internal EntityHigh *
Entity_set_frequency_high(GameState *game_state, u32 index_low)
{
    EntityHigh *entity_high = NULLPTR;
    EntityLow  *entity_low = game_state->entities_low + index_low;
    
    if(entity_low->index_high)
    {
        entity_high = game_state->entities_high_ + entity_low->index_high;
    }
    else
    {
        V2 camara_space_position = Entity_get_camera_space_position(game_state, entity_low);
        
        EntityHigh *high_entity  = Entity_set_frequency_high_(game_state,
                                                              entity_low,
                                                              index_low,
                                                              camara_space_position);
    }
    return entity_high;
}

internal void
Entity_set_frequency_low(GameState *game_state, u32 low_index)
{
    EntityLow *low_entity = &game_state->entities_low[low_index];
    
    u32 index_high = low_entity->index_high;
    
    if(index_high)
    {
        u32 last_index_high = game_state->entity_count_high - 1;
        
        if(index_high != last_index_high)
        {
            EntityHigh *last_entity    = game_state->entities_high_ + last_index_high;
            EntityHigh *deleted_entity = game_state->entities_high_ + index_high;
            
            *deleted_entity = *last_entity;
            game_state->entities_low[last_entity->index_low].index_high = index_high;
        }
        
        game_state->entity_count_high--;
        low_entity->index_high = 0;
    }
    
    return;
}

inline b32
Entity_validate_entity_pairs(GameState *game_state)
{
    b32 valid = 1;
    
    for(u32 index_high = 1; index_high < game_state->entity_count_high; index_high++)
    {
        EntityHigh *entity_high = game_state->entities_high_ +  index_high;
        valid = valid &&
            (game_state->entities_low[entity_high->index_low].index_high == index_high);
    }
    
    return valid;
}

internal void
Entity_check_frequency_by_area(GameState *game_state, V2 offset, RectV2 high_frequency_bounds)
{
    for(u32 index_high = 1;
        index_high < game_state->entity_count_high;)
    {
        EntityHigh *entity_high = game_state->entities_high_ +  index_high;
        EntityLow  *entity_low  = game_state->entities_low   + entity_high->index_low;
        
        V2_add(entity_high->position, offset, &entity_high->position);
        
        if(RectV2_is_in_rect(high_frequency_bounds, entity_high->position))
        {
            index_high++;
        }
        else
        {
            if(entity_low->type != EntityType_player)
            {
                ASSERT(game_state->entities_low[entity_high->index_low].index_high == index_high);
                Entity_set_frequency_low(game_state, entity_high->index_low);
            }
            else
            {
                index_high++;
            }
        }
        
    }
    
    return;
}

typedef struct CreateEntityLowResult CreateEntityLowResult;
struct CreateEntityLowResult
{
    EntityLow *entity_low;
    u32         index_low;
};

internal CreateEntityLowResult
Entity_create_entity_low(GameState *game_state, EntityType type, WorldCoord *position)
{
    ASSERT(game_state->entity_count_low < ARRAYCOUNT(game_state->entities_low));
    
    u32 index_low = game_state->entity_count_low++;
    
    EntityLow *entity_low = game_state->entities_low + index_low;
    
    memset(entity_low, 0, sizeof(EntityLow));
    entity_low->type     = type;
    
    if(position)
    {
        entity_low->position = *position;
        World_change_entity_location(game_state->world,
                                     index_low,
                                     0,
                                     position,
                                     &game_state->world_arena);
    }
    
    CreateEntityLowResult result;
    result.index_low  = index_low;
    result.entity_low = entity_low;
    
    return result;
}

inline Entity
Entity_get_entity_high(GameState *game_state,  u32 index_low)
{
    Entity result = { 0 };
    
    if((index_low > 0) && (index_low < game_state->entity_count_low))
    {
        result.low_index = index_low;
        result.low       = game_state->entities_low   + index_low;
        result.high      = Entity_set_frequency_high(game_state, index_low);
    }
    
    return result;
}


inline EntityLow *
Entity_get_entity_low(GameState *game_state,  u32 index_low)
{
    EntityLow *result = NULLPTR;
    
    if((index_low > 0) && (index_low < game_state->entity_count_low))
    {
        result = game_state->entities_low + index_low;
    }
    
    return result;
}

internal CreateEntityLowResult
Game_add_player(GameState *game_state, u32 player_number)
{
    WorldCoord position = game_state->camera_position;
    CreateEntityLowResult result = Entity_create_entity_low(game_state, EntityType_player, &position);
    
    result.entity_low->height          = 0.5f; //           UNITS: meters
    result.entity_low->width           = 1.0f; //           UNITS: meters
    result.entity_low->collides        = 1;
    
    if(game_state->camera_following_entity_index == 0)
    {
        game_state->camera_following_entity_index = result.index_low;
    }
    
    return result;
}


internal CreateEntityLowResult
Game_add_friendly(GameState *game_state, u32 tile_abs_x, u32 tile_abs_y, u32 tile_abs_z)
{
    WorldCoord position = World_tilecoord_to_chunkcoord(game_state->world,
                                                        tile_abs_x,
                                                        tile_abs_y,
                                                        tile_abs_z);
    
    CreateEntityLowResult result = Entity_create_entity_low(game_state, EntityType_friendly, &position);
    
    result.entity_low->height          = 0.5f; //           UNITS: meters
    result.entity_low->width           = 1.0f; //           UNITS: meters
    result.entity_low->collides        = 0;
    
    return result;
}


internal CreateEntityLowResult
Game_add_hostile(GameState *game_state, u32 tile_abs_x, u32 tile_abs_y, u32 tile_abs_z)
{
    WorldCoord position = World_tilecoord_to_chunkcoord(game_state->world,
                                                        tile_abs_x,
                                                        tile_abs_y,
                                                        tile_abs_z);
    
    CreateEntityLowResult result = Entity_create_entity_low(game_state, EntityType_hostile, &position);
    
    result.entity_low->height          = 0.5f; //           UNITS: meters
    result.entity_low->width           = 1.0f; //           UNITS: meters
    result.entity_low->collides        = 1;
    
    return result;
}

internal CreateEntityLowResult
Game_add_wall(GameState *game_state, u32 tile_abs_x, u32 tile_abs_y, u32 tile_abs_z)
{
    
    WorldCoord position = World_tilecoord_to_chunkcoord(game_state->world,
                                                        tile_abs_x,
                                                        tile_abs_y,
                                                        tile_abs_z);
    
    CreateEntityLowResult result = Entity_create_entity_low(game_state, EntityType_wall, &position);
    
    result.entity_low->height   = 1.4f; //UNITS: meters
    result.entity_low->width    = 1.4f; //UNITS: meters
    result.entity_low->collides = 1;
    
    return result;
}

internal void
Game_draw_mini_map(GameState *game_state,
                   game_back_buffer *back_buffer,
                   s32 tile_side_in_pixels,
                   s32 camera_range_x,
                   s32 camera_range_y,
                   s32 x, s32 y)
{
    World *world = game_state->world;
    
    f32 meters_to_pixels    =  ((f32)tile_side_in_pixels / (f32)world->side_in_meters_tile);
    
    f32 screen_center_x = 0.5f * (f32)back_buffer->width;
    f32 screen_center_y = 0.5f * (f32)back_buffer->height;
    
    s32 view_scale = 1;
    camera_range_x = 40 * view_scale;
    camera_range_y = 30 * view_scale;
    
    // NOTE(MIGUEL): camera_following_entity_index is probly wrong or brokne.. i dont understand the system
    EntityLow *camera_following_entity = Entity_get_entity_low(game_state, game_state->camera_following_entity_index);
    
    for(s32 rel_column = -camera_range_x; rel_column < camera_range_x; rel_column++)
    {
        for(s32 rel_row = -camera_range_y; rel_row < camera_range_y; rel_row++)
        {
            if(camera_following_entity)
            {
                s32 column = camera_following_entity->position.chunk_x + rel_column;
                s32 row    = camera_following_entity->position.chunk_y + rel_row;
                
                u32 tileid = 1;//Tile_get_tile_value(tilemap, column, row, camera_following_entity->position.chunk_z );
                //if(){} // TODO(MIGUEL): find way to render walls in the minimap
                for(u32 index_high = 0; index_high < game_state->entity_count_high; index_high++)
                {
                    EntityHigh *high = game_state->entities_high_ + index_high;
                    EntityLow  *low  = game_state->entities_low   + high->index_low;
                    
                    if(low->position.chunk_x == column &&
                       low->position.chunk_y == row)
                    {
                        tileid = 2;
                        break;
                    }
                }
                
                if(tileid > 0)
                {
                    f32 gray = 0.2f;
                    
                    //circle drawing shinanegans
                    f32 r = 40.0f;
                    f32 h = 4.0f;
                    f32 k = 3.0f;
                    
                    f32 center_x = (screen_center_x -
                                    (meters_to_pixels * camera_following_entity->position.rel_.x) +
                                    ((f32)rel_column * tile_side_in_pixels));
                    
                    f32 center_y = (screen_center_y +
                                    (meters_to_pixels * camera_following_entity->position.rel_.y) -
                                    ((f32)rel_row  * tile_side_in_pixels));
                    
                    f32 min_x = x + center_x - 0.5f * tile_side_in_pixels;
                    f32 min_y = y + center_y - 0.5f * tile_side_in_pixels;
                    f32 max_x = x + center_x + 0.5f * tile_side_in_pixels;
                    f32 max_y = y + center_y + 0.5f * tile_side_in_pixels;
                    
                    // FLOOR & WALLS
                    if(((s32)column == ceiling_f32_to_s32(((-sqrtf(r * 2 - powf(row - h, 2))) + k))) ||
                       ((s32)column == ceiling_f32_to_s32(  (sqrtf(r * 2 - powf(row - h, 2)) + k))))
                    {
                        gray = 0.4f;
                    }
                    
                    if(tileid == 1)
                    {
                        // FLOOR
                        Game_draw_rectangle(back_buffer,
                                            (V2){min_x,min_y},
                                            (V2){max_x,max_y},
                                            gray + 0.1f, gray + 0.5f, gray,
                                            1);
                        
                    }
                    else if(tileid == 2) 
                    {
                        // WALL
                        gray = 0.8f;
                        
                        Game_draw_rectangle(back_buffer,
                                            (V2){min_x,min_y},
                                            (V2){max_x,max_y},
                                            gray, gray, gray,
                                            1);
                    }
                    else if(tileid == 3)
                    {
                        // LADDER 
                        gray = 0.0f;
                        
                        Game_draw_rectangle(back_buffer,
                                            (V2){min_x,min_y},
                                            (V2){max_x,max_y},
                                            gray, gray, gray,
                                            0);
                    }
                    /// DEBUG
                    
                    V2 tile_span_in_meters = (V2){ (17 * 3) / 2 , (9 * 3) / 2};
                    V2 camera_point        = 
                    { 
                        (f32)game_state->camera_position.chunk_x,
                        (f32)game_state->camera_position.chunk_y 
                    };
                    
                    RectV2 high_frequency_bounds = RectV2_center_half_dim(game_state->camera_position.rel_,
                                                                          tile_span_in_meters);
                    
                    V2_add(camera_point, high_frequency_bounds.min, &high_frequency_bounds.min);
                    V2_add(camera_point, high_frequency_bounds.max, &high_frequency_bounds.max);
                    
                    tile_span_in_meters  = (V2){ 17/ 2 , 9/2};
                    RectV2 camera_bounds = RectV2_center_half_dim(game_state->camera_position.rel_,
                                                                  tile_span_in_meters);
                    
                    V2_add(camera_point, camera_bounds.min, &camera_bounds.min);
                    V2_add(camera_point, camera_bounds.max, &camera_bounds.max);
                    
                    
                    if((((column >= (s32)high_frequency_bounds.min.x) && (column <= (s32)high_frequency_bounds.max.x)) &&
                        ((row    == (s32)high_frequency_bounds.min.y) || (row    == (s32)high_frequency_bounds.max.y))) 
                       ||
                       ((row    >= (s32)high_frequency_bounds.min.y) && (row    <= (s32)high_frequency_bounds.max.y)) &&
                       ((column == (s32)high_frequency_bounds.min.x) || (column == (s32)high_frequency_bounds.max.x)))
                    {
                        Game_draw_rectangle(back_buffer,
                                            (V2){min_x,min_y},
                                            (V2){max_x,max_y},
                                            1.0f, 0.0f, 0.0f,
                                            0);
                    }
                    
                    if((((column >= (s32)camera_bounds.min.x) && (column <= (s32)camera_bounds.max.x)) &&
                        ((row    == (s32)camera_bounds.min.y) || (row    == (s32)camera_bounds.max.y))) 
                       ||
                       ((row    >= (s32)camera_bounds.min.y) && (row    <= (s32)camera_bounds.max.y)) &&
                       ((column == (s32)camera_bounds.min.x) || (column == (s32)camera_bounds.max.x)))
                    {
                        Game_draw_rectangle(back_buffer,
                                            (V2){min_x,min_y},
                                            (V2){max_x,max_y},
                                            0.0f, 0.0f, 1.0f,
                                            0);
                    }
                    
                    
                    //DEBUG - PLAYERS CURRENT TILE
                    for(int player_index = 0; player_index < 4; player_index++)
                    {
                        // NOTE(MIGUEL): get function set residency. no known way to check if enity exists or not
                        EntityLow *player = Entity_get_entity_low(game_state, game_state->player_controller_entity_index[player_index]);
                        
                        if(player &&
                           ((column == player->position.chunk_x) &&
                            (row    == player->position.chunk_y)) )
                        {
                            gray = 0.0f;
                            
                            // SHADOW
                            Game_draw_rectangle(back_buffer,
                                                (V2){min_x,min_y},
                                                (V2){max_x,max_y},
                                                gray + 0.1f, gray + 0.5f, gray,
                                                0);
                        }
                    }
                }
            }
        }
    }
    
    return;
}

internal void
Game_draw_bitmap(game_back_buffer *buffer, BitmapData *bitmap, f32 real_x, f32 real_y,
                 s32 align_x, s32 align_y, f32 c_alpha)
{
    real_x -= align_x;
    real_y -= align_y;
    
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
Game_debug_load_bmp(thread_context *thread, DEBUG_PlatformReadEntireFile *read_entire_file, u8 *file_name)
{
    BitmapData result = { 0 };
    
    debug_read_file_result read_result = read_entire_file(thread, file_name);
    
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

internal b32
Game_get_normalized_time_at_collision(f32 *normalized_time_at_closest_possible_collision,
                                      f32 wall_a,
                                      f32 rel_a, f32 rel_b,
                                      f32 position_delta_a, f32 position_delta_b,
                                      f32 min_b, f32 max_b)
{
    // NOTE(MIGUEL): a & b = generic coord components
    // NOTE(MIGUEL): t_min = time at closet collision
    // NOTE(MIGUEL): rel a & b = position of the plaber relative to the tile being tested
    // NOTE(MIGUEL): position_delta a & b = vector representing the plaber's direciton of travel
    // NOTE(MIGUEL): min_b and maa_b = ????
    
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
Game_player_move(GameState *game_state,  Entity entity, f32 delta_t, V2 acceleration)
{
    World *world = game_state->world;
    
    f32 accel_magnitude = V2_dot(acceleration, acceleration);
    
    if(accel_magnitude > 1.0f)
    {
        V2_scale(1.0f / square_root(accel_magnitude), &acceleration);
    }
    
    V2 v = entity.high->velocity;
    V2 a = acceleration;
    V2 old_pos = entity.high->position;
    //TilemapPosition new_pos = old_pos;
    f32 speed    = 300.0f; // m/s^2
    f32 friction = -12.0f;
    
    V2_scale(speed, &acceleration); // Tune the accleration with speed
    V2_scale(friction, &v); // Apply friction to acceleration
    V2_add  (acceleration, v, &acceleration);
    
    V2 position_delta = { 0 };
    a = acceleration;
    v = entity.high->velocity;
    // ACCELRATION COMPONENT
    V2_scale(0.5f , &a);
    V2_scale(square(delta_t), &a);
    // VELOCITY COMPONENT
    V2_scale(delta_t , &v);
    // JOIN ACCEL & VELOCITY COMPONENT
    V2_add  (a, v, &position_delta); // NOTE(MIGUEL): do not alter value! used a bit lower in the function
    
    // STORE VELOCITY EQUATION
    a = acceleration;
    v = entity.high->velocity;
    V2_add(a, v, &a);
    V2_scale(0.5f , &a);
    V2_scale(delta_t, &a);
    V2_add  (a, entity.high->velocity, &entity.high->velocity);
    
    // CALCULATED PLAYER NEW POSITION
    //V2 new_pos = { 0 };
    //V2_add(old_pos, position_delta, &new_pos);//Tile_offset(tilemap, old_pos, position_delta);
    
    
    
    for(u32 collision_resolve_attempt = 0; 
        collision_resolve_attempt < 4; collision_resolve_attempt++)
    {
        V2 wall_normal = { 0 };
        f32 normalized_time_of_pos_delta = 1.0f; // NORMALIZED SACALAR THAT REPS THE TIME STEP! NOT .033MS (MS PER FRAME)
        u32 hit_high_entity_index = 0;
        
        V2 desired_position;
        V2_add(entity.high->position, position_delta, &desired_position);
        
        for(u32 test_high_entity_index = 1; test_high_entity_index < game_state->entity_count_high; test_high_entity_index++)
        {
            if(test_high_entity_index != entity.low->index_high)
            { 
                Entity test_entity;
                test_entity.high = game_state->entities_high_ + test_high_entity_index;
                test_entity.low  = game_state->entities_low   + test_entity.high->index_low;
                
                if(test_entity.low->collides)
                {
                    
                    // NOTE(MIGUEL): Minkowski sum
                    f32 diameter_w = test_entity.low->width  + entity.low->width;
                    f32 diameter_h = test_entity.low->height + entity.low->height;
                    V2 min_corner = { diameter_w, diameter_h };
                    V2 max_corner = { diameter_w, diameter_h };
                    
                    // NOTE(MIGUEL): distancs away from tile center
                    V2_scale(-0.5f, &min_corner);
                    V2_scale( 0.5f, &max_corner);
                    
                    // NOTE(MIGUEL): old pos's Distance away from test tile in meters in x & y respectively
                    // NOTE(MIGUEL): Tile_subtract operand order maybe wrong...
                    V2 rel = { 0 };
                    V2_sub(entity.high->position, test_entity.high->position, &rel);
                    
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
                        hit_high_entity_index =  test_high_entity_index;
                    }
                    
                    // VERTICAL WALL LEFT FACE
                    if(Game_get_normalized_time_at_collision(&normalized_time_of_pos_delta, MinCowSkied_wall_left_x,
                                                             rel.x, rel.y,
                                                             position_delta.x, position_delta.y,
                                                             min_corner.y, max_corner.y))
                    {
                        wall_normal = (V2){-1.0, 0.0};
                        hit_high_entity_index = test_high_entity_index;
                    }
                    
                    // HORIZONTAL BOTTOM WALL
                    if(Game_get_normalized_time_at_collision(&normalized_time_of_pos_delta, MinCowSkied_wall_bottom_y,
                                                             rel.y, rel.x,
                                                             position_delta.y, position_delta.x,
                                                             min_corner.x, max_corner.x))
                    {
                        wall_normal = (V2){0.0, -1.0};
                        hit_high_entity_index = test_high_entity_index;
                    }
                    
                    // HORIZONTAL TOP WALL
                    if(Game_get_normalized_time_at_collision(&normalized_time_of_pos_delta, MinCowSkied_wall_top_y,
                                                             rel.y, rel.x,
                                                             position_delta.y, position_delta.x,
                                                             min_corner.x, max_corner.x))
                    {
                        wall_normal = (V2){0.0, 1.0};
                        hit_high_entity_index = test_high_entity_index;
                    }
                    
                    ASSERT((normalized_time_of_pos_delta <= 1.0f) && (normalized_time_of_pos_delta >= 0.0f));
                }
            }
        }
        
        // UPDATED PLAYER POSITION & STORE NEW PLAYER POSITION
        V2 scratch_position_delta = position_delta;
        V2_scale(normalized_time_of_pos_delta, &scratch_position_delta);
        V2_add(entity.high->position, scratch_position_delta ,&entity.high->position);
        
        if(hit_high_entity_index)
        {
            V2 scratch_wall_normal = wall_normal;
            // UPDATE VELOCITY VECTOR 
            V2_scale(V2_dot(entity.high->velocity, scratch_wall_normal), &scratch_wall_normal);
            V2_scale(1.0f, &scratch_wall_normal);
            V2_sub  (entity.high->velocity, scratch_wall_normal, &entity.high->velocity);
            
            // NEW POSITION FOR COLLISION TEST
            V2_sub(desired_position, entity.high->position, &position_delta);
            
            scratch_wall_normal = wall_normal;
            // UPDATE MOVEMENT VECTOR
            V2_scale(V2_dot(position_delta, scratch_wall_normal), &scratch_wall_normal);
            V2_scale(1.0f, &scratch_wall_normal);
            V2_sub  (position_delta, scratch_wall_normal, &position_delta);
            
            // STAIRS
            EntityHigh *hit_high_entity = game_state->entities_high_ + hit_high_entity_index;
            EntityLow  *hit_low_entity  = game_state->entities_low + hit_high_entity->index_low;
            entity.high->tile_abs_z += hit_low_entity->delta_tile_abs_z;
        }
        else
        {
            break;
        }
        
    }
    
    if((entity.high->velocity.x == 0.0f) &&
       (entity.high->velocity.y == 0.0f))
    {
    }
    else if(absolute_value(entity.high->velocity.x) > absolute_value(entity.high->velocity.y))
    {
        if(entity.high->velocity.x > 0)
        {
            entity.high->facing_direction = 0;
        }
        else
        {
            entity.high->facing_direction = 2;
        }
        
    }
    else if(absolute_value(entity.high->velocity.x) < absolute_value(entity.high->velocity.y))
    {
        if(entity.high->velocity.y > 0)
        {
            entity.high->facing_direction = 1;
        }
        else
        {
            entity.high->facing_direction = 3;
        }
    }
    
    
    WorldCoord new_position = World_map_to_chunkspace(game_state->world,
                                                      game_state->camera_position,
                                                      entity.high->position);
    
    World_change_entity_location(game_state->world,
                                 entity.high->index_low,
                                 &entity.low->position,
                                 &new_position,
                                 &game_state->world_arena);
    
    entity.low->position = new_position;
    
    return;
}

internal V2
Game_set_camera(GameState *game_state, WorldCoord new_camera_position)
{
    World *world = game_state->world;
    
    ASSERT(Entity_validate_entity_pairs(game_state));
    
    WorldDifference camera_position_delta = World_subtract(world,
                                                           &new_camera_position,
                                                           &game_state->camera_position);
    game_state->camera_position = new_camera_position;
    
    V2 tile_span_in_meters = (V2){ (17 * 3)/2 , (9 * 3)/2};
    
    V2 entity_offset_for_frame = camera_position_delta.dxy;
    
    //V2_scale((1 / 2.0f), &tile_span_in_meters);
    V2_scale(world->side_in_meters_tile, &tile_span_in_meters);
    
    RectV2 high_frequency_bounds = RectV2_center_half_dim((V2){0 , 0},
                                                          tile_span_in_meters);
    
    V2_scale(-1.0f, &entity_offset_for_frame);
    
    // NOTE(MIGUEL): set out of bounds entities low
    Entity_check_frequency_by_area(game_state, entity_offset_for_frame,
                                   high_frequency_bounds);
    
    ASSERT(Entity_validate_entity_pairs(game_state));
    
    V2_scale((2 / (f32)(world->side_in_meters_tile)), &tile_span_in_meters);
    
    
    // TODO(MIGUEL): 07/12/2021 | HMH DAY 058  | TIME 35:57
    WorldCoord minchunk_pos = World_map_to_chunkspace(world, new_camera_position, RectV2_min_corner(high_frequency_bounds));
    WorldCoord maxchunk_pos = World_map_to_chunkspace(world, new_camera_position, RectV2_max_corner(high_frequency_bounds));
    
    for(s32 chunk_y = minchunk_pos.chunk_y; chunk_y < maxchunk_pos.chunk_y; chunk_y++)
    {
        for(s32 chunk_x = minchunk_pos.chunk_x; chunk_x < maxchunk_pos.chunk_x; chunk_x++)
        {
            WorldChunk *chunk = World_get_worldchunk(world, chunk_x, chunk_y, new_camera_position.chunk_z, NULLPTR);
            
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
                        
                        if(entity_low->index_high == 0)
                        {
                            V2 camera_space_pos = Entity_get_camera_space_position(game_state, entity_low);
                            
                            if(RectV2_is_in_rect(high_frequency_bounds, camera_space_pos))
                            {
                                Entity_set_frequency_high_(game_state, entity_low, index_low, camera_space_pos);
                            }
                        }
                    }
                }
            }
        }
    }
    
    ASSERT(Entity_validate_entity_pairs(game_state));
    
    // TODO(MIGUEL): move entities into high set here
    
    return entity_offset_for_frame;
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
        Entity_create_entity_low(game_state, EntityType_null, NULLPTR);
        game_state->entity_count_high = 1;
        
        //~ PLAYER(s) INITIALIZATION
        
        //game_state->camera_position.chunk_x = 17 / 2;
        //game_state->camera_position.chunk_y =  9 / 2;
        
        
        //~ BITMAP LOADING
        game_state->back_drop = Game_debug_load_bmp(thread,
                                                    sge_memory->debug_platform_read_entire_file,
                                                    "../res/images/test_background.bmp");
        game_state->shadow    = Game_debug_load_bmp(thread,
                                                    sge_memory->debug_platform_read_entire_file,
                                                    "../res/images/test_hero_shadow.bmp");
        
        game_state->tree      = Game_debug_load_bmp(thread,
                                                    sge_memory->debug_platform_read_entire_file,
                                                    "../res/images/tree00.bmp");
        
        DEBUG_PlatformReadEntireFile *read_file_callback = sge_memory->debug_platform_read_entire_file;
        player_bitmaps               *bitmap             = game_state->playerbitmaps;
        
        //RIGHT
        bitmap->head  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/shadow_right_head.bmp");
        bitmap->torso = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_right_torso.bmp");
        bitmap->cape  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_right_cape.bmp");
        bitmap->align_x =  72;
        bitmap->align_y = 182;
        bitmap++;
        
        //BACK
        bitmap->head  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/shadow_front_head.bmp");
        bitmap->torso = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_back_torso.bmp");
        bitmap->cape  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_back_cape.bmp");
        bitmap->align_x =  72;
        bitmap->align_y = 182;
        bitmap++;
        
        //LEFT
        bitmap->head  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/shadow_left_head.bmp");
        bitmap->torso = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_left_torso.bmp");
        bitmap->cape  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_left_cape.bmp");
        bitmap->align_x =  72;
        bitmap->align_y = 182;
        bitmap++;
        
        //FRONT
        bitmap->head  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/shadow_front_head.bmp");
        bitmap->torso = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_front_torso.bmp");
        bitmap->cape  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_front_cape.bmp");
        bitmap->align_x =  72;
        bitmap->align_y = 182;
        
        //~ WORLD GENERATION
        
        MemoryArena_init(&game_state->world_arena,
                         sge_memory->permanent_storage_size  - sizeof(GameState),
                         (u8 *)sge_memory->permanent_storage + sizeof(GameState));
        
        
        game_state->world = MEMORY_ARENA_PUSH_STRUCT(&game_state->world_arena, World); 
        World *world  = game_state->world;
        World_init(world, 1.4f); 
        
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
        
        for(u32 screen_index = 0; screen_index < 100; screen_index++)
        {
            ASSERT(random_number_index < ARRAYCOUNT(random_number_table));
            u32 random_choice;
            if(1) //(door_up || door_down)
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
        
        initial_camera_position = World_tilecoord_to_chunkcoord(world,
                                                                camera_tile_x,
                                                                camera_tile_y,
                                                                camera_tile_z);
        
        Game_add_hostile(game_state,
                         camera_tile_x + 2,
                         camera_tile_y + 2,
                         camera_tile_z);
        
        
        Game_add_friendly(game_state,
                          camera_tile_x - 2,
                          camera_tile_y + 2,
                          camera_tile_z);
        
        Game_set_camera(game_state, initial_camera_position); 
        
        sge_memory->is_initialized = 1; 
    }
    
    World *world  = game_state->world;
    //Tilemap *tilemap = the_world->tilemap;
    
    s32 tile_side_in_pixels = 60;
    f32 meters_to_pixels    = ((f32)tile_side_in_pixels / (f32)world->side_in_meters_tile);
    
    f32 lower_left_x = -(f32)tile_side_in_pixels / (f32)2;
    f32 lower_left_y =  (f32)back_buffer->height;
    
    // ************************************************
    // INPUT RESPONSE
    //*************************************************
    for(u32 controller_index = 0; controller_index < 5; controller_index++)
    {
        game_controller_input *controller = &input->controllers[controller_index];
        
        u32 low_index = game_state->player_controller_entity_index[controller_index];
        
        if(low_index == 0)
        {
            if(controller->button_start.ended_down)
            { 
                CreateEntityLowResult result = Game_add_player(game_state, controller_index);
                game_state->player_controller_entity_index[controller_index] = result.index_low; 
            }
        }
        else
        {
            
            Entity controlled_entity = Entity_get_entity_high(game_state, low_index);
            
            V2 player_accel = { 0.0f };
            
            if(controller->is_analog)
            {
                // NOTE(MIGUEL): use analog mov tunning
                player_accel = (V2){ controller->stick_avg_x,controller->stick_avg_y };
                
            }
            else
            {
                if(controller->button_y.ended_down)
                {
                    // up
                    player_accel.y = 1.0f;
                }
                if(controller->button_a.ended_down)
                {
                    // down
                    player_accel.y = -1.0f;
                }
                
                if(controller->button_x.ended_down)
                {
                    // left
                    player_accel.x = -1.0f;
                }
                
                if(controller->button_b.ended_down)
                {
                    // right
                    player_accel.x = 1.0f;
                }
            }
            
            if(controller->action_up.ended_down)
            {
                controlled_entity.high->delta_z = 3.0f;
            }
            if(controller->shoulder_left.ended_down)
            {
                // focus camera on this player
                game_state->camera_following_entity_index = game_state->player_controller_entity_index[controller_index];
            }
            Game_player_move(game_state, controlled_entity, input->delta_t, player_accel);
        }
        
    } /// END OF INPUT LOOP
    
    
    /// CAMRERA MOVEMENT LOGIC
    Entity camera_following_entity = Entity_get_entity_high(game_state, game_state->camera_following_entity_index);
    
    if(camera_following_entity.high)
    {
        WorldCoord new_camera_position = game_state->camera_position;
        
        game_state->camera_position.chunk_z = camera_following_entity.low->position.chunk_z;
#if 0
        if(camera_following_entity.high->position.x > (9.0f * tilemap->side_in_meters_tile))
        {
            new_camera_position.tile_abs_x += 17; 
        }
        if(camera_following_entity.high->position.x < -(9.0f * tilemap->side_in_meters_tile))
        {
            new_camera_position.tile_abs_x -= 17; 
        }
        if(camera_following_entity.high->position.y > (5.0f * tilemap->side_in_meters_tile))
        {
            new_camera_position.tile_abs_y += 9; 
        }
        if(camera_following_entity.high->position.y < -(5.0f * tilemap->side_in_meters_tile))
        {
            new_camera_position.tile_abs_y -= 9; 
        }
#else
        V2 follow_speed = { 1.0f, 1.0f };
        V2 entity_offset_for_frame = { 0 };
        V2 camera_half_field_of_view =
        {
            (17.0f / 2.0f) * world->side_in_meters_tile,
            ( 9.0f / 2.0f) * world->side_in_meters_tile
        };
        
        entity_offset_for_frame.x = follow_speed.x * (camera_following_entity.high->position.x / camera_half_field_of_view.x);
        entity_offset_for_frame.y = follow_speed.y * (camera_following_entity.high->position.y / camera_half_field_of_view.y);
        
        new_camera_position = World_map_to_chunkspace(world, new_camera_position, entity_offset_for_frame);
#endif
        Game_set_camera(game_state, new_camera_position);
    }
    
    
    /// debug purp background clear
    Game_draw_rectangle(back_buffer,
                        (V2){0.0f, 0.0f} ,
                        (V2){(f32)back_buffer->width,(f32)back_buffer->height},
                        0.4f, 0.8f, 1.0f,
                        0);
    
    
    //game_draw_bitmap(back_buffer, &game_state->back_drop, 0.0f, 0.0f, 0, 0);
    
    f32 screen_center_x = 0.5f * (f32)back_buffer->width;
    f32 screen_center_y = 0.5f * (f32)back_buffer->height;
    
    
    s32 camera_range_x = 10;
    s32 camera_range_y = 20;
    for(s32 rel_row = -camera_range_x; rel_row < camera_range_x; rel_row++)
    {
        for(s32 rel_column = -camera_range_y; rel_column < camera_range_y; rel_column++)
        {
            s32 column = (s32)(game_state->camera_position.rel_.x / world->side_in_meters_tile) + rel_column;
            s32 row    = (s32)(game_state->camera_position.rel_.y / world->side_in_meters_tile) + rel_row;
            
            u32 tileid = 1;
            
            if(tileid > 0)
            {
                f32 gray = 0.0f;
                
                //circle drawing shinanegans
                f32 r = 20.0f; // * sinf(input->delta_t);
                f32 h = 4.0f;
                f32 k = 3.0f;
                
                f32 center_x = (screen_center_x -
                                (meters_to_pixels * game_state->camera_position.rel_.x) + ((f32)rel_column  * tile_side_in_pixels));
                
                f32 center_y = (screen_center_y +
                                (meters_to_pixels * game_state->camera_position.rel_.y) -
                                ((f32)rel_row     * tile_side_in_pixels));
                
                f32 min_x = center_x - 0.5f * tile_side_in_pixels;
                f32 min_y = center_y - 0.5f * tile_side_in_pixels;
                f32 max_x = center_x + 0.5f * tile_side_in_pixels;
                f32 max_y = center_y + 0.5f * tile_side_in_pixels;
#if 1
                // FLOOR & WALLS
                if(((s32)column == ceiling_f32_to_s32(((-sqrtf(r * 2 - powf(row - h, 2))) + k))) ||
                   ((s32)column == ceiling_f32_to_s32(  (sqrtf(r * 2 - powf(row - h, 2)) + k))))
                {
                    gray = 0.4f;
                }
#endif
                if(tileid == 1)
                {
                    // FLOOR
#if 1
                    Game_draw_rectangle(back_buffer,
                                        (V2){min_x,min_y},
                                        (V2){max_x,max_y},
                                        gray + 0.1f, gray + 0.5f, gray,
                                        1);
#endif
                }
                else if(tileid == 3)
                {
                    // UP LADDER 
                    gray = 0.85f;
                    
                    Game_draw_rectangle(back_buffer,
                                        (V2){min_x,min_y},
                                        (V2){max_x,max_y},
                                        gray, gray + 0.09f, gray + 0.1f,
                                        0);
                }
                else if(tileid == 4)
                {
                    // UP LADDER 
                    gray = 0.15f;
                    
                    Game_draw_rectangle(back_buffer,
                                        (V2){min_x,min_y},
                                        (V2){max_x,max_y},
                                        gray, gray, gray,
                                        0);
                }
                
                /*
                Entity entity = Entity_get(game_state, EntityResidence_high, 1);
                
                for(u32 entity_index = 0;
                    entity_index < game_state->entity_count; entity_index++)
                {
                    if((entity.residence == EntityResidence_high) &&
                       (entity.low->position.tile_abs_z == game_state->camera_position.tile_abs_z) &&
                       (entity.low->position.tile_abs_x == column) && 
                       (entity.low->position.tile_abs_y == row))
                    {
                        //DEBUG - PLAYERS CURRENT TILE
                        f32 gray = 0.0f;
                        
                        // SHADOW
                        game_draw_rectangle(back_buffer,
                                            (V2){min_x,min_y},
                                            (V2){max_x,max_y},
                                            gray + 0.1f, gray + 0.5f, gray);
                    }
            }
*/
            }
        }
    }
    
    game_state->clock += input->delta_t * 0.1f;
    
    //-Game_render_weird_shit(back_buffer, 10, 40, game_state->clock);
    
    if(game_state->player_controller_entity_index[0])
    {
        int i = 1;
    }
    //DRAW ALL HIGH ENTITIES
    for(u32 index_high = 1;
        index_high < game_state->entity_count_high; index_high++)
    {
        EntityHigh *entity_high = game_state->entities_high_ + index_high;
        EntityLow  *entity_low  = game_state->entities_low   + entity_high->index_low;
        
        Entity entity;
        entity.low = entity_low;
        entity.high = entity_high;
        // Game_update_entity(game_state, entity, delta_t);
        
        
        // JUMP CODE
        f32 delta_t      = input->delta_t;
        f32 acceleration = -9.8f;
        
        entity_high->z = 0.5f * acceleration * square(delta_t) + entity_high->delta_z * delta_t + entity_high->z;
        entity_high->delta_z = acceleration * delta_t + entity_high->delta_z;
        
        if(entity_high->z < 0.0f)
        {
            entity_high->z = 0.0f;
        }
        
        f32 shadow_alpha = 1.0f - 0.5f * entity_high->z;
        
        if(shadow_alpha < 0.0f)
        {
            shadow_alpha = 0.0f;
        }
        
        // END OF JUMP CODE
        f32 width  = entity_low->width;
        f32 height = entity_low->height;
        
        f32 entity_ground_point_x = screen_center_x + meters_to_pixels * entity_high->position.x;
        f32 entity_ground_point_y = screen_center_y - meters_to_pixels * entity_high->position.y;
        
        f32 entity_left = (screen_center_x - (0.5f * meters_to_pixels * width ));
        f32 entity_top  = (screen_center_y - (1.0f * meters_to_pixels * height));
        
        V2 entity_dimensions = { width, height };
        V2 entity_bottom_right = { 0 };
        
        // JUMP CODE
        f32 z = -meters_to_pixels * entity_high->z;
        
        V2 entity_top_left = 
        {
            entity_ground_point_x - 0.5f * meters_to_pixels * width, 
            entity_ground_point_y - 0.5f * meters_to_pixels * height
        };
        
        V2_scale(meters_to_pixels, &entity_dimensions);
        V2_add  (entity_top_left, entity_dimensions, &entity_bottom_right);
        
        
        f32 gray = 0.0f;
        
        switch(entity_low->type )
        {
            
            case EntityType_player:
            {
                
                player_bitmaps *playerbitmaps = &game_state->playerbitmaps[entity_high->facing_direction];
                Game_draw_bitmap(back_buffer, &game_state->shadow,
                                 entity_ground_point_x, entity_ground_point_y,
                                 playerbitmaps->align_x, playerbitmaps->align_y, shadow_alpha);
                
                Game_draw_bitmap(back_buffer, &playerbitmaps->torso,
                                 entity_ground_point_x, entity_ground_point_y + z,
                                 playerbitmaps->align_x, playerbitmaps->align_y, 1.0f);
                
                Game_draw_bitmap(back_buffer, &playerbitmaps->cape ,
                                 entity_ground_point_x, entity_ground_point_y + z,
                                 playerbitmaps->align_x, playerbitmaps->align_y, 1.0f);
                
                Game_draw_bitmap(back_buffer, &playerbitmaps->head,
                                 entity_ground_point_x, entity_ground_point_y + z,
                                 playerbitmaps->align_x, playerbitmaps->align_y, 1.0f);
            } break;
            case EntityType_wall:
            {
                // WALL
                gray = 0.8f;
                
#if 0
                Game_draw_rectangle(back_buffer,
                                    entity_top_left,
                                    entity_bottom_right,
                                    gray, gray, gray,
                                    1);
#endif
                
                Game_draw_bitmap(back_buffer, &game_state->tree,
                                 entity_ground_point_x, (entity_ground_point_y + z),
                                 (s32)(40), (s32)(80), 1.0f);
                
            } break;
            
            case EntityType_friendly: 
            {
                player_bitmaps *playerbitmaps = &game_state->playerbitmaps[entity_high->facing_direction];
                Game_draw_bitmap(back_buffer, &game_state->shadow,
                                 entity_ground_point_x, entity_ground_point_y,
                                 playerbitmaps->align_x, playerbitmaps->align_y, shadow_alpha);
                
                Game_draw_bitmap(back_buffer, &playerbitmaps->head,
                                 entity_ground_point_x, entity_ground_point_y + z,
                                 playerbitmaps->align_x, playerbitmaps->align_y, 1.0f);
                
            } break;
            case EntityType_hostile: 
            {
                player_bitmaps *playerbitmaps = &game_state->playerbitmaps[entity_high->facing_direction];
                Game_draw_bitmap(back_buffer, &game_state->shadow,
                                 entity_ground_point_x, entity_ground_point_y,
                                 playerbitmaps->align_x, playerbitmaps->align_y, shadow_alpha);
                
                Game_draw_bitmap(back_buffer, &playerbitmaps->torso,
                                 entity_ground_point_x, entity_ground_point_y + z,
                                 playerbitmaps->align_x, playerbitmaps->align_y, 1.0f);
                
            } break;
            default:
            {
                INVALID_CODE_PATH;
            } break;
        }
    }
    
#if 0
    Game_draw_mini_map(game_state,
                       back_buffer,
                       4,
                       200,
                       100,
                       300, 140);
#endif
    return;
}


internal void Game_update_sound_buffer(GameState *game_state, game_sound_output_buffer *sound_buffer, u32 tone_hz)
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
Game_draw_rectangle(game_back_buffer *buffer,
                    V2 min, V2 max, f32 r, f32 g, f32 b, b32 grid)
{
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
    
    /// bit pattern 0x AA RR GG BB
    u32 color = ((round_f32_to_u32(r * 255.0f) << 16) |
                 (round_f32_to_u32(g * 255.0f) <<  8) |
                 (round_f32_to_u32(b * 255.0f) <<  0));
    
    /// drawing
    u8 *line =  ((u8 *)  (buffer->data)            + 
                 (min_x * buffer->bytes_per_pixel) +
                 (min_y * buffer->pitch));
    
    for(s32 y = min_y; y < max_y; y++ )
    {
        u32 *pixel = (u32 *)line;
        
        for(s32 x = min_x; x < max_x; x++)
        {
            *pixel = color;
            
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
internal void Game_render_weird_shit(game_back_buffer *buffer, s32 x_offset, s32 y_offset, f32 time)
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

