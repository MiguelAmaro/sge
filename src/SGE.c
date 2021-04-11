#include "SGE.h"

#include "SGE_Platform.h"
#include "LAL.h"


#define PI_32BIT 3.14159265359


// TODO(MIGUEL): App cannnot Crash when stick is not connected
// TODO(MIGUEL): App cannnot Crash when MCU is not connected
// TODO(MIGUEL): App should give use an oppertunity to connect a device(stick, mcu) thoughout app life time
// TODO(MIGUEL): Integrate FreeType library
// TODO(MIGUEL): Implement a simple gui
// TODO(MIGUEL): GUI: No cursor, Buttons with key
// TODO(MIGUEL): Integrate Opencv once app has access to camera feed
// NOTE(MIGUEL): Check Network.h for Networking TODOS


global App *app = 0; // NOTE(MIGUEL): old deprecated shit
global Platform *platform = 0; // NOTE(MIGUEL): old deprecated shit


internal void game_update_sound_buffer(game_memory *sge_memory, game_sound_output_buffer *sound_buffer)
{
    game_state *sge_state = (game_state *)sge_memory->permanent_storage;
    
    local_persist f32 t_sin;
    
    u16 tone_hz     = sge_state->tone_hz;
    u16 tone_volume = 100;
    u16 wave_period = sound_buffer->samples_per_second / tone_hz;
    
    u16 *sample_out = sound_buffer->samples;
    
    for(u32 sample_index = 0; sample_index < sound_buffer->sample_count; sample_index++)
    {
        f32 sine_value   = sin(t_sin);
        u16 sample_value = (u16)(sine_value * tone_volume);
        
        *sample_out++ = sample_value;
        *sample_out++ = sample_value;
        
        t_sin += 2.0f * PI_32BIT * 1.0f / (f32)wave_period;
    }
    
    return;
}

internal void
game_render_weird_player(game_back_buffer *buffer, s32 player_x, s32 player_y)
{
    // TODO(MIGUEL): study this function
    u8 *end_of_buffer = ((u8 *)(buffer->data)     + 
                         (buffer->bytes_per_pixel * 
                          buffer->width)          +
                         (buffer->height          *
                          buffer->pitch));
    s32 top    = player_y;
    s32 bottom = player_y + 10;
    u32 color  = 0xFFff00ff;
    
    for(u32 x = player_x; x < player_x + 10; x++ )
    {
        u8 *pixel = (( u8 *)buffer->data             +
                     (  x * buffer->bytes_per_pixel) +
                     (top * buffer->pitch          ));
        
        for(u32 y = top; y < bottom; y++)
        {
            if((pixel >= buffer->data) &&
               (pixel < end_of_buffer))
            {
                *(u32 *)pixel = ((color * player_x ) ^ player_y);
                
            }
            
            pixel += buffer->pitch;
        }
    }
    
    return;
}

SGE_GET_SOUND_SAMPLES(SGEGetSoundSamples)
{
    game_update_sound_buffer(sge_memory, sound_buffer);
    
    return;
}

SGE_INIT(SGEInit)
{
    return;
}

SGE_UPDATE(SGEUpdate)
{
#if RION
    /*
    b32 app_should_quit = 0;
    platform = platform_   ;
    
    
    app = platform->permanent_storage;
    {
        //~ Handmade Hero stuff
        
        f32 move_speed  = -200.0f *  app->delta_time;
        app->delta_time = platform->current_time - platform->last_time;
        
        {
            glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, app->player_translate);
        }
        
        //~
        if(platform->key_down[KEY_c])
        {
            app_should_quit = 1;
        }
        if(platform->key_down[KEY_w])
        {
            glm_vec3_copy((vec3){0.0f, move_speed * 2, 0.0f}, app->player_translate);
            printf("shit \n");
        }
        
        if(platform->key_down[KEY_s])
        {
            glm_vec3_copy((vec3){0.0f, -move_speed * 8, 0.0f}, app->player_translate);
            printf("s\n");
        }
        
        if(platform->key_down[KEY_a])
        {
            glm_vec3_copy((vec3){move_speed, 0.0f, 0.0f}, app->player_translate);
            printf("a\n");
            
        }
        
        if(platform->key_down[KEY_d])
        {
            glm_vec3_copy((vec3){-move_speed, 0.0f, 0.0f}, app->player_translate);
            printf("d\n");
        }
        
        if(platform->key_down[KEY_q])
        {
            //glm_rotate(rotation, glm_rad(10.0f),(vec3){0.0f, 1.0f, 0.0f}); 
            printf("q\n");
        }
        
        
        if(platform->key_down[KEY_e])
        {
            //glm_rotate(rotation, glm_rad(10.0f),(vec3){0.0f, 1.0f, 0.0f}); 
            printf("e\n");
        }
        {
            //FLIGHTSTICK STUFFF
                        printf("Stick X: %d | Stick Y: %d \n\r", platform->stick_x, platform->stick_y);
                        u8 turn_value = ( 255 * ( (f32)platform->stick_x / 65535 ) );
                        printf("Turn Value: %d \n", turn_value);
                                                
                        win32_send_Message( &turn_value, 1);
        }
        
        // TODO(MIGUEL): Use the player_translate vector to calculate players new position
        game_render_weird_gradient(back_buffer, (platform->blue_offset), (platform->green_offset));
        
        return app_should_quit;
    }
    */
#else
    // NOTE(MIGUEL): CASEY HMH
    game_state *sge_state = (game_state *)sge_memory->permanent_storage;
    
    ASSERT((&input->controllers[0].terminator - &input->controllers[0].buttons[0]) ==
           (ARRAYCOUNT(input->controllers[0].buttons)));
    
    ASSERT(sizeof(sge_state) <= sge_memory->permanent_storage_size);
    
    
    if(!sge_memory->is_initialized)
    {
        sge_state->tone_hz      = 256;
        sge_state->offset_blue  =   0;
        sge_state->offset_green =   0;
        
        
        sge_state->player_x = 1280 / 2;
        sge_state->player_y = 720 - 120;
        
        
        sge_memory->is_initialized = 1 ;
        /*
        u8 *file_name = __FILE__;
                
        debug_read_file_result file = sge_memory->debug_platform_read_entire_file(file_name);
                
        if(file.contents)
        {
            sge_memory->debug_platform_write_entire_file("F:\\Dev\\SimpleGameEngine\\res\\test.out", file.contents_size, file.contents);
            sge_memory->debug_platform_free_file_memory (file.contents);
    }
*/
    }
    
    // ************************************************
    // INPUT RESPONSE
    //*************************************************
    for(u32 controller_index = 0; controller_index < 5; controller_index++)
    {
        game_controller_input *controller = &input->controllers[controller_index];
        
        if(controller->is_analog)
        {// NOTE(MIGUEL): analouge movement tunning
            //printf("%f \n", controller->stick_avg_x);
            /*glm_vec3_copy(
                        (vec3) {
                            8 * -(controller->stick_avg_y),
                            8 *   controller->stick_avg_x ,
                            0.0f
                        },  app->player_translate);
            */
            sge_state->offset_green += (s32)(40.0f *-(controller->stick_avg_y));
            sge_state->offset_blue  += (s32)(40.0f *  controller->stick_avg_x );
            
            sge_state->tone_hz = 512 + (u32)( 256.0f * controller->stick_avg_y);
            
        }
        else
        {// NOTE(MIGUEL): digital movement tunning
            if(controller->button_y.ended_down)
            {
                sge_state->offset_green -= 10;
                sge_state->player_y     -= 60;
            }
            if(controller->button_a.ended_down)
            {
                sge_state->offset_green += 10;
                sge_state->player_y     += 60;
            }
            if(controller->button_x.ended_down)
            {
                sge_state->offset_blue -= 10;
                sge_state->player_x    -= 60;
            }
            if(controller->button_b.ended_down)
            {
                sge_state->offset_blue += 10;
                sge_state->player_x    += 60;
            }
        }
        
        
        sge_state->player_x  += (s32)(60.0f *  controller->stick_avg_x );
        sge_state->player_y  += (s32)(60.0f * -controller->stick_avg_y );
        
        if(sge_state->t_jump > 0)
        {
            sge_state->player_y += (s32)(5.0f * sinf(0.5f * PI_32BIT * sge_state->t_jump));
        }
        if(controller->action_up.ended_down)
        {
            sge_state->t_jump = 4.0;
        }
        sge_state->t_jump -= 0.033f;
        
    }
    
    game_render_weird_gradient(back_buffer, (sge_state->offset_green), (sge_state->offset_blue), &(sge_state->t_sin));
    game_render_weird_player  (back_buffer, (sge_state->player_x)    , (sge_state->player_y));
    
    //~ END OF Handmade Hero stuff
#endif
    
    return;
}

// NOTE(MIGUEL): Platform Independent
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
            
            if(1)
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


