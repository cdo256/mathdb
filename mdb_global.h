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

typedef uintptr_t UP;
#define PS (sizeof(void*))
static_assert(PS == sizeof(UP), "unexpected pointer size");
static_assert(PS >= 4, "pointer must be at least 32bits");
