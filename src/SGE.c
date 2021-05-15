#include "sge.h"
#include "sge_intrinsics.h"
#include "stdio.h"

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

//~ TILECHUNK INTERFACE

inline u32
tilechunk_get_tile_value_unchecked(world *the_world, tile_chunk *tilechunk, u32 tile_x, u32 tile_y)
{
    ASSERT(tilechunk);
    ASSERT(tile_x < the_world->chunk_dimensions);
    ASSERT(tile_y < the_world->chunk_dimensions);
    
    u32 tilemap_value = tilechunk->tiles[tile_y * the_world->chunk_dimensions + tile_x];
    
    return(tilemap_value);
}

internal b32
tilechunk_get_tile_value(world *the_world, tile_chunk *tilechunk, u32 test_x, u32 test_y)
{
    u32 tilechunk_value = 0;
    
    if(tilechunk)
    {
        tilechunk_value = tilechunk_get_tile_value_unchecked(the_world, tilechunk, test_x, test_y);
    }
    
    return tilechunk_value;
}

//~ WORLD INTERFACE

inline tile_chunk * 
world_get_tile_chunk(world *the_world, s32 tilechunk_x, s32 tilechunk_y)
{
    tile_chunk *tilechunk = NULLPTR;
    
    if((tilechunk_x >= 0) && (tilechunk_x < the_world->tilechunk_count_x) &&
       (tilechunk_y >= 0) && (tilechunk_y < the_world->tilechunk_count_y))
    {
        tilechunk = &the_world->tilechunks[tilechunk_y * the_world->tilechunk_count_x + tilechunk_x];
    }
    
    return tilechunk;
}

inline void
world_recanonicalize_coord(world *the_world, s32 *tile, f32 *tile_rel)
{
    //offset = displacement, where units are tiles
    s32 offset = floor_f32_to_s32(*tile_rel / the_world->tile_side_in_meters );
    *tile += offset;
    *tile_rel -= offset * the_world->tile_side_in_meters;
    
    // TODO(MIGUEL): fix floating point math
    ASSERT(*tile_rel >= 0);
    ASSERT(*tile_rel <= the_world->tile_side_in_meters);
    
    return;
}

inline world_position
world_recanonicalize_position(world *the_world, world_position pos)
{
    world_position result = pos;
    
    world_recanonicalize_coord(the_world, &result.tile_abs_x, &result.tile_rel_x);
    world_recanonicalize_coord(the_world, &result.tile_abs_y, &result.tile_rel_y);
    
    return result;
}

inline tile_chunk_position
world_get_tile_chunk_position(world *the_world, u32 tile_abs_x, u32 tile_abs_y)
{
    tile_chunk_position result;
    
    result.tile_chunk_x = tile_abs_x >> WORLD_CHUNK_SHIFT;
    result.tile_chunk_y = tile_abs_y >> WORLD_CHUNK_SHIFT;
    
    result.tile_rel_x   = tile_abs_x  & WORLD_CHUNK_MASK;
    result.tile_rel_y   = tile_abs_y  & WORLD_CHUNK_MASK;
    
    return result;
}


internal u32
world_get_tile_value(world *the_world, u32 tile_abs_x, u32 tile_abs_y)
{
    tile_chunk_position chunk_pos = world_get_tile_chunk_position(the_world, tile_abs_x, tile_abs_y);
    tile_chunk         *tilechunk = world_get_tile_chunk(the_world, chunk_pos.tile_chunk_x, chunk_pos.tile_chunk_y);
    //tile_chunk         *tilechunk = world_get_tile_chunk(the_world, 0, 0);
    u32           tilechunk_value = tilechunk_get_tile_value(the_world,
                                                             tilechunk,
                                                             chunk_pos.tile_rel_x,
                                                             chunk_pos.tile_rel_y);
    
    
    return tilechunk_value;
}

internal b32
world_is_point_empty(world *the_world, world_position can_pos)
{
    b32 is_empty = 0;
    
    // NOTE(MIGUEL): tile system query when rendering to screen
    u32 tilechunk_value = world_get_tile_value(the_world, can_pos.tile_abs_x, can_pos.tile_abs_y);
    is_empty = (tilechunk_value == 0);
    
    return is_empty;
}

SGE_UPDATE(SGEUpdate)
{
    ASSERT((&input->controllers[0].terminator - &input->controllers[0].buttons[0]) ==
           (ARRAYCOUNT(input->controllers[0].buttons)));
    
    ASSERT(sizeof(game_state) <= sge_memory->permanent_storage_size);
    
    
    
#define TILE_MAP_COUNT_X (256)
#define TILE_MAP_COUNT_Y (256)
    // NOTE(MIGUEL): numbers reference their x & y positions
    u32 temp_tiles[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
        {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
        {1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
        {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1}
    };
    
    world the_world;
    the_world.chunk_dimensions = 256;
    
    the_world.tilechunk_count_x = 1;
    the_world.tilechunk_count_y = 1;
    
    tile_chunk tilechunk;
    tilechunk.tiles = (u32 *)temp_tiles;
    the_world.tilechunks = &tilechunk;
    the_world.tile_side_in_pixels   =  60;
    
    the_world.tile_side_in_meters   =   1.4f;
    the_world.meters_to_pixels      =  ((f32)the_world.tile_side_in_pixels / (f32)the_world.tile_side_in_meters);
    
    f32 player_height = 1.4f;
    f32 player_width  = 0.75f * player_height; 
    
    f32 lower_left_x = -(f32)the_world.tile_side_in_pixels / (f32)2;
    f32 lower_left_y =  (f32)back_buffer->height;
    
    game_state *sge_state = (game_state *)sge_memory->permanent_storage;
    if(!sge_memory->is_initialized)
    {   
        // NOTE(MIGUEL): testing
        
        sge_state->player_pos.tile_abs_x = 10;
        sge_state->player_pos.tile_abs_y = 10;
        sge_state->player_pos.tile_rel_x = 5.0f;
        sge_state->player_pos.tile_rel_y = 5.0f;
        
        FILE *f = fopen("../res/memdata.txt", "w");
        u32 *tiles = tilechunk.tiles;
        if(f)
        {
            
#if 0
#endif
            for(u32 i = 0; i < the_world.chunk_dimensions; i++)
            {
                for(u32 j = 0; j < the_world.chunk_dimensions; j++)
                {
                    char out;
                    if(*tiles++ == 0) out = '0';
                    else                        out = '1';
                    
                    fputc(out, f);
                }
                fputc('\n', f);
            }
        }
        sge_memory->is_initialized = 1;
    }
    
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
            
            delta_player_x *= 10.0;
            delta_player_y *= 10.0;
            
            /// get prev positon from state. add input to it. test new pos. updete pos in state if valid
            world_position new_player_pos  = sge_state->player_pos;
            new_player_pos.tile_rel_x     += delta_player_x * input->delta_t;
            new_player_pos.tile_rel_y     += delta_player_y * input->delta_t;
            new_player_pos                 = world_recanonicalize_position(&the_world, new_player_pos);
            
            world_position player_left = new_player_pos;
            player_left.tile_rel_x -= 0.5f * player_width;
            player_left = world_recanonicalize_position(&the_world, player_left);
            
            world_position player_right = new_player_pos ;
            player_right.tile_rel_x += 0.5f * player_width;
            player_right = world_recanonicalize_position(&the_world, player_right);
            
            if(world_is_point_empty(&the_world, player_left ) &&
               world_is_point_empty(&the_world, player_right) &&
               world_is_point_empty(&the_world, new_player_pos))
            {
                sge_state->player_pos = new_player_pos;
                
            }
        }
    } /// END OF INPUT LOOP
    
    /// debug purp background clear
    game_draw_rectangle(back_buffer,
                        0.0f, (f32)back_buffer->width,
                        0.0f, (f32)back_buffer->height,
                        0.1f, 0.0f, 1.0f);
    
    f32 center_x = 0.5f * (f32)back_buffer->width;
    f32 center_y = 0.5f * (f32)back_buffer->height;
    
    for(s32 rel_row = -10; rel_row < 10; rel_row++)
    {
        for(s32 rel_column = -20; rel_column < 20; rel_column++)
        {
            u32 column = sge_state->player_pos.tile_abs_x + rel_column;
            u32 row    = sge_state->player_pos.tile_abs_y + rel_row;
            
            u32 tileid = world_get_tile_value(&the_world, column, row);
            
            f32 gray = 0.5f;
            //circle drawing shinanegans
            f32 r = 20.0f; // * sinf(input->delta_t);
            f32 h = 4.0f;
            f32 k = 3.0f;
            
            if(((s32)column == ceiling_f32_to_s32(((-sqrtf(r * 2 - powf(row - h, 2))) + k))) ||
               ((s32)column == ceiling_f32_to_s32(  (sqrtf(r * 2 - powf(row - h, 2)) + k))))
            {
                gray = 0.4f;
            }
            
            if(tileid == 1) 
            {
                gray = 1.0f;
            }
            
            //debug
            if((column == sge_state->player_pos.tile_abs_x) &&
               (row    == sge_state->player_pos.tile_abs_y))
            {
                gray = 0.0f;
            }
            
            f32 min_x = center_x + ((f32)rel_column * the_world.tile_side_in_pixels);
            f32 min_y = center_y - ((f32)rel_row    * the_world.tile_side_in_pixels);	
            
            f32 max_x = min_x + the_world.tile_side_in_pixels;
            f32 max_y = min_y - the_world.tile_side_in_pixels;
            
            game_draw_rectangle(back_buffer,
                                min_x, max_x,
                                max_y, min_y,
                                gray + 0.3f, gray, gray);
        }
    }
    
    //DRAW PLAYER
    f32 player_left = (center_x +
                       the_world.meters_to_pixels    * sge_state->player_pos.tile_rel_x - 
                       (0.5f * the_world.meters_to_pixels  * player_width ));
    
    
    f32 player_top  = (center_y -
                       the_world.meters_to_pixels    * sge_state->player_pos.tile_rel_y -
                       (1.0f * the_world.meters_to_pixels * player_height));
    
    game_draw_rectangle(back_buffer,
                        player_left, player_left + the_world.meters_to_pixels * player_width,
                        player_top , player_top  + the_world.meters_to_pixels * player_height,
                        0.5f, 0.9f, 0.3f);
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

