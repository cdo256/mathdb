#include "mdb_global.h"
#include "mdb_alloc_mem.h" //IMPLEMENTS
#include "mdb_get_mem_info.h" //IMPLEMENTS
#include <stdlib.h>

static UP _allocations = 0;

void* MDB_stdcall MDB_Alloc(UP size) {
    void* block = malloc(size);
    if (block) _allocations++;
    return block;
}
void* MDB_stdcall MDB_Realloc(void* block, UP size) {
    if (!block) _allocations++;
    return realloc(block, size);
}
void MDB_stdcall MDB_Free(void* block) {
    if (block) _allocations--;
    return free(block);
}

UP MDB_stdcall MDB_GetAllocatedBlockCount() {
    return _allocations;
}
