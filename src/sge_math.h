/* date = May 23rd 2021 2:44 am */

#ifndef SGE_MATH_H
#define SGE_MATH_H

#include "LAL.h"

typedef union
{
    struct
    {
        f32 x;
        f32 y;
    };
    f32 e[2];
    
} v2;

inline v2
vec_negate_v2(v2 a)
{
    v2 result;
    
    result.x = -a.x;
    result.y = -a.y;
    
    return result;
}

inline v2
vec_scale_v2(v2 a, f32 scalar)
{
    v2 result;
    
    result.x = a.x * scalar;
    result.y = a.y * scalar;
    
    return result;
}

inline v2
vec_sub_v2(v2 a, v2 b)
{
    v2 result;
    
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    
    return result;
}

inline v2
vec_add_v2(v2 a, v2 b)
{
    v2 result;
    
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    
    return result;
}

inline f32
square(f32 a)
{
    f32 result = a * a;
    
    return result;
}


#endif //SGE_MATH_H
