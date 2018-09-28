#include "mdb_base.h"
#include "mdb_graph.h"

MDB_DRAFT MDB_stdcall
MDB_StartDraft(void);
MDB_NODE MDB_stdcall
MDB_DraftNode(MDB_DRAFT draft, MDB_NODETYPE type);
MDB_NODE MDB_stdcall
MDB_CreateConst(const char* name);
void MDB_stdcall
MDB_SetDraftRoot(MDB_DRAFT draft, MDB_NODE node);
//TODO: this may fail unexpectedly (eg. if it creates cycles)
// the method should check and return a bool/reason-code
void MDB_stdcall
MDB_AddLink(MDB_NODE src, MDB_LINKDESC link, MDB_NODE dst);
void MDB_stdcall
MDB_DiscardDraftNode(MDB_NODE node);
void MDB_stdcall
MDB_DiscardDraft(MDB_DRAFT draft);
void MDB_stdcall
MDB_DiscardLink(MDB_NODE src, MDB_LINKDESC link, MDB_NODE dst);
int32_t MDB_stdcall
MDB_CommitDraft(MDB_DRAFT draft);
