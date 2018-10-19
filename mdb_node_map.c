#include "mdb_global.h"
#include "mdb_free_node_map.h" //IMPLEMENTS
#include "mdb_all_generic_map.h"

MDB_NODEMAP MDB_stdcall MDB_CreateNodeMap(UP size) {
    return MDB_CreateGMap(size);
}

void MDB_stdcall MDB_FreeNodeMap(MDB_NODEMAP map) {
    MDB_FreeGMap(map);
}
MDB_NODE MDB_stdcall MDB_MapNode(MDB_NODEMAP map, MDB_NODE node) {
    assert(node);
    VAL* k = MDB_GLookup(map, node);
    if (k[0]) return k[1];
    else return 0;
}
void MDB_stdcall
MDB_WriteNodeMapEntry(MDB_NODEMAP map, MDB_NODE src, MDB_NODE dst) {
    assert(src && dst && dst != DELETED);
    VAL* k = MDB_GLookup(map, src);
    if (k[0]);
    else if (k[1] != DELETED) map->c++;
    else map->d--;
    k[0] = src; k[1] = dst;
}
int32_t MDB_stdcall
MDB_GrowNodeMap(MDB_NODEMAP map, UP inc) {
    return MDB_GrowGMap(map, inc);
}

void MDB_stdcall MDB_RemoveEntry(MDB_NODEMAP map, MDB_NODE node) {
    assert(node);
    VAL* k = MDB_GLookup(map, node);
    if (k[0]) {
        map->d++;
        k[0] = 0;
        k[1] = DELETED;
    }
}
