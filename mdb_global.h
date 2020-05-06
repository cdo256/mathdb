#pragma once
#include "mdb_base.h"

// This file contains arch checks and convenience macros.
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
#define verify(x) ((void)(x))
#endif

// We only support architectures that have consistent data pointer sizes
#define UNSUPPORTED "Unsupported architecture: "
#if UINTPTR_MAX == UINT64_MAX
#define PS 8
#elif UINTPTR_MAX == UINT32_MAX
#define PS 4
#else
#error UNSUPPORTED "data pointer must be either 32bits or 64bits"
#endif

static_assert(PS == sizeof(void*), UNSUPPORTED "unexpected data pointer size");
static_assert(sizeof(char*) == sizeof(long long*) && sizeof(char*) == sizeof(void*),
    UNSUPPORTED "inconsistent data pointer sizes");

#ifdef _MSC_VER
#pragma warning(disable:4820) // Ignore struct size since I want it to work on both 64-bit and 32-bit arch
#pragma warning(disable:4710) // Ignore 'funciton not inlined' since it's triggered by the likes of printf
#pragma warning(disable:4204) // Non-constant initializer is now in C99
#endif
