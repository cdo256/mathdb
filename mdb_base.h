#pragma once
#include <stdint.h>
#ifdef WIN32
#define MDB_stdcall __stdcall
#else
#define MDB_stdcall
#endif

#define MDB_DEBUG_MEM 0
