#pragma once
#include "mdb_base.h"
#include "mdb_node_map.h"

// Returns a map from 'pattern' nodes (captured by 'capture') to 'target' nodes if they match.
// Returns 0 if no match or an error occurred.
MDB_NODEMAP MDB_stdcall MDB_MatchPattern(
    MDB_NODE pattern,
    MDB_NODE target,
    MDB_NODE capture);

// Returns a collection of all nodes in 'collection' that match 'pattern' with variables captured by 'capture'.
MDB_NODE MDB_stdcall MDB_MatchAllNodes(
    MDB_NODE pattern,
    MDB_NODE collection,
    MDB_NODE capture);
