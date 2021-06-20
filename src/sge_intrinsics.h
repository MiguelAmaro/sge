/* date = May 9th 2021 8:26 pm */

#ifndef SGE_INTRINSICS_H
#define SGE_INTRINSICS_H

#include "math.h"


inline s32
sign_of(s32 value)
{
    s32 result =  (value >= 0) ? 1 : -1;
    
    return result;
}

inline f32
absolute_value(f32 value)
{
    f32 result = fabsf(value);
    
    return result;
}

inline s32
round_f32_to_s32(f32 value)
{
    s32 result = (s32)roundf(value);
    
    return result;
}

inline u32
round_f32_to_u32(f32 value)
{
    u32 result = (u32)roundf(value);
    
    return result;
}


inline s32
floor_f32_to_s32(f32 value)
{
    s32 result = (s32)floorf(value);
    
    return result;
}

inline s32
ceiling_f32_to_s32(f32 value)
{
    s32 result = (s32)ceilf(value);
    
    return result;
}

inline s32
truncate_f32_to_s32(f32 value)
{
    s32 result = (s32)value;
    
    return result;
}


inline f32
math_sin(f32 value)
{
    f32 result = sinf(value);
    
    return result;
}


inline f32
math_cos(f32 value)
{ 
    f32 result = cosf(value);
    
    return result;
}

inline f32
math_atan2(f32 x, f32 y)
{ 
    f32 result = atan2f(x, y);
    
    return result;
}


typedef struct bit_scan_result
{
    b32 found;
    u32 index;
} bit_scan_result;


inline bit_scan_result
find_least_significant_set_bit(u32 value)
{
    bit_scan_result result = {0};
    
    
#if COMPILER_MSVC
    result.found = _BitScanForward(&result.index, value);
#else
    for(u32 test = 0; test < 32; ++test)
    {
        if(value & (1 << test))
        {
            result.index = test;
            result.found = 1;
            
            break;
        }
    }
    
#endif
    
    return result;
}


#endif //SGE_INTRINSICS_H
