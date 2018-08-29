#include "mdb_manage.h"
#include "mdb_edit_graph.h"
#include "mdb_get_error.h"
#include "mdb_read_graph.h"
#include <stdio.h>
#include <string.h>

#define TEST_COUNT 6
#define PASS 1
#define FAIL 0

#define NAME 0
#define RUN 1

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
            if (MDB_GetError(&e)) return FAIL;
            MDB_NODETYPE t;
            uintptr_t childCount;
            char* str;
            MDB_NODE c[3];
            MDB_GetNodeInfo(eq, &t, &childCount, &str);
            if (strcmp("=", str) || t != MDB_CONST || childCount != 0) return FAIL;
            MDB_GetNodeInfo(plus, &t, &childCount, &str);
            if (strcmp("+", str) || t != MDB_CONST || childCount != 0) return FAIL;
            MDB_GetNodeInfo(one, &t, &childCount, &str);
            if (strcmp("1", str) || t != MDB_CONST || childCount != 0) return FAIL;
            MDB_GetNodeInfo(two, &t, &childCount, &str);
            if (strcmp("2", str) || t != MDB_CONST || childCount != 0) return FAIL;
            MDB_GetNodeInfo(lhs, &t, &childCount, &str);
            if (MDB_GetChildren(lhs, 0, 3, c) != 3) return FAIL;
            if (str || t != MDB_FORM || childCount != 3 ||
                c[0] != plus || c[1] != one || c[2] != one) return FAIL;
            MDB_GetNodeInfo(eqn, &t, &childCount, &str);
            if (MDB_GetChildren(eqn, 0, 3, c) != 3) return FAIL;
            if (str || t != MDB_FORM || childCount != 3 ||
                c[0] != eq || c[1] != lhs || c[2] != two) return FAIL;

            if (MDB_GetError(&e)) return FAIL;
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
            if (MDB_GetError(&e)) return FAIL;
            MDB_NODETYPE t;
            uintptr_t childCount;
            char* str;
            MDB_NODE c[2];
            MDB_GetNodeInfo(x, &t, &childCount, &str);
            if (strcmp("x", str) || t != MDB_CONST || childCount != 0) return FAIL;
            MDB_GetNodeInfo(y, &t, &childCount, &str);
            if (MDB_GetChildren(y, 0, 2, c) != childCount) return FAIL;
            if (str || t != MDB_FORM || childCount != 2 ||
                c[0] != x || c[1] != z) return FAIL;
            MDB_GetNodeInfo(z, &t, &childCount, &str);
            if (MDB_GetChildren(z, 0, 1, c) != childCount) return FAIL;
            if (str || t != MDB_POCKET || childCount != 1 ||
                c[0] != y) return FAIL;

            if (MDB_GetError(&e)) return FAIL;
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
            if (MDB_GetError(&e)) return FAIL;
            MDB_NODETYPE t;
            uintptr_t childCount;
            char* str;
            MDB_NODE c[3];
            MDB_GetNodeInfo(k, &t, &childCount, &str);
            if (strcmp("k", str)) return FAIL;
            MDB_GetNodeInfo(v0, &t, &childCount, &str);
            if (strcmp("v0", str)) return FAIL;
            MDB_GetNodeInfo(v1, &t, &childCount, &str);
            if (MDB_GetChildren(v1, 0, childCount, c) != childCount) return FAIL;
            if (str || t != MDB_FORM || childCount != 2 ||
                c[0] != k || c[1] != v0) return FAIL;
            MDB_GetNodeInfo(v2, &t, &childCount, &str);
            if (MDB_GetChildren(v2, 0, childCount, c) != childCount) return FAIL;
            if (str || t != MDB_FORM || childCount != 2 ||
                c[0] != k || c[1] != v1) return FAIL;
            MDB_GetNodeInfo(w, &t, &childCount, &str);
            if (MDB_GetChildren(w, 0, childCount, c) != childCount) return FAIL;
            if (str || t != MDB_WORLD || childCount != 3 ||
                c[0] != v0 || c[1] != v1 || c[2] != v2) return FAIL;
            if (MDB_GetError(&e)) return FAIL;
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
            if (MDB_GetError(&e)) return FAIL;
            MDB_NODETYPE t;
            uintptr_t childCount;
            char* str;
            MDB_GetNodeInfo(n2, &t, &childCount, &str);
            if (t != MDB_FORM) return FAIL;
            MDB_GetNodeInfo(n3, &t, &childCount, &str);
            if (t != MDB_POCKET) return FAIL;
            MDB_FreeGraph();
            return PASS;
        } return FAIL;
        case 5: { *name = "discard sketch";
            if (fn == NAME) return -1;
            MDB_error e;
            MDB_CreateGraph();
            MDB_SKETCH sketch = MDB_StartSketch();
            MDB_DiscardSketch(sketch);
            if (MDB_GetError(&e)) return FAIL;
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
    int result = Test(id,RUN,&s);
    MDB_error e;
    while (MDB_GetError(&e)); // Clear errors
    printf("%s\n", result ? "passed" : "FAILED");
    return result;
}

int main(int argc, char const* const* argv) {
    int failCount = 0;
    if (argc > 1) { // args are a list of test numbers
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
    if (failCount == 0)
        printf("All tests passed");
    else printf("*** %d/%d TESTS FAILED ***",failCount,TEST_COUNT);
    printf("\n");
}
