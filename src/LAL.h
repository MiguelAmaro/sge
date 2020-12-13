/* date = August 26th 2020 2:08 am */

#ifndef LAL_H
#define LAL_H

#include <stdint.h>

#define readonly        const
#define internal        static
#define local_persist   static
#define global static
#define ASSERT(Expression) if(!(Expression)) { *(int*)0 = 0; }

/*
#ifndef true
#define true  (1)
#endif
#ifndef false
#define false (0)
#endif
*/
typedef uint8_t  u8 ;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8 ;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

// IEEE754 32-Bit & 64-Bit floating point
typedef float  f32;
typedef double f64;

// Booleans
typedef u8  b8;
typedef u32 b32;



#endif //LAL_H
