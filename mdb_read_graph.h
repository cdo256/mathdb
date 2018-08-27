#include "mdb_base.h"

void MDB_stdcall
MDB_GetNodeInfo(MDB_NODE node,
    MDB_NODETYPE* type, uintptr_t* childCount, char** str);

// returns number of children written
uintptr_t MDB_stdcall
MDB_GetChildren(MDB_NODE node,
    uintptr_t startIndex, uintptr_t bufferCount, MDB_NODE* buffer);
