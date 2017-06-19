#ifndef __NY_TYPES_H__
#define __NY_TYPES_H__

#ifdef MSVC

typedef signed    __int8    sint8;
typedef unsigned  __int8    uint8;
typedef signed    __int16   sint16;
typedef unsigned  __int16   uint16;
typedef signed    __int32   sint32;
typedef unsigned  __int32   uint32;
typedef signed    __int64   sint64;
typedef unsigned  __int64   uint64;

#else

#include <stdint.h>

typedef int8_t    sint8;
typedef uint8_t    uint8;
typedef int16_t   sint16;
typedef uint16_t   uint16;
typedef int32_t   sint32;
typedef uint32_t   uint32;
typedef int64_t   sint64;
typedef uint64_t   uint64;

#endif

#endif
