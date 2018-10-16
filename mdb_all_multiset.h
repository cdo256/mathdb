#pragma once
#include "mdb_base.h"
#include <stdint.h>

typedef struct MDB_mset {
    uintptr_t s, c;
    uintptr_t* a;
} MDB_mset;

uintptr_t MDB_MSetAdd(MDB_mset* s, uintptr_t x); // return index of new x or ~0U on fail
uintptr_t MDB_MSetRemove(MDB_mset* s, uintptr_t x);// remove last occurrence only, returns index x used to occupy or ~0U if not found
uintptr_t MDB_MSetContains(MDB_mset* s, uintptr_t x); // returns last index of x or ~0U
void MDB_FreeMSet(MDB_mset* s);
