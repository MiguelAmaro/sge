/* date = May 9th 2021 8:26 pm */

#ifndef SGE_INTRINSICS_H
#define SGE_INTRINSICS_H

#include "math.h"

inline s32
round_f32_to_s32(f32 value)
{
    s32 result = (s32)(value + 0.5f);
    
    return result;
}

inline u32
round_f32_to_u32(f32 value)
{
    u32 result = (u32)(value + 0.5f);
    
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



#endif //SGE_INTRINSICS_H
