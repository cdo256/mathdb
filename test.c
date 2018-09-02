#include "mdb_global.h"
#include "mdb_manage.h"
#include "mdb_edit_graph.h"
#include "mdb_get_error.h"
#include "mdb_read_graph.h"
#include "mdb_get_mem_info.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#define TEST_COUNT 8
#define PASS 1
#define FAIL 0

#define NAME 0
#define RUN 1

#define check assert
//#define check(x) do if (!(x)) { assert(0); return FAIL; } while(0)

int Test(int i, int fn, char** const name) {
    switch (i) {
        case 0: { *name = "create free";
            if (fn == NAME) return -1;
            MDB_CreateGraph();
            MDB_error e;
            int ret = !MDB_GetError(&e);
            MDB_FreeGraph();
            return ret;
        } return FAIL;
        case 1: { *name = "basic formation construction";
            if (fn == NAME) return -1;
            MDB_CreateGraph();
            MDB_SKETCH sketch = MDB_StartSketch();
            MDB_NODE plus = MDB_CreateConst("+");
            MDB_NODE eq = MDB_CreateConst("=");
            MDB_NODE one = MDB_CreateConst("1");
            MDB_NODE two = MDB_CreateConst("2");
            MDB_NODE lhs = MDB_SketchNode(sketch, MDB_FORM);
            MDB_NODE eqn = MDB_SketchNode(sketch, MDB_FORM);
            MDB_SetSketchRoot(sketch, eqn);
            MDB_AddLink(lhs, MDB_APPLY, plus);
            MDB_AddLink(lhs, MDB_ARG0, one);
            MDB_AddLink(lhs, MDB_ARG1, one);
            MDB_AddLink(eqn, MDB_APPLY, eq);
            MDB_AddLink(eqn, MDB_ARG0, lhs);
            MDB_AddLink(eqn, MDB_ARG1, two);
            MDB_CommitSketch(sketch);
            MDB_error e;
            check(!MDB_GetError(&e));
            MDB_NODETYPE t;
            uintptr_t childCount;
            char* str;
            MDB_NODE c[3];
            MDB_GetNodeInfo(eq, &t, &childCount, &str);
            check(!strcmp("=", str) && t == MDB_CONST && childCount == 0);
            MDB_GetNodeInfo(plus, &t, &childCount, &str);
            check(!strcmp("+", str) && t == MDB_CONST && childCount == 0);
            MDB_GetNodeInfo(one, &t, &childCount, &str);
            check(!strcmp("1", str) && t == MDB_CONST && childCount == 0);
            MDB_GetNodeInfo(two, &t, &childCount, &str);
            check(!strcmp("2", str) && t == MDB_CONST && childCount == 0);
            MDB_GetNodeInfo(lhs, &t, &childCount, &str);
            check(MDB_GetChildren(lhs, 0, 3, c) == 3);
            check(!str && t == MDB_FORM && childCount == 3 &&
                c[0] == plus && c[1] == one && c[2] == one);
            MDB_GetNodeInfo(eqn, &t, &childCount, &str);
            check(MDB_GetChildren(eqn, 0, 3, c) == 3);
            check(!str && t == MDB_FORM && childCount == 3 &&
                c[0] == eq && c[1] == lhs && c[2] == two);

            check(!MDB_GetError(&e));
            MDB_FreeGraph();
            return PASS;
        } return FAIL;
        case 2: { *name = "simple cycle";
            if (fn == NAME) return -1;
            MDB_error e;
            MDB_CreateGraph();
            MDB_SKETCH sketch = MDB_StartSketch();
            MDB_NODE x = MDB_CreateConst("x");
            MDB_NODE y = MDB_SketchNode(sketch, MDB_FORM);
            MDB_NODE z = MDB_SketchNode(sketch, MDB_POCKET);
            MDB_SetSketchRoot(sketch, y);
            MDB_AddLink(y, MDB_APPLY, x);
            MDB_AddLink(y, MDB_ARG0, z);
            MDB_AddLink(z, MDB_ELEM, y);
            MDB_CommitSketch(sketch);
            check(!MDB_GetError(&e));
            MDB_NODETYPE t;
            uintptr_t childCount;
            char* str;
            MDB_NODE c[2];
            MDB_GetNodeInfo(x, &t, &childCount, &str);
            check(!strcmp("x", str) && t == MDB_CONST && childCount == 0);
            MDB_GetNodeInfo(y, &t, &childCount, &str);
            check(MDB_GetChildren(y, 0, 2, c) == childCount);
            check(!str && t == MDB_FORM && childCount == 2 &&
                c[0] == x && c[1] == z);
            MDB_GetNodeInfo(z, &t, &childCount, &str);
            check(MDB_GetChildren(z, 0, 1, c) == childCount);
            check(!str && t == MDB_POCKET && childCount == 1 &&
                c[0] == y);

            check(!MDB_GetError(&e));
            MDB_FreeGraph();
            return PASS;
        } return FAIL;
        case 3: { *name = "simple world";
            if (fn == NAME) return -1;
            MDB_error e;
            MDB_CreateGraph();
            MDB_SKETCH sketch = MDB_StartSketch();
            MDB_NODE w = MDB_SketchNode(sketch, MDB_WORLD);
            MDB_SetSketchRoot(sketch, w);
            MDB_CommitSketch(sketch);
            sketch = MDB_StartSketch();
            MDB_NODE k = MDB_CreateConst("k");
            MDB_NODE v0 = MDB_CreateConst("v0");
            MDB_NODE v1 = MDB_SketchNode(sketch, MDB_FORM);
            MDB_NODE v2 = MDB_SketchNode(sketch, MDB_FORM);
            MDB_AddLink(v1, MDB_APPLY, k);
            MDB_AddLink(v1, MDB_ARG0, v0);
            MDB_AddLink(v2, MDB_APPLY, k);
            MDB_AddLink(v2, MDB_ARG0, v1);
            MDB_SetSketchRoot(sketch, v2);
            MDB_CommitSketch(sketch);
            MDB_AddLink(w, MDB_ELEM, v0);
            MDB_AddLink(w, MDB_ELEM, v1);
            MDB_AddLink(w, MDB_ELEM, v2);
            check(!MDB_GetError(&e));
            MDB_NODETYPE t;
            uintptr_t childCount;
            char* str;
            MDB_NODE c[3];
            MDB_GetNodeInfo(k, &t, &childCount, &str);
            check(!strcmp("k", str));
            MDB_GetNodeInfo(v0, &t, &childCount, &str);
            check(!strcmp("v0", str));
            MDB_GetNodeInfo(v1, &t, &childCount, &str);
            check(MDB_GetChildren(v1, 0, childCount, c) == childCount);
            check(!str && t == MDB_FORM && childCount == 2 &&
                c[0] == k && c[1] == v0);
            MDB_GetNodeInfo(v2, &t, &childCount, &str);
            check(MDB_GetChildren(v2, 0, childCount, c) == childCount);
            check(!str && t == MDB_FORM && childCount == 2 &&
                c[0] == k && c[1] == v1);
            MDB_GetNodeInfo(w, &t, &childCount, &str);
            check(MDB_GetChildren(w, 0, childCount, c) == childCount);
            check(!str && t == MDB_WORLD && childCount == 3 &&
                c[0] == v0 && c[1] == v1 && c[2] == v2);
            check(!MDB_GetError(&e));
            MDB_FreeGraph();
            return PASS;
        } return FAIL;
        case 4: { *name = "discard sketch node";
            if (fn == NAME) return -1;
            MDB_error e;
            MDB_CreateGraph();
            MDB_SKETCH sketch = MDB_StartSketch();
            MDB_NODE n1 = MDB_SketchNode(sketch, MDB_WORLD);
            MDB_NODE n2 = MDB_SketchNode(sketch, MDB_FORM);
            MDB_SetSketchRoot(sketch, n1);
            MDB_DiscardSketchNode(sketch);
            MDB_NODE n3 = MDB_SketchNode(sketch, MDB_POCKET);
            MDB_SetSketchRoot(sketch, n3);
            MDB_CommitSketch(sketch);
            check(!MDB_GetError(&e));
            MDB_NODETYPE t;
            uintptr_t childCount;
            char* str;
            MDB_GetNodeInfo(n2, &t, &childCount, &str);
            check(t == MDB_FORM);
            MDB_GetNodeInfo(n3, &t, &childCount, &str);
            check(t == MDB_POCKET);
            MDB_FreeGraph();
            return PASS;
        } return FAIL;
        case 5: { *name = "discard sketch";
            if (fn == NAME) return -1;
            MDB_error e;
            MDB_CreateGraph();
            MDB_SKETCH sketch = MDB_StartSketch();
            MDB_DiscardSketch(sketch);
            check(!MDB_GetError(&e));
            MDB_FreeGraph();
            return PASS;
        } return FAIL;
        case 6: { *name = "memory leak 1";
            if (fn == NAME) return -1;
            MDB_CreateGraph();
            MDB_CreateConst("+");
            MDB_FreeGraph();
            return PASS;
        } return FAIL;
        case 7: { *name = "memory leak 2";
            if (fn == NAME) return -1;
            MDB_CreateGraph();
            //MDB_SKETCH sketch = MDB_StartSketch();
            //MDB_NODE w = MDB_SketchNode(sketch, MDB_WORLD);
            //MDB_SetSketchRoot(sketch, w);
            //MDB_CommitSketch(sketch);
            MDB_StartSketch();
            MDB_FreeGraph();
            return PASS;
        } return FAIL;
        case 8: { *name = "segfault";
            if (fn == NAME) return -1;
            MDB_CreateGraph();
            MDB_FreeGraph();
            MDB_CreateGraph();
            MDB_SKETCH sketch = MDB_StartSketch();
            MDB_NODE n1 = MDB_SketchNode(sketch, MDB_WORLD);
            MDB_SketchNode(sketch, MDB_FORM);
            MDB_SetSketchRoot(sketch, n1);
            MDB_DiscardSketchNode(sketch);
            MDB_NODE n3 = MDB_SketchNode(sketch, MDB_POCKET);
            MDB_SetSketchRoot(sketch, n3);
            MDB_CommitSketch(sketch);
            MDB_FreeGraph();
            return PASS;
        } return FAIL;
        case 9: { *name = "segfault 2";
            if (fn == NAME) return -1;
            MDB_CreateGraph();
            MDB_SKETCH sketch = MDB_StartSketch();
            MDB_NODE n1 = MDB_SketchNode(sketch, MDB_WORLD);
            MDB_SketchNode(sketch, MDB_FORM);
            MDB_SetSketchRoot(sketch, n1);
            MDB_DiscardSketchNode(sketch);
            MDB_NODE n3 = MDB_SketchNode(sketch, MDB_POCKET);
            MDB_SetSketchRoot(sketch, n3);
            MDB_CommitSketch(sketch);
            MDB_FreeGraph();
            return PASS;
        } return FAIL;
        default: printf("\nInvalid test id.\n"); return FAIL;
    }
}

// returns 1 iff the test succeeded
int RunTest(int id) {
    char* s;
    Test(id,NAME,&s);
    printf("Test %d (%s): ", id, s);
    UP startAllocCount = MDB_GetAllocatedBlockCount();
    int result = Test(id,RUN,&s);
    if (startAllocCount != MDB_GetAllocatedBlockCount()) result = FAIL;
    printf("allocated: %"PRIu64" blocks\n", MDB_GetAllocatedBlockCount());
    MDB_error e;
    if (MDB_GetError(&e)) result = FAIL;
    while (MDB_GetError(&e)); // Clear errors
    printf("%s\n", result ? "passed" : "FAILED");
    return result;
}

int main(int argc, char const* const* argv) {
    int failCount = 0;
    int testCount = TEST_COUNT;
    if (argc > 1) { // args are a list of test numbers
        testCount = argc-1;
        for (int i = 1; i < argc; i++) {
            int id;
            sscanf(argv[i], "%d", &id);
            failCount += !RunTest(id);
        }
    } else { // if no args supplied run all tests
        for (int i = 0; i < TEST_COUNT; i++) {
            failCount += !RunTest(i);
        }
    }
    if (testCount > 1) { // only print a summary if we ran multiple tests
        if (failCount == 0)
            printf("All tests passed");
        else printf("*** %d/%d TESTS FAILED ***",failCount,testCount);
        printf("\n");
    }
}
