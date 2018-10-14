#include "mdb_global.h"
#include "mdb_edit_graph.h"
#include "mdb_read_graph.h"
#include "mdb_use_node_map.h"
#include "mdb_search_graph.h" //IMPLEMENTS
#include "mdb_node_map.h"
#include "mdb_create_node_map.h"
#include "mdb_free_node_map.h"

MDB_NODE getcapture(MDB_NODE n) {
    if (MDB_Type(n) == MDB_FORM && MDB_ChildCount(n) >= 2) return MDB_Child(n,1);
    else return 0;
}

static s32 mdb_MatchPatternR(MDB_NODE pattern, MDB_NODE target, MDB_NODE capture, MDB_NODEMAP m) {
    if (target != pattern) {
        if (getcapture(pattern) == capture) {
            MDB_WriteNodeMapEntry(m, pattern, target);
            return 1;
        }
        if (MDB_ChildCount(pattern) != MDB_ChildCount(target) ||
            MDB_Type(pattern) != MDB_Type(target)) return 0;
        for (UP i = 0; i < MDB_ChildCount(target); i++) {
            if (!mdb_MatchPatternR(MDB_Child(pattern, i), MDB_Child(target, i), capture, m))
                return 0;
        }
        return (MDB_Type(pattern) != MDB_CONST);
    } else {
        MDB_WriteNodeMapEntry(m, pattern, target);
        return 1;
    }
}

MDB_NODEMAP MDB_stdcall MDB_MatchPattern(
    MDB_NODE pattern, MDB_NODE target, MDB_NODE capture) {

    MDB_NODEMAP m = MDB_CreateNodeMap(11);
    if (!m) return 0;
    if (!mdb_MatchPatternR(pattern, target, capture, m)) {
        MDB_FreeNodeMap(m); m = 0;
    }
    return m;
}

MDB_NODE MDB_stdcall MDB_MatchAllNodes(
    MDB_NODE pattern,
    MDB_NODE collection,
    MDB_NODE capture) {

    MDB_DRAFT s = MDB_StartDraft();
    MDB_NODE output = MDB_DraftNode(s, MDB_WORLD);
    MDB_SetDraftRoot(s, output);
    for (UP i = 0; i < MDB_ChildCount(collection); i++) {
        MDB_NODEMAP m = MDB_MatchPattern(pattern, MDB_Child(collection,i), capture);
        if (m) MDB_AddLink(output, MDB_ELEM, MDB_Child(collection,i));
        MDB_FreeNodeMap(m);
    }
    MDB_CommitDraft(s);
    return output;
}
