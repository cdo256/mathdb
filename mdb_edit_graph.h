#include "mdb_base.h"
#include "mdb_graph.h"

MDB_SKETCH MDB_stdcall
MDB_StartSketch(void);
MDB_NODE MDB_stdcall
MDB_SketchNode(MDB_SKETCH sketch, MDB_NODETYPE type);
MDB_NODE MDB_stdcall
MDB_CreateConst(const char* name);
void MDB_stdcall
MDB_SetSketchRoot(MDB_SKETCH sketch, MDB_NODE node);
//TODO: this may fail unexpectedly (eg. if it creates cycles)
// the method should check and return a bool/reason-code
void MDB_stdcall
MDB_AddLink(MDB_NODE src, MDB_LINKDESC link, MDB_NODE dst);
void MDB_stdcall
MDB_DiscardSketchNode(MDB_NODE node);
void MDB_stdcall
MDB_DiscardSketch(MDB_SKETCH sketch);
void MDB_stdcall
MDB_DiscardLink(MDB_NODE src, MDB_LINKDESC link, MDB_NODE dst);
int32_t MDB_stdcall
MDB_CommitSketch(MDB_SKETCH sketch);
