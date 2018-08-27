#include "mdb_global.h"
#include "mdb_graph.h"
#include "mdb_edit_graph.h" //IMPLEMENTS
#include "mdb_error.h"
#include <stdlib.h>
#include <string.h>

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

inline void* MDB_IdTableEntry(MDB_id_table* const t, UP idx) {
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
#define REP32(x) ((UP)(x) << 32) | (UP)(x))
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
    _nodeTable = MDB_CreateIdTable(UP,65536);
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
    bmp[idx>>3] = bmp[idx>>3] & ~(1<<(idx&7)) | val<<(idx&7);
}
void MDB_stdcall MDB_FreeGraph() {
    // Don't abort on error in cleanup function
    MDB_id_table* t = &_nodeTable;
    for (UP i = 0; i < t->end; i++) {
        if (MDB_ReadBit(t, i)) {
            MDB_FreeNode(i);
        }
    }
    MDB_FreeIdTable(t);
    t = &_sketchTable;
    for (UP i = 0; i < _sketchCount; i++)
        free(MDB_IdTableEntry(t, i)->nodes);
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
            UP* a = realloc(t->a, cap*t->sz);
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
        MDB_FreeTableSlot(&_sketchTable, sketch);
        return 0;
    } else return sketch;
}
void MDB_stdcall MDB_SetSketchRoot(MDB_SKETCH sketch, MDB_NODE node) {
    if (_state) return;
    MDB_sketch* s = MDB_IdTableEntry(&_sketchTable, sketch);
    assert(s->index == sketch);
    if (s->nodes[0] != 0) {
        error(MDB_EINVCALL);
        return;
    }
    s->nodes[0] = node;
    if (s->count <= 1) {
        error(MDB_EINVCALL);
        return;
    }
    s->nodes[node] = s->nodes[s->count--];
}

MDB_NODE MDB_stdcall MDB_SketchNode(MDB_SKETCH sketch, MDB_NODETYPE type) {
    if (_state) return 0;
    if (type & MDB_SKETCHFLAG || type & MDB_NODETYPEMASK == MDB_CONST) {
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
    MDB_sketch* s = MDB_IdTaleEntry(_sketchTable, sketch);
    if (s->count == s->cap) {
        UP cap = cap * 2;
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
    MDB_NODE node = MDB_AllocTableSlot(_nodeTable);
    if (!node) {
        free(n->n); free(n);
        return 0;
    }
    _nodeTable.a[node] = (UP)n;
    s->nodes[s->count++] = node;
    return node;
}
MDB_NODE MDB_stdcall MDB_CreateConst(char* const name) {
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
    n->name = strdup(name);
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
    MDB_node* n = *(node**)slot;
    MDB_sketch* s = MDB_IdTableEntry(&_sketchTable, n->sketch);
    assert(s->nodes[n->index] == node);
    s->nodes[n->index] = s->nodes[--s->count];
    assert(s->nodes[n->index])
    ((node*)MDN_IdTableEntry(&_nodeTable, s->nodes[n->index]))->index = n->index;
    s->nodes[s->count] = 0;
    MDB_FreeNode(node);
    *(UP*)slot = _nodeTable.freeList;
    _nodeTable.freeList = node;
}
void MDB_stdcall MDB_DiscardSketch(MDB_SKETCH sketch) {
    // Don't abort on error in cleanup function
    void* slot = MDB_IdTableEntry(&_sketchTable, sketch);
    MDB_sketch* s = (MDN_sketch*)slot;
    for (UP i = 0; i < s->cap; i++) {
        MDB_FreeNode(s->nodes[i]);
    }
    *(UP*)slot = _sketchTable.freeList;
    _sketchTable.freeList = sketch;
}

typedef struct MDB_scc_sketch_node_info {
    s32 onStack;
    UP sccIndex;
    UP lowLink;
} MDB_scc_sketch_node_info;

void MDB_SCCStep(UP i, MDB_sketch* s, MDB_scc_sketch_node_info* a, UP* index, UP* stack, UP* sp) {
    MDB_node* n = MDB_IdTableEntry(&_nodeTable, s->nodes[i]);
    a[i].sccIndex = a[i].lowLink = *index;
    a[i].onStack = 1;
    *index += 1;
    stack[(*sp)++] = i;
    for (UP j = 0; j < n->count; j++) {
        
    }
}

// returns 0 iff failed to commit
s32 MDB_stdcall MDB_CommitSketch(MDB_SKETCH sketch) {
    if (_state) return 0;
    MDB_sketch* s = (MDB_sketch*)MDB_IdTableEntry(&_sketchTable, sketch);
    for (UP i = 0; i < s->count; i++) {
        MDB_NODE n = s->nodes[i];
        if (!n) {
            error(MDB_EINCOMPLETESKETCH);
            return 0;
        }
    }
    // Tarjan's SCC algorithm
    MDB_scc_sketch_node_info* a = calloc(s->count+1,sizeof(MDB_scc_sketch_node_info));
    MDB_scc_sketch_node_info* stack = calloc(s->count+1,sizeof(MDB_scc_sketch_node_info));
    if (!a || !stack) {
        error(MDB_EMEM);
        free(a); free(stack);
        return 0;
    }
    UP index = 1;
    UP sp = 0;
    for (UP i = 0; i < s->count; i++)
        if (!indices[i]) MDB_SCCStep(i, s, a, &index, stack, sp);
    // Compute SCCs
    // Shrink allocated node lists for variable nodes and remove sketch only junk
}
