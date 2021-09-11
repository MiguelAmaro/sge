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

typedef struct ThreadContext ThreadContext;
struct ThreadContext 
{
    s32 place_holder;
};


//~ FILE I/O
#if SGE_INTERNAL

typedef struct DebugReadFileResult DebugReadFileResult;
struct DebugReadFileResult
{
    u32 contents_size;
    void  *contents;
};


#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(ThreadContext *thread, void *memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUG_PlatformFreeFileMemory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) DebugReadFileResult name(ThreadContext *thread, u8 *file_name)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUG_PlatformReadEntireFile);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) b32 name(ThreadContext *thread, u8 *file_name, u32 memory_size, void *memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUG_PlatformWriteEntireFile);

#endif

//~ INPUT

typedef struct GameButtonState GameButtonState;
struct GameButtonState
{
    u32 half_trans_count; // button half transition
    b32 ended_down;
} ;

typedef struct GameControllerInput GameControllerInput;
struct GameControllerInput
{
    b32 is_connected;
    
    /// Stick Input
    b32 is_analog   ;
    f32 stick_avg_x ;
    f32 stick_avg_y ;
    
    /// Buttons Input
    union
    {
        GameButtonState buttons[12];
        struct
        {
            // NOTE(MIGUEL): THIS WASNT MEANT TO REPRESENT THE SPECIFIC BUTTON ON A CONTROLLER
            //               JUST THE BEHAVIOR THAT IS SUPPOSED TO BE INFLUENCED BY A BUTTONS
            GameButtonState button_y; //Move Up
            GameButtonState button_a; //Move Down
            GameButtonState button_x; //Move Left
            GameButtonState button_b; //Move Right
            
            GameButtonState action_up   ;
            GameButtonState action_down ;
            GameButtonState action_right;
            GameButtonState action_left ;
            
            GameButtonState shoulder_left ;
            GameButtonState shoulder_right;
            
            GameButtonState button_start;
            GameButtonState button_back ;
            
            GameButtonState terminator;
        };
    };
};

typedef struct GameInput GameInput;
struct GameInput
{
    // TODO(MIGUEL): verify mouse buttons work HMH 023 - 025
    GameButtonState mouse_buttons[5];
    
    s32 mouse_x;
    s32 mouse_y;
    s32 mouse_z;
    
    f32 mouse_wheel_delta;
    f32 mouse_wheel_integral;
    
    // NOTE(MIGUEL): seconds_to_advance_over_update
    f32 delta_t;
    
    GameControllerInput controllers[5];
};

//~ MEMORY

typedef struct GameMemory GameMemory;
struct GameMemory
{
    b32 is_initialized   ;
    void *permanent_storage;
    u64   permanent_storage_size;
    void *transient_storage;
    u64   transient_storage_size;
    
    DEBUG_PlatformFreeFileMemory  *debug_platform_free_file_memory ;
    DEBUG_PlatformReadEntireFile  *debug_platform_read_entire_file ;
    DEBUG_PlatformWriteEntireFile *debug_platform_write_entire_file;
};

//~ GRAPHICS

typedef struct GameBackBuffer GameBackBuffer;
struct GameBackBuffer
{
    void *data     ;
    s32   width    ;
    s32   height   ;
    s32   pitch    ;
    u32   bytes_per_pixel;
};


//~ AUDIO

typedef struct GameSoundOutputBuffer GameSoundOutputBuffer;
struct GameSoundOutputBuffer
{
    u16 *samples          ;
    u32 sample_count      ;
    u32 samples_per_second;
    
};


#endif //SGE_PLATFORM_H
