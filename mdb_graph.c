#include "mdb_global.h"
#include "mdb_graph.h"
#include "mdb_edit_graph.h" //IMPLEMENTS
#include "mdb_read_graph.h" //IMPLEMENTS
#include "mdb_error.h"
#include "mdb_alloc_mem.h"
#include <stdlib.h>
#include <string.h>
#ifndef _NDEBUG
#include <stdio.h>
#endif

#define calloc MDB_CAlloc
#define malloc MDB_Alloc
#define free MDB_Free
#define realloc MDB_Realloc

static void mdb_DebugBreak(void) {}
#define debug mdb_DebugBreak

#define MDB_SKETCHFLAG 0x0100U
#define MDB_NODETYPEMASK 0x00FFU

// _errorFlags can be cleared by the user calling MDB_GetError() but mdb_state
// must be cleared by recreating the graph.
static u32 _errorFlags = 0;
static u32 mdb_state = MDB_EUNINIT;

typedef struct mdb_sketch {
    UP count, cap, index;
    MDB_NODE* nodes;
} mdb_sketch;

// strongly-connected component
typedef struct mdb_scc {
    UP count;       // count of n
    MDB_NODE* n;    // array of nodes in the scc
} mdb_scc;

typedef struct mdb_node {
    MDB_NODETYPE type;
    mdb_scc* g;         // a pointer to the scc when we're no longer a sketch
    MDB_SKETCH sketch;  // which sketch are we
    UP index;           // what index are we in this sketch?
    UP count, cap;      // number of children and size of n respectively
    MDB_NODE* n;        // array of children
    c8* name;           // our name if we're a const
} mdb_node;

typedef struct mdb_node_map {
    UP size, count;
    MDB_NODE* a;
} mdb_node_map;

typedef struct mdb_id_table {
    UP end, cap; // a[0..end) is the part we've touched, |a| = cap
    UP sz;       // size of each entry
    UP freeList; // index of next free slot; end: 0, adv: freeList = *(UP*)&a[freeList*sz]
    u8* a;       // array of size cap; each entry either freeList index, mdb_node* or mdb_sketch*
    u8* freeBmp; // reserve but don't use until deinit time
} mdb_id_table;

static void* MDB_IdTableEntry(mdb_id_table const* t, UP idx) {
    return (void*)&t->a[idx*t->sz];
}

static mdb_id_table mdb_nodeTable;
static mdb_id_table mdb_sketchTable;

static c8 const* const mdb_errorStr[] = {
    [0]="no error",
    [MDB_EMEMBIT]="out of memory",
    [MDB_EUNINITBIT]="the graph has not been created",
    [MDB_EINVCALLBIT]="this function call was unexpected",
    [MDB_EINVARGBIT]="a function was called with an invalid argument",
    [MDB_EINCOMPLETESKETCHBIT]="the sketch is not complete",
    [MDB_EINTOVERFLOWBIT]="an integer overflow has occurred. Consider running as 64bit",
};

s32 MDB_stdcall MDB_GetError(MDB_error* e, s32 clear) {
    if (!_errorFlags) {
        e->str = mdb_errorStr[0];
        e->id = MDB_ENONE;
        mdb_state &= MDB_EUNINIT; // clear all but the uninit bit.
        return 0;
    }
    for (int i = 0; i < 32; i++) {
        if ((1U<<i)&_errorFlags) {
            e->str = mdb_errorStr[i];
            e->id = (1U<<i);
            if (clear) _errorFlags &= ~e->id;
            return 1;
        }
    }
    return 0;
}

void mdb_SignalError(UP type) {
    _errorFlags |= type;
    mdb_state |= _errorFlags;
#ifndef _NDEBUG
    MDB_error e;
    MDB_GetError(&e, 0);
    fprintf(stderr, "fail\n%s\n", e.str);
    fail();
#endif
}
#define error(t) mdb_SignalError(t)
#define log(...) fprintf(stderr, __VA_ARGS__)
//#define log(...)

static inline UP mdb_Min(UP x, UP y) {
    return x > y ? y : x;
}
static inline UP mdb_Max(UP x, UP y) {
    return x < y ? y : x;
}

// Frees scc but doesn't update mdb_nodeTable or
void MDB_FreeNode(MDB_NODE node) {
    mdb_node* n = *(mdb_node**)MDB_IdTableEntry(&mdb_nodeTable, node);
    if (n->g) {
        n->g->count--;
        if (n->g->count == 0) {
            free(n->g->n); free(n->g);
        }
    }
    free(n->n);
    free(n->name);
    n->n = 0;
    free(n);
}

static UP mdb_RoundUp(UP x) {
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
static s32 mdb_IntLogPo2(UP x) {
    assert(x != 0 && x == mdb_RoundUp(x));

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
static mdb_id_table mdb_CreateIdTable(UP slotSize, UP cap) {
    assert(cap >= 1);
    mdb_id_table t;
    t.sz = mdb_RoundUp(slotSize);
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
static void mdb_FreeIdTable(mdb_id_table* table) {
    free(table->a); free(table->freeBmp);
}

void MDB_stdcall MDB_CreateGraph(void) {
	log("MDB_CreateGraph(): ");
    if (!(mdb_state & MDB_EUNINIT)) {
        error(MDB_EINVCALL);
        return;
    }
    mdb_nodeTable = mdb_CreateIdTable(PS,65536);
    mdb_sketchTable = mdb_CreateIdTable(sizeof(mdb_sketch),256);
    if (!mdb_nodeTable.a || !mdb_sketchTable.a) {
        error(MDB_EUNINIT);
        mdb_FreeIdTable(&mdb_nodeTable);
        mdb_FreeIdTable(&mdb_sketchTable);
        return;
    }
    mdb_state &= ~MDB_EUNINIT;
	log("done\n");
}
static void mdb_PopulateFreeBmp(mdb_id_table* t) {
    memset(t->freeBmp, 0, (t->end+7)>>3);
    for (UP p = t->freeList; p; p = (UP)t->a[p*t->sz]) {
        t->freeBmp[p>>3] |= 1 << (p & 7);
    }
}
static s32 mdb_ReadBit(u8* bmp, UP idx) {
    return (bmp[idx>>3] >> (idx&7)) & 1;
}
static void mdb_WriteBit(u8* bmp, UP idx, s32 val) {
    bmp[idx>>3] = (u8)((bmp[idx>>3] & ~(1<<(idx&7))) | (val<<(idx&7)));
}
// Node discarded when either of DiscardSketch, DiscardSketchNode, FreeGraph are caled
void MDB_stdcall MDB_FreeGraph(void) {
	log("MDB_FreeGraph(): ");
    // Don't abort on error in cleanup function
    mdb_id_table* t = &mdb_nodeTable;
    mdb_PopulateFreeBmp(t);
    for (UP i = 1; i < t->end; i++) {
        if (!mdb_ReadBit(t->freeBmp, i)) {
            MDB_FreeNode(i);
        }
    }
    mdb_FreeIdTable(t);
    t = &mdb_sketchTable;
    mdb_PopulateFreeBmp(t);
    for (UP i = 1; i < t->end; i++)
        if (!mdb_ReadBit(t->freeBmp, i))
            free(((mdb_sketch*)MDB_IdTableEntry(t, i))->nodes);
    mdb_FreeIdTable(t);
    // We will not forget errors that may have occrred during this graph's lifetime.
    // They must be cleared explicitly with repeated MDB_GetError()'s.
    mdb_state |= MDB_EUNINIT;
	log("done\n");
}

static UP mdb_AllocTableSlot(mdb_id_table* t) {
    assert(t->cap > 0);
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

MDB_SKETCH MDB_stdcall MDB_StartSketch(void) {
	log("MDB_StartSketch(): ");
    if (mdb_state) return 0;
    MDB_SKETCH sketch = mdb_AllocTableSlot(&mdb_sketchTable);
    if (!sketch) return 0; // error already signal, we have nothing to add
    mdb_sketch* s = MDB_IdTableEntry(&mdb_sketchTable, sketch);
    s->index = sketch;
    s->count = 1; // start at one because the first slot is for the root
    s->cap = 256;
    s->nodes = calloc(s->cap, PS); // need zero-ing
    if (!s->nodes) {
        error(MDB_EMEM);
        *(UP*)s = mdb_sketchTable.freeList;
        mdb_sketchTable.freeList = sketch;
        return 0;
    } else {
		log("%x\n",sketch);
		return sketch;
	}
}
static void mdb_MoveSketchNode(mdb_sketch* s, MDB_NODE node, UP idx) {
    s->nodes[idx] = node;
    mdb_node* n = *(mdb_node**)MDB_IdTableEntry(&mdb_nodeTable, node);
    n->index = idx;
}
void MDB_stdcall MDB_SetSketchRoot(MDB_SKETCH sketch, MDB_NODE node) {
	log("MDB_SetSketchRoot(sketch: %x, node: %x): ", sketch, node);
    if (mdb_state) return;
    mdb_sketch* s = MDB_IdTableEntry(&mdb_sketchTable, sketch);
    assert(s->index == sketch);
    if (s->nodes[0] != 0) {
        error(MDB_EINVCALL);
        return;
    }
    UP oldIndex = (*(mdb_node**)MDB_IdTableEntry(&mdb_nodeTable, node))->index;
    mdb_MoveSketchNode(s, node, 0);
    if (s->count <= 1) {
        error(MDB_EINVCALL);
        return;
    }
    mdb_MoveSketchNode(s, s->nodes[--s->count], oldIndex);
	log("done\n");
}

MDB_NODE MDB_stdcall MDB_SketchNode(MDB_SKETCH sketch, MDB_NODETYPE type) {
	log("MDB_SketchNode(sketch: %x, type: %x): ", sketch, type);
    if (mdb_state) return 0;
    if (type & MDB_SKETCHFLAG || (type & MDB_NODETYPEMASK) == MDB_CONST) {
        error(MDB_EINVARG);
        return 0;
    }
    mdb_node* n = malloc(sizeof(mdb_node));
    if (!n) {
        error(MDB_EMEM);
        return 0;
    }
    n->type = type | MDB_SKETCHFLAG;
    n->g = 0;
    n->sketch = sketch;
    n->cap = 1; n->count = 0;
    n->n = calloc(n->cap,PS);
    n->name = 0;
    if (!n->n) {
        error(MDB_EMEM);
        free(n);
        return 0;
    }
    mdb_sketch* s = MDB_IdTableEntry(&mdb_sketchTable, sketch);
    assert(s->cap > 0);
    if (s->count == s->cap) {
        UP cap = s->cap * 2;
        if (s->cap > cap) {
            error(MDB_EINTOVERFLOW);
            return 0;
        }
        MDB_NODE* nodes = realloc(s->nodes, cap*PS);
        if (!nodes) {
            error(MDB_EMEM);
            free(n->n); free(n); n->n = 0;
            return 0;
        }
        s->cap = cap;
        s->nodes = nodes;
    }
    n->index = s->count;
    MDB_NODE node = mdb_AllocTableSlot(&mdb_nodeTable);
    if (!node) {
        free(n->n); free(n); n->n = 0;
        return 0;
    }
    *(mdb_node**)MDB_IdTableEntry(&mdb_nodeTable, node) = n;
    s->nodes[s->count++] = node;
	log("%x\n", node);
    return node;
}
MDB_NODE MDB_stdcall MDB_CreateConst(char const* name) {
	log("MDB_CreateConst(name: %s): ", name);
    if (mdb_state) return 0;
    mdb_node* n = malloc(sizeof(mdb_node));
    if (!n) {
        error(MDB_EMEM);
        return 0;
    }
    n->type = MDB_CONST;
    n->g = malloc(sizeof(mdb_scc));
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
    UP len = strlen(name)+1;
    n->name = malloc(len);
    memcpy(n->name, name, len);
    if (!n->g->n || !n->name) {
        error(MDB_EMEM);
        free(n->g->n); free(n->name);
        free(n->g); free(n);
        return 0;
    }
    MDB_NODE node = mdb_AllocTableSlot(&mdb_nodeTable);
    if (!node) {
        free(n->g->n); free(n->name);
        free(n->g); free(n);
        return 0;
    }
    n->g->n[0] = node;
    *(mdb_node**)MDB_IdTableEntry(&mdb_nodeTable, node) = n;
	log("%x\n", node);
	return node;
}
void MDB_stdcall MDB_AddLink(MDB_NODE src, MDB_LINKDESC link, MDB_NODE dst) {
	log("MDB_AddLink(src: %x, link: %x, dst: %x): ",src,link,dst);
    if (mdb_state) return;
    mdb_node* n = *(mdb_node**)MDB_IdTableEntry(&mdb_nodeTable, src);
    assert(n->cap > 0);
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
        UP cap = mdb_RoundUp(idx+1);
        MDB_NODE* a = realloc(n->n, cap*PS);
        if (a) {
            n->n = a;
			for (UP i = n->cap; i < cap; i++)
				n->n[i] = 0;
            n->cap = cap;
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
	log("done\n");
}

void MDB_stdcall MDB_DiscardSketchNode(MDB_NODE node) {
	log("MDB_DiscardSketchNode(node: %x): ", node);
    // Don't abort on error in cleanup function
    void* slot = MDB_IdTableEntry(&mdb_nodeTable, node);
    mdb_node* n = *(mdb_node**)slot;
    mdb_sketch* s = MDB_IdTableEntry(&mdb_sketchTable, n->sketch);
    assert(s->nodes[n->index] == node);
    if (n->index == 0) s->nodes[0] = 0; // don't decrement count if n is the root node
    else {
        s->nodes[n->index] = s->nodes[--s->count];
        assert(s->nodes[n->index]);
        (*(mdb_node**)MDB_IdTableEntry(&mdb_nodeTable, s->nodes[n->index]))->index = n->index;
        s->nodes[s->count] = 0;
    }
    MDB_FreeNode(node);
    *(UP*)slot = mdb_nodeTable.freeList;
    mdb_nodeTable.freeList = node;
	log("done\n");
}
void MDB_stdcall MDB_DiscardSketch(MDB_SKETCH sketch) {
	log("MDB_DiscardSketch(sketch: %x): ", sketch);
    // Don't abort on error in cleanup function
    void* slot = MDB_IdTableEntry(&mdb_sketchTable, sketch);
    mdb_sketch* s = (mdb_sketch*)slot;
    for (UP i = 0; i < s->cap; i++) {
        if (s->nodes[i]) {
            MDB_FreeNode(s->nodes[i]);
			*(UP*)slot = mdb_nodeTable.freeList;
			mdb_nodeTable.freeList = i;
		}
    }
    free(s->nodes);
    *(UP*)slot = mdb_sketchTable.freeList;
    mdb_sketchTable.freeList = sketch;
	log("done\n");
}

typedef struct mdb_scc_sketch_node_info {
    s32 onStack;
    UP sccIndex;
    UP lowLink;
    mdb_scc* g;
} mdb_scc_sketch_node_info;

static s32 mdb_SCCStep(UP i, mdb_sketch* s, mdb_scc_sketch_node_info* a, UP* index, UP* stack, UP* sp) {
    mdb_node* v = *(mdb_node**)MDB_IdTableEntry(&mdb_nodeTable, s->nodes[i]);
    a[i].sccIndex = a[i].lowLink = *index;
    a[i].onStack = 1; a[i].g = 0;
    *index += 1;
    stack[(*sp)++] = i;
    // For each child
    for (MDB_NODE* child = v->n; child < v->n + v->count; child++) {
        mdb_node* w = *(mdb_node**)MDB_IdTableEntry(&mdb_nodeTable, *child);
        if (w->sketch && w->sketch != s->index) { // can't link to different uncommitted sketch
            error(MDB_EINVCALL);
            return 0;
        }
        if (a[w->index].sccIndex == 0) {
            if (!mdb_SCCStep(w->index, s, a, index, stack, sp)) return 0;
            a[i].lowLink = mdb_Min(a[i].lowLink, a[w->index].lowLink);
        } else if (a[w->index].onStack) {
            a[i].lowLink = mdb_Min(a[i].lowLink, a[w->index].sccIndex);
        }
    }
    // are we a root node?
    if (a[i].lowLink == a[i].sccIndex) {
        mdb_scc* g = malloc(sizeof(mdb_scc));
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
        if (g->count == 0) g->n = 0;
        else {
            MDB_NODE* nodes = realloc(g->n, g->count*PS);
            if (nodes) g->n = nodes;
        }
    }
    return 1;
}

// returns 0 iff failed to commit
s32 MDB_stdcall MDB_CommitSketch(MDB_SKETCH sketch) {
	log("MDB_CommitSketch(sketch: %x): ", sketch);
    if (mdb_state) return 0;
    void* slot = MDB_IdTableEntry(&mdb_sketchTable, sketch);
    mdb_sketch* s = (mdb_sketch*)slot;
    for (UP i = 0; i < s->count; i++) {
        MDB_NODE n = s->nodes[i];
        if (!n) {
            error(MDB_EINCOMPLETESKETCH);
            return 0;
        }
    }
    // Tarjan's SCC algorithm
    mdb_scc_sketch_node_info* a =
        calloc(s->count+1,sizeof(mdb_scc_sketch_node_info));
    UP* stack = malloc(s->count*PS);
    if (!a || !stack) {
        error(MDB_EMEM);
        free(a); free(stack);
        return 0;
    }
    UP index = 1;
    UP sp = 0;
    for (UP i = 0; i < s->count; i++) if (!a[i].sccIndex)
        if (!mdb_SCCStep(i, s, a, &index, stack, &sp)) {
            free(a); free(stack);
            return 0;
        }
    for (UP i = 0; i < s->count; i++) {
        assert(a[i].g);
        mdb_node* n = *(mdb_node**)MDB_IdTableEntry(&mdb_nodeTable, s->nodes[i]);
        n->sketch = 0; n->index = 0;
        n->type &= ~MDB_SKETCHFLAG;
        if (n->type != MDB_WORLD) { // shrink wrap
            if (n->count == 0) {
                free(n->n); n->n = 0; n->cap = 0;
            } else {
                MDB_NODE* ar = realloc(n->n, PS*n->count);
                if (ar) { n->n = ar; n->cap = n->count; }
            }
        }
        n->g = a[i].g;
    }
    free(a); free(stack); free(s->nodes);
    *(UP*)slot = mdb_sketchTable.freeList;
    mdb_sketchTable.freeList = sketch;
	log("1\n");
    return 1;
}


void MDB_stdcall
MDB_GetNodeInfo(MDB_NODE node,
    MDB_NODETYPE* type, uintptr_t* childCount, char** str) {

    if (mdb_state) {
        *type = MDB_INVALIDNODETYPE;
        *childCount = 0;
        *str = 0;
        return;
    }
    mdb_node* n = *(mdb_node**)MDB_IdTableEntry(&mdb_nodeTable, node);
    *type = n->type;
    *childCount = n->count;
    *str = (char*)n->name;
}

// returns number of children written
uintptr_t MDB_stdcall
MDB_GetChildren(MDB_NODE node,
    uintptr_t startIndex, uintptr_t bufferCount, MDB_NODE* buffer) {

    if (mdb_state) return 0;

    mdb_node* n = *(mdb_node**)MDB_IdTableEntry(&mdb_nodeTable, node);
    if (n->count < startIndex) {
        error(MDB_EINVARG);
        return 0;
    }
    UP count = mdb_Min(n->count - startIndex, bufferCount);
    memcpy(buffer, n->n+startIndex, count*PS);
    return count;
}



static mdb_node_map* mdb_CreateNodeMap(UP size) {
	mdb_node_map* m = malloc(sizeof(mdb_node_map));
	if (!m) {error(MDB_EMEM);return 0;}
	m->count = 0; m->size = size;
	m->a = calloc(m->size*2,PS);
	if (!m->a) {error(MDB_EMEM);free(m);return 0;}
	return m;
}
void MDB_stdcall MDB_FreeNodeMap(MDB_NODEMAP map) {
	if (!map) return;
	mdb_node_map* m = (mdb_node_map*)map;
	free(m->a);
	free(m);
}

static MDB_NODE* mdb_LookupNode(MDB_NODEMAP map, MDB_NODE n) {
	UP h = 0;mdb_node_map* m = (mdb_node_map*)map;
	for (int i =0;i<PS;i++) h=h*65599+((n>>(i*8))&255);
	h%=m->size;
	while (m->a[h*2]&&m->a[h*2]!=n) h = (h+1)%m->size;
	return &m->a[h*2];
}

// returns 1 if the map grew successfully, 2 if no map grow was required and 0 on failure to grow
// map is left in-tact on failure.
static int mdb_GrowNodeMap(mdb_node_map* m, UP c) {
	m->count+=c;
	if (m->count*3 >= 2*m->size) {
		mdb_node_map nm = *m;
		nm.size*=2;
		nm.a = realloc(nm.a,nm.size*PS);
		if (!nm.a) {error(MDB_EMEM);return 0;}
		memset(nm.a,0,nm.size*PS);
		for (UP i = 0; i < m->size*2; i+=2) {
			if (m->a[i]) memcpy(mdb_LookupNode((MDB_NODEMAP)&nm, m->a[i]), &m->a[i], 2*PS);
		}
		return 1;
	}
	return 2;
}
uintptr_t MDB_stdcall MDB_MapElemCount(MDB_NODEMAP map) {
	if (mdb_state) return 0;
	return ((mdb_node_map*)map)->size;
}
MDB_NODE MDB_stdcall MDB_LookupNode(MDB_NODEMAP map, MDB_NODE src) {
	if (mdb_state) return 0;
	return mdb_LookupNode(map, src)[1];
}

static s32 mdb_MatchPatternR(MDB_NODE pattern, MDB_NODE target, MDB_NODE capture, MDB_NODEMAP m) {
	mdb_node* p = *(mdb_node**)MDB_IdTableEntry(&mdb_nodeTable, pattern);
	mdb_node* t = *(mdb_node**)MDB_IdTableEntry(&mdb_nodeTable, target);
	if (target != pattern) {
		if (p->type == MDB_FORM && p->count >= 2 && p->n[1] == capture) {
			memcpy_s(mdb_LookupNode(m, pattern), 2*PS, (MDB_NODE[]){pattern, target}, 2*PS);
			return 1;
		}
		if (p->count != t->count || p->type != t->type) return 0;
		if (p->type == MDB_CONST) return 0; // already know that they're not equal.
		for (UP i = 0; i < t->count; i++) {
			if (!mdb_MatchPatternR(p->n[i], t->n[i], capture, m)) return 0;
		}
		return 1;
	} else {
		memcpy_s(mdb_LookupNode(m, pattern), 2*PS, (MDB_NODE[]){pattern, target}, 2*PS);
		return 1;
	}
}

MDB_NODEMAP MDB_stdcall MDB_MatchPattern(
    MDB_NODE pattern, MDB_NODE target, MDB_NODE capture) {
	if (mdb_state) return 0;
	MDB_NODEMAP m = (MDB_NODEMAP)mdb_CreateNodeMap(20);
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
	
	MDB_SKETCH s = MDB_StartSketch();
	MDB_NODE output = MDB_SketchNode(s, MDB_WORLD);
	MDB_SetSketchRoot(s, output);
	mdb_node* c = *(mdb_node**)MDB_IdTableEntry(&mdb_nodeTable, collection);
	assert(c->type == MDB_WORLD || c->type == MDB_POCKET);
	for (UP i = 0; i < c->count; i++) {
		MDB_NODEMAP m = MDB_MatchPattern(pattern, c->n[i], capture);
		if (m) MDB_AddLink(output, MDB_ELEM, c->n[i]);
		MDB_FreeNodeMap(m);
	}
	MDB_CommitSketch(s);
	return output;
}
