#pragma once
#include "mdb_base.h"

// This file contains arch checks and convienience macros.
// Don't include from external interfaces, use mdb_base.h instead.
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>

#if defined(_DEBUG) || defined(WIN32_DEBUG)
#define DEBUG 1
#else
#define DEBUG 0
#endif

#if DEBUG
#define verify(x) assert(x)
#else
#define verify(x) ((void)x)
#endif
#define fail() assert(0)

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;
typedef char c8;

// We only support architectures that have consistent data pointer sizes
#define UNSUPPORTED "Unsupported architecture: "
typedef uintptr_t UP; // Used basically everywhere

#if UINTPTR_MAX == UINT64_MAX
#define PS 8
#elif UINTPTR_MAX == UINT32_MAX
#define PS 4
#else
#error UNSUPPORTED "data pointer must be either 32bits or 64bits"
#endif

static_assert(PS == sizeof(void*), UNSUPPORTED "unexpected data pointer size");
static_assert(PS == sizeof(char*) && PS == sizeof(long long*),
    UNSUPPORTED "inconsistent data pointer sizes");
static_assert((uintptr_t)NULL == 0, UNSUPPORTED "non-zero null");

#ifdef _MSC_VER
#pragma warning(disable:4820) // Ignore struct size since I want it to work on both 64-bit and 32-bit arch
#pragma warning(disable:4710) // Ignore 'funciton not inlined' since it's triggered by the likes of printf
#pragma warning(disable:4204) // Non-constant initializer is now in C99
#endif
