#include "mdb_manage.h"
#include "mdb_edit_graph.h"
#include "mdb_get_error.h"
#include "mdb_read_graph.h"
#include <stdio.h>
#include <string.h>

#define TEST_COUNT 3
#define PASS 1
#define FAIL 0

#define NAME 0
#define RUN 1

int Test(int i, int fn, char** const name) {
    switch (i) {
        case 0: { *name = "create free";
            if (fn == NAME) return PASS;
            MDB_CreateGraph();
            MDB_error e;
            int ret = !MDB_GetError(&e);
            MDB_FreeGraph();
            return ret;
        } return FAIL;
        case 1: { *name = "basic formation construction";
            if (fn == NAME) return PASS;
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
            if (fn == NAME) return PASS;
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

        default: printf("\nInvalid test id.\n"); return FAIL;
    }
}

int main() {
    int failCount = 0;
    char* s;
    for (int i = 0; i < TEST_COUNT; i++) {
        Test(i,NAME,&s);
        printf("Test %d (%s): ", i, s);
        int result = Test(i,RUN,&s);
        MDB_error e;
        while (MDB_GetError(&e)); // Clear errors
        if (!result) failCount++;
        printf("%s\n", result ? "passed" : "failed");
    }
    if (failCount == 0)
        printf("All tests passed");
    else printf("*** %d/%d TESTS FAILED ***",failCount,TEST_COUNT);
    printf("\n");
}
