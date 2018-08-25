// This file contains arch checks and convienience macros.
// Don't include from external interfaces, use mdb_base.h instead.
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
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
typedef uintptr_t UP;
#define PS (sizeof(void*))

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
static_assert(NULL == 0, UNSUPPORTED "non-zero null");
