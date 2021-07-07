/* date = May 23rd 2021 2:44 am */

#ifndef SGE_MATH_H
#define SGE_MATH_H

#include "sge_platform.h"

typedef union V2 V2;
union V2
{
    struct
    {
        f32 x;
        f32 y;
    };
    f32 e[2];
    
};

inline V2
V2_negate(V2 a)
{
    V2 result;
    
    result.x = -a.x;
    result.y = -a.y;
    
    return result;
}

inline void
V2_scale(f32 scalar, V2 *dest)
{
    V2 *result = dest;
    
    result->x = dest->x * scalar;
    result->y = dest->y * scalar;
    
    return;
}

inline void
V2_sub(V2 a, V2 b, V2 *dest)
{
    V2 *result = dest;
    
    result->x = a.x - b.x;
    result->y = a.y - b.y;
    
    return;
}

inline void
V2_add(V2 a, V2 b, V2 *dest)
{
    V2 *result = dest;
    
    result->x = a.x + b.x;
    result->y = a.y + b.y;
    
    return;
}

inline void
V2_hadamard(V2 a, V2 b, V2 *dest)
{
    V2 *result = dest;
    
    result->x = a.x * b.x;
    result->y = a.y * b.y;
    
    return;
}

// aka inner product
inline f32
V2_dot(V2 a, V2 b)
{
    f32 result = (a.x * b.x) + (a.y * b.y);
    
    return result;
}

inline f32
V2_length_sq(V2 a)
{
    f32 result = V2_dot(a, a);
    
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

typedef struct RectV2 RectV2;
struct RectV2
{
    V2 min;
    V2 max;
};

inline RectV2
RectV2_min_max(V2 min, V2 max)
{
    RectV2 result;
    
    result.min = min;
    result.max = max;
    
    return result;
}

inline RectV2
RectV2_min_dim(V2 min, V2 max)
{
    RectV2 result;
    
    result.min = min;
    V2_add(min, max, &result.max);
    
    return result;
}


inline RectV2
RectV2_center_half_dim(V2 center, V2 half_dim)
{
    RectV2 result;
    
    V2_sub(center, half_dim, &result.min);
    V2_add(center, half_dim, &result.max);
    
    return result;
}

inline RectV2
RectV2_center_dim(V2 center, V2 dim)
{
    V2_scale(0.5f, &dim);
    RectV2 result = RectV2_center_half_dim(center, dim);
    
    return result;
}

inline b32
RectV2_is_in_rect(RectV2 rect, V2 test_position)
{
    b32 result = ((test_position.x >= rect.min.x) &&
                  (test_position.y >= rect.min.y) &&
                  (test_position.x <= rect.max.x) &&
                  (test_position.y <= rect.max.y));
    
    return result;
}


#endif //SGE_MATH_H
