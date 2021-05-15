/* date = August 26th 2020 2:08 am */

#ifndef LAL_H
#define LAL_H

#include <stdint.h>

//#define true  (1)
//#define false (0)

#define ASSERT(Expression) if(!(Expression)){ *(u32 *)0x00 = 0; }

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

#endif //LAL_H

