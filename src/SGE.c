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


global App *app = 0;
global Platform *platform = 0;

internal void game_update_sound_buffer(game_sound_output_buffer *sound_buffer, u32 tone_hz)
{
    local_persist f32 t_sin;
    //u16 tone_hz     = 256;
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

SGE_INIT(SGEInit)
{
    return;
}

SGE_UPDATE(SGEUpdate)
{
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
        
        // ************************************************
        // INPUT RESPONSE
        //*************************************************
        for(u32 controller_index = 0; controller_index < 4; controller_index++)
        {
            game_controller_input *controller = &input->controllers[controller_index];
            
            if(controller->is_analog)
            {// NOTE(MIGUEL): analouge movement tunning
                //printf("%f \n", controller->stick_avg_x);
                glm_vec3_copy(
                              (vec3) {
                                  move_speed * 8 * -(controller->stick_avg_y),
                                  move_speed * 8 *   controller->stick_avg_x ,
                                  0.0f
                              },  app->player_translate);
                
                platform->green_offset +=  move_speed * -(controller->stick_avg_y);
                platform->blue_offset  +=  move_speed *   controller->stick_avg_x ;
                
                tone_hz = 512 + (u32)( 256.0f * controller->stick_avg_y);
            }
            else
            {// NOTE(MIGUEL): digital movement tunning
                if(controller->button_x.ended_down)
                {
                    glm_vec3_copy((vec3){ move_speed * 8 * 100, 0.0f, 0.0f }, 
                                  app->player_translate);
                    
                }
                if(controller->button_y.ended_down)
                {
                    glm_vec3_copy((vec3){ 0.0f, move_speed * 8 * 100, 0.0f }, 
                                  app->player_translate);
                }
            }
        }
        //~ END OF Handmade Hero stuff
        
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
            /*
                        printf("Stick X: %d | Stick Y: %d \n\r", platform->stick_x, platform->stick_y);
                        u8 turn_value = ( 255 * ( (f32)platform->stick_x / 65535 ) );
                        printf("Turn Value: %d \n", turn_value);
                        
                        win32_send_Message( &turn_value, 1);
                        */
        }
        
        game_update_sound_buffer(sound_buffer, tone_hz);
        
        // TODO(MIGUEL): Use the player_translate vector to calculate players new position
        game_render_weird_gradient(back_buffer, (platform->blue_offset), (platform->green_offset));
    }
    
    
    return app_should_quit;
}

// NOTE(MIGUEL): Platform Independent
internal void game_render_weird_gradient(game_back_buffer *buffer, s32 x_offset, s32 y_offset)
{
    u8 *line = ((u8*)buffer->data);
    
    for(u32 y = 0; y < buffer->height; y++) 
    {
        u32 *pixel  = (u32 *)line;
        
        for(u32 x = 0; x < buffer->width; x++) 
        {
            *pixel++ = (// BLUE
                        (((x + x_offset)  <<  0) & 0x000000FF) | 
                        // GREEN
                        (((y + y_offset)  <<  8) & 0x0000FF00) | 
                        // RED
                        ((0x00            << 16) & 0x00FF0000) |
                        /// ALPHA
                        ((0x00            << 24) & 0xFF000000) );
        }
        
        line += buffer->pitch;
    }
    
    return;
}


