#include "sge.h"


// TODO(MIGUEL): App cannnot Crash when stick is not connected
// TODO(MIGUEL): App cannnot Crash when MCU is not connected
// TODO(MIGUEL): App should give use an oppertunity to connect a device(stick, mcu) thoughout app life time
// TODO(MIGUEL): Integrate FreeType library
// TODO(MIGUEL): Implement a simple gui
// TODO(MIGUEL): GUI: No cursor, Buttons with key
// TODO(MIGUEL): Integrate Opencv once app has access to camera feed
// NOTE(MIGUEL): Check Network.h for Networking TODOS
//fuck git

inline s32
round_f32_to_s32   (f32 value);

inline u32
round_f32_to_u32   (f32 value);

inline s32
truncate_f32_to_s32(f32 value); 

inline s32
floor_f32_to_s32(f32 value); 


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

//~ TILEMAP INTERFACE

inline u32
tilemap_get_tile_value_unchecked(world *the_world, tile_map *tilemap, s32 tile_x, s32 tile_y)
{
    ASSERT(tilemap);
    ASSERT((tile_x >= 0) && (tile_x < the_world->count_x) &&
           (tile_y >= 0) && (tile_y < the_world->count_y));
    
    u32 tilemap_value = tilemap->tiles[tile_y * the_world->count_x + tile_x];
    
    return(tilemap_value);
}

internal b32
tilemap_is_point_occupied(world *the_world, tile_map *tilemap, f32 test_x, f32 test_y)
{
    b32 is_occupied = 1;
    
    if(tilemap)
    {
        if((test_x >= 0) && (test_x < the_world->count_x) &&
           (test_y >= 0) && (test_y < the_world->count_y))
        {
            u32 tilemap_value = tilemap_get_tile_value_unchecked(the_world, tilemap, test_x, test_y);
            is_occupied = (tilemap_value == 1);
        }
    }
    
    return is_occupied;
}


//~ WORLD INTERFACE

inline tile_map * 
world_get_tilemap(world *the_world, s32 tilemap_x, s32 tilemap_y)
{
    tile_map *tilemap = NULLPTR;
    
    if((tilemap_x >= 0) && (tilemap_x < the_world->tilemap_count_x) &&
       (tilemap_y >= 0) && (tilemap_y < the_world->tilemap_count_y))
    {
        tilemap = &the_world->tilemaps[tilemap_y * the_world->tilemap_count_x + tilemap_x];
    }
    
    return tilemap;
}

inline canonical_position
world_get_canonical_position(world *the_world, raw_position pos)
{
    canonical_position result;
    
    result.tilemap_x = pos.tilemap_x;
    result.tilemap_y = pos.tilemap_y;
    
    f32 x = pos.x - the_world->upper_left_x; 
    f32 y = pos.y - the_world->upper_left_y;
    result.tile_x = floor_f32_to_s32((x) / the_world->tile_width );
    result.tile_y = floor_f32_to_s32((y) / the_world->tile_height);
    
    result.x = x - result.tile_x * the_world->tile_width;
    result.y = y - result.tile_y * the_world->tile_height;
    
    ASSERT(result.x >= 0);
    ASSERT(result.y >= 0);
    ASSERT(result.x < the_world->tile_width);
    ASSERT(result.y < the_world->tile_height);
    
    if(result.tile_x < 0)
    {
        result.tile_x = the_world->count_x + result.tile_x;
        --result.tilemap_x;
    }
    if(result.tile_y < 0)
    { 
        result.tile_y = the_world->count_x + result.tile_y;
        --result.tilemap_y;
    }
    if(result.tile_x >= the_world->count_x)
    {
        result.tile_x = result.tile_x - the_world->count_x;
        ++result.tilemap_x;
    }
    if(result.tile_y >= the_world->count_y)
    {
        result.tile_y = result.tile_y - the_world->count_y;
        ++result.tilemap_y;
    }
    
    return result;
}

internal b32
world_is_point_empty(world *the_world, raw_position test_pos)
{
    b32 is_empty = 0;
    
    canonical_position can_pos = world_get_canonical_position(the_world, test_pos); 
    tile_map *tilemap = world_get_tilemap(the_world, can_pos.tilemap_x, can_pos.tilemap_y);
    is_empty = !tilemap_is_point_occupied(the_world, tilemap, can_pos.tile_x, can_pos.tile_y);
    
    return is_empty;
}

SGE_UPDATE(SGEUpdate)
{
    ASSERT((&input->controllers[0].terminator - &input->controllers[0].buttons[0]) ==
           (ARRAYCOUNT(input->controllers[0].buttons)));
    
    ASSERT(sizeof(game_state) <= sge_memory->permanent_storage_size);
    
    
    
#define TILE_MAP_COUNT_X (17)
#define TILE_MAP_COUNT_Y ( 9)
    // NOTE(MIGUEL): numbers reference their x & y positions
    u32 tiles_00[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
        { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1 },
        { 1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1 },
        { 1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1 },
        { 1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1 },
        { 1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 0 },
        { 1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1 },
        { 1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 1,  0, 0, 0, 0, 1 },
        { 1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1 },
        { 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1 },
    };
    
    u32 tiles_01[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
        { 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1 },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1 },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 2, 0, 1 },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1 },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0 },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1 },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1 },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1 },
        { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1 },
    };
    
    
    u32 tiles_10[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
        { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1 },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1 },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 2, 0, 1 },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1 },
        { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1 },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1 },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1 },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1 },
        { 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1 },
    };
    
    
    u32 tiles_11[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
        { 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1 },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1 },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 2, 0, 1 },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1 },
        { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1 },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1 },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1 },
        { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1 },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1 },
    };
    
    tile_map tilemaps[2][2];
    
    world the_world;
    the_world.tilemap_count_x   = 2;
    the_world.tilemap_count_y   = 2;
    the_world.tilemaps = (tile_map *)tilemaps;
    
    the_world.count_x      =  TILE_MAP_COUNT_X;
    the_world.count_y      =  TILE_MAP_COUNT_Y;
    the_world.upper_left_x =  -30.0f;
    the_world.upper_left_y =    0.0f;
    the_world.tile_width   =   60.0f;
    the_world.tile_height  =   60.0f;
    
    tilemaps[0][0].tiles = (u32 *)tiles_00;
    tilemaps[0][1].tiles = (u32 *)tiles_10;
    tilemaps[1][0].tiles = (u32 *)tiles_01;
    tilemaps[1][1].tiles = (u32 *)tiles_11;
    
    
    f32 player_width  = 0.75f * the_world.tile_width; 
    f32 player_height = 1.00f * the_world.tile_height;
    
    game_state *sge_state = (game_state *)sge_memory->permanent_storage;
    if(!sge_memory->is_initialized)
    {   
        sge_state->player_x = 150;
        sge_state->player_y = 150;
        
        sge_memory->is_initialized = 1;
    }
    
    
    tile_map *tilemap = world_get_tilemap(&the_world, sge_state->player_tilemap_x, sge_state->player_tilemap_y);
    ASSERT(tilemap);
    
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
                delta_player_y = -1.0f;
            }
            
            if(controller->button_a.ended_down)
            {
                // down
                delta_player_y = 1.0f;
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
            
            delta_player_x *= 100.0;
            delta_player_y *= 100.0;
            
            f32 new_player_x = sge_state->player_x + delta_player_x * input->delta_t;
            f32 new_player_y = sge_state->player_y + delta_player_y * input->delta_t;
            
            raw_position player_pos = 
            {
                sge_state->player_tilemap_x,
                sge_state->player_tilemap_y,
                new_player_x,
                new_player_y,
            };
            
            raw_position player_left = player_pos;
            player_left.x -= 0.5f * player_width;
            
            raw_position player_right = player_pos;
            player_right.x += 0.5f * player_width;
            
            if(world_is_point_empty(&the_world, player_left ) &&
               world_is_point_empty(&the_world, player_right) &&
               world_is_point_empty(&the_world, player_pos))
            {
                canonical_position can_pos = world_get_canonical_position(&the_world, player_pos);
                
                sge_state->player_tilemap_x = can_pos.tilemap_x;
                sge_state->player_tilemap_y = can_pos.tilemap_y;
                
                sge_state->player_x = the_world.upper_left_x + the_world.tile_width * can_pos.tile_x + can_pos.x;
                sge_state->player_y = the_world.upper_left_y + the_world.tile_height * can_pos.tile_y + can_pos.y;
            }
        }
    } /// END OF INPUT LOOP
    
    /// debug purp background clear
    game_draw_rectangle(back_buffer,
                        0.0f, (f32)back_buffer->width,
                        0.0f, (f32)back_buffer->height,
                        0.1, 0.0, 1.0);
    
    
    for(u32 row = 0; row < the_world.count_y; row++)
    {
        for(u32 column = 0; column < the_world.count_x; column++)
        {
            
            u32 tileid = tilemap_get_tile_value_unchecked(&the_world, tilemap, column, row);
            
            f32 gray = 0.5f; 
            
            if(tileid == 1)
            {
                gray = 1.0f;
            }
            
            f32 min_x = the_world.upper_left_x + ((f32)column * the_world.tile_width);
            f32 min_y = the_world.upper_left_y + ((f32)row    * the_world.tile_height);	
            
            f32 max_x = min_x + the_world.tile_width ;
            f32 max_y = min_y + the_world.tile_height;
            
            game_draw_rectangle(back_buffer,
                                min_x, max_x,
                                min_y, max_y,
                                gray + 0.3, gray, gray);
        }
    }
    
    
    f32 player_left = sge_state->player_x - (0.5f * player_width );
    f32 player_top  = sge_state->player_y - (1.0f * player_height);
    
    game_draw_rectangle(back_buffer,
                        player_left, player_left + player_width,
                        player_top , player_top  + player_height,
                        0.5, 0.9, 0.3);
    return;
}


internal void game_update_sound_buffer(game_state *state, game_sound_output_buffer *sound_buffer, u32 tone_hz)
{
    local_persist f32 t_sin;
    
    u16 tone_volume = 100;
    u16 wave_period = sound_buffer->samples_per_second / tone_hz;
    
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


inline s32
round_f32_to_s32(f32 value)
{
    s32 result = (s32)(value + 0.5f);
    
    return result;
}

inline u32
round_f32_to_u32(f32 value)
{
    u32 result = (u32)(value + 0.5f);
    
    return result;
}

inline s32
truncate_f32_to_s32(f32 value)
{
    s32 result = (s32)value;
    
    return result;
}


#include "math.h"
inline s32
floor_f32_to_s32(f32 value)
{
    s32 result = (s32)floorf(value);
    
    return result;
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
            *pixel++ = color;
            
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

