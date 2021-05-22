#include "sge.h"
#include "sge_random.h"
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
    game_state *sge_state = (game_state *)sge_memory->permanent_storage;
    
    game_update_sound_buffer(sge_state, sound_buffer, 400);
    
    return;
}

SGE_INIT(SGEInit)
{
    // NOTE(MIGUEL): get rid of this it makes no sense
    
    return;
}


internal void
game_draw_mini_map(game_state *state,
                   game_back_buffer *back_buffer,
                   s32 tile_side_in_pixels,
                   s32 camera_range_x,
                   s32 camera_range_y,
                   s32 x, s32 y)
{
    tile_map *tilemap = state->the_world->tilemap;
    
    f32 meters_to_pixels    = ((f32)tile_side_in_pixels / (f32)tilemap->tile_side_in_meters);
    
    f32 screen_center_x = 0.5f * (f32)back_buffer->width;
    f32 screen_center_y = 0.5f * (f32)back_buffer->height;
    
    for(s32 rel_row = -camera_range_x; rel_row < camera_range_x; rel_row++)
    {
        for(s32 rel_column = -camera_range_y; rel_column < camera_range_y; rel_column++)
        {
            u32 column = state->player_pos.tile_abs_x + rel_column;
            u32 row    = state->player_pos.tile_abs_y + rel_row;
            
            u32 tileid = tile_get_tile_value(tilemap, column, row, state->player_pos.tile_abs_z );
            
            if(tileid > 0)
            {
                f32 gray = 0.2f;
                
                //circle drawing shinanegans
                f32 r = 20.0f; // * sinf(input->delta_t);
                f32 h = 4.0f;
                f32 k = 3.0f;
                
                f32 center_x = (screen_center_x -
                                (meters_to_pixels * state->player_pos.tile_rel_x) + ((f32)rel_column           * tile_side_in_pixels));
                
                f32 center_y = (screen_center_y +
                                (meters_to_pixels * state->player_pos.tile_rel_y) -
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
                                        min_x, max_x,
                                        min_y, max_y,
                                        gray + 0.1f, gray + 0.5f, gray);
                    
                }
                else if(tileid == 2) 
                {
                    // WALL
                    gray = 0.8f;
                    
                    game_draw_rectangle(back_buffer,
                                        min_x, max_x,
                                        min_y, max_y,
                                        gray, gray, gray);
                }
                else if(tileid == 3)
                {
                    // LADDER 
                    gray = 0.0f;
                    
                    game_draw_rectangle(back_buffer,
                                        min_x, max_x,
                                        min_y, max_y,
                                        gray, gray, gray);
                }
                
                //DEBUG - PLAYERS CURRENT TILE
                if((column == state->player_pos.tile_abs_x) &&
                   (row    == state->player_pos.tile_abs_y))
                {
                    gray = 0.0f;
                    // SHADOW
                    game_draw_rectangle(back_buffer,
                                        min_x, max_x,
                                        min_y, max_y,
                                        gray + 0.1f, gray + 0.5f, gray);
                }
            }
        }
    }
    
    
    return;
}

internal void
game_draw_bitmap(game_back_buffer *buffer, bitmap_data *bitmap, f32 real_x, f32 real_y)
{
    /// rounding / ruling
    s32 min_x = round_f32_to_s32(real_x);
    s32 min_y = round_f32_to_s32(real_y);
    
    s32 max_x = round_f32_to_s32(real_x + (f32)bitmap->width );
    s32 max_y = round_f32_to_s32(real_y + (f32)bitmap->height);
    
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
    
    // NOTE(MIGUEL): why is it supposed to be pixel_height - 1
    u32 *src_row  = bitmap->pixels + (bitmap->width * (bitmap->height - 1));
    u8  *dest_row = ((u8 *)  (buffer->data)            + 
                     (min_x * buffer->bytes_per_pixel) +
                     (min_y * buffer->pitch));
    
    for(s32 y = min_y; y < max_y; y++)
    {
        u32 *dest = (u32 *)dest_row;
        u32 *src  = (u32 *)src_row;
        
        for(s32 x = min_x; x < max_x; x++)
        {
            f32 a = (f32)((*src >> 24) & 0xFF) / 255.0f;
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

internal bitmap_data
debug_load_bmp(thread_context *thread, DEBUG_PlatformReadEntireFile *read_entire_file, u8 *file_name)
{
    bitmap_data result = { 0 };
    
    debug_read_file_result read_result = read_entire_file(thread, file_name);
    
    //ASSERT(read_result.contents);
    if(read_result.contents_size > 0)
    {
        bitmap_header *header = (bitmap_header *)read_result.contents;
        
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

SGE_UPDATE(SGEUpdate)
{
    ASSERT((&input->controllers[0].terminator - &input->controllers[0].buttons[0]) ==
           (ARRAYCOUNT(input->controllers[0].buttons)));
    
    ASSERT(sizeof(game_state) <= sge_memory->permanent_storage_size);
    
    
    f32 player_height = 1.4f;
    f32 player_width  = 0.75f * player_height; 
    
    
    game_state *sge_state = (game_state *)sge_memory->permanent_storage;
    if(!sge_memory->is_initialized)
    {   
        sge_state->player_pos.tile_abs_x = 1;
        sge_state->player_pos.tile_abs_y = 3;
        sge_state->player_pos.tile_rel_x = 5.0f;
        sge_state->player_pos.tile_rel_y = 5.0f;
        
        //~ BITMAP LOADING
        sge_state->back_drop    = debug_load_bmp(thread,
                                                 sge_memory->debug_platform_read_entire_file,
                                                 "../res/images/test_background.bmp");
        sge_state->player_head  = debug_load_bmp(thread,
                                                 sge_memory->debug_platform_read_entire_file,
                                                 "../res/images/shadow_front_head.bmp");
        sge_state->player_torso = debug_load_bmp(thread,
                                                 sge_memory->debug_platform_read_entire_file,
                                                 "../res/images/test_hero_front_torso.bmp");
        sge_state->player_cape  = debug_load_bmp(thread,
                                                 sge_memory->debug_platform_read_entire_file,
                                                 "../res/images/test_hero_front_cape.bmp");
        sge_state->debug_bmp    = debug_load_bmp(thread,
                                                 sge_memory->debug_platform_read_entire_file,
                                                 "../res/images/structuredart.bmp");
        
        
        //~ WORLD GENERATION
        
        memory_arena_init(&sge_state->the_world_arena,
                          sge_memory->permanent_storage_size  - sizeof(game_state),
                          (u8 *)sge_memory->permanent_storage + sizeof(game_state));
        
        
        sge_state->the_world = MEMORY_ARENA_PUSH_STRUCT(&sge_state->the_world_arena, world); 
        world *the_world  = sge_state->the_world;
        
        the_world->tilemap = MEMORY_ARENA_PUSH_STRUCT(&sge_state->the_world_arena, tile_map);
        tile_map *tilemap  = the_world->tilemap;
        
        
        tilemap->chunk_shift      = 4; //TILE_DEFAULT_CHUNK_SHIFT;
        tilemap->chunk_mask       = (1 << tilemap->chunk_shift) - 1; //TILE_DEFAULT_CHUNK_MASK ;
        tilemap->chunk_dimensions = (1 << tilemap->chunk_shift); //1 << TILE_DEFAULT_CHUNK_SHIFT; //256
        
        tilemap->tilechunk_count_x = 128;
        tilemap->tilechunk_count_y = 128;
        tilemap->tilechunk_count_z =   3;
        
        tilemap->tilechunks =
            MEMORY_ARENA_PUSH_ARRAY(&sge_state->the_world_arena,
                                    tilemap->tilechunk_count_x *
                                    tilemap->tilechunk_count_y *
                                    tilemap->tilechunk_count_z,
                                    tile_chunk);
        
        tilemap->tile_side_in_meters   =   1.4f;
        
        u32 tiles_per_chunk_width  = 17;
        u32 tiles_per_chunk_height =  9;
        
        u32 random_number_index = 0;
        u32 screenx = 0;
        u32 screeny = 0;
        u32 tile_abs_z = 0;
        
        b32 door_left   = 0;
        b32 door_top    = 0;
        b32 door_right  = 0;
        b32 door_bottom = 0;
        b32 door_up   = 0;
        b32 door_down = 0;
        
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
                    tile_set_tile_value(&sge_state->the_world_arena,
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
    
    world *the_world  = sge_state->the_world;
    tile_map *tilemap = the_world->tilemap;
    
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
        
        if(controller->is_analog)
        {
            // NOTE(MIGUEL): use analog mov tunning
        }
        else
        {
            // NOTE(MIGUEL): use digital mov tunning
            f32 delta_player_x = 0.0;
            f32 delta_player_y = 0.0;
            
            if(controller->button_y.ended_down)
            {
                // upa
                delta_player_y = 1.0f;
                
            }
            if(controller->button_a.ended_down)
            {
                // down
                delta_player_y = -1.0f;
            }
            
            if(controller->button_x.ended_down)
            {
                // left
                delta_player_x = -1.0f;
            }
            
            if(controller->button_b.ended_down)
            {
                // right
                delta_player_x = 1.0f;
            }
            
            
            f32 player_speed = 2.0f;
            if(controller->action_up.ended_down)
            {
                // arrow up
                player_speed = 10.0f;
            }
            
            delta_player_x *= player_speed;
            delta_player_y *= player_speed;
            
            /// get prev positon from state. add input to it. test new pos. updete pos in state if valid
            tile_map_position new_player_pos  = sge_state->player_pos;
            new_player_pos.tile_rel_x     += delta_player_x * input->delta_t;
            new_player_pos.tile_rel_y     += delta_player_y * input->delta_t;
            new_player_pos                 = tile_recanonicalize_position(tilemap, new_player_pos);
            
            tile_map_position player_left = new_player_pos;
            player_left.tile_rel_x -= 0.5f * player_width;
            player_left = tile_recanonicalize_position(tilemap, player_left);
            
            tile_map_position player_right = new_player_pos ;
            player_right.tile_rel_x += 0.5f * player_width;
            player_right = tile_recanonicalize_position(tilemap, player_right);
            
            if(tile_is_point_empty(tilemap, player_left ) &&
               tile_is_point_empty(tilemap, player_right) &&
               tile_is_point_empty(tilemap, new_player_pos))
            {
                if(!tile_is_on_same_tile(&sge_state->player_pos, &new_player_pos))
                {
                    u32 new_tile_value =  tile_get_tile_value_tilemap_pos(tilemap, new_player_pos);
                    
                    if(new_tile_value == 3)
                    {
                        new_player_pos.tile_abs_z++;
                        
                    }
                    else if(new_tile_value == 4)
                    {
                        new_player_pos.tile_abs_z--;
                    }
                }
                sge_state->player_pos = new_player_pos;
                
            }
        }
    } /// END OF INPUT LOOP
    
    /// debug purp background clear
    game_draw_rectangle(back_buffer,
                        0.0f, (f32)back_buffer->width,
                        0.0f, (f32)back_buffer->height,
                        0.4f, 0.8f, 1.0f);
    
    
    game_draw_bitmap(back_buffer, &sge_state->back_drop, 100.0f, 100.0f);
    
    
    f32 screen_center_x = 0.5f * (f32)back_buffer->width;
    f32 screen_center_y = 0.5f * (f32)back_buffer->height;
    
    s32 camera_range_x = 100;
    s32 camera_range_y = 200;
    for(s32 rel_row = -camera_range_x; rel_row < camera_range_x; rel_row++)
    {
        for(s32 rel_column = -camera_range_y; rel_column < camera_range_y; rel_column++)
        {
            u32 column = sge_state->player_pos.tile_abs_x + rel_column;
            u32 row    = sge_state->player_pos.tile_abs_y + rel_row;
            
            u32 tileid = tile_get_tile_value(tilemap, column, row, sge_state->player_pos.tile_abs_z );
            
            if(tileid > 0)
            {
                f32 gray = 0.2f;
                
                //circle drawing shinanegans
                f32 r = 20.0f; // * sinf(input->delta_t);
                f32 h = 4.0f;
                f32 k = 3.0f;
                
                f32 center_x = (screen_center_x -
                                (meters_to_pixels * sge_state->player_pos.tile_rel_x) + ((f32)rel_column           * tile_side_in_pixels));
                
                f32 center_y = (screen_center_y +
                                (meters_to_pixels * sge_state->player_pos.tile_rel_y) -
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
#if 0
                    game_draw_rectangle(back_buffer,
                                        min_x, max_x,
                                        min_y, max_y,
                                        gray + 0.1f, gray + 0.5f, gray);
#endif
                }
                else if(tileid == 2) 
                {
                    // WALL
                    gray = 0.8f;
                    
                    game_draw_rectangle(back_buffer,
                                        min_x, max_x,
                                        min_y, max_y,
                                        gray, gray, gray);
                }
                else if(tileid == 3)
                {
                    // UP LADDER 
                    gray = 0.85f;
                    
                    game_draw_rectangle(back_buffer,
                                        min_x, max_x,
                                        min_y, max_y,
                                        gray, gray + 0.09f, gray + 0.1f);
                }
                else if(tileid == 4)
                {
                    // UP LADDER 
                    gray = 0.15f;
                    
                    game_draw_rectangle(back_buffer,
                                        min_x, max_x,
                                        min_y, max_y,
                                        gray, gray, gray);
                }
                
                //DEBUG - PLAYERS CURRENT TILE
                if((column == sge_state->player_pos.tile_abs_x) &&
                   (row    == sge_state->player_pos.tile_abs_y))
                {
                    gray = 0.0f;
                    // SHADOW
                    game_draw_rectangle(back_buffer,
                                        min_x, max_x,
                                        min_y, max_y,
                                        gray + 0.1f, gray + 0.5f, gray);
                }
            }
        }
    }
    
    //DRAW PLAYER
    f32 player_left = (screen_center_x - (0.5f * meters_to_pixels * player_width ));
    f32 player_top  = (screen_center_y - (1.0f * meters_to_pixels * player_height));
    
    game_draw_rectangle(back_buffer,
                        player_left, player_left + meters_to_pixels * player_width,
                        player_top , player_top  + meters_to_pixels * player_height,
                        0.9f, 0.9f, 0.0f);
    
    game_draw_bitmap(back_buffer, &sge_state->player_torso, player_left, player_top);
    game_draw_bitmap(back_buffer, &sge_state->player_cape, player_left, player_top);
    game_draw_bitmap(back_buffer, &sge_state->player_head, player_left, player_top);
    game_draw_bitmap(back_buffer, &sge_state->debug_bmp, 1000.0, 1000.0);
    
    
    
#if 1
    game_draw_mini_map(sge_state,
                       back_buffer,
                       6,
                       200,
                       100,
                       300, 140);
#endif
    return;
}


internal void game_update_sound_buffer(game_state *state, game_sound_output_buffer *sound_buffer, u32 tone_hz)
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
                    f32 real_min_x, f32 real_max_x,
                    f32 real_min_y, f32 real_max_y,
                    f32 r, f32 g, f32 b)
{
    /// rounding / ruling
    s32 min_x = round_f32_to_s32(real_min_x);
    s32 min_y = round_f32_to_s32(real_min_y);
    
    s32 max_x = round_f32_to_s32(real_max_x);
    s32 max_y = round_f32_to_s32(real_max_y);
    
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

