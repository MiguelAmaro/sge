#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <xinput.h>
#include <math.h>
#include <mmsystem.h>
#include <dsound.h>
#include "Win32_SGE.h"
#include "cglm/mat4.h"
#include "math.h"
#include "SGE_Program_Options.h"
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb/stb_image.h"
//#include <stdio.h>
//#include "SGE_FlightStick.c"
//#include "SGE_SerialPort.h"
//#include "SGE_OpenGL.h"

// NOTE(MIGUEL): HMH Plan & Track
// TODO(MIGUEL): HMH 015(debugio - when he uses file io)
// TODO(MIGUEL): HMH 014(gamememory - if its very different than ryans)
// TODO(MIGUEL): HMH 018
// TODO(MIGUEL): HMH 019
// TODO(MIGUEL): HMH 020
// TODO(MIGUEL): HMH 023
// TODO(MIGUEL): HMH 024
// TODO(MIGUEL): HMH 028
// TODO(MIGUEL): HMH ...

//DONE!!! TODO(MIGUEL): Make a fucntion tx hat can read one file with vertex and fragment shader code
// TODO(MIGUEL): Make a cube using the wiki opengl tutorial
// TODO(MIGUEL): Create a movable object
// TODO(MIGUEL): Make a tile map and implement some basic colision detection
// TODO(MIGUEL): Apply some kenemtics to movement  
// TODO(MIGUEL): Add some simple collisin detection learingin opengl
// TODO(MIGUEL): Create a networking layer
// TODO(MIGUEL): Make a shader that outputs a sinewave
// TODO(MIGUEL): LOOK OVER GAME ENGINE ARCH BOOK FOR NECCESARY ENGINE SYSTEMS AND UPDATE TODOS
// TODO(MIGUEL): Add openCV

#define RENDER_OPENGL (0)

#define ARRAYCOUNT(array) (sizeof(array) /  sizeof(array[0]))
#define PI_32BIT 3.14159265359


//~ GLOBALS
global Platform g_platform = {0};

global win32_back_buffer g_main_window_back_buffer = { 0 }; // NOTE(MIGUEL): platform copy of the backbuffer
global u32 g_window_width  = 0; // NOTE(MIGUEL): Moving to app_state
global u32 g_window_height = 0; // NOTE(MIGUEL): Moving to app_state
global u64 g_tick_frequency = { 0 };

global IDirectSoundBuffer *g_secondary_sound_buffer = (void *)0x00;

//~ XINPUT SUPPORT

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE     *pState    )
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(x_input_get_state_stub) { return ERROR_DEVICE_NOT_CONNECTED; }

global x_input_get_state *XInputGetState_ = x_input_get_state_stub;
#define XInputGetState XInputGetState_


#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(x_input_set_state_stub) { return ERROR_DEVICE_NOT_CONNECTED; }

global x_input_set_state *XInputSetState_ = x_input_set_state_stub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);


internal void win32_xinput_load_functions()
{
    HMODULE xinput_lib;
    
    xinput_lib   = LoadLibrary("xinput1_4.dll"); 
    
    if(!xinput_lib)
    { xinput_lib = LoadLibrary("xinput1_3.dll"); }
    
    if(!xinput_lib)
    { xinput_lib = LoadLibrary("xinput9_1_0.dll"); }
    
    if(xinput_lib)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(xinput_lib, "XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(xinput_lib, "XInputSetState");
    }
    
    return;
}


internal void
win32_process_pending_messages(game_controller_input *keyboard_controller);

// ************************
// OPENGL ABSTRACTIONS
// -----------------------
// VERTEX BUFFER SETUP
// INDEX BUFFER SETUP
// VERTEX ARRAYS
// ***********************

global win32_state win32_state_;

global Entity Sprite = {0};
global OpenGL_Render_Info sprite_render_info;
global vec4 vec_color       = { 1.0f, 1.0f, 1.0f };

global mat4 translation = GLM_MAT4_ZERO_INIT;// NOTE(MIGUEL): This is in the App stuct
global mat4 scale       = GLM_MAT4_ZERO_INIT;
global mat4 rotation    = GLM_MAT4_ZERO_INIT;
global vec3 scalefactor = { 100.0f, 100.0f, 0.0f };
// TODO(MIGUEL): 
// NOTE(MIGUEL): Should there just be one projectin matrix for everthing
global Entity Nick = {0};
global OpenGL_Render_Info nick_render_info;

global vec4 nick_color       = { 1.0f, 1.0f, 1.0f };
global mat4 nick_translation = GLM_MAT4_ZERO_INIT;
global mat4 nick_scale       = GLM_MAT4_ZERO_INIT;
global mat4 nick_rotation    = GLM_MAT4_ZERO_INIT;
global vec3 nick_scalefactor = { 100.0f, 100.0f, 0.0f };


global BITMAPINFO BitmapInfo;
global void *BitmapMemory;
global int   BitmapWidth ;
global int   BitmapHeight;
global int   BytesPerPixel = 4;

//#define true  (1)
//#define false (0)
/*
*/
internal void
win32_begin_recording_input(win32_state *state, s32 input_record_index)
{
    state->input_record_index = input_record_index;
    
    u8 *file_name = "engine_input.sgei";
    
    state->record_handle = CreateFileA(file_name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    
    return;
}

internal void
win32_end_recording_input(win32_state *state)
{
    CloseHandle(state->record_handle);
    
    return;
}

internal void
win32_begin_input_playback(win32_state *state, game_input *new_input)
{
    //state->input_record_index = input_record_index;
    
    u8 *file_name = "engine_input.sgei";
    state->playback_handle = CreateFileA(file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    
    return;
}

internal void
win32_end_input_playback(win32_state *state)
{
    CloseHandle(state->playback_handle);
    
    return;
}


internal void
win32_record_input(win32_state *state, game_input *new_input)
{ 
    u32 bytes_written;
    
    WriteFile(state->record_handle, new_input, sizeof(new_input), &bytes_written, 0);
    
    return;
}

internal void
win32_playback_input(win32_state *state, game_input *new_input)
{
    u32 bytes_read;
    if(ReadFile(state->record_handle, new_input, sizeof(new_input), &bytes_read, 0))
    {
        u32 playing_index = state->input_playback_index;
        win32_end_input_playback  (state);
        win32_begin_input_playback(state, new_input);
    }
    
    return;
}

internal void
win32_xinput_process_digital_button(game_button_state *state_new, game_button_state *state_old,
                                    u32 xinput_button_state     , u32 button_mask             )
{
    state_new->ended_down       = (xinput_button_state & button_mask) == button_mask;
    state_new->half_trans_count = (state_old->ended_down != state_new->ended_down)? 1 : 0;
    
    return;
}

internal f32
win32_xinput_process_stick(s16 value, s16 deadzone_threshold)
{
    f32 result = 0.0f;
    
    /// FUN FACT: NO DEADZONE CAUSE THE PLANT TO DRIFT
    if(value >  deadzone_threshold){ result = (f32)value / 32767.0f; }
    if(value < -deadzone_threshold){ result = (f32)value / 32768.0f; }
    
    return result;
}

internal void
win32_process_keyboard_message(game_button_state *state_new, b32 is_down)
{
    if(state_new->ended_down != is_down)
    {
        state_new->ended_down = is_down;
        ++state_new->half_trans_count;
    }
    
    return;
}

void win32_client_draw(win32_back_buffer *buffer, HDC device_context, u32 window_width, u32 window_height)
{
    StretchDIBits(device_context,
                  0, 0,  window_width,  window_height,
                  0, 0, buffer->width, buffer->height,
                  buffer->data, &(buffer->info),
                  DIB_RGB_COLORS, SRCCOPY);
    
    return;
}

void win32_back_buffer_init(win32_back_buffer *back_buffer)
{
    back_buffer->width                        =         INITIAL_WINDOW_WIDTH ;
    back_buffer->height                       =         INITIAL_WINDOW_HEIGHT;
    back_buffer->bytes_per_pixel              =                             4;
    back_buffer->pitch = (back_buffer->width * back_buffer->bytes_per_pixel) ;
    
    back_buffer->info.bmiHeader.biSize        =      sizeof(BITMAPINFOHEADER);
    back_buffer->info.bmiHeader.biWidth       =         back_buffer->width   ; 
    back_buffer->info.bmiHeader.biHeight      = -((s32)(back_buffer->height)); 
    back_buffer->info.bmiHeader.biPlanes      =                             1;
    back_buffer->info.bmiHeader.biBitCount    =                            32;
    back_buffer->info.bmiHeader.biCompression =                        BI_RGB;
    
    // NOTE(MIGUEL): Data will be copied from the app_back_buffer in transient memory
    back_buffer->data = VirtualAlloc(0, (back_buffer->width * back_buffer->height) * back_buffer->bytes_per_pixel, 
                                     MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    
    return;
}

void win32_back_buffer_resize(win32_back_buffer *back_buffer, u32 width, u32 height)
{
    if(back_buffer->data)
    {
        VirtualFree(back_buffer->data, 0, MEM_RELEASE);
    }
    
    back_buffer->width                        =                         width;
    back_buffer->height                       =                        height;
    back_buffer->pitch =  (back_buffer->width * back_buffer->bytes_per_pixel);
    
    back_buffer->info.bmiHeader.biSize        =      sizeof(BITMAPINFOHEADER);
    back_buffer->info.bmiHeader.biWidth       =         back_buffer->width   ; 
    back_buffer->info.bmiHeader.biHeight      = -((s32)(back_buffer->height)); 
    back_buffer->info.bmiHeader.biPlanes      =                             1;
    back_buffer->info.bmiHeader.biBitCount    =                            32;
    back_buffer->info.bmiHeader.biCompression =                        BI_RGB;
    
    // NOTE(MIGUEL): Data will be copied from the app_back_buffer in transient memory
    back_buffer->data = VirtualAlloc(0, (back_buffer->width * back_buffer->height) * back_buffer->bytes_per_pixel, 
                                     MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    
    return;
}
void win32_client_get_dimensions(RECT client_rect, u32 *width, u32 *height)
{
    *width  = client_rect.right  - client_rect.left;
    *height = client_rect.bottom - client_rect.top ;
    
    return;
}

void win32_directsound_init(HWND window, u32 samples_per_second, LPDIRECTSOUNDBUFFER *secondary_sound_buffer, u32 sound_buffer_size)
{
    // NOTE(MIGUEL): Load the lib
    HMODULE direct_sound_lib = LoadLibraryA("dsound.dll");
    
    if(direct_sound_lib)
    {
        // NOTE(MIGUEL): Get a direct object - cooperative
        IDirectSound        *direct_sound     ;
        direct_sound_create *DirectSoundCreate;
        
        DirectSoundCreate = (direct_sound_create *)GetProcAddress(direct_sound_lib, "DirectSoundCreate");
        
        if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &direct_sound, 0)))
        {
            WAVEFORMATEX wave_format = { 0 };
            wave_format.wFormatTag      =  WAVE_FORMAT_PCM;
            wave_format.nChannels       =   2;
            wave_format.wBitsPerSample  =  16;
            wave_format.nSamplesPerSec  =  samples_per_second;
            wave_format.nBlockAlign     = (wave_format.nChannels      * wave_format.wBitsPerSample) / 8;
            wave_format.nAvgBytesPerSec = (wave_format.nSamplesPerSec * wave_format.nBlockAlign);
            wave_format.cbSize          =   0;
            
            // NOTE(MIGUEL): Create a primary buffer - to set the mode/audio out format
            if(SUCCEEDED(direct_sound->lpVtbl->SetCooperativeLevel(direct_sound, window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC primary_sound_buffer_description = { 0 };
                primary_sound_buffer_description.dwSize  = sizeof(DSBUFFERDESC) ;
                primary_sound_buffer_description.dwFlags = DSBCAPS_PRIMARYBUFFER;
                
                IDirectSoundBuffer *primary_sound_buffer = { 0 };
                
                if(SUCCEEDED(direct_sound->lpVtbl->CreateSoundBuffer(direct_sound,
                                                                     &primary_sound_buffer_description,
                                                                     &primary_sound_buffer,
                                                                     0)))
                {
                    HRESULT result = primary_sound_buffer->lpVtbl->SetFormat(primary_sound_buffer, &wave_format);
                    
                    if(SUCCEEDED(result))
                    {
                        /*   // NOTE(MIGUEL): !!! FORMAT SET !!!*/ 
                        OutputDebugStringA("format set!!!");
                    }
                    else { /*// NOTE(MIGUEL): diagnostic */ }
                } 
                else     { /*// NOTE(MIGUEL): diagnostic */ }
            }
            else         { /*// NOTE(MIGUEL): diagnostic */ }
            
            // NOTE(MIGUEL): Create a secondary buffer
            DSBUFFERDESC secondary_sound_buffer_description  = { 0 };
            secondary_sound_buffer_description.dwSize        = sizeof(DSBUFFERDESC);
            secondary_sound_buffer_description.dwFlags       = 0                   ;
            secondary_sound_buffer_description.dwBufferBytes = sound_buffer_size   ;
            secondary_sound_buffer_description.lpwfxFormat   = &wave_format        ;
            
            HRESULT result = direct_sound->lpVtbl->CreateSoundBuffer(direct_sound,
                                                                     &secondary_sound_buffer_description,
                                                                     secondary_sound_buffer,
                                                                     0);
            
            if(SUCCEEDED(result))
            {
                OutputDebugStringA("soundbuffer created");
            }
            else { /*// NOTE(MIGUEL): diagnostic */ }
        }
        else     { /*// NOTE(MIGUEL): diagnostic */ }
    }
    else         { /*// NOTE(MIGUEL): diagnostic */ }
    
    return;
}

typedef struct
{
    b32 is_playing           ;
    u32 samples_per_second   ;
    u32 tone_hz              ;
    s32 tone_volume          ;
    u32 wave_period          ;
    u32 running_sample_index ;
    u32 bytes_per_sample     ;
    u32 secondary_buffer_size;
    f32 t_sin                ;
    u32 latency_sample_count ;
} win32_sound_output;


void win32_clear_sound_buffer(win32_sound_output *sound_output)
{
    
    void *buffer_region;
    void *buffer_region_remainder;
    u32   buffer_region_size;
    u32   buffer_region_remainder_size;
    
    if(SUCCEEDED(g_secondary_sound_buffer->lpVtbl->Lock(g_secondary_sound_buffer,
                                                        0                       , sound_output->secondary_buffer_size,
                                                        &buffer_region          , &buffer_region_size,
                                                        &buffer_region_remainder, &buffer_region_remainder_size,
                                                        0)))
    {
        u32 buffer_region_sample_count           = buffer_region_size           / sound_output->bytes_per_sample;
        u32 buffer_region_remainder_sample_count = buffer_region_remainder_size / sound_output->bytes_per_sample;
        
        u16 *sample_out = (u16 *)buffer_region;
        
        for(u32 sample_index = 0; sample_index < buffer_region_sample_count; sample_index++)
        {
            *sample_out++ = 0;
            *sample_out++ = 0;
            
            sound_output->running_sample_index++;
        }
        
        sample_out = (u16 *)buffer_region_remainder;
        
        for(u32 sample_index = 0; sample_index < buffer_region_remainder_sample_count; sample_index++)
        {
            *sample_out++ = 0;
            *sample_out++ = 0;
            
            sound_output->running_sample_index++;
        }
        
    }
    
    g_secondary_sound_buffer->lpVtbl->Unlock(g_secondary_sound_buffer,
                                             buffer_region           , buffer_region_size,
                                             buffer_region_remainder , buffer_region_remainder_size);
    
    return;
}

void win32_fill_sound_buffer(win32_sound_output *sound_output, u32 byte_to_lock, u32 bytes_to_write, game_sound_output_buffer *source_buffer)
{
    
    void *buffer_region;
    void *buffer_region_remainder;
    u32   buffer_region_size;
    u32   buffer_region_remainder_size;
    
    if(SUCCEEDED(g_secondary_sound_buffer->lpVtbl->Lock(g_secondary_sound_buffer,
                                                        byte_to_lock            , bytes_to_write,
                                                        &buffer_region          , &buffer_region_size,
                                                        &buffer_region_remainder, &buffer_region_remainder_size,
                                                        0)))
    {
        u32 buffer_region_sample_count           = buffer_region_size           / sound_output->bytes_per_sample;
        u32 buffer_region_remainder_sample_count = buffer_region_remainder_size / sound_output->bytes_per_sample;
        
        u16 *sample_in  = source_buffer->samples;
        u16 *sample_out = (u16 *)buffer_region;
        
        for(u32 sample_index = 0; sample_index < buffer_region_sample_count; sample_index++)
        {
            *sample_out++ = *sample_in++;
            *sample_out++ = *sample_in++;
            
            sound_output->running_sample_index++;
        }
        
        
        sample_out = (u16 *)buffer_region_remainder;
        
        for(u32 sample_index = 0; sample_index < buffer_region_remainder_sample_count; sample_index++)
        {
            *sample_out++ = *sample_in++;
            *sample_out++ = *sample_in++;
            
            sound_output->running_sample_index++;
        }
        
        g_secondary_sound_buffer->lpVtbl->Unlock(g_secondary_sound_buffer,
                                                 buffer_region           , buffer_region_size,
                                                 buffer_region_remainder , buffer_region_remainder_size);
    }
    
    return;
}

void win32_debug_draw_vertical_line(win32_back_buffer *back_buffer, u32 x, u32 top, u32 bottom, u32 color)
{
    u8 *pixel = (( u8 *)back_buffer->data             +
                 (  x * back_buffer->bytes_per_pixel) +
                 (top * back_buffer->pitch          ));
    
    for(u32 y = top; y < bottom; y++)
    {
        *(u32 *)pixel *= color;
        
        pixel += back_buffer->pitch;
    }
    
    return;
}


inline void win32_draw_sound_time_marker(win32_back_buffer  *back_buffer ,
                                         win32_sound_output *sound_output,
                                         f32 buffer_conversion_factor,
                                         u32 padding_x, u32 top, u32 bottom,
                                         u32 cursor_pos, u32 color)
{
    
    ASSERT(cursor_pos < sound_output->secondary_buffer_size);
    
    f32 offset              = buffer_conversion_factor * cursor_pos;
    
    u32 marker_x_pos        = padding_x + offset ;
    
    win32_debug_draw_vertical_line(back_buffer, marker_x_pos, top, bottom, color);
    
    return;
}


void win32_debug_sync_display(win32_back_buffer *back_buffer, win32_debug_sound_time_marker *markers, u32 marker_count, 
                              win32_sound_output *sound_output,
                              f32 target_seconds_per_frame)
{
    u32 padding_x = 16;
    u32 padding_y = 16;
    
    u32 top    = padding_y;
    u32 bottom = back_buffer->height - padding_y;
    
    f32 buffer_conversion_factor = (f32)(back_buffer->width - (padding_x * 2)) / (f32)sound_output->secondary_buffer_size;
    
    // NOTE(MIGUEL): for HMH 015
    for(u32 marker_index = 0; marker_index < marker_count; marker_index++)
    {
        win32_debug_sound_time_marker *current_marker = &markers[marker_index];
        
        win32_draw_sound_time_marker(back_buffer                 ,
                                     sound_output                ,
                                     buffer_conversion_factor    ,
                                     padding_x                   ,
                                     top, bottom                 , 
                                     current_marker->play_cursor ,
                                     0xFFff0000);
        
        win32_draw_sound_time_marker(back_buffer                  ,
                                     sound_output                 ,
                                     buffer_conversion_factor     ,
                                     padding_x                    ,
                                     top, bottom                  , 
                                     current_marker->write_cursor ,
                                     0xFF0000ff);
    }
    
    return;  
}

inline LARGE_INTEGER win32_get_current_tick(void)
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    
    return result;
}

// NOTE(MIGUEL): inline supporeted in C ?????
inline f32 win32_get_seconds_elapsed(LARGE_INTEGER start_tick, LARGE_INTEGER end_tick)
{
    f32 seconds_elapsed = ((f32)(end_tick.QuadPart - start_tick.QuadPart) / (f32)g_tick_frequency);
    
    return seconds_elapsed;
}

int CALLBACK 
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode) 
{
    u8 exe_file_name[MAX_PATH];
    
    DWORD size_of_file_name = GetModuleFileName(0, exe_file_name, sizeof(exe_file_name));
    
    u8 *one_past_last_slash = exe_file_name +  size_of_file_name;
    
    for(u8 *scan = exe_file_name; *scan; ++scan)
    {
        if(*scan == '\\')
        {
            one_past_last_slash = scan + 1;
        }
    }
    
    AllocConsole();
    
    freopen("CONOUT$", "w", stdout);
    
    HANDLE Debug_console = GetStdHandle(STD_OUTPUT_HANDLE );
    
    /*************************************************************************/
    /*************************************************************************/
    /*                                                                       */
    /*                      W I N D O W   S E T U P                          */
    /*                                                                       */
    /*************************************************************************/
    /*************************************************************************/
    
    //~ DEFINING WINDOW CLASS TO REGISTER
    WNDCLASS WindowClass      = {0};
    WindowClass.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc   = win32_Main_Window_Procedure;
    WindowClass.hInstance     = Instance;
    //WindowClass.hIcon;
    WindowClass.lpszClassName = "MyWindowClass";
    
    
    //~ REGISTER WINDOW CLASS WITH OS
    if(RegisterClass(&WindowClass)) 
    {
        
        //~ CREATE THE WINDOW AND DISPLAY IT
        
        HWND window = CreateWindowEx(0, WindowClass.lpszClassName,
                                     "Simple Game Engine",
                                     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                     20, 20,
                                     INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT,
                                     0, 0, Instance, 0);
        
        // TODO: figure out why app crashes if stick is not connected
        //This is for the joystick!!!
        //CreateDevice(Window, Instance);
        /*
                                                                
HDC   gl_device_context = GetDC(Window);
        HGLRC gl_render_context = win32_Init_OpenGL(gl_device_context);
                                                                                                                                
        ASSERT(gladLoadGL());
                                                                                                                                
        u32 gl_major = 0;
        u32 gl_minor = 0;
                                
        glGetIntegerv(GL_MAJOR_VERSION, &gl_major);
        glGetIntegerv(GL_MINOR_VERSION, &gl_minor);
        printf       ("OPENGL VERSION: %d.%d \n"  , 
                    gl_major, gl_minor);
        */
        // NOTE(MIGUEL): This should Init on users command
        //win32_serial_Port_Init();
        
        //~ BACKBUFFER INTIT
        
        HDC device_context = GetDC(window);
        win32_back_buffer_init(&g_main_window_back_buffer);
        
        //~ TIMING STUFF
#define FRAMES_OF_AUDIO_LATENCY (3)
#define GAME_UPDATE_HZ (60 / 2)
        u32 monitor_refresh_hz       = 60;
        u32 game_update_hz           = monitor_refresh_hz / 2;
        f32 target_seconds_per_frame = 1.0f / (f32)game_update_hz;
        
        
        u32 scheduler_granularity_millis = 1;
        b32 sleep_granularity_was_set    = ( timeBeginPeriod(scheduler_granularity_millis) == TIMERR_NOERROR);
        
        if(!sleep_granularity_was_set)
        {
            printf("sleep granularity was not set\n");
        }
        
        
        /*************************************************************************/
        /*************************************************************************/
        /*                                                                       */
        /*                              M E M O R Y                              */
        /*                                                                       */
        /*************************************************************************/
        /*************************************************************************/
        // PLATFORM INITIALIZATION
        {
            g_platform.permanent_storage_size = PERMANENT_STORAGE_SIZE;
            g_platform.permanent_storage      = VirtualAlloc(0, 
                                                             g_platform.permanent_storage_size,
                                                             MEM_COMMIT | MEM_RESERVE,
                                                             PAGE_READWRITE);
            
            // TODO(MIGUEL): Add transient storage
            g_platform.permanent_storage_size = TRANSIENT_STORAGE_SIZE;
            g_platform.transient_storage      = VirtualAlloc(0, 
                                                             g_platform.transient_storage_size,
                                                             MEM_COMMIT | MEM_RESERVE,
                                                             PAGE_READWRITE);
            
            g_platform.frames_per_second_target = 60.0f;
            
        }
        
        if(window)
        {
            MSG Message;
            
            /// LOAD DEPENDINCIES
            win32_game_code Game = win32_load_game_code();
            win32_xinput_load_functions();
            
            Game.Init(&g_platform);
            
            // **************************************
            // InitGL STUFF
            //
            // // TODO(MIGUEL): Find a better system for this shit
            // // NOTE(MIGUEL): 02/26/2021 - Ditching GL & following HMH SW Rendering approach for learning
            // **************************************
#if RENDER_OPENGL
            //~ INIT SPRITE
            
            f32 sprite_vertices[] = { 
                // pos      // tex
                0.0f, 1.0f, 0.0f, 1.0f,
                1.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f, 
                
                0.0f, 1.0f, 0.0f, 1.0f,
                1.0f, 1.0f, 1.0f, 1.0f,
                1.0f, 0.0f, 1.0f, 0.0f
            };
            
            Sprite.vertices = sprite_vertices;
            
            //printf("gen vert arrays addr: %llx#2 ", (u64)&glGenVertexArrays);
            //printf("gen vert arrays addr: %llx#2 ", (u64)&glGetError);
            
            OpenGL_VertexBuffer_Create  (&(sprite_render_info.vertex_buffer_id), sprite_vertices, sizeof(sprite_vertices));
            
            GL_Call(glGenVertexArrays(1, &sprite_render_info.vertex_attributes_id));
            GL_Call(glBindVertexArray(    sprite_render_info.vertex_attributes_id));
            
            GL_Call(glEnableVertexAttribArray(0));
            GL_Call(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void *)0x00));
            
            
            // THE AFFECTS OF THIS MIGHT NOT BE APPARENT UNSLESS THERE ARE CERTAIN CONDITIONS
            GL_Call(glGenTextures(1, &sprite_render_info.texture));
            GL_Call(glBindTexture(GL_TEXTURE_2D, sprite_render_info.texture));
            // CONFIGUE OPENGL WRAPPING OPTIONS
            GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
            GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));
            // CONFIGURE OPENGL FILTERING OPTIONS
            GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST    ));
            GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR     ));
            
            //GL_Call(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, texture_border_color)); 
            
            // LOAD TEXTURE
            s32 sprite_tex_width, sprite_tex_height, sprite_nrChannels;
            //stbi_set_flip_vertically_on_load(true);  
            u8 *sprite_tex_data = stbi_load("../res/images/geo.png", &sprite_tex_width, &sprite_tex_height, &sprite_nrChannels, STBI_rgb_alpha); 
            if(sprite_tex_data)
            {
                //printf("Tex Data: \n %d | %d | %s  \n", sprite_tex_width, sprite_tex_height, sprite_tex_data);
                //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                // NOTE(MIGUEL): NO AFFECT
                GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sprite_tex_width, sprite_tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, sprite_tex_data));
                GL_Call(glGenerateMipmap(GL_TEXTURE_2D));
            }
            else
            {
                printf("Failed to load texture");
            }
            
            stbi_image_free(sprite_tex_data);
            
            
            // THIS SHADER MAyBE FUCKED UP
            ReadAShaderFile(&sprite_render_info.shader, "../res/shaders/SpriteShader.glsl");
            
            
            // NOTE(MIGUEL): ISSUES HERE !!!! 
            GL_Call(glUseProgram(sprite_render_info.shader));
            GL_Call(glUniform1i(glGetUniformLocation(sprite_render_info.shader, "sprite_texture"), 1));
            
            // NOTE(MIGUEL): deleting the folowing 2 lines changes it from black to brown
            
            // FOR LATER
            mat4 projection;
            GL_Call(glm_ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f, projection));
            GL_Call(glUniformMatrix4fv(glGetUniformLocation(sprite_render_info.shader, "projection"), 1, GL_FALSE, (f32 *)projection));
            
            // UNBIND BUFFER
            GL_Call(glBindBuffer(GL_ARRAY_BUFFER, 0)); 
            GL_Call(glBindVertexArray(0));
            
            sprite_render_info.uniform_model      = glGetUniformLocation(sprite_render_info.shader, "model");
            sprite_render_info.uniform_color      = glGetUniformLocation(sprite_render_info.shader, "spriteColor");
            sprite_render_info.uniform_projection = glGetUniformLocation(sprite_render_info.shader, "projection");
            
            
            //~ INIT SPRITE NICK
            
            // NOTE(MIGUEL): Use GEO Sprite vertices
            
            Nick.vertices = sprite_vertices;
            
            OpenGL_VertexBuffer_Create(&nick_render_info.vertex_buffer_id, sprite_vertices, sizeof(sprite_vertices));
            
            //GL_Call(glBindBuffer(GL_ARRAY_BUFFER, Nick.vertex_Attributes));
            GL_Call(glGenVertexArrays(1, &nick_render_info.vertex_attributes_id));
            GL_Call(glBindVertexArray(nick_render_info.vertex_attributes_id));
            GL_Call(glEnableVertexAttribArray(0));
            GL_Call(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void *)0x00));
            
            
            // THE AFFECTS OF THIS MIGHT NOT BE APPARENT UNSLESS THERE ARE CERTAIN CONDITIONS
            GL_Call(glGenTextures(1, &nick_render_info.texture));
            GL_Call(glBindTexture(GL_TEXTURE_2D, nick_render_info.texture));
            // CONFIGUE OPENGL WRAPPING OPTIONS
            GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S    , GL_MIRRORED_REPEAT));
            GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T    , GL_MIRRORED_REPEAT));
            // CONFIGURE OPENGL FILTERING OPTIONS
            GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST        ));
            GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR         ));
            
            //GL_Call(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, texture_border_color)); 
            
            // LOAD TEXTURE
            //s32 sprite_tex_width, sprite_tex_height, sprite_nrChannels;
            //stbi_set_flip_vertically_on_load(true);  
            sprite_tex_data = stbi_load("../res/images/nick.png", &sprite_tex_width, &sprite_tex_height, &sprite_nrChannels, STBI_rgb_alpha); 
            
            if(sprite_tex_data)
            {
                // NOTE(MIGUEL): NO AFFECT
                GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sprite_tex_width, sprite_tex_height, 0, GL_RGBA,      GL_UNSIGNED_BYTE, sprite_tex_data));
                GL_Call(glGenerateMipmap(GL_TEXTURE_2D));
            }
            else
            {
                printf("Failed to load texture");
            }
            
            stbi_image_free(sprite_tex_data);
            
            
            // THIS SHADER MAyBE FUCKED UP
            ReadAShaderFile(&nick_render_info.shader, "../res/shaders/Nick.glsl");
            
            
            // NOTE(MIGUEL): ISSUES HERE !!!! 
            GL_Call(glUseProgram(nick_render_info.shader));
            GL_Call(glUniform1i(glGetUniformLocation(nick_render_info.shader, "sprite_texture"), 1));
            
            // NOTE(MIGUEL): deleting the folowing 2 lines changes it from black to brown
            
            // FOR LATER
            mat4 nick_projection;
            GL_Call(glm_ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f, nick_projection));
            GL_Call(glUniformMatrix4fv(glGetUniformLocation(nick_render_info.shader, "projection"), 1, GL_FALSE, (f32 *)nick_projection));
            
            // UNBIND BUFFER
            GL_Call(glBindBuffer(GL_ARRAY_BUFFER, 0));
            GL_Call(glBindVertexArray(0));
            
            nick_render_info.uniform_model      = glGetUniformLocation(nick_render_info.shader, "model"      );
            nick_render_info.uniform_color      = glGetUniformLocation(nick_render_info.shader, "spriteColor");
            nick_render_info.uniform_input      = glGetUniformLocation(nick_render_info.shader, "mousePos"   );
            nick_render_info.uniform_projection = glGetUniformLocation(nick_render_info.shader, "projection" );
#endif
            
            //~ DIRECT SOUND INIT
            
            win32_sound_output sound_output = {0}; 
            sound_output.is_playing            =     0;
            sound_output.samples_per_second    = 48000;
            sound_output.tone_hz               =   256;
            sound_output.tone_volume           =    10;
            sound_output.wave_period           = sound_output.samples_per_second / sound_output.tone_hz;
            sound_output.running_sample_index  =     0;
            sound_output.bytes_per_sample      = sizeof(u16) * 2 ;
            sound_output.latency_sample_count  = FRAMES_OF_AUDIO_LATENCY * (sound_output.samples_per_second / game_update_hz);
            sound_output.secondary_buffer_size = sound_output.samples_per_second * sound_output.bytes_per_sample;
            
            u16 *samples = (u16 *)VirtualAlloc(0, sound_output.secondary_buffer_size,
                                               MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            
            win32_directsound_init(window, sound_output.samples_per_second, 
                                   &g_secondary_sound_buffer, 
                                   sound_output.secondary_buffer_size);
            
            win32_clear_sound_buffer(&sound_output);
            
            g_secondary_sound_buffer->lpVtbl->Play(g_secondary_sound_buffer,0, 0, DSBPLAY_LOOPING);
            
            
            u32 debug_sound_time_marker_index = 0;
            win32_debug_sound_time_marker debug_sound_time_markers[GAME_UPDATE_HZ / 2] = { 0 };
            u32 last_play_cursor = 0;
            b32 sound_is_valid   = 0;
            
            //-
            
            game_input  input[2]  = { 0 };
            game_input *input_new = &input[0];
            game_input *input_old = &input[1];
            
            //~ TIMING STUFF
#if RION
            s64 performance_counter_frequency = 1;
            {
                LARGE_INTEGER freq = { 0LL };
                QueryPerformanceFrequency(&freq);
                performance_counter_frequency = freq.QuadPart;
            }
            
            LARGE_INTEGER begin_frame_time_data = { 0LL };
            LARGE_INTEGER end_frame_time_data   = { 0LL };
#else // NOTE(MIGUEL): CASEY HMH
            
            LARGE_INTEGER frequency_query_result = { 0LL };
            QueryPerformanceFrequency(&frequency_query_result);
            
            // NOTE(MIGUEL): regular u64 to not have to deal with union shinanegans when computing ms/frame
            g_tick_frequency = frequency_query_result.QuadPart;
            
            LARGE_INTEGER tick_work_start = win32_get_current_tick(); // NOTE(MIGUEL): is last tick counter HMH
            
            // NOTE(MIGUEL): for profiling
            u64 start_cycle_count = __rdtsc(); // NOTE(MIGUEL): is last cycle counter HMH
#endif
            u32 load_counter = 120; // Frames
            
            
            /*************************************************************************/
            /*************************************************************************/
            /*                                                                       */
            /*                        M A I N    L O O P                             */
            /*                                                                       */
            /*************************************************************************/
            /*************************************************************************/
            
            while(!g_platform.quit)
            {
                
                //~ HOUSEKEEPING
                
                // LIVE CODE EDITTING
                if(load_counter++ == 120)
                {
                    win32_unload_game_code(&Game);
                    Game = win32_load_game_code();
                    load_counter = 0;
                }
                
#if RION
                // TIMING STUFF
                g_platform.last_time     = g_platform.current_time;
                g_platform.current_time += 1 / g_platform.frames_per_second_target;
                s64 desired_frame_time_counts = performance_counter_frequency / g_platform.frames_per_second_target;
                
                QueryPerformanceCounter(&begin_frame_time_data);
                // END OF TIMING STUFF
#else // NOTE(MIGUEL): CASEY HMH
                
                
#endif
                
                /*************************************************************************/
                /*************************************************************************/
                /*                                                                       */
                /*                                 I N P U T                             */
                /*                                                                       */
                /*************************************************************************/
                /*************************************************************************/
                
                //~ CONTROLLER PROCESSING
                // KEYBOARD
                /*
                game_controller_input *keyboard_controller_old = get_controller(input_old, 0);
                game_controller_input *keyboard_controller_new = get_controller(input_new, 0);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
                //keyboard_controller_new = { 0 }; // Fuck this
                memset(keyboard_controller_new, 0, sizeof(game_controller_input));
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
                keyboard_controller_new->is_connected = 1;
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
                for(u32 button_index = 0; button_index < ARRAY_COUNT(keyboard_controller_new->buttons); button_index++)
                {
                    keyboard_controller_new->buttons[button_index].ended_down = keyboard_controller_old->buttons[button_index].ended_down;
                }
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
                win32_process_pending_messages(&input_new->controllers[0]);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
                //if(GlobalPause)
                // MOUSE
                POINT mouse_pos;
                GetCursorPos(&mouse_pos);
                ScreenToClient(Window, &mouse_pos);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
                input_new->mouse_x = mouse_pos.x;
                input_new->mouse_y = mouse_pos.y;
                input_new->mouse_z = 0; // For Mouse Wheel
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
                win32_process_keyboard_message(&input_new->mouse_buttons[0],
                                            GetKeyState(VK_LBUTTON) & (1 << 15));
                win32_process_keyboard_message(&input_new->mouse_buttons[1],
                                            GetKeyState(VK_MBUTTON) & (1 << 15));
                win32_process_keyboard_message(&input_new->mouse_buttons[2],
                                            GetKeyState(VK_RBUTTON) & (1 << 15));
                win32_process_keyboard_message(&input_new->mouse_buttons[3],
                                            GetKeyState(VK_XBUTTON1) & (1 << 15));
                win32_process_keyboard_message(&input_new->mouse_buttons[4],
                                            GetKeyState(VK_XBUTTON2) & (1 << 15));
                */
                //~ GAMEPAD CONTROLLER PROCESSING
                win32_process_pending_messages(&input_new->controllers[0]);
                
                DWORD max_controller_count = XUSER_MAX_COUNT;
                
                for(u32 controller_index = 0; controller_index < max_controller_count; controller_index++)
                {
                    XINPUT_STATE controller_state;
                    
                    game_controller_input *controller_snapshot_old = &input_old->controllers[controller_index];
                    game_controller_input *controller_snapshot_new = &input_new->controllers[controller_index];
                    
                    if(XInputGetState(controller_index, &controller_state) == ERROR_SUCCESS)
                    {
                        XINPUT_GAMEPAD *Pad = &controller_state.Gamepad;
                        controller_snapshot_new->is_connected = 1;
                        
                        b32 button_start = (Pad->wButtons & XINPUT_GAMEPAD_START     );
                        b32 button_back  = (Pad->wButtons & XINPUT_GAMEPAD_BACK      );
                        
                        b32 dpad_up      = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP   );
                        b32 dpad_down    = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN );
                        b32 dpad_left    = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT );
                        b32 dpad_right   = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                        
                        
                        // DEADZONE PROCESSING 
                        controller_snapshot_new->is_analog   = 1;
                        controller_snapshot_new->stick_avg_x = win32_xinput_process_stick(Pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                        
                        controller_snapshot_new->stick_avg_y = win32_xinput_process_stick(Pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                        
                        // STICK JOLT TO BUTTON PRESS CONVERSION
                        f32 threshold = 0.5f;
                        win32_xinput_process_digital_button(&controller_snapshot_new->button_x, 
                                                            &controller_snapshot_old->button_x,
                                                            (controller_snapshot_new->stick_avg_x < -threshold ? 0 : 1)
                                                            , 1);
                        
                        win32_xinput_process_digital_button(&controller_snapshot_new->button_x, 
                                                            &controller_snapshot_old->button_x,
                                                            (controller_snapshot_new->stick_avg_x >  threshold ? 0 : 1)
                                                            , 1);
                        
                        win32_xinput_process_digital_button(&controller_snapshot_new->button_y, 
                                                            &controller_snapshot_old->button_y,
                                                            (controller_snapshot_new->stick_avg_y < -threshold ? 1 : 0)
                                                            , 1);
                        
                        win32_xinput_process_digital_button(&controller_snapshot_new->button_y, 
                                                            &controller_snapshot_old->button_y,
                                                            (controller_snapshot_new->stick_avg_y >  threshold ? 1 : 0)
                                                            , 1);
                        
                        
                        
                        win32_xinput_process_digital_button(&controller_snapshot_new->shoulder_left, 
                                                            &controller_snapshot_old->shoulder_left,
                                                            Pad->wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER);
                        
                        win32_xinput_process_digital_button(&controller_snapshot_new->shoulder_right, 
                                                            &controller_snapshot_old->shoulder_right,
                                                            Pad->wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER);
                        
                        win32_xinput_process_digital_button(&controller_snapshot_new->button_a, 
                                                            &controller_snapshot_old->button_a,
                                                            Pad->wButtons, XINPUT_GAMEPAD_A);
                        
                        win32_xinput_process_digital_button(&controller_snapshot_new->button_b, 
                                                            &controller_snapshot_old->button_b,
                                                            Pad->wButtons, XINPUT_GAMEPAD_B);
                        
                        win32_xinput_process_digital_button(&controller_snapshot_new->button_x, 
                                                            &controller_snapshot_old->button_x,
                                                            Pad->wButtons, XINPUT_GAMEPAD_X);
                        
                        win32_xinput_process_digital_button(&controller_snapshot_new->button_y, 
                                                            &controller_snapshot_old->button_y,
                                                            Pad->wButtons, XINPUT_GAMEPAD_Y);
                        
                        win32_xinput_process_digital_button(&controller_snapshot_new->button_start, 
                                                            &controller_snapshot_old->button_start,
                                                            Pad->wButtons, XINPUT_GAMEPAD_START);
                        
                        win32_xinput_process_digital_button(&controller_snapshot_new->button_back, 
                                                            &controller_snapshot_old->button_back,
                                                            Pad->wButtons, XINPUT_GAMEPAD_BACK);
                        
                    }// Controller Plugged in
                    else
                    {}// Controller not available
                }
                
                /*
                                if(g_platform.input_record_index)
                                {
                                    win32_record_input(g_platform, new_input);
                                }
                                if(g_platform.input_play_index)
                                {
                                    win32_playback_input(g_platform, new_input);
                                }
                                */
                
                /*************************************************************************/
                /*************************************************************************/
                /*                                                                       */
                /*                              U P D A T E                              */
                /*                                                                       */
                /*************************************************************************/
                /*************************************************************************/
                
                //~ AUDIO - DIRECTSOUND
                
                // NOTE(MIGUEL): Compute how muuch to fill and where
                u32 target_cursor  = 0;
                u32 byte_to_lock   = 0;
                u32 bytes_to_write = 0;
                // NOTE(MIGUEL): play & write cursors moved to SoftW render section
                
                if(sound_is_valid)
                {
                    // NOTE(MIGUEL): this for changes in the tone_hz made by the plat_indie layer
                    sound_output.wave_period = sound_output.samples_per_second / sound_output.tone_hz;
                    
                    target_cursor = (last_play_cursor + (sound_output.latency_sample_count * sound_output.bytes_per_sample)) % sound_output.secondary_buffer_size;
                    
                    byte_to_lock  = (sound_output.running_sample_index * sound_output.bytes_per_sample) % sound_output.secondary_buffer_size; 
                    
                    if(byte_to_lock > target_cursor)
                    {
                        bytes_to_write  = sound_output.secondary_buffer_size - byte_to_lock;
                        bytes_to_write += target_cursor;
                    }
                    else
                    {
                        bytes_to_write  = target_cursor - byte_to_lock;
                    }
                }
                
                game_sound_output_buffer sound_buffer = { 0 };
                sound_buffer.samples_per_second = sound_output.samples_per_second;
                sound_buffer.sample_count       = bytes_to_write / sound_output.bytes_per_sample;
                sound_buffer.samples            =  samples;
                
                //~ GRAPHICS - SOFTWARE
                
                win32_back_buffer_resize(&g_main_window_back_buffer, g_window_width, g_window_height);
                game_back_buffer back_buffer = { 0 };
                back_buffer.width  = g_main_window_back_buffer.width ;
                back_buffer.height = g_main_window_back_buffer.height;
                back_buffer.pitch  = g_main_window_back_buffer.pitch ;
                back_buffer.data   = g_main_window_back_buffer.data  ;
                
                // NOTE(MIGUEL): What shoud the func name be? see line 86
                g_platform.quit |= Game.Update(&g_platform, input_new, &back_buffer, &sound_buffer, sound_output.tone_hz);
                
                /*************************************************************************/
                /*************************************************************************/
                /*                                                                       */
                /*                              O U T P U T                              */
                /*                                                                       */
                /*************************************************************************/
                /*************************************************************************/
                
                //~ GRAPHICS - HARDWARE - OPENGL
#if RENDER_OPENGL
                // NOTE(MIGUEL): 02/26/2021 - NO OPENGL RENDERING YET
                local_persist b32 first_render = true;
                
                //glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
                glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
                
                // NOTE(MIGUEL): A lot of fuckery going on here
                mat4 model                    = GLM_MAT4_ZERO_INIT; // constructor
                mat4 nick_model               = GLM_MAT4_ZERO_INIT; // constructor
                nick_render_info.matrix_model = (f32 *)&nick_model;
                
                GL_Call(glUseProgram(sprite_render_info.shader));
                
                if(first_render)
                {
                    glm_translate_make(translation     , (vec3){200.0f, 200.0f, 0.0f});
                    glm_scale_make    (scale           , (vec3){200.0f, 200.0f, 1.0f});
                    glm_rotate_make   (rotation        , glm_rad(10.0f) ,(vec3){0.0f, 1.0f, 0.0f});
                    
                    glm_translate_make(nick_translation, (vec3){1.0f, 1.0f, 0.0f});
                    glm_scale_make    (nick_scale      , (vec3){200.0f, 200.0f, 1.0f});
                    glm_rotate_make   (nick_rotation   , glm_rad(10.0f) ,(vec3){0.0f, 1.0f, 0.0f});
                    
                    Helpers_Display _Matrix4(translation, "Translate Matrix");
                    Helpers_Display_Matrix4(scale      , "Scale Matrix"    );
                    Helpers_Display_Matrix4(rotation   , "Rotate Matrix"   );
                    
                    first_render = false;
                }
                //rotate++;
                
                //~SPRITE RENDERER_00 BACKGROUND
                GL_Call(glUniformMatrix4fv(sprite_render_info.uniform_model, 1, GL_FALSE, (f32 *)model));
                GL_Call(glUniform3f       (sprite_render_info.uniform_color, color[0],color[1], color[2]));
                
                GL_Call(glActiveTexture(GL_TEXTURE1));
                GL_Call(glBindTexture  (GL_TEXTURE_2D, sprite_render_info.texture));
                
                // Enables the alpha channel
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                
                //glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
                
                GL_Call(glBindVertexArray(sprite_render_info.vertex_attributes_id));
                GL_Call(glDrawArrays     (GL_TRIANGLES, 0, 6));
                GL_Call(glBindVertexArray(0));
                
                
                
                //~SPRITE RENDERER_01
                vec3 player_translate;
                glm_vec3_copy(((App *)(g_platform.permanent_storage))->player_translate, player_translate);
                
                glm_translate(translation, player_translate);
                
                glm_mat4_mulN((mat4 *[]){&translation, &scale }, 2, model);
                
                //glm_mat4_print(model, stdout);
                
                //Helpers_Display_Matrix4(model, "Model Matrix");
                
                GL_Call(glUniformMatrix4fv(sprite_render_info.uniform_model, 1, GL_FALSE, (f32 *)model));
                GL_Call(glUniform3f       (sprite_render_info.uniform_color, color[0],color[1], color[2]));
                
                GL_Call(glActiveTexture(GL_TEXTURE1));
                GL_Call(glBindTexture  (GL_TEXTURE_2D, sprite_render_info.texture));
                
                // Enables the alpha channel
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                
                //glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
                
                GL_Call(glBindVertexArray(sp rite_render_info.vertex_attributes_id));
                GL_Call(glDrawArrays     (GL_TRIANGLES, 0, 6));
                GL_Call(glBindVertexArray(0));
                
                
                //~ SPRITE RENDERER_02
                
                GL_Call(glUseProgram(nick_render_info.shader));
                
                glm_translate       (nick_translation, (vec3){ ( cos(g_platform.current_time) ), (  sin(g_platform.current_time) ), 0.0f} );
                glm_mat4_mulN       ((mat4 *[]){&nick_translation, &nick_scale }, 2, nick_model);
                
                win32_OpenGL_draw_sprite(g_platform, &nick_render_info, (f32 *)&nick_color );
                
                SwapBuffers(gl_device_context);
#endif
                
                
                
                //~ AUDIO - DIRECTSOUND
                if(sound_is_valid)
                {
                    win32_fill_sound_buffer(&sound_output, byte_to_lock, bytes_to_write, &sound_buffer);
                }
                
                
                
                //~ TIMING - FRAME IDLE
#if RION
                QueryPerformanceCounter(&end_frame_time_data);
                
                //printf("Frame data: %lld <- ( %lld - %lld ) \n", (end_frame_time_data.QuadPart - begin_frame_time_data.QuadPart), end_frame_time_data.QuadPart, begin_frame_time_data.QuadPart);
                // NOTE(MIGUEL): Wait any time, if neccssary
                // TODO(MIGUEL): think about changing target fps if current target is not met
                {
                    s64 counts_in_frame = end_frame_time_data.QuadPart - begin_frame_time_data.QuadPart;
                    s64 counts_to_wait  = desired_frame_time_counts    - counts_in_frame               ;
                    
                    LARGE_INTEGER begin_wait_time_data;
                    LARGE_INTEGER   end_wait_time_data;
                    
                    QueryPerformanceCounter(&begin_wait_time_data);
                    
                    while(counts_to_wait > 0)
                    {
                        QueryPerformanceCounter(&end_wait_time_data);
                        counts_to_wait      -= end_wait_time_data.QuadPart - begin_wait_time_data.QuadPart;
                        begin_wait_time_data = end_wait_time_data;
                    }
                }
                
#else // NOTE(MIGUEL): CASEY HMH
                
                
                LARGE_INTEGER tick_work_end  = win32_get_current_tick();
                LARGE_INTEGER tick_frame_end = { 0 };
                f32 seconds_elapsed_for_work = win32_get_seconds_elapsed(tick_work_start, tick_work_end);
                
                f32 seconds_elapsed_for_frame = seconds_elapsed_for_work;
                
                u32 mil = 0;
                // NOTE(MIGUEL): Go Idle
                if(seconds_elapsed_for_frame < target_seconds_per_frame)
                {
                    
                    if(sleep_granularity_was_set)
                    {
                        u32 millis_to_sleep = (u32)(1000.0f * (target_seconds_per_frame - seconds_elapsed_for_frame));
                        mil = millis_to_sleep;
                        if(millis_to_sleep > 0)
                        {
                            Sleep(millis_to_sleep);
                        }
                    }
                    
                    // NOTE(MIGUEL): testing shit goes here
                    f32 test_seconds_elapsed_for_frame = win32_get_seconds_elapsed(tick_work_start, win32_get_current_tick());
                    //ASSERT(test_seconds_elapsed_for_frame < target_seconds_per_frame);
                    
                    LARGE_INTEGER tick_idle;
                    while(seconds_elapsed_for_frame < target_seconds_per_frame)
                    {
                        tick_idle   = win32_get_current_tick();
                        seconds_elapsed_for_frame = win32_get_seconds_elapsed(tick_work_start, tick_idle);
                    }
                }
                else
                {
                    // NOTE(MIGUEL): missed frame rate
                }
                
                tick_frame_end = win32_get_current_tick();
                
                
#endif
                
                
                //~ SOFTWARE RENDERING
                
#if SGE_INTERNAL
                //- DEBUG - AUDIO - DIRECT SOUND 
                win32_debug_sync_display(&g_main_window_back_buffer,
                                         debug_sound_time_markers,
                                         ARRAYCOUNT(debug_sound_time_markers),
                                         &sound_output,
                                         target_seconds_per_frame);
                //- DEBUG - AUDIO - DIRECT SOUND (END) 
#endif
                
                win32_client_draw(&g_main_window_back_buffer, device_context,
                                  g_window_width, g_window_height);
                
                
                u32 play_cursor    = 0;
                u32 write_cursor   = 0;
                
                
                if((g_secondary_sound_buffer->lpVtbl->GetCurrentPosition(g_secondary_sound_buffer, 
                                                                         &play_cursor, &write_cursor) == DS_OK))
                {
                    last_play_cursor = play_cursor;
                    
                    if(!sound_is_valid)
                    {
                        sound_output.running_sample_index = write_cursor / sound_output.bytes_per_sample;
                    }
                    sound_is_valid = 1;
                }
                else
                {
                    sound_is_valid = 0;
                }
                
                
#if SGE_INTERNAL
                //- DEBUG - AUDIO - DIRECT SOUND 
                // NOTE(MIGUEL): play & write cursors declared uptop
                // NOTE(MIGUEL): function implementation not finished
                {
                    win32_debug_sound_time_marker *marker = &debug_sound_time_markers[debug_sound_time_marker_index++ % (GAME_UPDATE_HZ / 2)];
                    
                    marker->play_cursor  = play_cursor ;
                    marker->write_cursor = write_cursor;
                    
                } 
                //- DEBUG - AUDIO - DIRECT SOUND (END)
#endif
                
                // *************************************************
                // HOUSEKEEPING
                // *************************************************
                //~ TIMING STUFF
                
                // NOTE(MIGUEL): for profiling
                u64 end_cycle_count = __rdtsc(); // NOTE(MIGUEL): is last cycle counter HMH
                
                u64 cycles_elapsed        = end_cycle_count - start_cycle_count; 
                f32 mega_cycles_per_frame = ((f32)cycles_elapsed / (f32)(1000.0f * 1000.0f));
                
                u64 ticks_elapsed         = ( tick_frame_end.QuadPart - tick_work_start.QuadPart                );
                f32 millis_per_frame      = 1000.0f * win32_get_seconds_elapsed(tick_work_start, tick_frame_end);
                
                u32 frames_per_second     = g_tick_frequency / ticks_elapsed;
                
                printf("ms/frame %f | %dFPS | %.02f Mcycles/frame \n", millis_per_frame, frames_per_second, mega_cycles_per_frame);
                
                
                tick_work_start   = tick_frame_end ;
                start_cycle_count = end_cycle_count;
                
                
                //~ INPUT STUFF
                game_input *temp;
                temp      = input_new;
                input_new = input_old;
                input_old = temp;
                
            }
            
            //CloseHandle(global_Device.comm);//Closing the Serial Port
            FreeConsole();
            
        }
        else { /*// NOTE(MIGUEL): diagnostic */ }
    }
    else     { /*// NOTE(MIGUEL): diagnostic */ }
    
    return (0);
}


// NOTE(MIGUEL): Sould I get a higher version of opengl via glad?
//PFNWGLGETEXTENSIONSSTRINGARBPROC  wglGetExtensionsStringARB;
//PFNWGLCHOOSEPIXELFORMATARBPROC    wglChoosePixelFormatARB;
//PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
//PFNGLGETSTRINGPROC                 glGetString;

typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc, HGLRC hShareContext,
                                                     const int *attribList);
wglCreateContextAttribsARB_type *wglCreateContextAttribsARB;

typedef BOOL WINAPI wglChoosePixelFormatARB_type(HDC hdc, const int *piAttribIList,
                                                 const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
wglChoosePixelFormatARB_type    *wglChoosePixelFormatARB;

internal void
win32_process_pending_messages(game_controller_input *keyboard_controller)
{
    MSG message;
    
    while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        switch(message.message)
        {
            case WM_QUIT:
            {
                g_platform.quit = true;
            }  break;
            
            case WM_SYSKEYUP:
            
            case WM_SYSKEYDOWN:
            
            case WM_KEYDOWN:
            
            case WM_KEYUP:
            {
                u32 vk_code          = (u32)message.wParam;
                b32 was_down         = ((message.lParam & (1 << 30)) != 0);
                b32 is_down          = ((message.lParam & (1 << 31)) == 0);
                
                if(was_down != is_down)
                {
                    if     (vk_code == 'W')
                    {
                        //MoveUp
                        win32_process_keyboard_message(&keyboard_controller->button_y, is_down);
                    }
                    else if(vk_code == 'A')
                    {
                        //MoveLeft
                        win32_process_keyboard_message(&keyboard_controller->button_x, is_down);
                    }
                    else if(vk_code == 'S')
                    {
                        //MoveDown
                        win32_process_keyboard_message(&keyboard_controller->button_a, is_down);
                    }
                    else if(vk_code == 'D')
                    {
                        //MoveRight
                        win32_process_keyboard_message(&keyboard_controller->button_b, is_down);
                    }
                    else if(vk_code == 'Q')
                    {
                        win32_process_keyboard_message(&keyboard_controller->shoulder_left, is_down);
                    }
                    else if(vk_code == 'E')
                    {
                        win32_process_keyboard_message(&keyboard_controller->shoulder_left, is_down);
                    }
                    else if(vk_code == VK_UP)
                    {
                        win32_process_keyboard_message(&keyboard_controller->action_up, is_down);
                    }
                    else if(vk_code == VK_LEFT)
                    {
                        win32_process_keyboard_message(&keyboard_controller->action_left, is_down);
                    }
                    else if(vk_code == VK_DOWN)
                    {
                        win32_process_keyboard_message(&keyboard_controller->action_down, is_down);
                    }
                    else if(vk_code == VK_RIGHT)
                    {
                        win32_process_keyboard_message(&keyboard_controller->action_right, is_down);
                    }
                    else if(vk_code == VK_ESCAPE)
                    {
                        win32_process_keyboard_message(&keyboard_controller->button_start, is_down);
                    }
                    else if(vk_code == VK_SPACE) 
                    {
                        win32_process_keyboard_message(&keyboard_controller->button_back, is_down);
                    }
#if HANDMADE_INTERNAL
                    else if(vk_code == 'P')
                    {
                        if(is_down)
                        {
                            GlobalPause = !GlobalPause;
                        }
                    }
#endif
                    b32 alt_key_was_down = ( message.lParam & (1 << 29));
                    if((vk_code == VK_F4) && alt_key_was_down)
                    {
                        g_platform.quit = true;
                    }
                    
                }
            } break;
            
            case WM_MOUSEMOVE:
            {
                //g_platform.mouse_x_direction = g_platform.mouse_x < (l_param & 0x0000FFFF)? (u32)(1): (u32)(-1);
                g_platform.mouse_x =  (message.lParam & 0x0000FFFF);
                
                //g_platform.mouse_y_direction = g_platform.mouse_y < (l_param & 0xFFFF0000 >> 16)? (u32)(-1): (u32)(1);
                g_platform.mouse_y = ((message.lParam & 0xFFFF0000) >> 16);
                
            }  break;
            
            default:
            {
                TranslateMessage(&message);
                DispatchMessage (&message);
            } break;
        }
    }
    
    return;
}

internal LRESULT CALLBACK 
win32_Main_Window_Procedure(HWND window, UINT message , WPARAM w_param, LPARAM l_param) 
{
    LRESULT result = 0;
    
    u32 key_code  =     0;
    u32 key_index =     0;
    b32 key_down  = false; 
    
    switch(message)
    {
        case WM_SIZE:
        {
            RECT client_rect;
            GetClientRect(window, &client_rect); //Get RECT of window excludes borders
            
            g_platform.window_width  = client_rect.right  - client_rect.left;
            g_platform.window_height = client_rect.bottom - client_rect.top ;
            
            g_window_width  = client_rect.right  - client_rect.left;
            g_window_height = client_rect.bottom - client_rect.top;
        } break;
        
        case WM_CLOSE:
        {
            g_platform.quit = true;
        } break;
        
        case WM_DESTROY:
        {
            g_platform.quit = true;
        } break;
        
        case WM_PAINT:
        {
            // NOTE(MIGUEL): WILL PAINT WITH OPEN GL NOT WINDOWS
            RECT        client_rect   ;
            HDC         device_context;
            PAINTSTRUCT paint         ;
            
            device_context = BeginPaint(window, &paint);
            
            GetClientRect(window, &client_rect); //Get RECT of window excludes borders
            
            u32 client_width ;
            u32 client_height;
            win32_client_get_dimensions(client_rect, &client_width, &client_height);
            
            win32_client_draw(&g_main_window_back_buffer, device_context,
                              g_window_width, g_window_height);
            EndPaint(window, &paint);
        } break;
        
        case WM_KEYDOWN:
        {
            key_down  = message == WM_KEYDOWN;
            key_code  = w_param;
            key_index = 0;
            
            if(key_code  >= 'A' && key_code <= 'Z')
            {
                key_index = KEY_a + (key_code - 'A');
            }
            g_platform.key_down[key_index] = key_down;
        } break;
        // TODO(MIGUEL): Add code for WM_KEYUP
        case WM_KEYUP:
        {
            key_down  = message == WM_KEYDOWN;
            key_code  = w_param;
            key_index = 0;
            
            if(key_code >= 'A' && key_code <= 'Z')
            { 
                key_index = KEY_a + (key_code - 'A');
            }
            g_platform.key_down[key_index] = key_down;
            
            //~ Extra:Live Loop Stuff
            if(key_code == 'L')
            {
                /*
*/
                if(win32_state_.input_record_index == 0)
                {
                    win32_begin_recording_input(&win32_state_, 1);
                }
                else
                {
                    win32_end_recording_input(&win32_state_   );
                    //win32_playback_input     (&win32_state_, input_new);
                }
            }
        } break;
        
        case WM_MOUSEMOVE:
        {
            //g_platform.mouse_x_direction = g_platform.mouse_x < (l_param & 0x0000FFFF)? (u32)(1): (u32)(-1);
            g_platform.mouse_x =  (l_param & 0x0000FFFF);
            
            //g_platform.mouse_y_direction = g_platform.mouse_y < (l_param & 0xFFFF0000 >> 16)? (u32)(-1): (u32)(1);
            g_platform.mouse_y = ((l_param & 0xFFFF0000) >> 16);
            
        } break;
        
        default:
        {
            OutputDebugStringA("Default\n");
            result = DefWindowProc(window, message, w_param, l_param);
        } break;
    }
    
    return(result);
}

internal void
win32_Init_OpenGL_Extensions(void)
{
    WNDCLASS dummy_window_class = {
        .style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc   = DefWindowProcA,
        .hInstance     = GetModuleHandle(0), // this function sus
        .lpszClassName = "Dummy_WGL_Ext_Init",
    };
    
    if (!RegisterClass(&dummy_window_class)) {
        printf("Failed to register dummy OpenGL window.");
    }
    
    HWND dummy_window = CreateWindowEx(
                                       0,
                                       dummy_window_class.lpszClassName,
                                       "Dummy OpenGL Window",
                                       0,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       0,
                                       0,
                                       dummy_window_class.hInstance,
                                       0);
    
    if (!dummy_window) {
        printf("Failed to create dummy OpenGL window.");
    }
    
    HDC dummy_dc = GetDC(dummy_window);
    
    PIXELFORMATDESCRIPTOR desired_pixel_format = {
        .nSize        = sizeof(PIXELFORMATDESCRIPTOR),
        .nVersion     = 1,
        .iPixelType   = PFD_TYPE_RGBA,
        .dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .cColorBits   = 32,
        .cAlphaBits   = 8,
        .iLayerType   = PFD_MAIN_PLANE,
        .cDepthBits   = 24,
        .cStencilBits = 8,
    };
    
    int pixel_format = ChoosePixelFormat(dummy_dc, &desired_pixel_format);
    if (!pixel_format) {
        printf("Failed to find a suitable pixel format.");
    }
    if (!SetPixelFormat(dummy_dc, pixel_format, &desired_pixel_format)) {
        printf("Failed to set the pixel format.");
    }
    
    HGLRC dummy_context = wglCreateContext(dummy_dc);
    if (!dummy_context) {
        printf("Failed to create a dummy OpenGL rendering context.");
    }
    
    if (!wglMakeCurrent(dummy_dc, dummy_context)) {
        printf("Failed to activate dummy OpenGL rendering context.");
    }
    
    wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type*)wglGetProcAddress("wglCreateContextAttribsARB");
    wglChoosePixelFormatARB    = (wglChoosePixelFormatARB_type*   )wglGetProcAddress("wglChoosePixelFormatARB"   );
    
    wglMakeCurrent  (dummy_dc, 0);
    wglDeleteContext(dummy_context);
    ReleaseDC       (dummy_window, dummy_dc);
    DestroyWindow   (dummy_window);
    
    return;
}


internal HGLRC
win32_Init_OpenGL(HDC real_dc)
{
    win32_Init_OpenGL_Extensions();
    
    // Now we can choose a pixel format the modern way, using wglChoosePixelFormatARB.
    s32 pixel_format_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB ,     GL_TRUE,
        WGL_ACCELERATION_ARB  ,     WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB    ,     WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB    ,     32,
        WGL_DEPTH_BITS_ARB    ,     24,
        WGL_STENCIL_BITS_ARB  ,      8, 0
    };
    
    s32 pixel_format;
    u32 num_formats;
    wglChoosePixelFormatARB(real_dc, pixel_format_attribs, 0, 1, 
                            &pixel_format, &num_formats);
    if (!num_formats) {
        printf("Failed to set the OpenGL 3.3 pixel format.");
    }
    
    PIXELFORMATDESCRIPTOR pfd;
    DescribePixelFormat(real_dc, pixel_format, sizeof(pfd), &pfd);
    
    if (!SetPixelFormat(real_dc, pixel_format, &pfd)) 
    { printf("Failed to set the OpenGL 3.3 pixel format."); }
    
    // Specify that we want to create an OpenGL 3.3 core profile context
    int gl_version_attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 6,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };
    
    HGLRC gl_version_context = wglCreateContextAttribsARB(real_dc, 0, gl_version_attribs);
    if (!gl_version_context) 
    {
        printf("Failed to create %d.%d rendering context.", 
               gl_version_attribs[1], 
               gl_version_attribs[3]);
    }
    
    if (!wglMakeCurrent(real_dc, gl_version_context)) 
    {
        printf("Failed to activate OpenGL %d.%d rendering context.", 
               gl_version_attribs[1], 
               gl_version_attribs[3]);
    }
    
    return gl_version_context;
}


internal FILETIME
win32_get_last_write_time(u8 *file_name)
{
    FILETIME last_write_time = { 0 };
    
    WIN32_FIND_DATA file_info;
    HANDLE file_handle = FindFirstFile(file_name, &file_info);
    
    if(file_handle != INVALID_HANDLE_VALUE)
    {
        last_write_time = file_info.ftLastWriteTime;
        FindClose(file_handle);
    }
    
    return last_write_time;
}

internal win32_game_code
win32_load_game_code(void)
{
    win32_game_code result = {0};
    
    u8 *source_dll_name = "SGE.dll"     ;
    u8 *  temp_dll_name = "SGE_temp.dll";
    
    result.dll_last_write_time = win32_get_last_write_time(source_dll_name);
    
    CopyFile(source_dll_name, temp_dll_name, FALSE);
    result.SGE_DLL= LoadLibraryA(temp_dll_name);
    
    if(result.SGE_DLL)
    {
        result.Update = (SGE_Update *)GetProcAddress(result.SGE_DLL, "SGEUpdate");
        result.Init   = (SGE_Init   *)GetProcAddress(result.SGE_DLL, "SGEInit"  );
        
        result.is_valid = (result.Update && result.Init);
    }
    if(!(result.is_valid))
    {
        result.Init   = SGEInitStub  ;
        result.Update = SGEUpdateStub;
    }
    
    return result;
}

internal void
win32_unload_game_code(win32_game_code *game)
{
    if(game)
    {
        FreeLibrary(game->SGE_DLL);
    }
    
    game->is_valid = false;
    game->Init     = SGEInitStub  ;
    game->Update   = SGEUpdateStub;
    
    return;
}


