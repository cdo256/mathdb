#pragma once
#include "mdb_base.h"
#include "mdb_error.h"

// Fetches and clears the next error, returning 0 iff there are no more errors
// Errors may not be retrieved in the same order they occurred.
// Multiple errors will be returned as a single error.
// You shouldn't free error->str
int32_t MDB_stdcall MDB_GetError(MDB_error* error, int32_t clear);
