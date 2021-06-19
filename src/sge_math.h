/* date = May 23rd 2021 2:44 am */

#ifndef SGE_MATH_H
#define SGE_MATH_H

#include "LAL.h"

typedef union v2 v2;
union v2
{
    struct
    {
        f32 x;
        f32 y;
    };
    f32 e[2];
    
};

inline v2
vec_negate_v2(v2 a)
{
    v2 result;
    
    result.x = -a.x;
    result.y = -a.y;
    
    return result;
}

inline void
vec_scale_v2(f32 scalar, v2 *dest)
{
    v2 *result = dest;
    
    result->x = dest->x * scalar;
    result->y = dest->y * scalar;
    
    return;
}

inline v2
vec_sub_v2(v2 a, v2 b)
{
    v2 result;
    
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    
    return result;
}

inline void
vec_add_v2(v2 a, v2 b, v2 *dest)
{
    v2 *result = dest;
    
    result->x = a.x + b.x;
    result->y = a.y + b.y;
    
    return;
}

// aka inner product
inline f32
vec_dot_v2(v2 a, v2 b)
{
    f32 result = (a.x * b.x) + (a.y * b.y);
    
    return result;
}

inline f32
vec_length_sq_v2(v2 a)
{
    f32 result = vec_dot_v2(a, a);
    
    return result;
}

inline f32
square(f32 a)
{
    f32 result = a * a;
    
    return result;
}

inline f32
square_root(f32 a)
{
    f32 result = sqrtf(a);
    
    return result;
}

#endif //SGE_MATH_H
