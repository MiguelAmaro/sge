/* date = January 25th 2021 0:25 pm */

#ifndef SGE_H
#define SGE_H

#include "windows.h"
#include "SGE_Platform.h"
#include "cglm/mat4.h" // NOTE(MIGUEL): kinda wanna do my own thing isstead

#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

// NOTE(MIGUEL): not imporntant right now
typedef struct
{
    void *data     ;
    s32   width    ;
    s32   height   ;
    s32   pitch    ;
} game_back_buffer;


typedef struct
{
    u16 *samples          ;
    u32 sample_count      ;
    u32 samples_per_second;
    
} game_sound_output_buffer;

////////// THESE STRUCTS ARE ALL SHIT FOR XBOX CONTROLLER A DAKLFJD;AKDJF;ADKFJ

// NOTE(MIGUEL): imporntant ones
typedef struct
{
    u32 half_trans_count; // button half transition
    b32 ended_down;
} game_button_state;

/// Example of a Discriminated Union
typedef struct
{
    b32 is_connected;
    
    /// Stick Input
    b32 is_analog   ;
    f32 stick_avg_x ;
    f32 stick_avg_y ;
    
    /// Buttons Input
    union
    {
        game_button_state buttons[12];
        struct
        {
            // NOTE(MIGUEL): THIS WASNT MEANT TO REPRESENT THE SPECIFIC BUTTON ON A CONTROLLER
            //               JUST THE BEHAVIOR THAT IS SUPPOSED TO BE INFLUENCED BY A BUTTONS
            game_button_state button_a; //Move Down
            game_button_state button_b; //Move Right
            game_button_state button_x; //Move Left
            game_button_state button_y; //Move Up
            
            game_button_state action_up   ;
            game_button_state action_down ;
            game_button_state action_right;
            game_button_state action_left ;
            
            game_button_state shoulder_left ;
            game_button_state shoulder_right;
            
            game_button_state button_start;
            game_button_state button_back ;
        };
    };
} game_controller_input;

typedef struct
{
    game_button_state mouse_buttons[5];
    
    s32 mouse_x;
    s32 mouse_y;
    s32 mouse_z;
    
    game_controller_input controllers[5];
} game_input;

typedef struct App {
    f32 delta_time;
    
    s32 player_x;
    s32 player_y;
    
    vec3 player_translate;
} App;


//~ FUNCTION DECLERATIONS

#define SGE_INIT(    name) void name(Platform *platform_)
typedef SGE_INIT(SGE_Init);
SGE_INIT(SGEInitStub)
{ return; } 
// NOTE(MIGUEL): what should the return value be??? Any value that matches the function signiture. The stub is just a place holder/ fallback if we cant load the real thing

#define SGE_UPDATE(      name) b32 name(Platform *platform_, game_input *input, game_back_buffer *back_buffer, game_sound_output_buffer *sound_buffer, u32 tone_hz)
typedef SGE_UPDATE(SGE_Update);
SGE_UPDATE(SGEUpdateStub)
{ return 0; }


internal void game_render_weird_gradient(game_back_buffer *buffer, s32 x_offset, s32 y_offset);

inline game_controller_input *get_controller(game_input *input, u32 controller_index)
{
    ASSERT(controller_index < (u32)ARRAY_COUNT(input->controllers));
    game_controller_input *result = &input->controllers[controller_index];
    
    return result;
}

#endif //SGE_H
