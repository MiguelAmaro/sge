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


//~ GLOBALS
global b32 g_running = true ;
global b32 g_pause   = false;

global win32_back_buffer g_main_window_back_buffer = { 0 }; // NOTE(MIGUEL): platform copy of the backbuffer
global u32 g_window_width  = 0; // NOTE(MIGUEL): Moving to app_state
global u32 g_window_height = 0; // NOTE(MIGUEL): Moving to app_state
global u64 g_tick_frequency = { 0 };
global b32 g_DEBUG_show_cursor  = 0;
global WINDOWPLACEMENT g_window_position = { sizeof(g_window_position) };

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

void win32_toggle_fullscreen(HWND window )
{
    // NOTE(MIGUEL): raymond cheng
    DWORD style = GetWindowLong(window, GWL_STYLE);
    
    if (style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO monitor_info = { sizeof(monitor_info) };
        
        if (GetWindowPlacement(window, &g_window_position) &&
            GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &monitor_info))
        {
            SetWindowLong(window, GWL_STYLE,
                          style & ~WS_OVERLAPPEDWINDOW);
            
            SetWindowPos(window, HWND_TOP,
                         monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.top ,
                         monitor_info.rcMonitor.right  - monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else 
    {
        SetWindowLong(window, GWL_STYLE,
                      style | WS_OVERLAPPEDWINDOW);
        
        SetWindowPlacement(window, &g_window_position);
        
        SetWindowPos(window, NULL,
                     0, 0, 0, 0,
                     SWP_NOMOVE        | 
                     SWP_NOSIZE        |
                     SWP_NOZORDER      |
                     SWP_NOOWNERZORDER |
                     SWP_FRAMECHANGED  );
    }
    
    return;
}


//~ DEBUG FILE I/O

DEBUG_PLATFORM_READ_ENTIRE_FILE (debug_platform_read_entire_file );
DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);
DEBUG_PLATFORM_FREE_FILE_MEMORY (debug_platform_free_file_memory );

DEBUG_PLATFORM_READ_ENTIRE_FILE (debug_platform_read_entire_file )
{
    DebugReadFileResult result = {0};
    
    HANDLE file_handle = CreateFileA(file_name,
                                     GENERIC_READ,
                                     FILE_SHARE_READ,
                                     0,
                                     OPEN_EXISTING,
                                     0, 0);
    
    if(file_handle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER file_size;
        
        if(GetFileSizeEx(file_handle, &file_size))
        {
            u32 file_size_32bit = safe_truncate_u64(file_size.QuadPart );
            
            result.contents = VirtualAlloc(0, file_size_32bit, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            
            
            if(result.contents)
            {
                u32 bytes_read;
                if(ReadFile(file_handle, result.contents, file_size_32bit,
                            &bytes_read, 0) && file_size_32bit == bytes_read)
                {
                    result.contents_size = file_size_32bit;
                }
                else
                {
                    // NOTE(MIGUEL): Failed file read!!!
                    debug_platform_free_file_memory(thread, result.contents);
                    result.contents = 0;
                }
            }
            else { /*// NOTE(MIGUEL): logging */ }
        }
        else     { /*// NOTE(MIGUEL): logging */ }
        
        CloseHandle(file_name);
    }
    else         { /*// NOTE(MIGUEL): logging */ }
    
    return result;
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file)
{
    b32 result = false;
    
    HANDLE file_handle = CreateFileA(file_name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    
    if(file_handle != INVALID_HANDLE_VALUE)
    {
        DWORD bytes_written;
        if(WriteFile(file_handle, memory, memory_size, &bytes_written, 0))
        {
            // NOTE(casey): File read successfully
            result = (bytes_written == memory_size);
        }
        else { /*// NOTE(MIGUEL): logging */ }
        
        CloseHandle(file_handle);
    }
    else     { /*// NOTE(MIGUEL): logging */ }
    
    
    return(result);
}


DEBUG_PLATFORM_FREE_FILE_MEMORY (debug_platform_free_file_memory )
{
    if(memory)
    {
        VirtualFree(memory, 0, MEM_RELEASE);
    }
    
    return;
}

//~ DYANAMIC LOADING

internal void
win32_xinput_load_functions()
{
    HMODULE xinput_lib;
    
    xinput_lib   = LoadLibrary("xinput1_4.dll"  ); 
    
    if(!xinput_lib)
    { xinput_lib = LoadLibrary("xinput1_3.dll"  ); }
    
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
win32_process_pending_messages(win32_state *state, game_controller_input *keyboard_controller);

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

internal 
void win32_print_last_sys_error(void)
{
    LPTSTR error_msg;
    u32 error_code    = GetLastError();
    u32 error_msg_len = 0;
    
    error_msg_len = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                   FORMAT_MESSAGE_FROM_SYSTEM     ,
                                   NULL                           ,
                                   error_code                     ,
                                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                   (u8 *)&error_msg, 0, NULLPTR);
    
    OutputDebugStringA(error_msg);
    
    LocalFree(error_msg);
    
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

internal void
win32_back_buffer_set_size(win32_back_buffer *back_buffer, u32 width, u32 height)
{
    if(back_buffer->data)
    {
        VirtualFree(back_buffer->data, 0, MEM_RELEASE);
    }
    
    back_buffer->width                        =                         width;
    back_buffer->height                       =                        height;
    back_buffer->bytes_per_pixel              =                             4;
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

internal void
win32_directsound_init(HWND window, u32 samples_per_second, LPDIRECTSOUNDBUFFER *secondary_sound_buffer, u32 sound_buffer_size)
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

internal void
win32_clear_sound_buffer(win32_sound_output *sound_output)
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

internal void
win32_fill_sound_buffer(win32_sound_output *sound_output, u32 byte_to_lock, u32 bytes_to_write, game_sound_output_buffer *source_buffer)
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

internal void
win32_debug_draw_vertical_line(win32_back_buffer *back_buffer, u32 x, u32 top, u32 bottom, u32 color)
{
    // TODO(MIGUEL): study this function
    if(top <= 0)
    {
        top = 0;
    }
    
    if(bottom > back_buffer->height)
    {
        bottom = back_buffer->height;
    }
    
    if((x >= 0) && (x < back_buffer->width))
    {
        u8 *pixel = (( u8 *)back_buffer->data             +
                     (  x * back_buffer->bytes_per_pixel) +
                     (top * back_buffer->pitch          ));
        
        for(u32 y = top; y < bottom; y++)
        {
            *(u32 *)pixel = color;
            
            pixel += back_buffer->pitch;
        }
    }
    
    return;
}


inline void win32_draw_sound_time_marker(win32_back_buffer  *back_buffer ,
                                         win32_sound_output *sound_output,
                                         f32 buffer_conversion_factor,
                                         u32 padding_x, u32 top, u32 bottom,
                                         u32 cursor_pos, u32 color)
{
    f32 offset              = buffer_conversion_factor * cursor_pos;
    
    u32 marker_x_pos        = padding_x + (u32)offset ;
    
    win32_debug_draw_vertical_line(back_buffer, marker_x_pos, top, bottom, color);
    
    return;
}


internal void
win32_debug_sync_display(win32_back_buffer *back_buffer, win32_debug_sound_time_marker *markers, u32 marker_count, 
                         u32 last_marker_index, win32_sound_output *sound_output,
                         f32 target_seconds_per_frame)
{
    u32 padding_x = 16;
    u32 padding_y = 16;
    
    u32 line_height = 64;
    
    f32 buffer_conversion_factor = (f32)(back_buffer->width - (padding_x * 2)) / (f32)sound_output->secondary_buffer_size;
    
    // NOTE(MIGUEL): for HMH 015
    for(u32 marker_index = 0; marker_index < marker_count; marker_index++)
    {
        win32_debug_sound_time_marker *current_marker = &markers[marker_index];
        
        ASSERT(current_marker->output_play_cursor  < sound_output->secondary_buffer_size);
        ASSERT(current_marker->output_write_cursor < sound_output->secondary_buffer_size);
        ASSERT(current_marker->output_location     < sound_output->secondary_buffer_size);
        ASSERT(current_marker->flip_play_cursor    < sound_output->secondary_buffer_size);
        ASSERT(current_marker->flip_write_cursor   < sound_output->secondary_buffer_size);
        
        u32 top    =               padding_y;
        u32 bottom = line_height + padding_y;
        
        u32 play_color  = 0xFFffffff;
        u32 write_color = 0xFFff0000;
        
        
        if(marker_index == last_marker_index)
        { 
            top    += line_height + padding_y;
            bottom += line_height + padding_y;
            
            play_color  = 0xFFff00ff;
            write_color = 0xFF00ff00;
            
            
            win32_draw_sound_time_marker(back_buffer                 ,
                                         sound_output                ,
                                         buffer_conversion_factor    ,
                                         padding_x                   ,
                                         top, bottom                 , 
                                         current_marker->output_play_cursor,
                                         play_color);
            
            
            win32_draw_sound_time_marker(back_buffer                 ,
                                         sound_output                ,
                                         buffer_conversion_factor    ,
                                         padding_x                   ,
                                         top, bottom                 , 
                                         current_marker->output_write_cursor,
                                         play_color);
            
            
            top    += line_height + padding_y;
            bottom += line_height + padding_y;
            
            
            win32_draw_sound_time_marker(back_buffer                 ,
                                         sound_output                ,
                                         buffer_conversion_factor    ,
                                         padding_x                   ,
                                         top, bottom                 , 
                                         current_marker->output_location,
                                         play_color);
            
            win32_draw_sound_time_marker(back_buffer                 ,
                                         sound_output                ,
                                         buffer_conversion_factor    ,
                                         padding_x                   ,
                                         top, bottom                 , 
                                         current_marker->output_location +
                                         current_marker->output_byte_count,
                                         play_color);
            
            
            top    += line_height + padding_y;
            bottom += line_height + padding_y;
            
            
        }
        
        
        win32_draw_sound_time_marker(back_buffer                 ,
                                     sound_output                ,
                                     buffer_conversion_factor    ,
                                     padding_x                   ,
                                     top, bottom                 , 
                                     current_marker->flip_play_cursor ,
                                     play_color);
        
        win32_draw_sound_time_marker(back_buffer                  ,
                                     sound_output                    ,
                                     buffer_conversion_factor     ,
                                     padding_x                    ,
                                     top, bottom                  , 
                                     current_marker->flip_write_cursor ,
                                     write_color);
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
WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int show_code) 
{
    /*************************************************************************/
    /*************************************************************************/
    /*                                                                       */
    /*                      W I N D O W   S E T U P                          */
    /*                                                                       */
    /*************************************************************************/
    /*************************************************************************/
    
    WNDCLASS WindowClass      = {0};
    WindowClass.style         = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc   = win32_Main_Window_Procedure;
    WindowClass.hInstance     = instance;
    //WindowClass.hIcon;
    WindowClass.hCursor       = LoadCursor(0, IDC_ARROW);
    WindowClass.lpszClassName = "MyWindowClass";
    
    
    if(RegisterClass(&WindowClass)) 
    {
        g_window_width  = INITIAL_WINDOW_WIDTH ;
        g_window_height = INITIAL_WINDOW_HEIGHT;
        
        HWND window = CreateWindowExA(0,//WS_EX_TOPMOST | WS_EX_LAYERED, 
                                      WindowClass.lpszClassName,
                                      "Simple Game Engine",
                                      WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                      20, 20,
                                      g_window_width, g_window_height,
                                      0, 0, instance, 0);
        
        
        //~ BACKBUFFER INTIT
        
        HDC device_context = GetDC(window);
        
        win32_back_buffer_set_size(&g_main_window_back_buffer, 960, 540);
        
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
        win32_state state_win32 = { 0 };
        game_memory sge_memory  = { 0 };
        {
#if SGE_INTERNAL
            LPVOID base_address = (LPVOID)TERABYTES(2);
#else
            LPVOID base_address = 0;
#endif
            sge_memory.permanent_storage_size = PERMANENT_STORAGE_SIZE;
            sge_memory.transient_storage_size = TRANSIENT_STORAGE_SIZE;
            
            state_win32.main_memory_block_size = (sge_memory.permanent_storage_size +
                                                  sge_memory.transient_storage_size);
            
            state_win32.main_memory_block = VirtualAlloc(base_address, 
                                                         (size_t)state_win32.main_memory_block_size,
                                                         MEM_COMMIT | MEM_RESERVE,
                                                         PAGE_READWRITE);
            
            // TODO(MIGUEL): Add transient storage
            sge_memory.permanent_storage = ((u8 *)state_win32.main_memory_block);
            
            sge_memory.transient_storage = ((u8 *)sge_memory.permanent_storage +
                                            sge_memory.permanent_storage_size);
            
            //g_platform.frames_per_second_target = 60.0f;
            
        }
        sge_memory.debug_platform_read_entire_file  = debug_platform_read_entire_file ;
        sge_memory.debug_platform_write_entire_file = debug_platform_write_entire_file;
        sge_memory.debug_platform_free_file_memory  = debug_platform_free_file_memory ;
        
        
        // NOTE(MIGUEL): wtf does this even do???
        win32_get_exe_file_name(&state_win32);
        
        u8 game_code_dll_full_path_source[WIN32_STATE_FILE_NAME_COUNT];
        win32_build_exe_path_file_name(&state_win32, "SGE.dll",
                                       sizeof(game_code_dll_full_path_source), game_code_dll_full_path_source);
        
        u8 game_code_dll_full_path_temp  [WIN32_STATE_FILE_NAME_COUNT];
        win32_build_exe_path_file_name(&state_win32, "SGE_temp.dll",
                                       sizeof(game_code_dll_full_path_temp), game_code_dll_full_path_temp);
        
        u8 game_code_dll_full_path_lock  [WIN32_STATE_FILE_NAME_COUNT];
        win32_build_exe_path_file_name(&state_win32, "lock.tmp",
                                       sizeof(game_code_dll_full_path_lock), game_code_dll_full_path_lock);
        
        
        if(window)
        {
            MSG Message;
            
            /// LOAD DEPENDINCIES
            win32_game_code Game = win32_load_game_code(game_code_dll_full_path_source,
                                                        game_code_dll_full_path_temp,
                                                        game_code_dll_full_path_lock);
            win32_xinput_load_functions();
            
            // NOTE(MIGUEL): may remove
            //Game.init(&sge_memory);
            
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
            sound_output.fill_pos_tolerance    = FRAMES_OF_AUDIO_LATENCY * (sound_output.samples_per_second *
                                                                            sound_output.bytes_per_sample   /
                                                                            game_update_hz) / 4;
            
            u16 *samples = (u16 *)VirtualAlloc(0, sound_output.secondary_buffer_size,
                                               MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            
            win32_directsound_init(window, sound_output.samples_per_second, 
                                   &g_secondary_sound_buffer, 
                                   sound_output.secondary_buffer_size);
            
            win32_clear_sound_buffer(&sound_output);
            
            g_secondary_sound_buffer->lpVtbl->Play(g_secondary_sound_buffer,0, 0, DSBPLAY_LOOPING);
            
            
            u32 debug_sound_time_marker_index = 0;
            win32_debug_sound_time_marker debug_sound_time_markers[GAME_UPDATE_HZ / 2] = { 0 };
            u32 audio_latency_bytes  = 0;
            f32 audio_latency_millis = 0.0f;
            b32 sound_is_valid       = 0;
            
            
            //-INPUT 
            
            
            for(int replay_index = 1; replay_index < ARRAYCOUNT(state_win32.input_replay_buffers);
                ++replay_index )
            {
                win32_replay_buffer *replay_buffer = &state_win32.input_replay_buffers[replay_index];
                
                // TODO(casey): Recording system still seems to take too long
                // on record start - find out what Windows is doing and if
                // we can speed up / defer some of that processing.
                
                win32_input_get_file_location(&state_win32, stream_state, replay_index,
                                              sizeof(replay_buffer->file_name), replay_buffer->file_name);
                
                replay_buffer->file_handle = CreateFileA(replay_buffer->file_name,
                                                         GENERIC_WRITE | 
                                                         GENERIC_READ,
                                                         0, 0, CREATE_ALWAYS, 
                                                         0, 0);
                
                LARGE_INTEGER max_size;
                
                max_size.QuadPart = state_win32.main_memory_block_size;
                
                replay_buffer->memory_map = CreateFileMapping(replay_buffer->file_handle,
                                                              0, PAGE_READWRITE,
                                                              max_size.HighPart,
                                                              max_size.LowPart, 0);
                
                replay_buffer->memory_block = MapViewOfFile(replay_buffer->memory_map,
                                                            FILE_MAP_ALL_ACCESS,
                                                            0, 0,
                                                            state_win32.main_memory_block_size);
                if(replay_buffer->memory_block )
                {
                }
                else
                {
                    // TODO(casey): Diagnostic
                }
            }
            
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
            
            while(g_running)
            {
                //~ LIVE CODE EDITTING
                FILETIME new_dll_write_time = win32_get_last_write_time(game_code_dll_full_path_source);
                {
                    
                    
                    if(CompareFileTime(&new_dll_write_time, &Game.dll_last_write_time) != 0)
                    {
                        win32_unload_game_code(&Game);
                        Game = win32_load_game_code(game_code_dll_full_path_source,
                                                    game_code_dll_full_path_temp,
                                                    game_code_dll_full_path_lock);
                        load_counter = 0;
                    }
                }
                
                /*************************************************************************/
                /*************************************************************************/
                /*                                                                       */
                /*                                 I N P U T                             */
                /*                                                                       */
                /*************************************************************************/
                /*************************************************************************/
                
                //~ CONTROLLER PROCESSING
                // KEYBOARD
                
                input_new->delta_t = target_seconds_per_frame; 
                
                game_controller_input *keyboard_controller_snapshot_old = get_controller(input_old, 0);
                game_controller_input *keyboard_controller_snapshot_new = get_controller(input_new, 0);
                
                memset(keyboard_controller_snapshot_new, 0, sizeof(game_controller_input));
                
                keyboard_controller_snapshot_new->is_connected = 1;
                
                for(u32 button_index = 0; button_index < ARRAYCOUNT(keyboard_controller_snapshot_new->buttons); button_index++)
                {
                    keyboard_controller_snapshot_new->buttons[button_index].ended_down = (keyboard_controller_snapshot_old->buttons[button_index].ended_down);
                }
                
                win32_process_pending_messages(&state_win32, keyboard_controller_snapshot_new);
                
                if(!g_pause)
                {
                    // MOUSE
                    POINT mouse_pos;
                    GetCursorPos(&mouse_pos);
                    ScreenToClient(window, &mouse_pos);
                    
                    input_new->mouse_x = mouse_pos.x;
                    input_new->mouse_y = mouse_pos.y;
                    input_new->mouse_z = 0; // For Mouse Wheel
                    
                    win32_process_keyboard_message(&input_new->mouse_buttons[0],
                                                   GetKeyState(VK_LBUTTON)  & (1 << 15));
                    win32_process_keyboard_message(&input_new->mouse_buttons[1],
                                                   GetKeyState(VK_MBUTTON)  & (1 << 15));
                    win32_process_keyboard_message(&input_new->mouse_buttons[2],
                                                   GetKeyState(VK_RBUTTON)  & (1 << 15));
                    win32_process_keyboard_message(&input_new->mouse_buttons[3],
                                                   GetKeyState(VK_XBUTTON1) & (1 << 15));
                    win32_process_keyboard_message(&input_new->mouse_buttons[4],
                                                   GetKeyState(VK_XBUTTON2) & (1 << 15));
                    
                    //~ GAMEPAD CONTROLLER PROCESSING
                    win32_process_pending_messages(&state_win32, &input_new->controllers[0]);
                    
                    DWORD max_controller_count = XUSER_MAX_COUNT;
                    
                    for(u32 controller_index = 0; controller_index < max_controller_count; controller_index++)
                    {
                        u32 gamepad_controller_index = controller_index + 1;
                        
                        game_controller_input *controller_snapshot_old = get_controller(input_old,
                                                                                        gamepad_controller_index);
                        game_controller_input *controller_snapshot_new = get_controller(input_new,
                                                                                        gamepad_controller_index);
                        XINPUT_STATE controller_state;
                        
                        if(XInputGetState(controller_index, &controller_state) == ERROR_SUCCESS)
                        { 
                            XINPUT_GAMEPAD *pad = &controller_state.Gamepad;
                            controller_snapshot_new->is_connected = 1;
                            
                            b32 button_start = (pad->wButtons & XINPUT_GAMEPAD_START     );
                            b32 button_back  = (pad->wButtons & XINPUT_GAMEPAD_BACK      );
                            
                            b32 dpad_up      = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP   );
                            b32 dpad_down    = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN );
                            b32 dpad_left    = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT );
                            b32 dpad_right   = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                            
                            
                            // DEADZONE PROCESSING 
                            controller_snapshot_new->stick_avg_x = win32_xinput_process_stick(pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                            
                            controller_snapshot_new->stick_avg_y = win32_xinput_process_stick(pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                            
                            // DPAD PROCESSING
                            if((controller_snapshot_new->stick_avg_x != 0.0f) ||
                               (controller_snapshot_new->stick_avg_y != 0.0f))
                            {
                                controller_snapshot_new->is_analog = true;
                            }
                            if(pad->wButtons & XINPUT_GAMEPAD_DPAD_UP)
                            {
                                controller_snapshot_new->stick_avg_y = 1.0f ;
                                controller_snapshot_new->is_analog    = false;
                            }
                            
                            if(pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
                            {
                                controller_snapshot_new->stick_avg_y = -1.0f;
                                controller_snapshot_new->is_analog    = false;
                            }
                            
                            if(pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
                            {
                                controller_snapshot_new->stick_avg_x = -1.0f;
                                controller_snapshot_new->is_analog    = false;
                            }
                            
                            if(pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
                            {
                                controller_snapshot_new->stick_avg_x = 1.0f ;
                                controller_snapshot_new->is_analog    = false;
                            }
                            
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
                                                                pad->wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER);
                            
                            win32_xinput_process_digital_button(&controller_snapshot_new->shoulder_right, 
                                                                &controller_snapshot_old->shoulder_right,
                                                                pad->wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER);
                            
                            win32_xinput_process_digital_button(&controller_snapshot_new->button_a, 
                                                                &controller_snapshot_old->button_a,
                                                                pad->wButtons, XINPUT_GAMEPAD_A);
                            
                            win32_xinput_process_digital_button(&controller_snapshot_new->button_b, 
                                                                &controller_snapshot_old->button_b,
                                                                pad->wButtons, XINPUT_GAMEPAD_B);
                            
                            win32_xinput_process_digital_button(&controller_snapshot_new->button_x, 
                                                                &controller_snapshot_old->button_x,
                                                                pad->wButtons, XINPUT_GAMEPAD_X);
                            
                            win32_xinput_process_digital_button(&controller_snapshot_new->button_y, 
                                                                &controller_snapshot_old->button_y,
                                                                pad->wButtons, XINPUT_GAMEPAD_Y);
                            
                            win32_xinput_process_digital_button(&controller_snapshot_new->button_start, 
                                                                &controller_snapshot_old->button_start,
                                                                pad->wButtons, XINPUT_GAMEPAD_START);
                            
                            win32_xinput_process_digital_button(&controller_snapshot_new->button_back, 
                                                                &controller_snapshot_old->button_back,
                                                                pad->wButtons, XINPUT_GAMEPAD_BACK);
                            
                        }
                        else 
                        { /*// NOTE(MIGUEL): Controller not avalailable - logging */
                            controller_snapshot_new->is_connected = 0;
                        }
                    }
                    
                    
                    if(state_win32.input_record_index)
                    {
                        win32_input_record(&state_win32, input_new);
                    }
                    if(state_win32.input_playback_index)
                    {
                        win32_input_playback(&state_win32, input_new);
                    }
                    
                    
                    /*************************************************************************/
                    /*************************************************************************/
                    /*                                                                       */
                    /*                              U P D A T E                              */
                    /*                                                                       */
                    /*************************************************************************/
                    /*************************************************************************/
                    
                    //~ GRAPHICS - SOFTWARE
                    
                    //win32_back_buffer_resize(&g_main_window_back_buffer, g_window_width, g_window_height);
                    thread_context thread = {0};
                    
                    game_back_buffer back_buffer = { 0 };
                    back_buffer.width           = g_main_window_back_buffer.width ;
                    back_buffer.height          = g_main_window_back_buffer.height;
                    back_buffer.pitch           = g_main_window_back_buffer.pitch ;
                    back_buffer.data            = g_main_window_back_buffer.data  ;
                    back_buffer.bytes_per_pixel = g_main_window_back_buffer.bytes_per_pixel;
                    
                    // NOTE(MIGUEL): What shoud the func name be? see line 86
                    Game.update(&thread, &sge_memory, input_new, &back_buffer);
                    
                    
                    //~ AUDIO - DIRECTSOUND
                    
                    u32  play_cursor;
                    u32 write_cursor;
                    
                    if((g_secondary_sound_buffer->lpVtbl->GetCurrentPosition(g_secondary_sound_buffer, 
                                                                             &play_cursor, &write_cursor) == DS_OK))
                    {  
                        if(!sound_is_valid)
                        {
                            sound_output.running_sample_index = write_cursor / sound_output.bytes_per_sample;
                            sound_is_valid = 1;
                        }
                        // NOTE(MIGUEL): this for changes in the tone_hz made by the plat_indie layer
                        //sound_output.wave_period = sound_output.samples_per_second / sound_output.tone_hz;
                        
                        
                        // NOTE(MIGUEL): Compute how muuch to fill and where
                        u32 frame_samples_size_estimate = (sound_output.samples_per_second *
                                                           sound_output.bytes_per_sample)  / game_update_hz;
                        u32 expected_frame_boundry = play_cursor + frame_samples_size_estimate;
                        u32 byte_to_lock  = ((sound_output.running_sample_index * 
                                              sound_output.bytes_per_sample)    % 
                                             (sound_output.secondary_buffer_size)); 
                        
                        
                        u32 safe_write_cursor = write_cursor;
                        if(safe_write_cursor < play_cursor)
                        {
                            safe_write_cursor += sound_output.secondary_buffer_size;
                        }
                        ASSERT(safe_write_cursor >= play_cursor);
                        
                        safe_write_cursor += sound_output.fill_pos_tolerance;
                        
                        b32 audio_card_is_low_latency = (safe_write_cursor < expected_frame_boundry);
                        
                        u32 target_cursor  = 0;
                        if(audio_card_is_low_latency)
                        {
                            
                            target_cursor = (expected_frame_boundry + 
                                             frame_samples_size_estimate);
                        }
                        else
                        {
                            target_cursor = (write_cursor                + 
                                             frame_samples_size_estimate + 
                                             sound_output.fill_pos_tolerance);
                        }
                        target_cursor =  target_cursor % sound_output.secondary_buffer_size;
                        
                        u32 bytes_to_write = 0;
                        if(byte_to_lock > target_cursor)
                        {
                            bytes_to_write  = sound_output.secondary_buffer_size - byte_to_lock;
                            bytes_to_write += target_cursor;
                        }
                        else
                        {
                            bytes_to_write  = target_cursor - byte_to_lock;
                        }
                        
                        game_sound_output_buffer sound_buffer = { 0 };
                        sound_buffer.samples_per_second = sound_output.samples_per_second;
                        sound_buffer.sample_count       = bytes_to_write / sound_output.bytes_per_sample;
                        sound_buffer.samples            =  samples;
                        
                        Game.get_sound_samples(&thread, &sge_memory, &sound_buffer);
                        
                        
                        /*************************************************************************/
                        /*************************************************************************/
                        /*                                                                       */
                        /*                          O U T P U T - A U D I O                      */
                        /*                                                                       */
                        /*************************************************************************/
                        /*************************************************************************/
                        
#if SGE_INTERNAL
                        u32 next_index = ((debug_sound_time_marker_index) % 
                                          (ARRAYCOUNT(debug_sound_time_markers)));
                        
                        win32_debug_sound_time_marker *marker = &debug_sound_time_markers[next_index];
                        
                        marker->output_play_cursor  = play_cursor;
                        marker->output_write_cursor = write_cursor;
                        marker->output_location     = byte_to_lock;
                        marker->output_byte_count   = bytes_to_write;
                        
                        u32 unwrapped_write_cursor = write_cursor;
                        
                        if(unwrapped_write_cursor < play_cursor)
                        {
                            unwrapped_write_cursor += sound_output.secondary_buffer_size;
                        }
                        
                        audio_latency_bytes = unwrapped_write_cursor - play_cursor;
                        
                        audio_latency_millis = ( 1000.0f * ((f32)audio_latency_bytes / (f32)sound_output.bytes_per_sample) 
                                                / (f32)sound_output.samples_per_second);
                        /*
                        printf("BTL: %6u |"
                               "TC: %6u |"
                               "BTW: %4u - PC: %6u |"
                               "WC: %6u |"
                               "DELTA: %4ubytes |"
                               "LATENCY: %.4fms"
                               "\n",
                               byte_to_lock,
                               target_cursor,
                               bytes_to_write,
                               play_cursor,
                               write_cursor,
                               audio_latency_bytes,
                                   audio_latency_millis);
                        */
#endif
                        win32_fill_sound_buffer(&sound_output, byte_to_lock, bytes_to_write, &sound_buffer);
                    }
                    else
                    {
                        sound_is_valid = 0;
                    }
                    
                    /*************************************************************************/
                    /*************************************************************************/
                    /*                                                                       */
                    /*                          T I M I N G - I D L E                        */
                    /*                                                                       */
                    /*************************************************************************/
                    /*************************************************************************/
                    LARGE_INTEGER tick_frame_end = { 0 };
                    LARGE_INTEGER tick_work_end  = win32_get_current_tick();
                    
                    f32 seconds_elapsed_for_work = win32_get_seconds_elapsed(tick_work_start, tick_work_end);
                    
                    f32 seconds_elapsed_for_frame = seconds_elapsed_for_work;
                    
                    u32 mil = 0;// TODO(MIGUEL): debug
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
                        
                        // NOTE(MIGUEL): always fails!!!!
                        if(test_seconds_elapsed_for_frame < target_seconds_per_frame)
                        {
                            // TODO(MIGUEL): log missed sleep 
                            
                        }
                        
                        
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
                        // TODO(MIGUEL): logfing
                    }
                    
                    tick_frame_end = win32_get_current_tick();
                    
                    
                    /*************************************************************************/
                    /*************************************************************************/
                    /*                                                                       */
                    /*                     O U T P U T - G R A P H I C S                     */
                    /*                                                                       */
                    /*************************************************************************/
                    /*************************************************************************/
                    
                    
                    //~ SOFTWARE RENDERING
                    
#if SGE_INTERNAL
                    //- DEBUG - AUDIO - DIRECT SOUND 
                    
                    /*
                    win32_debug_sync_display(&g_main_window_back_buffer,
                                             debug_sound_time_markers,
                                             ARRAYCOUNT(debug_sound_time_markers), 
                                             (debug_sound_time_marker_index % ARRAYCOUNT(debug_sound_time_markers)) - 1,
                                             &sound_output,
                                             target_seconds_per_frame);*/
                    //- DEBUG - AUDIO - DIRECT SOUND (END) 
#endif
                    
                    HDC                     device_context   ;
                    win32_window_dimensions window_dimensions;
                    
                    device_context     = GetDC(window);
                    window_dimensions  = win32_window_get_dimensions(window);
                    
                    win32_window_display(&g_main_window_back_buffer,
                                         device_context,
                                         window_dimensions.width,
                                         window_dimensions.height);
                    
                    ReleaseDC(window, device_context);
                    
                    // *************************************************
                    // HOUSEKEEPING
                    // *************************************************
                    //~ DEBUG - AUDIO - DIRECT SOUND 
#if SGE_INTERNAL
                    // NOTE(MIGUEL): play & write cursors declared uptop
                    // NOTE(MIGUEL): function implementation not finished
                    {
                        u32 play_cursor    = 0;
                        u32 write_cursor   = 0;
                        // NOTE(MIGUEL): this may fuck me
                        
                        if((g_secondary_sound_buffer->lpVtbl->GetCurrentPosition(g_secondary_sound_buffer, 
                                                                                 &play_cursor, &write_cursor) == DS_OK))
                        { 
                            
                            u32 next_index = ((debug_sound_time_marker_index) % 
                                              (ARRAYCOUNT(debug_sound_time_markers)));
                            
                            win32_debug_sound_time_marker *marker = &debug_sound_time_markers[next_index];
                            
                            ASSERT(next_index < ARRAYCOUNT(debug_sound_time_markers));
                            
                            
                            
                            marker->flip_play_cursor  =  play_cursor;
                            marker->flip_write_cursor = write_cursor;
                        } 
                        
                        // NOTE(MIGUEL): casey puts this at the bottom of the  loop
                        debug_sound_time_marker_index++;
                    }
#endif
                    
                    //~ TIMING STUFF
                    
                    // NOTE(MIGUEL): for profiling
                    u64 end_cycle_count = __rdtsc(); // NOTE(MIGUEL): is last cycle counter HMH
                    
                    u64 cycles_elapsed        = end_cycle_count - start_cycle_count; 
                    f32 mega_cycles_per_frame = ((f32)cycles_elapsed / (f32)(1000.0f * 1000.0f));
                    
                    u64 ticks_elapsed         = ( tick_frame_end.QuadPart - tick_work_start.QuadPart                );
                    f32 millis_per_frame      = 1000.0f * win32_get_seconds_elapsed(tick_work_start, tick_frame_end);
                    
                    u32 frames_per_second     = (u32)g_tick_frequency / (u32)ticks_elapsed;
                    
                    u8 timing_log[256];
                    
                    _snprintf(timing_log, sizeof(timing_log),"%fms/frame |"
                              "FPS: %d |"
                              "%.02f Mcycles/frame"
                              "\n", 
                              millis_per_frame,
                              frames_per_second,
                              mega_cycles_per_frame);
                    
                    OutputDebugStringA(timing_log);
                    
                    tick_work_start   = tick_frame_end ;
                    start_cycle_count = end_cycle_count;
                    
                    
                    //~ INPUT STUFF
                    game_input *temp;
                    temp      = input_new;
                    input_new = input_old;
                    input_old = temp;
                    
                } // NOTE(MIGUEL): global_pause_scope
            }
            
            //CloseHandle(global_Device.comm);//Closing the Serial Port
            //FreeConsole();
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
win32_process_pending_messages(win32_state *state, game_controller_input *keyboard_controller)
{
    MSG message;
    
    while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        switch(message.message)
        {
            case WM_QUIT:
            {
                g_running = false;
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
                        win32_process_keyboard_message(&keyboard_controller->button_back, is_down);
                    }
                    else if(vk_code == VK_SPACE) 
                    {
                        win32_process_keyboard_message(&keyboard_controller->button_start, is_down);
                    }
                    
#if SGE_INTERNAL
                    if(vk_code == 'P')
                    {
                        if(is_down)
                        {
                            g_pause= !g_pause;
                        }
                    }
                    //~ Extra:Live Loop Stuff
                    if(vk_code == 'L')
                    {
                        if(is_down)
                        {
                            if(state->input_playback_index == 0)
                            {
                                
                                if(state->input_record_index == 0)
                                {
                                    win32_input_begin_recording(state, 1);
                                }
                                else
                                {
                                    win32_input_end_recording (state  );
                                    win32_input_begin_playback(state, 1);
                                }
                            }
                            else
                            {
                                win32_input_end_playback(state);
                            }
                        }
                    }
                    
#endif
                    if(is_down)
                    {
                        
                        b32 alt_key_was_down = ( message.lParam & (1 << 29));
                        if((vk_code == VK_F4) && alt_key_was_down)
                        {
                            g_running = false;
                        }
                        if((vk_code == VK_RETURN) && alt_key_was_down)
                        {
                            if(message.hwnd)
                            {
                                win32_toggle_fullscreen(message.hwnd );
                            }
                        }
                    }
                }
            } break;
            
            default:
            {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            } break;
        }
    }
    
    return;
}

internal LRESULT CALLBACK 
win32_Main_Window_Procedure(HWND window, UINT message , WPARAM w_param, LPARAM l_param) 
{
    LRESULT result = 0;
    
    switch(message)
    {
        case WM_CLOSE:
        { g_running = false; }
        break;
        
        case WM_SETCURSOR:
        {
            if(g_DEBUG_show_cursor)
            {
                LoadCursor(0, IDC_ARROW);
            }
            else
            {
                result = DefWindowProcA(window, message, w_param, l_param);
            }
            
        }
        break;
        
        case WM_DESTROY:
        { g_running = false; }
        break;
        
        case WM_ACTIVATEAPP:
        { 
#if 0
            w_param == TRUE? 
                (SetLayeredWindowAttributes(window, RGB(0, 0, 0), 255, LWA_ALPHA)) :
            (SetLayeredWindowAttributes(window, RGB(0, 0, 0), 64 , LWA_ALPHA)) ; 
#endif
        }
        break;
        
        case WM_PAINT:
        {
            // TODO(MIGUEL): update this to HMH 025
            win32_window_dimensions window_dimensions = { 0 };
            HDC         device_context;
            PAINTSTRUCT paint         ;
            
            // NOTE(MIGUEL): window dinmension args 
            //               are ignored in the followin funciton
            device_context    = BeginPaint(window, &paint);
            window_dimensions = win32_window_get_dimensions(window);
            
            win32_window_display(&g_main_window_back_buffer,
                                 device_context,
                                 window_dimensions.width,
                                 window_dimensions.height);
            
            EndPaint(window, &paint);
        } break;
        
        case WM_SYSKEYDOWN:
        
        case WM_SYSKEYUP:
        
        case WM_KEYDOWN:
        
        case WM_KEYUP:
        {
            ASSERT(!"Keyboard input came in through a non-dispatch message!");
        } break;
        
        default:
        {
            result = DefWindowProcA(window, message, w_param, l_param);
        } break;
    }
    
    return result;
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
    
    WIN32_FILE_ATTRIBUTE_DATA file_info;
    
    if(GetFileAttributesEx(file_name, GetFileExInfoStandard, &file_info))
    {
        last_write_time = file_info.ftLastWriteTime;
    }
    
    return last_write_time;
}

internal win32_game_code
win32_load_game_code(u8 *source_dll_name, u8 *temp_dll_name, u8 *lock_file_name)
{
    win32_game_code result = { 0 };
    
    WIN32_FILE_ATTRIBUTE_DATA ignored;
    if(!GetFileAttributesEx(lock_file_name, GetFileExInfoStandard, &ignored))
    {
        result.dll_last_write_time = win32_get_last_write_time(source_dll_name);
        
        CopyFile(source_dll_name, temp_dll_name, FALSE);
        result.SGE_DLL = LoadLibraryA(temp_dll_name);
        
        if(result.SGE_DLL)
        {
            result.update            = (SGE_Update         *)GetProcAddress(result.SGE_DLL, "SGEUpdate"         );
            result.init              = (SGE_Init           *)GetProcAddress(result.SGE_DLL, "SGEInit"           );
            result.get_sound_samples = (SGE_GetSoundSamples*)GetProcAddress(result.SGE_DLL, "SGEGetSoundSamples");
            
            result.is_valid = (result.update && 
                               result.init   &&
                               result.get_sound_samples);
        }
    }
    if(!(result.is_valid))
    {
        result.init   = 0;
        result.update = 0;
        result.get_sound_samples = 0;
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
    game->init     = 0;
    game->update   = 0;
    game->get_sound_samples = 0;
    
    return;
}

internal void
win32_get_exe_file_name(win32_state *state)
{
    u32 file_name_size = GetModuleFileNameA(0, state->exe_file_name, sizeof(state->exe_file_name));
    
    state->one_past_last_exe_file_name_slash = state->exe_file_name;
    
    for(char *scan = state->exe_file_name; *scan; ++scan)
    {
        if(*scan == '\\')
        {
            state->one_past_last_exe_file_name_slash = scan + 1;
        }
    }
    
    return;
}

internal void
win32_build_exe_path_file_name(win32_state *state, u8 *file_name,
                               int dest_count, u8 *dest)
{
    string_concat(state->one_past_last_exe_file_name_slash - state->exe_file_name, state->exe_file_name,
                  string_get_length(file_name), file_name,
                  dest_count, dest);
    
    return;
}

internal void
string_concat(size_t source_a_count, u8 *source_a,
              size_t source_b_count, u8 *source_b,
              size_t dest_count    , u8 *dest    )
{
    // TODO(MIGUEL): dest bounds checking!
    
    for(u32 index = 0; index < source_a_count; index++)
    {
        *dest++ = *source_a++;
    }
    
    for(u32 index = 0; index < source_b_count; index++)
    {
        *dest++ = *source_b++;
    }
    
    *dest++ = 0;
}


internal u32
string_get_length(u8 *string)
{
    u32 count = 0;
    
    while(*string++) { ++count; }
    
    return count;
}


//~ WINDOW INTERFACE
internal win32_window_dimensions
win32_window_get_dimensions(HWND window)
{
    win32_window_dimensions dimensions;
    RECT client_rect                  ;
    
    //Get RECT of window excludes borders
    GetClientRect(window, &client_rect);
    
    dimensions.width  = client_rect.right  - client_rect.left;
    dimensions.height = client_rect.bottom - client_rect.top ;
    
    return dimensions;
}


internal void
win32_window_display(win32_back_buffer *buffer, HDC device_context, u32 window_width, u32 window_height)
{
    
    if((window_width  == buffer->width  * 2) &&
       (window_height == buffer->height * 2))
    {
        
        StretchDIBits(device_context,
                      0, 0,
                      buffer->width * 2, buffer->height * 2,
                      0, 0,
                      buffer->width,   buffer->height,
                      buffer->data , &(buffer->info),
                      DIB_RGB_COLORS, SRCCOPY);
    }
    else
    {
        s32 offset_x = 10;
        s32 offset_y = 10;
        
        // NOTE(MIGUEL): clear extra window area to black
        PatBlt(device_context,
               (0)           , (0)       , 
               (window_width), (offset_y),
               BLACKNESS);
        
        PatBlt(device_context,
               (0)           , (offset_y + buffer->height),
               (window_width), (window_width),
               BLACKNESS);
        
        PatBlt(device_context,
               (0)       , (0)            ,
               (offset_x), (window_height),
               BLACKNESS);
        
        PatBlt(device_context,
               (offset_x + buffer->width), (0)            ,
               (window_width)            , (window_height),
               BLACKNESS);
        
        
        StretchDIBits(device_context,
                      offset_x, offset_y,
                      buffer->width, buffer->height,
                      0, 0,
                      buffer->width,   buffer->height,
                      buffer->data , &(buffer->info),
                      DIB_RGB_COLORS, SRCCOPY);
    }
    
    return;
}


//~ INPUT RECORDING INTERFACE

internal void
win32_input_get_file_location(win32_state *state, b32 input_stream,
                              u32 slot_index, u32 dest_count, u8 *dest)
{
    char temp[64];
    
    wsprintf(temp, "loop_edit_%d_%s.sgei", slot_index, input_stream ? "input" : "state");
    
    win32_build_exe_path_file_name(state, temp, dest_count, dest);
    
    return;
}

internal win32_replay_buffer *
win32_input_get_replay_buffer(win32_state *state, u32 index)
{
    ASSERT(index > 0);
    ASSERT(index < ARRAYCOUNT(state->input_replay_buffers));
    
    win32_replay_buffer *result = &state->input_replay_buffers[index];
    
    return result;
}

//- RECORDING 

internal void
win32_input_begin_recording(win32_state *state, s32 input_record_index)
{
    // NOTE(MIGUEL): input recording setup
    
    win32_replay_buffer *replay_buffer = win32_input_get_replay_buffer(state, input_record_index);
    
    // NOTE(MIGUEL): mem_block is the mapped file
    if(replay_buffer->memory_block)
    {
        state->input_record_index = input_record_index;
        
        u8 file_name[WIN32_STATE_FILE_NAME_COUNT];
        
        win32_input_get_file_location(state,
                                      stream_input,
                                      input_record_index,
                                      sizeof(file_name),
                                      file_name);
        
        state->input_record_handle = CreateFileA(file_name,
                                                 GENERIC_WRITE,
                                                 0, 0,
                                                 CREATE_ALWAYS,
                                                 0, 0);
        
        // NOTE(MIGUEL): Store state at this point to reset on playback
        CopyMemory(replay_buffer->memory_block, state->main_memory_block, state->main_memory_block_size);
    }
    
    return;
}


internal void
win32_input_record(win32_state *state, game_input *new_input)
{ 
    u32 bytes_written = 0;
    
    
    WriteFile(state->input_record_handle,
              new_input,
              sizeof(*new_input),
              &bytes_written, 0);
    
    return;
}


internal void
win32_input_end_recording(win32_state *state)
{
    CloseHandle(state->input_record_handle);
    state->input_record_index = 0;
    
    return;
}

//- PLAYBACK 

internal void
win32_input_begin_playback(win32_state *state, u32 input_playback_index)
{
    
    win32_replay_buffer *replay_buffer = win32_input_get_replay_buffer(state, input_playback_index);
    
    if(replay_buffer->memory_block)
    {
        state->input_playback_index = input_playback_index;
        
        u8 file_name[WIN32_STATE_FILE_NAME_COUNT];
        
        win32_input_get_file_location(state,
                                      stream_input,
                                      input_playback_index,
                                      sizeof(file_name),
                                      file_name);
        
        state->input_playback_handle = CreateFileA(file_name,
                                                   GENERIC_READ,
                                                   0, 0,
                                                   OPEN_EXISTING,
                                                   0, 0);
        
        // NOTE(MIGUEL): reset the state
        CopyMemory(state->main_memory_block, replay_buffer->memory_block, state->main_memory_block_size);
    }
    
    return;
}

internal void
win32_input_playback(win32_state *state, game_input *new_input)
{
    u32 bytes_read = 0;
    
    printf("playing input back \n");
    // NOTE(MIGUEL): Overwrite actual controller input
    //               with pre-recorded input in file
    if(ReadFile(state->input_playback_handle,
                new_input,
                sizeof(*new_input),
                &bytes_read, 0))
    {
        if(bytes_read == 0)
        {
            u32 input_playback_index = state->input_playback_index;
            
            win32_input_end_playback  (state);
            win32_input_begin_playback(state, input_playback_index);
            
            // NOTE(MIGUEL): why another read???
            ReadFile(state->input_playback_handle,
                     new_input,
                     sizeof(*new_input),
                     &bytes_read, 0);
        }
    }
    
    return;
}

internal void
win32_input_end_playback(win32_state *state)
{
    printf("playback end \n");
    CloseHandle(state->input_playback_handle);
    state->input_playback_index = 0;
    
    return;
}


