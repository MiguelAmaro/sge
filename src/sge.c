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
    
    game_update_sound_buffer(game_state, sound_buffer, 400);
    
    return;
}

SGE_INIT(SGEInit)
{
    // NOTE(MIGUEL): get rid of this it makes no sense
    
    return;
}


internal void
game_draw_mini_map(GameState *state,
                   game_back_buffer *back_buffer,
                   s32 tile_side_in_pixels,
                   s32 camera_range_x,
                   s32 camera_range_y,
                   s32 x, s32 y)
{
    Tilemap *tilemap = state->world->tilemap;
    
    f32 meters_to_pixels    = ((f32)tile_side_in_pixels / (f32)tilemap->tile_side_in_meters);
    
    f32 screen_center_x = 0.5f * (f32)back_buffer->width;
    f32 screen_center_y = 0.5f * (f32)back_buffer->height;
    
    // NOTE(MIGUEL): dont think this is neccessary for mini map
    Entity *entity = &state->entities[state->camera_following_entity_index];
    
    for(s32 rel_row = -camera_range_x; rel_row < camera_range_x; rel_row++)
    {
        for(s32 rel_column = -camera_range_y; rel_column < camera_range_y; rel_column++)
        {
            u32 column = entity->pos.tile_abs_x + rel_column;
            u32 row    = entity->pos.tile_abs_y + rel_row;
            
            u32 tileid = Tile_get_tile_value(tilemap, column, row, entity->pos.tile_abs_z );
            
            if(tileid > 0)
            {
                f32 gray = 0.2f;
                
                //circle drawing shinanegans
                f32 r = 20.0f; // * sinf(input->delta_t);
                f32 h = 4.0f;
                f32 k = 3.0f;
                
                f32 center_x = (screen_center_x -
                                (meters_to_pixels * entity->pos.tile_rel_.x) +
                                ((f32)rel_column * tile_side_in_pixels));
                
                f32 center_y = (screen_center_y +
                                (meters_to_pixels * entity->pos.tile_rel_.y) -
                                ((f32)rel_row     * tile_side_in_pixels));
                
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
                    game_draw_rectangle(back_buffer,
                                        (v2){min_x,min_y},
                                        (v2){max_x,max_y},
                                        gray + 0.1f, gray + 0.5f, gray);
                    
                }
                else if(tileid == 2) 
                {
                    // WALL
                    gray = 0.8f;
                    
                    game_draw_rectangle(back_buffer,
                                        (v2){min_x,min_y},
                                        (v2){max_x,max_y},
                                        gray, gray, gray);
                }
                else if(tileid == 3)
                {
                    // LADDER 
                    gray = 0.0f;
                    
                    game_draw_rectangle(back_buffer,
                                        (v2){min_x,min_y},
                                        (v2){max_x,max_y},
                                        gray, gray, gray);
                }
                
                //DEBUG - PLAYERS CURRENT TILE
                for(int player_index = 0; player_index < 4; player_index++)
                {
                    Entity *player = &state->entities[state->player_controller_entity_index[player_index]];
                    
                    if(player->exists &&
                       ((column == player->pos.tile_abs_x) &&
                        (row    == player->pos.tile_abs_y)) )
                    {
                        gray = 0.0f;
                        
                        // SHADOW
                        game_draw_rectangle(back_buffer,
                                            (v2){min_x,min_y},
                                            (v2){max_x,max_y},
                                            gray + 0.1f, gray + 0.5f, gray);
                    }
                }
            }
        }
    }
    
    
    return;
}

internal void
game_draw_bitmap(game_back_buffer *buffer, BitmapData *bitmap, f32 real_x, f32 real_y,
                 s32 align_x, s32 align_y)
{
    real_x -= align_x;
    real_y -= align_y;
    
    /// rounding / ruling
    s32 min_x = round_f32_to_s32(real_x);
    s32 min_y = round_f32_to_s32(real_y);
    
    s32 max_x = round_f32_to_s32(real_x + (f32)bitmap->width );
    s32 max_y = round_f32_to_s32(real_y + (f32)bitmap->height);
    
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
debug_load_bmp(thread_context *thread, DEBUG_PlatformReadEntireFile *read_entire_file, u8 *file_name)
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

internal void
testwall(f32 wall_x, f32 rel_x, f32 rel_y, f32 position_delta_x, f32 position_delta_y, f32 min_y, f32 max_y, f32 *t_min)
{
    f32 t_epsilon = 0.0001f; // floating point tolerance
    
    if(position_delta_y != 0.0f)
    {
        f32 t_result = (wall_x - rel_x) / position_delta_x;
        f32 y        = rel_y + t_result * position_delta_y;
        
        if((t_result >= 0) && (*t_min > t_result))
        {
            if((y >= min_y) && (y <= max_y))
            {
                *t_min = MAXIMUM(0.0f, t_result - t_epsilon);
            }
        }
    }
    
    return;
}

internal void
player_move(GameState *state,  Entity *entity, f32 delta_t, v2 acceleration)
{
    Tilemap *tilemap = state->world->tilemap;
    
    f32 accel_magnitude = v2_dot(acceleration, acceleration);
    
    if(accel_magnitude > 1.0f)
    {
        v2_scale(1.0f / square_root(accel_magnitude), &acceleration);
    }
    
    v2 v = entity->velocity;
    v2 a = acceleration;
    TilemapPosition old_pos = entity->pos;
    //TilemapPosition new_pos = old_pos;
    f32 speed    = 100.0f; // m/s^2
    f32 friction = -12.0f;
    
    v2_scale(speed, &acceleration); // Tune the accleration with speed
    v2_scale(friction, &v); // Apply friction to acceleration
    v2_add  (acceleration, v, &acceleration);
    
    v2 position_delta = { 0 };
    a = acceleration;
    v = entity->velocity;
    // ACCELRATION COMPONENT
    v2_scale(0.5f , &a);
    v2_scale(square(delta_t), &a);
    // VELOCITY COMPONENT
    v2_scale(delta_t , &v);
    v2_add  (a, v, &position_delta); // NOTE(MIGUEL): do not alter value! used a bit lower in the function
    
    // STORE VELOCITY EQUATION
    a = acceleration;
    v = entity->velocity;
    v2_add(a, v, &a);
    v2_scale(0.5f , &a);
    v2_scale(delta_t, &a);
    v2_add  (a, entity->velocity, &entity->velocity);
    
    TilemapPosition new_pos = Tile_offset(tilemap, old_pos, position_delta);
    // UPDATED PLAYER POSITION
    new_pos = Tile_recanonicalize_position(tilemap, new_pos);
    
#if 0 
    
    TilemapPosition new_pos_left = new_pos;
    new_pos_left.tile_rel.x -= 0.5f * entity->width; 
    new_pos_left = Tile_recanonicalize_position(tilemap, new_pos_left);
    
    TilemapPosition new_pos_right = new_pos ;
    new_pos_right.tile_rel.x += 0.5f * entity->width;
    new_pos_right = Tile_recanonicalize_position(tilemap, new_pos_right);
    
    b32 collided = 0;
    
    TilemapPosition collided_pos = { 0 };
    if(!Tile_is_point_empty(tilemap, new_pos) )
    {
        collided_pos = new_pos;
        collided = 1;
    }
    if(!Tile_is_point_empty(tilemap, new_pos_left) )
    {
        collided_pos = new_pos_left;
        collided = 1;
    }
    if(!Tile_is_point_empty(tilemap, new_pos_right) )
    {
        collided_pos = new_pos_right;
        collided = 1;
    }
    
    
    if(collided)
    {
        v2 *player_vel = &entity->velocity;
        v2 normal = { 0 ,0 };
        
        if(collided_pos.tile_abs_x < entity->pos.tile_abs_x)
        {
            normal.x = 1;
            normal.y = 0;
        }
        if(collided_pos.tile_abs_x > entity->pos.tile_abs_x)
        {
            normal.x = -1;
            normal.y =  0;
        }
        if(collided_pos.tile_abs_y < entity->pos.tile_abs_y)
        {
            normal.x = 0;
            normal.y = 1;
        }
        if(collided_pos.tile_abs_y > entity->pos.tile_abs_y)
        {
            normal.x =  0;
            normal.y = -1;
        }
        
        v2_scale(v2_dot(*player_vel, normal), &normal);
        v2_scale(1.0f, &normal);
        *player_vel = v2_sub(*player_vel, normal);
    }
    else
    {
        entity->pos = new_pos;
    }
#else
    // NOTE(MIGUEL): FAILS MOVING LEFT ON PLAYER INIT AND OTHER CASES
    // NOTE(MIGUEL): HACK: MOVE RIGHT THEN DOWN ACTIVATES COLLISION DETECTION
#if 0
    u32 min_tile_x          = MINIMUM(old_pos.tile_abs_x, new_pos.tile_abs_x);
    u32 min_tile_y          = MINIMUM(old_pos.tile_abs_y, new_pos.tile_abs_y);
    u32 one_past_max_tile_x = MAXIMUM(old_pos.tile_abs_x, new_pos.tile_abs_x) + 1;
    u32 one_past_max_tile_y = MAXIMUM(old_pos.tile_abs_y, new_pos.tile_abs_y) + 1;
#else
    u32 start_tile_x = old_pos.tile_abs_x;
    u32 start_tile_y = old_pos.tile_abs_y;
    u32 end_tile_x   = new_pos.tile_abs_x;
    u32 end_tile_y   = new_pos.tile_abs_y;
    
    s32 delta_x = sign_of(end_tile_x - start_tile_x);
    s32 delta_y = sign_of(end_tile_y - start_tile_y);
#endif
    
    u32 tile_abs_z = entity->pos.tile_abs_z;
    f32 t_min = 1.0f;
    
    u32 tile_abs_y = start_tile_y; 
    for(;;)
    {
        u32 tile_abs_x = start_tile_x; 
        for(;;)
        {
            TilemapPosition test_tile_pos = Tile_centered_tile_point(tile_abs_x, tile_abs_y, tile_abs_z );
            u32             tile_value    = Tile_get_tile_value(tilemap,
                                                                test_tile_pos.tile_abs_x,
                                                                test_tile_pos.tile_abs_y,
                                                                test_tile_pos.tile_abs_z);
            
            if(!Tile_is_tile_value_empty(tile_value))
            {
                v2 min_corner = { tilemap->tile_side_in_meters, tilemap->tile_side_in_meters };
                v2 max_corner = { tilemap->tile_side_in_meters, tilemap->tile_side_in_meters };
                v2_scale(-0.5f, &min_corner);
                v2_scale( 0.5f, &max_corner);
                
                // NOTE(MIGUEL): Tile_subtract operand order maybe wrong...
                TilemapDifference rel_old_pos = Tile_subtract(tilemap,
                                                              &old_pos,
                                                              &test_tile_pos);
                
                v2 rel = rel_old_pos.dxy;
                
                testwall(min_corner.x,
                         rel.x, rel.y,
                         position_delta.x, position_delta.y,
                         min_corner.y, max_corner.y,
                         &t_min);
                
                testwall(max_corner.x,
                         rel.x, rel.y,
                         position_delta.x, position_delta.y,
                         min_corner.y, max_corner.y,
                         &t_min);
                
                testwall(min_corner.y,
                         rel.y, rel.x,
                         position_delta.y, position_delta.x,
                         min_corner.x, max_corner.x,
                         &t_min);
                
                testwall(max_corner.y,
                         rel.y, rel.x,
                         position_delta.y, position_delta.x,
                         min_corner.x, max_corner.x,
                         &t_min);
            }
            
            if(tile_abs_x == end_tile_x )
            {
                break;
            }
            else
            {
                tile_abs_x += delta_x;
            }
        }
        
        if(tile_abs_y == end_tile_y)
        {
            break;
        }
        else
        {
            tile_abs_y += delta_y;
        }
    }
    
    
    new_pos = old_pos;
    // UPDATED PLAYER POSITION
    v2_scale(t_min, &position_delta);
    new_pos = Tile_offset(tilemap, new_pos, position_delta);
    // STORE NEW PLAYER POSITION
    entity->pos = new_pos;
    
#endif
    
    if(!Tile_is_on_same_tile(&old_pos, &entity->pos))
    {
        u32 new_tile_value =  Tile_get_tile_value_tilemap_pos(tilemap, entity->pos);
        
        if(new_tile_value == 3)
        {
            entity->pos.tile_abs_z++;
            
        }
        else if(new_tile_value == 4)
        {
            entity->pos.tile_abs_z--;
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


inline Entity *
entity_get(GameState *state, u32 index)
{
    Entity *entity = NULLPTR;
    
    if((index > 0) && (index < ARRAYCOUNT(state->entities)))
    {
        entity = &state->entities[index];
    }
    
    return entity;
}


internal void
player_init(GameState *state, u32 entity_index)
{
    Entity *entity = entity_get(state, entity_index);
    
    entity->exists = 1;
    entity->pos.tile_abs_x = 1;
    entity->pos.tile_abs_y = 3;
    entity->pos.tile_rel_.x = 0.0f; //acceptable range: -0.7 : 0.7
    entity->pos.tile_rel_.y = 0.0f; //acceptable range: -0.7 : 0.7
    entity->height         = 1.4f;
    entity->width          = 0.75f * entity->height; 
    
    if(!entity_get(state, state->camera_following_entity_index))
    {
        state->camera_following_entity_index = entity_index;
    }
    
    return;
}

internal u32
entity_add(GameState *state)
{
    ASSERT(state->entity_count < ARRAYCOUNT(state->entities));
    
    u32 entity_index = ++state->entity_count;
    
    Entity *entity = &state->entities[entity_index];
    
    memset(entity, 0, sizeof(entity));
    
    return entity_index;
}

SGE_UPDATE(SGEUpdate)
{
    ASSERT((&input->controllers[0].terminator - &input->controllers[0].buttons[0]) ==
           (ARRAYCOUNT(input->controllers[0].buttons)));
    
    ASSERT(sizeof(GameState) <= sge_memory->permanent_storage_size);
    
    
    GameState *game_state = (GameState *)sge_memory->permanent_storage;
    if(!sge_memory->is_initialized)
    {   
        //~ PLAYER(s) INITIALIZATION
        
        game_state->camera_pos.tile_abs_x = 17 / 2;
        game_state->camera_pos.tile_abs_y =  9 / 2;
        
        
        //~ BITMAP LOADING
        game_state->back_drop    = debug_load_bmp(thread,
                                                  sge_memory->debug_platform_read_entire_file,
                                                  "../res/images/test_background.bmp");
        
        DEBUG_PlatformReadEntireFile *read_file_callback = sge_memory->debug_platform_read_entire_file;
        player_bitmaps               *bitmap             = game_state->playerbitmaps;
        
        //RIGHT
        bitmap->head  = debug_load_bmp(thread, read_file_callback, "../res/images/shadow_right_head.bmp");
        bitmap->torso = debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_right_torso.bmp");
        bitmap->cape  = debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_right_cape.bmp");
        bitmap->align_x =  72;
        bitmap->align_y = 182;
        bitmap++;
        
        //BACK
        bitmap->head  = debug_load_bmp(thread, read_file_callback, "../res/images/shadow_front_head.bmp");
        bitmap->torso = debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_back_torso.bmp");
        bitmap->cape  = debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_back_cape.bmp");
        bitmap->align_x =  72;
        bitmap->align_y = 182;
        bitmap++;
        
        //LEFT
        bitmap->head  = debug_load_bmp(thread, read_file_callback, "../res/images/shadow_left_head.bmp");
        bitmap->torso = debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_left_torso.bmp");
        bitmap->cape  = debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_left_cape.bmp");
        bitmap->align_x =  72;
        bitmap->align_y = 182;
        bitmap++;
        
        //FRONT
        bitmap->head  = debug_load_bmp(thread, read_file_callback, "../res/images/shadow_front_head.bmp");
        bitmap->torso = debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_front_torso.bmp");
        bitmap->cape  = debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_front_cape.bmp");
        bitmap->align_x =  72;
        bitmap->align_y = 182;
        
        
        
        //~ WORLD GENERATION
        
        MemoryArena_init(&game_state->world_arena,
                         sge_memory->permanent_storage_size  - sizeof(GameState),
                         (u8 *)sge_memory->permanent_storage + sizeof(GameState));
        
        
        game_state->world = MEMORY_ARENA_PUSH_STRUCT(&game_state->world_arena, World); 
        World *the_world  = game_state->world;
        
        the_world->tilemap = MEMORY_ARENA_PUSH_STRUCT(&game_state->world_arena, Tilemap);
        int test = 2;
        test += 2;
        Tilemap *tilemap  = the_world->tilemap;
        
        
        tilemap->chunk_shift      = 4; //TILE_DEFAULT_CHUNK_SHIFT;
        tilemap->chunk_mask       = (1 << tilemap->chunk_shift) - 1; //TILE_DEFAULT_CHUNK_MASK ;
        tilemap->chunk_dimensions = (1 << tilemap->chunk_shift); //1 << TILE_DEFAULT_CHUNK_SHIFT; //256
        
        tilemap->tilechunk_count_x = 128;
        tilemap->tilechunk_count_y = 128;
        tilemap->tilechunk_count_z =   3;
        
        tilemap->tilechunks =
            MEMORY_ARENA_PUSH_ARRAY(&game_state->world_arena,
                                    tilemap->tilechunk_count_x *
                                    tilemap->tilechunk_count_y *
                                    tilemap->tilechunk_count_z,
                                    TileChunk);
        
        tilemap->tile_side_in_meters   =   1.4f;
        
        u32 tiles_per_chunk_width  = 17;
        u32 tiles_per_chunk_height =  9;
        
        u32 random_number_index = 0;
#if 0
        u32 screenx = (u32)0xFFFFFFFF / 2;
        u32 screeny = (u32)0xFFFFFFFF / 2;
#else
        u32 screenx = 0;
        u32 screeny = 0;
#endif
        
        u32 tile_abs_z = 0;
        
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
                if(tile_abs_z  == 0)
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
                    
                    Tile_set_tile_value(&game_state->world_arena,
                                        the_world->tilemap,
                                        tile_abs_x, tile_abs_y, tile_abs_z,
                                        tile_value);
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
                if(tile_abs_z == 0)
                {
                    tile_abs_z = 1;
                }
                else
                {
                    tile_abs_z = 0;
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
        
        sge_memory->is_initialized = 1;
    }
    
    World *the_world  = game_state->world;
    Tilemap *tilemap = the_world->tilemap;
    
    s32 tile_side_in_pixels = 60;
    f32 meters_to_pixels    = ((f32)tile_side_in_pixels / (f32)tilemap->tile_side_in_meters);
    
    f32 lower_left_x = -(f32)tile_side_in_pixels / (f32)2;
    f32 lower_left_y =  (f32)back_buffer->height;
    
    // ************************************************
    // INPUT RESPONSE
    //*************************************************
    for(u32 controller_index = 0; controller_index < 5; controller_index++)
    {
        game_controller_input *controller = &input->controllers[controller_index];
        
        Entity *controlled_entity = entity_get(game_state, game_state->player_controller_entity_index[controller_index]);
        
        if(controlled_entity)
        {
            v2 player_accel = { 0.0f };
            
            if(controller->is_analog)
            {
                // NOTE(MIGUEL): use analog mov tunning
                player_accel = (v2){ controller->stick_avg_x,controller->stick_avg_y };
                
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
            
            player_move(game_state, controlled_entity, input->delta_t, player_accel);
        }
        else
        {
            if(controller->button_start.ended_down)
            { 
                u32 entity_index = entity_add(game_state);
                player_init(game_state,  entity_index);
                game_state->player_controller_entity_index[controller_index] = entity_index; 
            }
        }
    } /// END OF INPUT LOOP
    
    
    Entity *camera_following_entity = entity_get(game_state, game_state->camera_following_entity_index);
    
    if(camera_following_entity)
    {
        
        game_state->camera_pos.tile_abs_z = camera_following_entity->pos.tile_abs_z;
        
        TilemapDifference diff = Tile_subtract(tilemap,
                                               &camera_following_entity->pos,
                                               &game_state->camera_pos);
        
        
        if(diff.dxy.x > (9.0f * tilemap->tile_side_in_meters))
        {
            game_state->camera_pos.tile_abs_x += 17; 
        }
        if(diff.dxy.x < -(9.0f * tilemap->tile_side_in_meters))
        {
            game_state->camera_pos.tile_abs_x -= 17; 
        }
        if(diff.dxy.y > (5.0f * tilemap->tile_side_in_meters))
        {
            game_state->camera_pos.tile_abs_y += 9; 
        }
        if(diff.dxy.y < -(5.0f * tilemap->tile_side_in_meters))
        {
            game_state->camera_pos.tile_abs_y -= 9; 
        }
    }
    
    
    /// debug purp background clear
    game_draw_rectangle(back_buffer,
                        (v2){0.0f, 0.0f} ,
                        (v2){(f32)back_buffer->width,(f32)back_buffer->height},
                        0.4f, 0.8f, 1.0f);
    
    
    //game_draw_bitmap(back_buffer, &game_state->back_drop, 0.0f, 0.0f, 0, 0);
    
    
    f32 screen_center_x = 0.5f * (f32)back_buffer->width;
    f32 screen_center_y = 0.5f * (f32)back_buffer->height;
    
    s32 camera_range_x = 100;
    s32 camera_range_y = 200;
    for(s32 rel_row = -camera_range_x; rel_row < camera_range_x; rel_row++)
    {
        for(s32 rel_column = -camera_range_y; rel_column < camera_range_y; rel_column++)
        {
            u32 column = game_state->camera_pos.tile_abs_x + rel_column;
            u32 row    = game_state->camera_pos.tile_abs_y + rel_row;
            
            u32 tileid = Tile_get_tile_value(tilemap, column, row, game_state->camera_pos.tile_abs_z );
            
            if(tileid > 0)
            {
                f32 gray = 0.2f;
                
                //circle drawing shinanegans
                f32 r = 20.0f; // * sinf(input->delta_t);
                f32 h = 4.0f;
                f32 k = 3.0f;
                
                f32 center_x = (screen_center_x -
                                (meters_to_pixels * game_state->camera_pos.tile_rel_.x) + ((f32)rel_column           * tile_side_in_pixels));
                
                f32 center_y = (screen_center_y +
                                (meters_to_pixels * game_state->camera_pos.tile_rel_.y) -
                                ((f32)rel_row     * tile_side_in_pixels));
                
                f32 min_x = center_x - 0.5f * tile_side_in_pixels;
                f32 min_y = center_y - 0.5f * tile_side_in_pixels;
                f32 max_x = center_x + 0.5f * tile_side_in_pixels;
                f32 max_y = center_y + 0.5f * tile_side_in_pixels;
                
                // FLOOR & WALLS
                if(((s32)column == ceiling_f32_to_s32(((-sqrtf(r * 2 - powf(row - h, 2))) + k))) ||
                   ((s32)column == ceiling_f32_to_s32(  (sqrtf(r * 2 - powf(row - h, 2)) + k))))
                {
                    gray = 0.4f;
                }
                
                if(tileid == 1)
                {
                    // FLOOR
#if 1
                    game_draw_rectangle(back_buffer,
                                        (v2){min_x,min_y},
                                        (v2){max_x,max_y},
                                        gray + 0.1f, gray + 0.5f, gray);
#endif
                }
                else if(tileid == 2) 
                {
                    // WALL
                    gray = 0.8f;
                    
                    game_draw_rectangle(back_buffer,
                                        (v2){min_x,min_y},
                                        (v2){max_x,max_y},
                                        gray, gray, gray);
                }
                else if(tileid == 3)
                {
                    // UP LADDER 
                    gray = 0.85f;
                    
                    game_draw_rectangle(back_buffer,
                                        (v2){min_x,min_y},
                                        (v2){max_x,max_y},
                                        gray, gray + 0.09f, gray + 0.1f);
                }
                else if(tileid == 4)
                {
                    // UP LADDER 
                    gray = 0.15f;
                    
                    game_draw_rectangle(back_buffer,
                                        (v2){min_x,min_y},
                                        (v2){max_x,max_y},
                                        gray, gray, gray);
                }
                
                
                Entity *entity = &game_state->entities[1];
                
                for(u32 entity_index = 0;
                    entity_index < game_state->entity_count; entity_index++, entity++)
                {
                    if(entity->exists &&
                       (entity->pos.tile_abs_z == game_state->camera_pos.tile_abs_z) &&
                       (entity->pos.tile_abs_x == column) && 
                       (entity->pos.tile_abs_y == row))
                    {
                        //DEBUG - PLAYERS CURRENT TILE
                        f32 gray = 0.0f;
                        
                        // SHADOW
                        game_draw_rectangle(back_buffer,
                                            (v2){min_x,min_y},
                                            (v2){max_x,max_y},
                                            gray + 0.1f, gray + 0.5f, gray);
                    }
                }
            }
        }
    }
    
    
    //DRAW PLAYER
    
    Entity *entity = &game_state->entities[1];
    
    for(u32 entity_index = 0;
        entity_index < game_state->entity_count; entity_index++, entity++)
    {
        if(entity->exists && (entity->pos.tile_abs_z == game_state->camera_pos.tile_abs_z))
        {
            TilemapDifference diff = Tile_subtract(tilemap,
                                                   &entity->pos,
                                                   &game_state->camera_pos);
            
            f32 player_left = (screen_center_x - (0.5f * meters_to_pixels * entity->width ));
            f32 player_top  = (screen_center_y - (1.0f * meters_to_pixels * entity->height));
            
            f32 player_ground_point_x = screen_center_x + meters_to_pixels * diff.dxy.x;
            f32 player_ground_point_y = screen_center_y - meters_to_pixels * diff.dxy.y;
            
            v2 player_dimensions = { entity->width, entity->height };
            v2 player_bottom_right = { 0 };
            v2 player_top_left = 
            {
                player_ground_point_x - 0.5f * meters_to_pixels * entity->width, 
                player_ground_point_y - meters_to_pixels * entity->height
            };
            
            v2_scale(meters_to_pixels, &player_dimensions);
            v2_add(player_top_left, player_dimensions, &player_bottom_right);
            
            
            game_draw_rectangle(back_buffer,
                                player_top_left,
                                player_bottom_right,
                                0.9f, 0.9f, 0.0f);
            
            player_bitmaps *playerbitmaps = &game_state->playerbitmaps[entity->facing_direction];
            game_draw_bitmap(back_buffer, &playerbitmaps->torso,
                             player_ground_point_x, player_ground_point_y,
                             playerbitmaps->align_x, playerbitmaps->align_y);
            
            game_draw_bitmap(back_buffer, &playerbitmaps->cape ,
                             player_ground_point_x, player_ground_point_y,
                             playerbitmaps->align_x, playerbitmaps->align_y);
            
            game_draw_bitmap(back_buffer, &playerbitmaps->head,
                             player_ground_point_x, player_ground_point_y,
                             playerbitmaps->align_x, playerbitmaps->align_y);
        }
    }
    
#if 1
    game_draw_mini_map(game_state,
                       back_buffer,
                       6,
                       200,
                       100,
                       300, 140);
#endif
    return;
}


internal void game_update_sound_buffer(GameState *state, game_sound_output_buffer *sound_buffer, u32 tone_hz)
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
        u16 sample_value = 0;
#endif
        
        *sample_out++ = sample_value;
        *sample_out++ = sample_value;
        
#if 0
        t_sin += 2.0f * PI_32BIT * 1.0f / (f32)wave_period;
#endif
    }
    
    return;
} 


internal void
game_draw_rectangle(game_back_buffer *buffer,
                    v2 min, v2 max, f32 r, f32 g, f32 b)
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
            if(x > (max_x - 2))
            {
                *pixel = 0xAAaaaaaa;
            }
            if(y > (max_y - 2))
            {
                *pixel = 0xAAaaaaaa;
            }
            
            pixel++;
        }
        
        line += buffer->pitch;
    }
    
    return;
}

#if 0
internal void game_render_weird_gradient(game_back_buffer *buffer, s32 x_offset, s32 y_offset, f32 *delta_t)
{
    u8 *line = ((u8*)buffer->data);
    
    for(u32 y = 0; y < buffer->height; y++) 
    {
        
        u32 *pixel  = (u32 *)line;
        
        for(u32 x = 0; x < buffer->width; x++) 
        {
            *pixel = (// BLUE
                      ((( x_offset)  <<  0) & 0x000044FF) | 
                      // GREEN
                      (((u32)(x * sinf(PI_32BIT))          <<  8) & 0x0000FF00) | 
                      // RED
                      (((x + x_offset)  << 16) & 0x00FF0000) |
                      /// ALPHA
                      ((0x00            << 24) & 0xFF000000) );
            
            if(0 )
            {
                f32 wav_res = (f32)y_offset / 10.0f;
                f32 norm_x = (f32)x / (f32)buffer->width ;
                f32 norm_y = (f32)y / (f32)buffer->height;
                
                f32 y_comp = norm_y;
                f32 x_comp = sinf(*delta_t * 4.0f) * sinf(2.0f * PI_32BIT * norm_x + x_offset * *delta_t) + 0.5f;
                
                //printf("N-Y: %f | X-CMP: %f  \n", y_comp, x_comp);
                if((s32)(y_comp * wav_res) >= (s32)(x_comp * wav_res))
                {
                    *pixel = (// BLUE
                              (((30)  <<  0) & 0x000044FF) | 
                              // GREEN
                              (((u32)(80 * sin(*delta_t))          <<  8) & 0x0000FF00) | 
                              // RED
                              (((x + x_offset)  << 16) & 0x00FF0000) |
                              /// ALPHA
                              ((0x00            << 24) & 0xFF000000) );
                }
                
                
                if((s32)(y_comp * wav_res) == (s32)(x_comp * wav_res))
                {
                    *pixel = (// BLUE
                              (((30 + y_offset)  <<  0) & 0x000044FF) | 
                              // GREEN
                              (((u32)(80 * sin(*delta_t))          <<  8) & 0x0000FF00) | 
                              // RED
                              (((x + x_offset)  << 16) & 0x00FF0000) |
                              /// ALPHA
                              ((0x00            << 24) & 0xFF000000) );
                }
            }
            
            pixel++;
        }
        
        *delta_t += PI_32BIT / 90000;
        line += buffer->pitch;
    }
    
    return;
}
#endif
