#pragma once
#include "mdb_base.h"
#include "mdb_generic_map.h"

MDB_generic_map* MDB_stdcall MDB_CreateGMap(uintptr_t size);
void MDB_stdcall MDB_FreeGMap(MDB_generic_map* map);
uintptr_t* MDB_stdcall MDB_GLookup(MDB_generic_map* map, uintptr_t n);

// returns 1 if the map grew successfully, 2 if no map grow was required and 0 on failure to grow
// map is left in-tact on failure.
int32_t MDB_stdcall MDB_GrowGMap(MDB_generic_map* m, uintptr_t c);
