#include "mdb_base.h"
#include <stdint.h>

typedef struct MDB_generic_map {
	uintptr_t c,s,d;
	uintptr_t* a;
} MDB_generic_map;

MDB_generic_map* MDB_stdcall MDB_CreateGMap(uintptr_t size);
void MDB_stdcall MDB_FreeGMap(MDB_generic_map* map);
uintptr_t* MDB_stdcall MDB_GLookup(MDB_generic_map* map, uintptr_t n);

// returns 1 if the map grew successfully, 2 if no map grow was required and 0 on failure to grow
// map is left in-tact on failure.
int32_t MDB_stdcall MDB_GrowGMap(MDB_generic_map* m, uintptr_t c);
