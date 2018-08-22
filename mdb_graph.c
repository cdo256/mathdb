#include "mdb_graph.h"
#include "mdb_edit_graph.h" //IMPLEMENTS
#include <stdlib.h>
#include <assert.h>
#define fail() assert(0)

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

typedef uintptr_t UP;
#define PS (sizeof(void*))
static_assert(PS == sizeof(UP));
static_assert(PS >= 4);

static int failFlag = 0;

typedef struct MDB_sketch {
    UP count, cap, index;
    MDB_NODE* nodes;
} MDB_sketch;

typedef struct MDB_scc {
    UP count;
    MDB_NODE n[];
} MDB_scc;

typedef struct MDB_node {
    MDB_NODETYPE type;
    u32 count;
    MDB_NODE n[];
} MDB_node;

static UP _sketchCap = 0;
static UP _sketchCount = 0;
static MDB_sketch* _sketches = 0;

#ifdef NDEBUG
#define CheckSketch(s)
#define CheckSketchNode(s,n)
#define CheckReadWrite(x)
#define CheckReadWriteArr(x,c)
#else
#define CheckSketch(s) MDB_CheckSketch(s)
#define CheckSketchNode(s,n) MDB_CheckSketchNode(s,n)
#define CheckReadWrite(x) MDB_CheckReadWriteMem(&(x),sizeof(x))
#define CheckReadWriteArr(x,c) MDB_CheckReadWriteMem(x,sizeof(*(x))*(c))
#endif

void MDB_CheckReadWriteMem(void* start, UP size) {
    u8* volatile tmp = start;
    u8** volatile startAddr = &tmp;
    memmove(*startAddr, start, size);
}

void MDB_CheckReadMem(void* start, UP size) {
    u8* volatile tmp = start;
    u8* volatile* startAddr = &tmp;
    int c;
    int* volatile d = &c;
    *d = memcmp(*startAddr, start, size);
}

void MDB_CheckSketch(MDB_SKETCH sketch) {
    assert(sketch && sketch <= _sketchCount);
    MDB_sketch* s = &_sketches[sketch];
    assert(s->index == sketch) return 0;
    assert(s->nodes);
    CheckReadWriteArr(s->nodes, s->cap);
}

void MDB_CheckNode(MDB_NODE node) {
    assert(node);
    MDB_NODETYPE type = node->type & ~MDB_STETCHFLAG;
    assert(type == MDB_POCKET || type == MDB_WORLD ||
        type == MDB_CONST || type == MDB_FORMATION);
    CheckReadWriteArr(node->n, node->count);
}

void MDB_CheckSketchNode(MDB_SKETCH sketch, MDB_NODE node) {
    MDB_CheckNode(node);
    assert(node->type & MDB_SKETCHFLAG);
}

MDB_SKETCH __stdcall MDB_StartSketch() {
    if (failFlag) return 0;
    if (_sketchCount == _sketchCap) {
        if (_sketches == 0) {
            _sketchCap = 256;
        } else _sketchCap *= 2;
        MDB_sketch* newSketches;
        newSketches = realloc(_sketches, _sketchCap*PS);
        if (newSketches) {
            _sketches = newSketches;
        } else {
            failFlag = 1;
            fail();
            return 0;
        }
    }
    MDB_sketch* sketch = &_sketches[++_sketchCount];
    sketch->index = _sketchCount;
    sketch->count = 1; // start at one because the first slot is for the root
    sketch->cap = 256;
    sketch->nodes = calloc(sketch->cap, PS);
    if (!sketch->nodes) {
        failFlag = 1;
        sketchCount--;
        fail();
        return 0;
    } else return sketch->index;
}
void __stdcall MDB_SetSketchRoot(MDB_SKETCH sketch, MDB_NODE node) {
    if (failFlag) return 0;
    CheckSketch(sketch); CheckSketchNode(sketch, node);
    MDB_sketch* s = &_sketches[sketch];
    assert(s->index == sketch);
    assert(s->nodes[0] == 0);
    s->nodes[0] = node;
    assert(_nodeCount > 1);
    _nodes[node] = _nodes[_nodeCount--];
}
MDB_NODE __stdcall MDB_SketchNode(MDB_SKETCH sketch, MDB_NODETYPE type) {

}
void __stdcall MDB_AddLink(MDB_NODE src, MDB_LINKDESC link, MDB_NODE dst);
void __stdcall MDB_DiscardSketchNode(MDB_NODE node);
void __stdcall MDB_FinaliseSketch(MDB_SKETCH sketch);
MDB_NODE __stdcall MDB_FinishSketch(MDB_SKETCH sketch);
void __stdcall MDB_DeleteNode(MDB_NODE node);
