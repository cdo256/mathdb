#pragma once
#include "mdb_base.h"

MDB_NODETYPE MDB_stdcall MDB_Type(MDB_NODE node);
uintptr_t MDB_stdcall MDB_ChildCount(MDB_NODE node);
char const* MDB_stdcall MDB_NodeName(MDB_NODE node);
uintptr_t MDB_stdcall MDB_Child(MDB_NODE node, uintptr_t idx);
