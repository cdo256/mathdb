#include "mdb_global.h"
#include "mdb_graph.h"
#include "mdb_edit_graph.h" //IMPLEMENTS
#include "mdb_read_graph.h" //IMPLEMENTS
#include "mdb_error.h"
#include "mdb_alloc_mem.h"
#include <stdlib.h>
#include <string.h>

#define malloc MDB_Alloc
#define free MDB_Free
#define realloc MDB_Realloc

#define MDB_SKETCHFLAG 0x0100U
#define MDB_NODETYPEMASK 0x00FFU

// _errorFlags can be cleared by the user calling MDB_GetError() but _state
// must be cleared by recreating the graph.
static u32 _errorFlags = 0;
static u32 _state = MDB_EUNINIT;

typedef struct MDB_sketch {
    UP count, cap, index;
    MDB_NODE* nodes;
} MDB_sketch;

// strongly-connected component
typedef struct MDB_scc {
    UP count;       // count of n
    MDB_NODE* n;    // array of nodes in the scc
} MDB_scc;

typedef struct MDB_node {
    MDB_NODETYPE type;
    MDB_scc* g;         // a pointer to the scc when we're no longer a sketch
    MDB_SKETCH sketch;  // which sketch are we
    UP index;           // what index are we in this sketch?
    UP count, cap;      // number of children and size of n respectively
    MDB_NODE* n;        // array of children
    c8* name;           // our name if we're a const
} MDB_node;

typedef struct MDB_id_table {
    UP end, cap; // a[0..end) is the part we've touched, |a| = cap
    UP sz;       // size of each entry
    UP freeList; // index of next free slot; end: 0, adv: freeList = *(UP*)&a[freeList*sz]
    u8* a;       // array of size cap; each entry either freeList index, MDB_node* or MDB_sketch*
    u8* freeBmp; // reserve but don't use until deinit time
} MDB_id_table;

void* MDB_IdTableEntry(MDB_id_table const* t, UP idx) {
    return (void*)&t->a[idx*t->sz];
}

static MDB_id_table _nodeTable;
static MDB_id_table _sketchTable;

static c8 const* const _errorStr[] = {
    [0]="no error",
    [MDB_EMEMBIT]="out of memory",
    [MDB_EUNINITBIT]="the graph has not been created",
    [MDB_EINVCALLBIT]="this function call was unexpected",
    [MDB_EINVARGBIT]="a function was called with an invalid argument",
    [MDB_EINCOMPLETESKETCHBIT]="the sketch is not complete",
    [MDB_EINTOVERFLOWBIT]="an integer overflow has occurred. Consider running as 64bit",
};

s32 MDB_stdcall MDB_GetError(MDB_error* e) {
    if (!_errorFlags) {
        e->str = _errorStr[0];
        e->id = MDB_ENONE;
        _state &= MDB_EUNINIT; // clear all but the uninit bit.
        return 0;
    }
    for (int i = 0; i < 32; i++) {
        if ((1U<<i)&_errorFlags) {
            e->str = _errorStr[i];
            e->id = (1U<<i);
            _errorFlags &= ~e->id;
            return 1;
        }
    }
    return 0;
}

void MDB_SignalError(UP type) {
    _errorFlags |= type;
    _state |= _errorFlags;
    fail();
}
#define error(t) MDB_SignalError(t)

static inline UP MDB_Min(UP x, UP y) {
    return x > y ? y : x;
}
static inline UP MDB_Max(UP x, UP y) {
    return x < y ? y : x;
}

// Frees scc but doesn't update _nodeTable or
void MDB_FreeNode(MDB_NODE node) {
    MDB_node* n = *(MDB_node**)MDB_IdTableEntry(&_nodeTable, node);
    if (n->g) {
        n->g->count--;
        if (n->g->count == 0) free(n->g);
    }
    free(n->n);
    free(n->name);
    free(n);
}

UP MDB_RoundUp(UP x) {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
#if PS > 4
    x |= x >> 32;
#endif
    return x+1;
}

// only works for Po2 x
s32 MDB_IntLogPo2(UP x) {
    assert(x != 0 && x == MDB_RoundUp(x));

// Duplicates to upper DWORD when using 64bit pointers
#if PS == 8
#define REP32(x) (((UP)(x) << 32) | (UP)(x))
#else
#define REP32(x) ((UP)(x))
#endif
    s32 r = 0;
#if PS > 4
    r += (x > UINT32_MAX) << 5;
#endif
    r += !!(x & REP32(0xFFFF0000UL)) << 4;
    r += !!(x & REP32(0xFF00FF00UL)) << 3;
    r += !!(x & REP32(0xF0F0F0F0UL)) << 2;
    r += !!(x & REP32(0xCCCCCCCCUL)) << 1;
    r += !!(x & REP32(0xAAAAAAAAUL)) << 0;
    return r;
#undef REP32
}
// succeeded iff result.a != 0
MDB_id_table MDB_CreateIdTable(UP slotSize, UP cap) {
    assert(cap >= 1);
    MDB_id_table t;
    t.sz = MDB_RoundUp(slotSize);
    assert(t.sz >= PS);
    t.cap = cap;
    t.end = 1;
    t.freeList = 0;
    t.freeBmp = malloc((cap+7)>>3);
    t.a = malloc(cap*PS);
    if (!t.freeBmp || !t.a) {
        error(MDB_EMEM);
        free(t.freeBmp); free(t.a);
        t.freeBmp = 0; t.a = 0;
        t.cap = 0;
    }
    return t;
}

// Only deletes table itself.
// Deleting entries needs to be done manually before calling this function.
void MDB_FreeIdTable(MDB_id_table* table) {
    free(table->a); free(table->freeBmp);
}

void MDB_stdcall MDB_CreateGraph() {
    if (!(_state & MDB_EUNINIT)) {
        error(MDB_EINVCALL);
        return;
    }
    _nodeTable = MDB_CreateIdTable(PS,65536);
    _sketchTable = MDB_CreateIdTable(sizeof(MDB_sketch),256);
    if (!_nodeTable.a || !_sketchTable.a) {
        error(MDB_EUNINIT);
        MDB_FreeIdTable(&_nodeTable);
        MDB_FreeIdTable(&_sketchTable);
        return;
    }
    _state &= ~MDB_EUNINIT;
}
void MDB_PopulateFreeBmp(MDB_id_table* t) {
    memset(t->freeBmp, 0, (t->end+7)>>3);
    for (UP p = t->freeList; p; p = (UP)t->a[p*t->sz]) {
        t->freeBmp[p>>3] |= 1 << (p & 7);
    }
}
s32 MDB_ReadBit(u8* bmp, UP idx) {
    return (bmp[idx>>3] >> (idx&7)) & 1;
}
void MDB_WriteBit(u8* bmp, UP idx, s32 val) {
    bmp[idx>>3] = (bmp[idx>>3] & ~(1<<(idx&7))) | val<<(idx&7);
}
void MDB_stdcall MDB_FreeGraph() {
    // Don't abort on error in cleanup function
    MDB_id_table* t = &_nodeTable;
    MDB_PopulateFreeBmp(t);
    for (UP i = 0; i < t->end; i++) {
        if (MDB_ReadBit(t->freeBmp, i)) {
            MDB_FreeNode(i);
        }
    }
    MDB_FreeIdTable(t);
    t = &_sketchTable;
    MDB_PopulateFreeBmp(t);
    for (UP i = 0; i < t->end; i++) if (MDB_ReadBit(t->freeBmp, i))
        free(((MDB_sketch*)MDB_IdTableEntry(t, i))->nodes);
    MDB_FreeIdTable(t);
    // We will not forget errors that may have occrred during this graph's lifetime.
    // They must be cleared explicitly with repeated MDB_GetError()'s.
    _state |= MDB_EUNINIT;
}

UP MDB_AllocTableSlot(MDB_id_table* t) {
    UP slot = 0;
    if (t->freeList) {
        slot = t->freeList;
        t->freeList = *(UP*)&t->a[t->freeList*t->sz];
    } else {
        if (t->end == t->cap) {
            UP cap = t->cap * 2; // may overflow on 32bit
            if (cap < t->cap) {
                error(MDB_EINTOVERFLOW);
                return 0;
            }
            u8* a = realloc(t->a, cap*t->sz);
            u8* bmp = malloc((cap+7)>>3);
            if (!a || !bmp) {
                error(MDB_EMEM);
                free(a); free(bmp);
                return 0;
            }
            t->cap = cap;
            free(t->freeBmp);
            t->freeBmp = bmp;
            t->a = a;
        }
        slot = t->end++;
    }
    return slot;
}

MDB_SKETCH MDB_stdcall MDB_StartSketch() {
    if (_state) return 0;
    MDB_SKETCH sketch = MDB_AllocTableSlot(&_sketchTable);
    if (!sketch) return 0; // error already signal, we have nothing to add
    MDB_sketch* s = MDB_IdTableEntry(&_sketchTable, sketch);
    s->index = sketch;
    s->count = 1; // start at one because the first slot is for the root
    s->cap = 256;
    s->nodes = calloc(s->cap, PS); // need zero-ing
    if (!s->nodes) {
        error(MDB_EMEM);
        *(UP*)s = _sketchTable.freeList;
        _sketchTable.freeList = sketch;
        return 0;
    } else return sketch;
}
void MDB_MoveSketchNode(MDB_sketch* s, MDB_NODE node, UP idx) {
    s->nodes[idx] = node;
    MDB_node* n = *(MDB_node**)MDB_IdTableEntry(&_nodeTable, node);
    n->index = idx;
}
void MDB_stdcall MDB_SetSketchRoot(MDB_SKETCH sketch, MDB_NODE node) {
    if (_state) return;
    MDB_sketch* s = MDB_IdTableEntry(&_sketchTable, sketch);
    assert(s->index == sketch);
    if (s->nodes[0] != 0) {
        error(MDB_EINVCALL);
        return;
    }
    UP oldIndex = (*(MDB_node**)MDB_IdTableEntry(&_nodeTable, node))->index;
    MDB_MoveSketchNode(s, node, 0);
    if (s->count <= 1) {
        error(MDB_EINVCALL);
        return;
    }
    MDB_MoveSketchNode(s, s->nodes[--s->count], oldIndex);
}

MDB_NODE MDB_stdcall MDB_SketchNode(MDB_SKETCH sketch, MDB_NODETYPE type) {
    if (_state) return 0;
    if (type & MDB_SKETCHFLAG || (type & MDB_NODETYPEMASK) == MDB_CONST) {
        error(MDB_EINVARG);
        return 0;
    }
    MDB_node* n = malloc(sizeof(MDB_node));
    if (!n) {
        error(MDB_EMEM);
        return 0;
    }
    n->type = type | MDB_SKETCHFLAG;
    n->g = 0;
    n->sketch = sketch;
    n->cap = 1; n->count = 0;
    n->n = calloc(PS,1);
    n->name = 0;
    if (!n->n) {
        error(MDB_EMEM);
        free(n);
        return 0;
    }
    MDB_sketch* s = MDB_IdTableEntry(&_sketchTable, sketch);
    if (s->count == s->cap) {
        UP cap = s->cap * 2;
        if (s->cap > cap) {
            error(MDB_EINTOVERFLOW);
            return 0;
        }
        MDB_NODE* nodes = realloc(s->nodes, cap*PS);
        if (!nodes) {
            error(MDB_EMEM);
            free(n->n); free(n);
            return 0;
        }
        s->cap = cap;
        s->nodes = nodes;
    }
    n->index = s->count;
    MDB_NODE node = MDB_AllocTableSlot(&_nodeTable);
    if (!node) {
        free(n->n); free(n);
        return 0;
    }
    *(MDB_node**)MDB_IdTableEntry(&_nodeTable, node) = n;
    s->nodes[s->count++] = node;
    return node;
}
MDB_NODE MDB_stdcall MDB_CreateConst(char const* name) {
    if (_state) return 0;
    MDB_node* n = malloc(sizeof(MDB_node));
    if (!n) {
        error(MDB_EMEM);
        return 0;
    }
    n->type = MDB_CONST;
    n->g = malloc(sizeof(MDB_scc));
    if (!n->g) {
        error(MDB_EMEM);
        free(n);
        return 0;
    }
    n->g->count = 1;
    n->g->n = malloc(PS);
    n->sketch = 0;
    n->index = n->count = n->cap = 0;
    n->n = 0;
    n->name = malloc(strlen(name)+1);
    strcpy(n->name, name);
    if (!n->g->n || !n->name) {
        error(MDB_EMEM);
        free(n->g->n); free(n->name);
        free(n->g); free(n);
        return 0;
    }
    MDB_NODE node = MDB_AllocTableSlot(&_nodeTable);
    if (!node) {
        free(n->g->n); free(n->name);
        free(n->g); free(n);
        return 0;
    }
    n->g->n[0] = node;
    *(MDB_node**)MDB_IdTableEntry(&_nodeTable, node) = n;
    return node;
}
void MDB_stdcall MDB_AddLink(MDB_NODE src, MDB_LINKDESC link, MDB_NODE dst) {
    if (_state) return;
    MDB_node* n = *(MDB_node**)MDB_IdTableEntry(&_nodeTable, src);

    MDB_NODETYPE type = n->type & MDB_NODETYPEMASK;
    if (!(n->type & MDB_SKETCHFLAG) && type != MDB_WORLD) {
        error(MDB_EINVARG);
        return;
    }
    UP idx = 0;
    if (type == MDB_FORM) {
        if (link == MDB_APPLY) {
            idx = 0;
        } else if (link & MDB_ARG) {
            idx = (link & ~MDB_ARG) + 1;
        } else {
            error(MDB_EINVARG);
            return;
        }
    } else if (link == MDB_ELEM) {
        idx = n->count;
    } else {
        error(MDB_EINVARG);
        return;
    }
    if (idx >= n->cap) {
        UP cap = MDB_RoundUp(idx);
        MDB_NODE* a = realloc(n->n, cap*PS);
        if (a) {
            n->n = a;
            n->cap = cap;
            for (int i = n->cap; i < cap; i++)
                n->n[i] = 0;
        } else {
            error(MDB_EMEM);
            return;
        }
    }
    if (n->n[idx]) {
        error(MDB_EINVARG);
        return;
    }
    n->n[idx] = dst;
    n->count++;
}

void MDB_stdcall MDB_DiscardSketchNode(MDB_NODE node) {
    // Don't abort on error in cleanup function
    void* slot = MDB_IdTableEntry(&_nodeTable, node);
    MDB_node* n = *(MDB_node**)slot;
    MDB_sketch* s = MDB_IdTableEntry(&_sketchTable, n->sketch);
    assert(s->nodes[n->index] == node);
    if (n->index == 0) s->nodes[0] = 0; // don't decrement count if n is the root node
    else {
        s->nodes[n->index] = s->nodes[--s->count];
        assert(s->nodes[n->index]);
        (*(MDB_node**)MDB_IdTableEntry(&_nodeTable, s->nodes[n->index]))->index = n->index;
        s->nodes[s->count] = 0;
    }
    MDB_FreeNode(node);
    *(UP*)slot = _nodeTable.freeList;
    _nodeTable.freeList = node;
}
void MDB_stdcall MDB_DiscardSketch(MDB_SKETCH sketch) {
    // Don't abort on error in cleanup function
    void* slot = MDB_IdTableEntry(&_sketchTable, sketch);
    MDB_sketch* s = (MDB_sketch*)slot;
    for (UP i = 0; i < s->cap; i++) {
        if (s->nodes[i])
            MDB_FreeNode(s->nodes[i]);
    }
    *(UP*)slot = _sketchTable.freeList;
    _sketchTable.freeList = sketch;
}

typedef struct MDB_scc_sketch_node_info {
    s32 onStack;
    UP sccIndex;
    UP lowLink;
    MDB_scc* g;
} MDB_scc_sketch_node_info;

s32 MDB_SCCStep(UP i, MDB_sketch* s, MDB_scc_sketch_node_info* a, UP* index, UP* stack, UP* sp) {
    MDB_node* v = *(MDB_node**)MDB_IdTableEntry(&_nodeTable, s->nodes[i]);
    a[i].sccIndex = a[i].lowLink = *index;
    a[i].onStack = 1; a[i].g = 0;
    *index += 1;
    stack[(*sp)++] = i;
    // For each child
    for (MDB_NODE* child = v->n; child < v->n + v->count; child++) {
        MDB_node* w = *(MDB_node**)MDB_IdTableEntry(&_nodeTable, *child);
        if (w->sketch && w->sketch != s->index) { // can't link to different uncommitted sketch
            error(MDB_EINVCALL);
            return 0;
        }
        if (a[w->index].sccIndex == 0) {
            if (!MDB_SCCStep(w->index, s, a, index, stack, sp)) return 0;
            a[i].lowLink = MDB_Min(a[i].lowLink, a[w->index].lowLink);
        } else if (a[w->index].onStack) {
            a[i].lowLink = MDB_Min(a[i].lowLink, a[w->index].sccIndex);
        }
    }
    // are we a root node?
    if (a[i].lowLink == a[i].sccIndex) {
        MDB_scc* g = malloc(sizeof(MDB_scc));
        if (!g) {
            error(MDB_EMEM);
            return 0;
        } else a[i].g = g;
        //TODO(cdo): can we pre-determine the scc size? Maybe by keeping track
        // of position in stack instead of the boolean 'onStack'.
        UP cap = 32;
        g->count = 0;
        g->n = malloc(cap*PS);
        if (!g->n) {
            error(MDB_EMEM);
            free(g);
            a[i].g = 0; //TODO(cdo): is this needed? Aren't we going to discard
                        // the array on failure regardless?
            return 0;
        }
        UP j;
        do {
            j = stack[--*sp];
            a[j].onStack = 0;
            if (g->count == cap) {
                UP newCap = cap*2;
                MDB_NODE* nodes = realloc(g->n, newCap*PS);
                if (!nodes) {
                    error(MDB_EMEM);
                    free(g->n); free(g);
                    return 0;
                }
                g->n = nodes;
                cap = newCap;
            }
            a[j].g = g;
            g->n[g->count++] = s->nodes[j];
        } while (j!=i);
        MDB_NODE* nodes = realloc(g->n, g->count*PS);
        if (nodes) g->n = nodes;
    }
    return 1;
}

// returns 0 iff failed to commit
s32 MDB_stdcall MDB_CommitSketch(MDB_SKETCH sketch) {
    if (_state) return 0;
    void* slot = MDB_IdTableEntry(&_sketchTable, sketch);
    MDB_sketch* s = (MDB_sketch*)slot;
    for (UP i = 0; i < s->count; i++) {
        MDB_NODE n = s->nodes[i];
        if (!n) {
            error(MDB_EINCOMPLETESKETCH);
            return 0;
        }
    }
    // Tarjan's SCC algorithm
    MDB_scc_sketch_node_info* a =
        calloc(s->count+1,sizeof(MDB_scc_sketch_node_info));
    UP* stack = malloc(s->count*PS);
    if (!a || !stack) {
        error(MDB_EMEM);
        free(a); free(stack);
        return 0;
    }
    UP index = 1;
    UP sp = 0;
    for (UP i = 0; i < s->count; i++) if (!a[i].sccIndex)
        if (!MDB_SCCStep(i, s, a, &index, stack, &sp)) {
            free(a); free(stack);
            return 0;
        }
    for (UP i = 0; i < s->count; i++) {
        assert(a[i].g);
        MDB_node* n = *(MDB_node**)MDB_IdTableEntry(&_nodeTable, s->nodes[i]);
        n->sketch = 0; n->index = 0;
        n->type &= ~MDB_SKETCHFLAG;
        if (n->type != MDB_WORLD) { // shrink wrap
            MDB_NODE* a = realloc(n->n, PS*n->count);
            if (a) { n->n = a; n->cap = n->count; }
        }
        n->g = a[i].g;
    }
    free(a); free(stack);
    *(UP*)slot = _sketchTable.freeList;
    _sketchTable.freeList = sketch;
    return 1;
}


void MDB_stdcall
MDB_GetNodeInfo(MDB_NODE node,
    MDB_NODETYPE* type, uintptr_t* childCount, char** str) {

    if (_state) {
        *type = MDB_INVALIDNODETYPE;
        *childCount = 0;
        *str = 0;
        return;
    }
    MDB_node* n = *(MDB_node**)MDB_IdTableEntry(&_nodeTable, node);
    *type = n->type;
    *childCount = n->count;
    *str = (char*)n->name;
}

// returns number of children written
uintptr_t MDB_stdcall
MDB_GetChildren(MDB_NODE node,
    uintptr_t startIndex, uintptr_t bufferCount, MDB_NODE* buffer) {

    if (_state) return 0;

    MDB_node* n = *(MDB_node**)MDB_IdTableEntry(&_nodeTable, node);
    if (n->count <= startIndex) {
        error(MDB_EINVARG);
        return 0;
    }
    UP count = MDB_Min(n->count - startIndex, bufferCount);
    memcpy(buffer, n->n+startIndex, count*PS);
    return count;
}
