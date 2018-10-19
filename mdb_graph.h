#pragma once
#include "mdb_base.h"
typedef struct mdb_node* MDB_NODE;
typedef struct mdb_draft* MDB_DRAFT;
typedef uintptr_t MDB_NODETYPE;
typedef uintptr_t MDB_LINKDESC;
#define MDB_WORLD 0x0001UL
#define MDB_CONST 0x0002UL
#define MDB_FORM 0x0003UL
#define MDB_POCKET 0x0004UL

#define MDB_APPLY 0x0001UL
#define MDB_ARG 0x1000UL
#define MDB_ARG0 0x1000UL
#define MDB_ARG1 0x1001UL
#define MDB_ARG2 0x1002UL
#define MDB_ARG3 0x1003UL
#define MDB_ARG4 0x1004UL
#define MDB_ARG5 0x1005UL
#define MDB_ELEM 0x0002UL
