#pragma once
#include <stdint.h>
#ifdef WIN32
#define MDB_stdcall __stdcall
#else
#define MDB_stdcall
#endif
typedef void* VAL;
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;
typedef char c8;
typedef uintptr_t UP;
