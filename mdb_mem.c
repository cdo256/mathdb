#include "mdb_global.h"
#include "mdb_alloc_mem.h" //IMPLEMENTS
#include "mdb_get_mem_info.h" //IMPLEMENTS
#include <stdlib.h>

static UP _allocations = 0;

void* MDB_stdcall MDB_CAlloc(UP count, UP size) {
    void* block = calloc(count, size);
    if (block) _allocations++;
    return block;
}
void* MDB_stdcall MDB_Alloc(UP size) {
    void* block = malloc(size);
    if (block) _allocations++;
    return block;
}
void* MDB_stdcall MDB_Realloc(void* block, UP size) {
    assert(size != 0);
    if (!block) _allocations++;
    return realloc(block, size);
}
void MDB_stdcall MDB_Free(void* block) {
    if (block) _allocations--;
    free(block);
}

UP MDB_stdcall MDB_GetAllocatedBlockCount(void) {
    return _allocations;
}
