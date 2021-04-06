/* date = January 4th 2021 6:48 pm */

#ifndef FLIGHTCONTROL_PLATFORM_H
#define FLIGHTCONTROL_PLATFORM_H

#include "LAL.h"

// NOTE(MIGUEL): This is an Xmacro. Do some research on this
enum {
#define key(name, str) KEY_##name,
#include "Platform_Key_List.inc"
    KEY_MAX
};

typedef struct Platform
{
    b32 quit;
    u32 window_height;
    u32 window_width;
    s32 blue_offset ;
    s32 green_offset;
    
    // STORAGE
    void *permanent_storage;
    u32   permanent_storage_size;
    void *transient_storage;
    u32   transient_storage_size;
    
    // INPUT
    b32 key_down[KEY_MAX];
    // NOTE(MIGUEL): replace by handame hero implementation
    f32 frames_per_second_target;
    f32 mouse_x;
    f32 mouse_y;
    b32 mouse_leftb_down;
    b32 mouse_leftb_pressed;
    b32 mouse_rightb_down;
    b32 mouse_rightb_pressed;
    s32 stick_x;
    s32 stick_y;
    
    // TIME
    f32 current_time;
    f32 last_time   ;
    
    f32 r;
    f32 g;
    f32 b;
} Platform;




#endif //FLIGHTCONTROL_PLATFORM_H
