#pragma once
#include "mdb_base.h"

typedef struct MDB_generic_map {
    uintptr_t c,s,d;
    VAL* a;
} MDB_generic_map;
