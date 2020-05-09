#pragma once
#include "mdb_base.h"


typedef struct MDB_vector {
    uintptr_t s, c;
    uintptr_t* a;
} MDB_vector;

void MDB_stdcall MDB_FreeVector(MDB_vector* v);
uintptr_t MDB_stdcall MDB_SetVectorSize(MDB_vector* v, uintptr_t size, uintptr_t fill); // sets exact size, fills with fill
uintptr_t MDB_stdcall MDB_GrowVector(MDB_vector* v, uintptr_t size, uintptr_t fill); // geometric growth
uintptr_t MDB_stdcall MDB_VectorPush(MDB_vector* v, uintptr_t x);
