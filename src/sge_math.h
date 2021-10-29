/* date = May 23rd 2021 2:44 am */

#ifndef SGE_MATH_H
#define SGE_MATH_H

#include "sge_platform.h"

//~ General Functions

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


//~ VECTORS
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

typedef union V3 V3;
union V3
{
    struct
    {
        f32 x;
        f32 y;
        f32 z;
    };
    struct
    {
        V2 xy;
        V2 ignored_;
    };
    struct
    {
        f32 r;
        f32 g;
        f32 b;
    };
    struct
    {
        f32 w;
        f32 h;
        f32 d;
    };
    f32 e[3];
    
};

typedef union V4 V4;
union V4
{
    struct
    {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };
    struct
    {
        f32 r;
        f32 g;
        f32 b;
        f32 a;
    };
    struct
    {
        V2 xy;
        V2 zw;
    };
    struct
    {
        V2 xyx;
        V2 ignored_;
    };
    f32 e[4];
};

//~ RECTANGLES


typedef struct RectV2 RectV2;
struct RectV2
{
    V2 min;
    V2 max;
};

typedef struct RectV3 RectV3;
struct RectV3
{
    V3 min;
    V3 max;
};


//~ VECTOR 2D

inline V2
V2_init_uniform(f32 a)
{
    V2 result;
    
    result.x = a;
    result.y = a;
    
    return result;
}

inline V2
V2_init_2f32(f32 x, f32 y)
{
    V2 result;
    
    result.x = x;
    result.y = y;
    
    return result;
}

inline V2
V2_init_f32(f32 x)
{
    V2 result;
    
    result.x = x;
    result.y = 0.0f;
    
    return result;
}

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

/// DOT PRODUCT
inline f32
V2_inner(V2 a, V2 b)
{
    f32 result = (a.x * b.x) + (a.y * b.y);
    
    return result;
}

inline f32
V2_length_sq(V2 a)
{
    f32 result = V2_inner(a, a);
    
    return result;
}

inline f32
V2_length(V2 a)
{
    f32 result = square_root(V2_length_sq(a)); 
    
    return result;
}

//~ VECTOR 3D

inline V3
V3_init_uniform(f32 a)
{
    V3 result;
    
    result.x = a;
    result.y = a;
    result.z = a;
    
    return result;
}

inline V3
V3_init_3f32(f32 x, f32 y, f32 z)
{
    V3 result;
    
    result.x = x;
    result.y = y;
    result.z = z;
    
    return result;
}

inline V3
V3_init_2f32(f32 x, f32 y)
{
    V3 result;
    
    result.x = x;
    result.y = y;
    result.z = 0.0f;
    
    return result;
}

inline V3
V3_init_v2(V2 xy, f32 z)
{
    V3 result;
    
    result.xy = xy;
    result.z  =  z;
    
    return result;
}

inline b32
V3_are_equal(V3 a, V3 b)
{
    b32 result = ((a.x == b.x) &&
                  (a.y == b.y) &&
                  (a.z == b.z));
    
    return result;
}

inline V3
V3_negate(V3 a)
{
    V3 result;
    
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    
    return result;
}

inline void
V3_scale(f32 scalar, V3 *dest)
{
    V3 *result = dest;
    
    result->x = dest->x * scalar;
    result->y = dest->y * scalar;
    result->z = dest->z * scalar;
    
    return;
}

inline void
V3_sub(V3 a, V3 b, V3 *dest)
{
    V3 *result = dest;
    
    result->x = a.x - b.x;
    result->y = a.y - b.y;
    result->z = a.z - b.z;
    
    return;
}

inline void
V3_add(V3 a, V3 b, V3 *dest)
{
    V3 *result = dest;
    
    result->x = a.x + b.x;
    result->y = a.y + b.y;
    result->z = a.z + b.z;
    
    return;
}

inline void
V3_hadamard(V3 a, V3 b, V3 *dest)
{
    V3 *result = dest;
    
    result->x = a.x * b.x;
    result->y = a.y * b.y;
    result->z = a.z * b.z;
    
    return;
}

// TODO(MIGUEL): BROKEN AF!!! FIND THE CORRECT MATH!!!
/// DOT PRODUCT 
inline f32
V3_inner(V3 a, V3 b)
{
    f32 result =
        (a.x * b.x) +
        (a.y * b.y) +
        (a.z * b.z);
    
    return result;
}

inline f32
V3_length_sq(V3 a)
{
    f32 result = V3_inner(a, a);
    
    return result;
}

inline f32
V3_length(V3 a)
{
    f32 result = square_root(V3_length_sq(a)); 
    
    return result;
}


//~ VECTOR 4D
inline V4
V4_negate(V4 a)
{
    V4 result;
    
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    result.w = -a.w;
    
    return result;
}

inline void
V4_scale(f32 scalar, V4 *dest)
{
    V4 *result = dest;
    
    result->x = dest->x * scalar;
    result->y = dest->y * scalar;
    result->z = dest->z * scalar;
    result->w = dest->w * scalar;
    
    return;
}

inline void
V4_sub(V4 a, V4 b, V4 *dest)
{
    V4 *result = dest;
    
    result->x = a.x - b.x;
    result->y = a.y - b.y;
    result->z = a.z - b.z;
    result->w = a.w - b.w;
    
    return;
}

inline void
V4_add(V4 a, V4 b, V4 *dest)
{
    V4 *result = dest;
    
    result->x = a.x + b.x;
    result->y = a.y + b.y;
    result->z = a.z + b.z;
    result->w = a.w + b.w;
    
    return;
}

inline void
V4_hadamard(V4 a, V4 b, V4 *dest)
{
    V4 *result = dest;
    
    result->x = a.x * b.x;
    result->y = a.y * b.y;
    result->z = a.z * b.z;
    result->w = a.w * b.w;
    
    return;
}

// aka inner product
inline f32
V4_dot(V4 a, V4 b)
{
    f32 result =
        (a.x * b.x) +
        (a.y * b.y) +
        (a.z * b.z) +
        (a.w * b.w);
    
    return result;
}

inline f32
V4_length_sq(V4 a)
{
    f32 result = V4_dot(a, a);
    
    return result;
}

//~ RECTANGLE 2D

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

inline V2
RectV2_min_corner(RectV2 rect)
{
    V2 result = rect.min;
    
    return result;
}

inline V2
RectV2_max_corner(RectV2 rect)
{
    V2 result = rect.max;
    
    return result;
}

inline RectV2 
RectV2_add_radius_to(RectV2 a, f32 radius_w, f32 radius_h)
{
    RectV2 result;
    
    V2_sub(a.min, (V2){radius_w, radius_h}, &result.min);
    V2_add(a.max, (V2){radius_w, radius_h}, &result.max);
    
    return result;
}


//~ RECTANGLE 3D

inline RectV3
RectV3_min_max(V3 min, V3 max)
{
    RectV3 result;
    
    result.min = min;
    result.max = max;
    
    return result;
}

inline b32
RectV3_is_in_rect(RectV3 rect, V3 test_position)
{
    b32 result = ((test_position.x >= rect.min.x) &&
                  (test_position.x <= rect.max.x) &&
                  (test_position.y >= rect.min.y) &&
                  (test_position.y <= rect.max.y) &&
                  (test_position.z >= rect.min.z) &&
                  (test_position.z <= rect.max.z));
    
    return result;
}


inline V3
RectV3_min_corner(RectV3 rect)
{
    V3 result = rect.min;
    
    return result;
}

inline V3
RectV3_max_corner(RectV3 rect)
{
    V3 result = rect.max;
    
    return result;
}

inline RectV3 
RectV3_add_radius_to(RectV3 a, V3 radius)
{
    RectV3 result;
    
    V3_sub(a.min, radius, &result.min);
    V3_add(a.max, radius, &result.max);
    
    return result;
}

inline RectV3
RectV3_center_half_dim(V3 center, V3 half_dim)
{
    RectV3 result;
    
    V3_sub(center, half_dim, &result.min);
    V3_add(center, half_dim, &result.max);
    
    return result;
}


#endif //SGE_MATH_H

