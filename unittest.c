#include "mdb_global.h"
#include "mdb_manage.h"
#include "mdb_edit_graph.h"
#include "mdb_get_error.h"
#include "mdb_read_graph.h"
#include "mdb_get_mem_info.h"
#include "mdb_search_graph.h"
#include "mdb_read_map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#define PASS 1
#define FAIL 0

#define NAME 0
#define RUN 1
#define COUNT 2

void MDB_DebugBreak2(void) {}
#define debug MDB_DebugBreak2

#define check(x) if (!(x)) debug()
//#define check(x) do if (!(x)) { assert(0); return FAIL; } while(0)

#define ND MDB_NODE
#define NM MDB_NODEMAP
#define SK MDB_SKETCH

ND A9(SK s, ND o, ND a0,ND a1,ND a2,ND a3,ND a4,ND a5,ND a6,ND a7,ND a8) {
    ND n = MDB_SketchNode(s, MDB_FORM);
    MDB_AddLink(n, MDB_APPLY, o);
    MDB_AddLink(n, MDB_ARG0, a0);
    MDB_AddLink(n, MDB_ARG0, a1);
    MDB_AddLink(n, MDB_ARG0, a2);
    MDB_AddLink(n, MDB_ARG0, a3);
    MDB_AddLink(n, MDB_ARG0, a4);
    MDB_AddLink(n, MDB_ARG0, a5);
    MDB_AddLink(n, MDB_ARG0, a6);
    MDB_AddLink(n, MDB_ARG0, a7);
    MDB_AddLink(n, MDB_ARG0, a8);
    return n;
}
ND A8(SK s, ND o, ND a0,ND a1,ND a2,ND a3,ND a4,ND a5,ND a6,ND a7) {
    ND n = MDB_SketchNode(s, MDB_FORM);
    MDB_AddLink(n, MDB_APPLY, o);
    MDB_AddLink(n, MDB_ARG0, a0);
    MDB_AddLink(n, MDB_ARG0, a1);
    MDB_AddLink(n, MDB_ARG0, a2);
    MDB_AddLink(n, MDB_ARG0, a3);
    MDB_AddLink(n, MDB_ARG0, a4);
    MDB_AddLink(n, MDB_ARG0, a5);
    MDB_AddLink(n, MDB_ARG0, a6);
    MDB_AddLink(n, MDB_ARG0, a7);
    return n;
}
ND A7(SK s, ND o, ND a0,ND a1,ND a2,ND a3,ND a4,ND a5,ND a6) {
    ND n = MDB_SketchNode(s, MDB_FORM);
    MDB_AddLink(n, MDB_APPLY, o);
    MDB_AddLink(n, MDB_ARG0, a0);
    MDB_AddLink(n, MDB_ARG0, a1);
    MDB_AddLink(n, MDB_ARG0, a2);
    MDB_AddLink(n, MDB_ARG0, a3);
    MDB_AddLink(n, MDB_ARG0, a4);
    MDB_AddLink(n, MDB_ARG0, a5);
    MDB_AddLink(n, MDB_ARG0, a6);
    return n;
}
ND A6(SK s, ND o, ND a0,ND a1,ND a2,ND a3,ND a4,ND a5) {
    ND n = MDB_SketchNode(s, MDB_FORM);
    MDB_AddLink(n, MDB_APPLY, o);
    MDB_AddLink(n, MDB_ARG0, a0);
    MDB_AddLink(n, MDB_ARG0, a1);
    MDB_AddLink(n, MDB_ARG0, a2);
    MDB_AddLink(n, MDB_ARG0, a3);
    MDB_AddLink(n, MDB_ARG0, a4);
    MDB_AddLink(n, MDB_ARG0, a5);
    return n;
}
ND A5(SK s, ND o, ND a0,ND a1,ND a2,ND a3,ND a4) {
    ND n = MDB_SketchNode(s, MDB_FORM);
    MDB_AddLink(n, MDB_APPLY, o);
    MDB_AddLink(n, MDB_ARG0, a0);
    MDB_AddLink(n, MDB_ARG0, a1);
    MDB_AddLink(n, MDB_ARG0, a2);
    MDB_AddLink(n, MDB_ARG0, a3);
    MDB_AddLink(n, MDB_ARG0, a4);
    return n;
}
ND A4(SK s, ND o, ND a0,ND a1,ND a2,ND a3) {
    ND n = MDB_SketchNode(s, MDB_FORM);
    MDB_AddLink(n, MDB_APPLY, o);
    MDB_AddLink(n, MDB_ARG0, a0);
    MDB_AddLink(n, MDB_ARG0, a1);
    MDB_AddLink(n, MDB_ARG0, a2);
    MDB_AddLink(n, MDB_ARG0, a3);
    return n;
}
ND A3(SK s, ND o, ND a0,ND a1,ND a2) {
    ND n = MDB_SketchNode(s, MDB_FORM);
    MDB_AddLink(n, MDB_APPLY, o);
    MDB_AddLink(n, MDB_ARG0, a0);
    MDB_AddLink(n, MDB_ARG1, a1);
    MDB_AddLink(n, MDB_ARG2, a2);
    return n;
}
ND A2(SK s, ND o, ND a0,ND a1) {
    ND n = MDB_SketchNode(s, MDB_FORM);
    MDB_AddLink(n, MDB_APPLY, o);
    MDB_AddLink(n, MDB_ARG0, a0);
    MDB_AddLink(n, MDB_ARG1, a1);
    return n;
}
ND A1(SK s, ND o, ND a0) {
    ND n = MDB_SketchNode(s, MDB_FORM);
    MDB_AddLink(n, MDB_APPLY, o);
    MDB_AddLink(n, MDB_ARG0, a0);
    return n;
}

#define con MDB_CreateConst
#define sketch MDB_StartSketch
#define sfree MDB_DiscardSketch
#define graph MDB_CreateGraph
#define gfree MDB_FreeGraph
#define root MDB_SetSketchRoot
#define commit MDB_CommitSketch
#define mfree MDB_FreeNodeMap
#define node MDB_SketchNode
#define link MDB_AddLink
#define lookup MDB_LookupNode

int Test(int i, int fn, char const** name) {
    switch (i) {
        case 0: { *name = "create free";
            if (fn == NAME) return -1;
            MDB_CreateGraph();
            MDB_error e;
            int ret = !MDB_GetError(&e,1);
            MDB_FreeGraph();
            return ret;
        }
        case 1: { *name = "basic formation construction";
            if (fn == NAME) return -1;
            MDB_CreateGraph();
            MDB_SKETCH s = MDB_StartSketch();
            MDB_NODE plus = MDB_CreateConst("+");
            MDB_NODE eq = MDB_CreateConst("=");
            MDB_NODE one = MDB_CreateConst("1");
            MDB_NODE two = MDB_CreateConst("2");
            MDB_NODE lhs = MDB_SketchNode(s, MDB_FORM);
            MDB_NODE eqn = MDB_SketchNode(s, MDB_FORM);
            MDB_SetSketchRoot(s, eqn);
            MDB_AddLink(lhs, MDB_APPLY, plus);
            MDB_AddLink(lhs, MDB_ARG0, one);
            MDB_AddLink(lhs, MDB_ARG1, one);
            MDB_AddLink(eqn, MDB_APPLY, eq);
            MDB_AddLink(eqn, MDB_ARG0, lhs);
            MDB_AddLink(eqn, MDB_ARG1, two);
            MDB_CommitSketch(s);
            MDB_error e;
            check(!MDB_GetError(&e,1));
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

            check(!MDB_GetError(&e,1));
            MDB_FreeGraph();
            return PASS;
		}
        case 2: { *name = "simple cycle";
            if (fn == NAME) return -1;
            MDB_error e;
            MDB_CreateGraph();
            MDB_SKETCH s = MDB_StartSketch();
            MDB_NODE x = MDB_CreateConst("x");
            MDB_NODE y = MDB_SketchNode(s, MDB_FORM);
            MDB_NODE z = MDB_SketchNode(s, MDB_POCKET);
            MDB_SetSketchRoot(s, y);
            MDB_AddLink(y, MDB_APPLY, x);
            MDB_AddLink(y, MDB_ARG0, z);
            MDB_AddLink(z, MDB_ELEM, y);
            MDB_CommitSketch(s);
            check(!MDB_GetError(&e,1));
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

            check(!MDB_GetError(&e,1));
            MDB_FreeGraph();
            return PASS;
		}
        case 3: { *name = "simple world";
            if (fn == NAME) return -1;
            MDB_error e;
            MDB_CreateGraph();
            MDB_SKETCH s = MDB_StartSketch();
            MDB_NODE w = MDB_SketchNode(s, MDB_WORLD);
            MDB_SetSketchRoot(s, w);
            MDB_CommitSketch(s);
            s = MDB_StartSketch();
            MDB_NODE k = MDB_CreateConst("k");
            MDB_NODE v0 = MDB_CreateConst("v0");
            MDB_NODE v1 = MDB_SketchNode(s, MDB_FORM);
            MDB_NODE v2 = MDB_SketchNode(s, MDB_FORM);
            MDB_AddLink(v1, MDB_APPLY, k);
            MDB_AddLink(v1, MDB_ARG0, v0);
            MDB_AddLink(v2, MDB_APPLY, k);
            MDB_AddLink(v2, MDB_ARG0, v1);
            MDB_SetSketchRoot(s, v2);
            MDB_CommitSketch(s);
            MDB_AddLink(w, MDB_ELEM, v0);
            MDB_AddLink(w, MDB_ELEM, v1);
            MDB_AddLink(w, MDB_ELEM, v2);
            check(!MDB_GetError(&e,1));
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
            check(!MDB_GetError(&e,1));
            MDB_FreeGraph();
            return PASS;
		}
        case 4: { *name = "discard s node";
            if (fn == NAME) return -1;
            MDB_error e;
            MDB_CreateGraph();
            MDB_SKETCH s = MDB_StartSketch();
            MDB_NODE n1 = MDB_SketchNode(s, MDB_WORLD);
            MDB_NODE n2 = MDB_SketchNode(s, MDB_FORM);
            MDB_SetSketchRoot(s, n1);
            MDB_DiscardSketchNode(s);
            MDB_NODE n3 = MDB_SketchNode(s, MDB_POCKET);
            MDB_SetSketchRoot(s, n3);
            MDB_CommitSketch(s);
            check(!MDB_GetError(&e,1));
            MDB_NODETYPE t;
            uintptr_t childCount;
            char* str;
            MDB_GetNodeInfo(n2, &t, &childCount, &str);
            check(t == MDB_FORM);
            MDB_GetNodeInfo(n3, &t, &childCount, &str);
            check(t == MDB_POCKET);
            MDB_FreeGraph();
            return PASS;
		}
        case 5: { *name = "discard sketch";
            if (fn == NAME) return -1;
            MDB_error e;
            MDB_CreateGraph();
            MDB_SKETCH s = MDB_StartSketch();
            MDB_DiscardSketch(s);
            check(!MDB_GetError(&e,1));
            MDB_FreeGraph();
            return PASS;
        }
        case 6: { *name = "memory leak 1";
            if (fn == NAME) return -1;
            MDB_CreateGraph();
            MDB_CreateConst("+");
            MDB_FreeGraph();
            return PASS;
        }
        case 7: { *name = "memory leak 2";
            if (fn == NAME) return -1;
            MDB_CreateGraph();
            //MDB_SKETCH s = MDB_StartSketch();
            //MDB_NODE w = MDB_SketchNode(s, MDB_WORLD);
            //MDB_SetSketchRoot(s, w);
            //MDB_CommitSketch(s);
            MDB_StartSketch();
            MDB_FreeGraph();
            return PASS;
        }
        case 8: { *name = "segfault";
            if (fn == NAME) return -1;
            MDB_CreateGraph();
            MDB_FreeGraph();
            MDB_CreateGraph();
            MDB_SKETCH s = MDB_StartSketch();
            MDB_NODE n1 = MDB_SketchNode(s, MDB_WORLD);
            MDB_SketchNode(s, MDB_FORM);
            MDB_SetSketchRoot(s, n1);
            MDB_DiscardSketchNode(s);
            MDB_NODE n3 = MDB_SketchNode(s, MDB_POCKET);
            MDB_SetSketchRoot(s, n3);
            MDB_CommitSketch(s);
            MDB_FreeGraph();
            return PASS;
        }
        case 9: { *name = "segfault 2";
            if (fn == NAME) return -1;
            MDB_CreateGraph();
            MDB_SKETCH s = MDB_StartSketch();
            MDB_NODE n1 = MDB_SketchNode(s, MDB_WORLD);
            MDB_SketchNode(s, MDB_FORM);
            MDB_SetSketchRoot(s, n1);
            MDB_DiscardSketchNode(s);
            MDB_NODE n3 = MDB_SketchNode(s, MDB_POCKET);
            MDB_SetSketchRoot(s, n3);
            MDB_CommitSketch(s);
            MDB_FreeGraph();
            return PASS;
        }
        case 10: { *name = "const match";
            if (fn == NAME) return -1;
            MDB_CreateGraph();
            MDB_NODE a = MDB_CreateConst("a");
            MDB_NODE b = MDB_CreateConst("b");
            MDB_NODE c = MDB_CreateConst("c");
            MDB_NODEMAP map = MDB_MatchPattern(a,b,c);
            check(!map);
            map = MDB_MatchPattern(a,a,c);
            check(map);
			MDB_FreeNodeMap(map);
            MDB_FreeGraph();
            return PASS;
        }
        case 11: { *name = "simple formation match";
            if (fn == NAME) return -1;
            graph();
            SK s = sketch();
            ND var = con("var");
            ND a = con("a");
            ND b = con("b");
            ND plus = con("+");
            ND one = con("1");
            ND two = A2(s,plus,one,one);
            ND c = con("c");
            MDB_NODE va = A2(s,var,c,a);
            MDB_NODE vb = A2(s,var,c,b);
            MDB_NODE sum = A2(s,plus,va,vb);
            root(s, sum);
            commit(s);
            NM map = MDB_MatchPattern(sum, two,c);
            check(map != 0);
            check(MDB_LookupNode(map, va) == one);
            check(MDB_LookupNode(map, vb) == one);
			mfree(map);
            gfree();
            return PASS;
        }
		case 12: {*name = "simple pattern search";
			if (fn == NAME) return -1;
			graph();
			SK s = sketch();
			ND var = con("var");
			ND a = con("a");
			ND plus = con("+");
			ND one = con("1");
			ND two = con("2");
			ND c = con("c");
			ND va = A2(s,var,c,a);
			ND opo = A2(s,plus,one,one);
			ND opt = A2(s,plus,one,two);
			ND tpo = A2(s,plus,two,one);
			ND tpt = A2(s,plus,two,two);
			ND world = node(s, MDB_WORLD);
			link(world,MDB_ELEM,opo);
			link(world,MDB_ELEM,opt);
			link(world,MDB_ELEM,tpo);
			link(world,MDB_ELEM,tpt);
			root(s,world);
			ND apt = A2(s,plus,va,two);
			commit(s);
			ND w = MDB_MatchAllNodes(apt,world,c);
			check(w);
			MDB_NODETYPE t; UP cc; char* str;
			MDB_GetNodeInfo(w, &t, &cc, &str);
			check(cc == 2 && !str);
			check(t == MDB_POCKET || t == MDB_WORLD);//TODO: which one is correct (or should we allow both)?
			ND children[2];
			check(MDB_GetChildren(w, 0, 2, children) == 2);
			check((children[0]==opt && children[1]==tpt) ||
				(children[1]==opt && children[0]==tpt));
			gfree();
			return PASS;
		}
		case 13: {*name = "discard non-empty sketch";
			if (fn == NAME) return -1;
			graph();
			SK s = sketch();
			node(s, MDB_POCKET);
			sfree(s);
			gfree();
			return PASS;
		} break;
		case 14: {*name = "creating node after discarding sketch";
			if (fn == NAME) return -1;
			graph();
			con("a");
			SK s = sketch();
			node(s,MDB_WORLD);
			con("b");
			sfree(s);
			con("c");
			con("d");
			gfree();
			return PASS;
		} break;
		default: {
            if (fn == COUNT) return 15;
            fprintf(stderr, "\nInvalid test id.\n");
        } return FAIL;
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
    //printf("total allocation: %"PRIu64" blocks\n", MDB_GetAllocatedBlockCount());
    MDB_error e;
    if (MDB_GetError(&e,1)) result = FAIL;
    while (MDB_GetError(&e,1)); // Clear errors
    printf("%s\n", result ? "passed" : "FAILED");
    return result;
}

#if 0
int main(int argc, char const* const* argv) {
    int failCount = 0;
    char* s;
    int testCount = Test(-1,COUNT,&s);
    if (argc > 1) { // args are a list of test numbers
        testCount = argc-1;
        for (int i = 1; i < argc; i++) {
            int id;
            sscanf_s(argv[i], "%d", &id);
            failCount += !RunTest(id);
        }
    } else { // if no args supplied run all tests
        for (int i = 0; i < testCount; i++) {
            failCount += !RunTest(i);
        }
    }
    if (testCount > 1) { // only print a summary if we ran multiple tests
        if (failCount == 0)
            printf("All tests passed");
        else printf("*** %d/%d TESTS FAILED ***",failCount,testCount);
        printf("\n");
    }
#ifdef _WIN32
	system("pause");
#endif
}
#endif