#include "mdb_manage.h"
#include "mdb_edit_graph.h"
#include "mdb_get_error.h"
#include <stdio.h>

#define TEST_COUNT 2
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
            int ret = !MDB_GetError(&e);
            MDB_FreeGraph();
            return ret;
        } return FAIL;
        default: printf("\nInvalid test id.\n");
    }
}

int main() {
    int failCount = 0;
    char* s;
    for (int i = 0; i < TEST_COUNT; i++) {
        Test(i,NAME,&s);
        int result = Test(i,RUN,&s);
        printf("Running test %d (%s)... ", i, s);
        if (!result) failCount++;
        printf("%s\n", result ? "passed" : "failed");
    }
    if (failCount == 0)
        printf("All tests passed");
    else printf("*** %d TESTS FAILED ***",failCount);
    printf("\n");
}
