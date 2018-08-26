#include "mdb_global.h"
#include "mdb_graph.h"
#include "mdb_edit_graph.h" //IMPLEMENTS
#include "mdb_error.h"
#include <stdlib.h>
#include <string.h>

#define MDB_SKETCHFLAG 0x0100U
#define MDB_NODETYPEMASK 0x00FFU

static int err = MDB_EUNINIT;

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

static UP _sketchCap;
static UP _sketchCount;
static MDB_sketch* _sketches;

typedef struct MDB_node_table {
    UP end, cap; // a[0..end) is the part we've touched, |a| = cap
    UP freeList; // index of next free slot; end: 0, adv: freeList = a[freeList]
    UP* a;       // array of size cap; each entry either freeList index or MDB_node*
    u8* freeBmp; // reserve but don't use until deinit time
} MDB_node_table;

MDB_node_table _nodeTable;

const c8 const* _errorStr[] = {
    [0]="no error",
    [MDB_EMEMBIT]="out of memory",
    [MDB_EUNINITBIT]="the graph has not been created",
    [MDB_EINVCALLBIT]="this function call was unexpected",
    [MDB_EINVARGBIT]="a function was called with an invalid argument",
    [MDB_EINCOMPLETESKETCHBIT]="the sketch is not complete",
};

s32 MDB_stdcall MDB_GetError(MDB_error* e) {
    if (!err) {
        e->str = _errorStr[0];
        e->id = MDB_ENONE;
        return 0;
    }
    for (int i = 0; i < 8*PS; i++) {
        if ((1U<<i)&err) {
            e->str = _errorStr[i];
            e->id = (1U<<i);
            return 1;
        }
    }
    return 0;
}

void MDB_SignalError(UP type) {
    err |= type;
    fail();
}
#define error(t) MDB_SignalError(t)

// Deletes scc but doesn't update _nodeTable
void MDB_FreeNode(MDB_NODE node) {
    MDB_node* n = (MDB_node*)_nodeTable.a[node];
    if (n->g) {
        n->g->count--;
        if (n->g->count == 0) free(n->g);
    }
    free(n->n);
    free(n->name);
    free(n);
}

void MDB_stdcall MDB_CreateGraph() {
    if (err != MDB_EUNINIT) {
        err |= MDB_EINVCALL;
        return;
    }
    _sketchCap = 256;
    _sketchCount = 0;
    _sketches = malloc(_sketchCap*PS);
    if (!_sketches) {
        err |= MDB_EMEM;
        return;
    }
    _nodeTable.cap = 65536;
    _nodeTable.end = 1;
    _nodeTable.freeList = 0;
    _nodeTable.freeBmp = malloc((_nodeTable.cap+7)>>3);
    _nodeTable.a = malloc(_nodeTable.cap*PS);
    if (!_nodeTable.a || !_nodeTable.freeBmp) {
        err |= MDB_EMEM;
        free(_sketches);
        free(_nodeTable.a);
        free(_nodeTable.freeBmp);
        return;
    }
    err = 0;
}
void MDB_stdcall MDB_FreeGraph() {
    // Don't abort on error in cleanup function
    MDB_node_table* t = &_nodeTable;
    memset(t->freeBmp, 0, (t->end+7)>>3);
    for (UP p = t->freeList; p; p = t->a[p]) {
        t->freeBmp[p>>3] |= 1 << (p & 7);
    }
s32 MDB_ReadBit(u8* bmp, UP idx) {
    return (bmp[idx>>3] >> (idx&7)) & 1;
}
void MDB_WriteBit(u8* bmp, UP idx, s32 val) {
    bmp[idx>>3] = bmp[idx>>3] & ~(1<<(idx&7)) | val<<(idx&7);
}
    for (UP i = 0; i < t->end; i++) {
        if (t->freeBmp[i>>3] & (1<<(i&7))) {
            MDB_FreeNode(i);
        }
    }
    free(t->freeBmp); free(t->a);
    for (UP i = 0; i < _sketchCount; i++)
        free(_sketches[i].nodes);
    free(_sketches);
    // Don't forget any errors that may have occrred.
    // They must be cleared explicitly with repeated MDB_GetError()'s.
    err |= MDB_EUNINIT;
}

MDB_SKETCH MDB_stdcall MDB_StartSketch() {
    if (err) return 0;
    if (_sketchCount == _sketchCap) {
        _sketchCap *= 2;
        MDB_sketch* newSketches;
        newSketches = realloc(_sketches, _sketchCap*PS);
        if (newSketches) {
            _sketches = newSketches;
        } else {
            error(MDB_EMEM);
            _sketchCap /= 2;
            return 0;
        }
    }
    MDB_sketch* sketch = &_sketches[++_sketchCount];
    sketch->index = _sketchCount;
    sketch->count = 1; // start at one because the first slot is for the root
    sketch->cap = 256;
    sketch->nodes = calloc(sketch->cap, PS);
    if (!sketch->nodes) {
        error(MDB_EMEM);
        _sketchCount--;
        return 0;
    } else return sketch->index;
}
void MDB_stdcall MDB_SetSketchRoot(MDB_SKETCH sketch, MDB_NODE node) {
    if (err) return;
    MDB_sketch* s = &_sketches[sketch];
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
MDB_NODE MDB_GetNextTableSlot() {
    MDB_node_table* t = &_nodeTable;
    UP node = 0;
    if (t->freeList) {
        node = t->freeList;
        t->freeList = t->a[t->freeList];
    } else {
        if (t->end == t->cap) {
            t->cap *= 2;
            UP* a = realloc(t->a, t->cap);
            u8* bmp = malloc((t->cap+7)>>3);
            if (!a || !bmp) {
                error(MDB_EMEM);
                t->cap /= 2;
                free(a); free(bmp);
                return 0;
            }
            free(t->freeBmp);
            t->freeBmp = bmp;
            t->a = a;
        }
        node = t->end++;
    }
    return node;
}

MDB_NODE MDB_stdcall MDB_SketchNode(MDB_SKETCH sketch, MDB_NODETYPE type) {
    if (err) return 0;
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
    MDB_sketch* s = &_sketches[sketch];
    if (s->count == s->cap) {
        s->cap *= 2;
        MDB_NODE* nodes = realloc(s->nodes, s->cap*PS);
        if (!nodes) {
            error(MDB_EMEM);
            s->cap /= 2;
            free(n->n); free(n);
            return 0;
        }
        s->nodes = nodes;
    }
    n->index = s->count;
    MDB_NODE node = MDB_GetNextTableSlot();
    if (!node) {
        free(n->n); free(n);
        return 0;
    }
    _nodeTable.a[node] = (UP)n;
    s->nodes[s->count++] = node;
    return node;
}
MDB_NODE MDB_stdcall MDB_CreateConst(const char* name) {
    if (err) return 0;
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
    if (!n->g->n) {
        error(MDB_EMEM);
        free(n); free(n->g);
        free(n->g->n); free(n->name);
        return 0;
    }
    MDB_NODE node = MDB_GetNextTableSlot();
    if (!node) {
        free(n); free(n->g);
        free(n->g->n); free(n->name);
        return 0;
    }
    n->g->n[0] = node;
    _nodeTable.a[node] = (UP)n;
    return node;
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
void MDB_stdcall MDB_AddLink(MDB_NODE src, MDB_LINKDESC link, MDB_NODE dst) {
    if (err) return;
    MDB_node* n = (MDB_node*)_nodeTable.a[src];

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
}
void MDB_stdcall MDB_DiscardSketch(MDB_SKETCH sketch) {
    // Don't abort on error in cleanup function
    MDB_sketch** slot = &_sketches[sketch];
    MDB_sketch* s = *slot;
    for (int i = s->count-1; i >= 0; i--) {
        MDB_FreeNode(s->nodes[i]);
    }
    //TODO(cdo): _sketches freelist
}
void MDB_stdcall MDB_CommitSketch(MDB_SKETCH sketch) {
    if (err) return;
    MDB_sketch* s = &_sketches[sketch];
    for (int i = 0; i < s->count; i++) {
        MDB_NODE n = s->nodes[i];
        if (!n) {
            error(MDB_EINCOMPLETESKETCH);
            return;
        }
    }
}
