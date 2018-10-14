#pragma once
#include "mdb_base.h"
#include "mdb_graph.h"
#include "mdb_node_map.h"

MDB_NODE* MDB_stdcall MDB_LookupNode(MDB_NODEMAP map, MDB_NODE n);
void MDB_stdcall MDB_WriteNodeMapEntry(MDB_NODEMAP map, MDB_NODE src, MDB_NODE dst);
int32_t MDB_stdcall MDB_GrowNodeMap(MDB_NODEMAP map, uintptr_t inc);
uintptr_t MDB_NodeMapElemCount(MDB_NODEMAP map);
MDB_NODE MDB_stdcall MDB_MapNode(MDB_NODEMAP map, MDB_NODE node);
void MDB_stdcall MDB_RemoveEntry(MDB_NODEMAP map, MDB_NODE node);