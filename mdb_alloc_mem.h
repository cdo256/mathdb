#pragma once
#include "mdb_base.h"


void* MDB_stdcall MDB_CAlloc(UP count, UP size);
void* MDB_stdcall MDB_Alloc(UP size);
void* MDB_stdcall MDB_Realloc(void* block, UP size);
void MDB_stdcall MDB_Free(void* block);
