/* date = January 4th 2021 6:48 pm */

#ifndef SGE_PLATFORM_H
#define SGE_PLATFORM_H

#include <stdint.h>

//~ USEFUL CONSTANTS
#define PI_32BIT 3.14159265359f


//~ HELPER MACROS

#define ARRAYCOUNT(array) (sizeof(array) /  sizeof(array[0]))

#define MINIMUM(a, b) ((a < b) ? (a) : (b))
#define MAXIMUM(a, b) ((a > b) ? (a) : (b))


//~ DEBUGGING

#if SGE_SLOW
#define ASSERT(expression) if(!(expression)){ *(u32 *)0x00 = 0; }
#else
#define ASSERT(expression)
#endif

#define INVALID_CODE_PATH ASSERT(!"Invalid Code Path");

//~ ARCH SPECIFIC

#define NULLPTR       ((void *)0x00)
#define readonly        const
#define internal        static
#define local_persist   static
#define global          static

#define KILOBYTES(size) (         (size) * 1024LL)
#define MEGABYTES(size) (KILOBYTES(size) * 1024LL)
#define GIGABYTES(size) (MEGABYTES(size) * 1024LL)
#define TERABYTES(size) (GIGABYTES(size) * 1024LL)

typedef size_t memory_index;

typedef uint8_t  u8 ;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef  int8_t  s8 ;
typedef  int16_t s16;
typedef  int32_t s32;
typedef  int64_t s64;

typedef  int8_t  b8 ;
typedef  int16_t b16;
typedef  int32_t b32;
typedef  int64_t b64;

typedef float f32;
typedef float f64;


//~ COMPILER SPECIFIC

#ifndef COMPILER_MSVC
#define COMPILER_MSVC 0
#endif

#ifndef COMPILER_LLVM
#define COMPILER_LLVM 0
#endif

#if !COMPILER_MSVC && !COMPILER_LLVM
#if _MSC_VER
#undef  COMPILER_MSVC
#define COMPILER_MSVC 1

#else
#undef  COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif

#if COMPILER_MSVC
#include <intrin.h>
#pragma intrinsic(_BitScanForward)
#endif

//~ ASYNC

typedef struct
{
    s32 place_holder;
} thread_context;


//~ FILE I/O
#if SGE_INTERNAL

typedef struct
{
    u32 contents_size;
    void  *contents;
} debug_read_file_result;


#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(thread_context *thread, void *memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUG_PlatformFreeFileMemory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(thread_context *thread, u8 *file_name)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUG_PlatformReadEntireFile);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) b32 name(thread_context *thread, u8 *file_name, u32 memory_size, void *memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUG_PlatformWriteEntireFile);

#endif

//~ INPUT

typedef struct
{
    u32 half_trans_count; // button half transition
    b32 ended_down;
} game_button_state;

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
            game_button_state button_y; //Move Up
            game_button_state button_a; //Move Down
            game_button_state button_x; //Move Left
            game_button_state button_b; //Move Right
            
            game_button_state action_up   ;
            game_button_state action_down ;
            game_button_state action_right;
            game_button_state action_left ;
            
            game_button_state shoulder_left ;
            game_button_state shoulder_right;
            
            game_button_state button_start;
            game_button_state button_back ;
            
            game_button_state terminator;
        };
    };
} game_controller_input;

typedef struct
{
    // TODO(MIGUEL): verify mouse buttons work HMH 023 - 025
    game_button_state mouse_buttons[5];
    
    s32 mouse_x;
    s32 mouse_y;
    s32 mouse_z;
    
    // NOTE(MIGUEL): seconds_to_advance_over_update
    f32 delta_t;
    
    game_controller_input controllers[5];
} game_input;

//~ MEMORY

typedef struct
{
    b32 is_initialized   ;
    void *permanent_storage;
    u64   permanent_storage_size;
    void *transient_storage;
    u64   transient_storage_size;
    
    DEBUG_PlatformFreeFileMemory  *debug_platform_free_file_memory ;
    DEBUG_PlatformReadEntireFile  *debug_platform_read_entire_file ;
    DEBUG_PlatformWriteEntireFile *debug_platform_write_entire_file;
} game_memory;

//~ GRAPHICS

typedef struct
{
    void *data     ;
    s32   width    ;
    s32   height   ;
    s32   pitch    ;
    u32   bytes_per_pixel;
} game_back_buffer;


//~ AUDIO

typedef struct
{
    u16 *samples          ;
    u32 sample_count      ;
    u32 samples_per_second;
    
} game_sound_output_buffer;


#endif //SGE_PLATFORM_H
