#include "mdb_graph.h"

MDB_SKETCH __stdcall MDB_StartSketch();
MDB_NODE __stdcall MDB_SketchNode(MDB_SKETCH sketch, MDB_NODETYPE type);
MDB_NODE __stdcall MDB_CreateConst(u8* name);
void __stdcall MDB_SetSketchRoot(MDB_SKETCH sketch, MDB_NODE node);
void __stdcall MDB_AddLink(MDB_NODE src, MDB_LINKDESC link, MDB_NODE dst);
void __stdcall MDB_DiscardSketchNode(MDB_NODE node);
void __stdcall MDB_DiscardSketch(MDB_SKETCH sketch);
void __stdcall MDB_DiscardLink(MBD_NODE src, MDB_LINKDESC link, MDB_NODE dst);
void __stdcall MDB_FinishSketch(MDB_SKETCH sketch);
