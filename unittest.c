#include "mdb_global.h"
#include "mdb_manage.h"
#include "mdb_get_error.h"
#include "mdb_search_graph.h"
#include "mdb_edit_graph.h"
#include "mdb_read_graph.h"
#include "mdb_get_mem_info.h"
#include "mdb_free_node_map.h"
#include "mdb_use_node_map.h"
#include <stdio.h>
#define PASS 1
#define FAIL 0

#define NAME 0
#define RUN 1
#define COUNT 2
#define check(x) assert(x)

#define ND MDB_NODE
#define NM MDB_NODEMAP
#define SK MDB_DRAFT

ND A9(SK, ND, ND, ND, ND, ND, ND, ND, ND, ND, ND);
ND A8(SK, ND, ND, ND, ND, ND, ND, ND, ND, ND);
ND A7(SK, ND, ND, ND, ND, ND, ND, ND, ND);
ND A6(SK, ND, ND, ND, ND, ND, ND, ND);
ND A5(SK, ND, ND, ND, ND, ND, ND);
ND A4(SK, ND, ND, ND, ND, ND);
ND A3(SK, ND, ND, ND, ND);
ND A2(SK, ND, ND, ND);
ND A1(SK, ND, ND);
ND A0(SK, ND);

ND A9(SK s, ND o, ND a0, ND a1, ND a2, ND a3, ND a4, ND a5, ND a6, ND a7, ND a8) {
    ND n = MDB_DraftNode(s, MDB_FORM);
    MDB_AddLink(n, MDB_APPLY, o);
    MDB_AddLink(n, MDB_ARG0, a0);
    MDB_AddLink(n, MDB_ARG1, a1);
    MDB_AddLink(n, MDB_ARG2, a2);
    MDB_AddLink(n, MDB_ARG3, a3);
    MDB_AddLink(n, MDB_ARG4, a4);
    MDB_AddLink(n, MDB_ARG5, a5);
    MDB_AddLink(n, MDB_ARG6, a6);
    MDB_AddLink(n, MDB_ARG7, a7);
    MDB_AddLink(n, MDB_ARG8, a8);
    return n;
}
ND A8(SK s, ND o, ND a0, ND a1, ND a2, ND a3, ND a4, ND a5, ND a6, ND a7) {
    ND n = MDB_DraftNode(s, MDB_FORM);
    MDB_AddLink(n, MDB_APPLY, o);
    MDB_AddLink(n, MDB_ARG0, a0);
    MDB_AddLink(n, MDB_ARG1, a1);
    MDB_AddLink(n, MDB_ARG2, a2);
    MDB_AddLink(n, MDB_ARG3, a3);
    MDB_AddLink(n, MDB_ARG4, a4);
    MDB_AddLink(n, MDB_ARG5, a5);
    MDB_AddLink(n, MDB_ARG6, a6);
    MDB_AddLink(n, MDB_ARG7, a7);
    return n;
}
ND A7(SK s, ND o, ND a0, ND a1, ND a2, ND a3, ND a4, ND a5, ND a6) {
    ND n = MDB_DraftNode(s, MDB_FORM);
    MDB_AddLink(n, MDB_APPLY, o);
    MDB_AddLink(n, MDB_ARG0, a0);
    MDB_AddLink(n, MDB_ARG1, a1);
    MDB_AddLink(n, MDB_ARG2, a2);
    MDB_AddLink(n, MDB_ARG3, a3);
    MDB_AddLink(n, MDB_ARG4, a4);
    MDB_AddLink(n, MDB_ARG5, a5);
    MDB_AddLink(n, MDB_ARG6, a6);
    return n;
}
ND A6(SK s, ND o, ND a0, ND a1, ND a2, ND a3, ND a4, ND a5) {
    ND n = MDB_DraftNode(s, MDB_FORM);
    MDB_AddLink(n, MDB_APPLY, o);
    MDB_AddLink(n, MDB_ARG0, a0);
    MDB_AddLink(n, MDB_ARG1, a1);
    MDB_AddLink(n, MDB_ARG2, a2);
    MDB_AddLink(n, MDB_ARG3, a3);
    MDB_AddLink(n, MDB_ARG4, a4);
    MDB_AddLink(n, MDB_ARG5, a5);
    return n;
}
ND A5(SK s, ND o, ND a0, ND a1, ND a2, ND a3, ND a4) {
    ND n = MDB_DraftNode(s, MDB_FORM);
    MDB_AddLink(n, MDB_APPLY, o);
    MDB_AddLink(n, MDB_ARG0, a0);
    MDB_AddLink(n, MDB_ARG1, a1);
    MDB_AddLink(n, MDB_ARG2, a2);
    MDB_AddLink(n, MDB_ARG3, a3);
    MDB_AddLink(n, MDB_ARG4, a4);
    return n;
}
ND A4(SK s, ND o, ND a0, ND a1, ND a2, ND a3) {
    ND n = MDB_DraftNode(s, MDB_FORM);
    MDB_AddLink(n, MDB_APPLY, o);
    MDB_AddLink(n, MDB_ARG0, a0);
    MDB_AddLink(n, MDB_ARG1, a1);
    MDB_AddLink(n, MDB_ARG2, a2);
    MDB_AddLink(n, MDB_ARG3, a3);
    return n;
}
ND A3(SK s, ND o, ND a0, ND a1, ND a2) {
    ND n = MDB_DraftNode(s, MDB_FORM);
    MDB_AddLink(n, MDB_APPLY, o);
    MDB_AddLink(n, MDB_ARG0, a0);
    MDB_AddLink(n, MDB_ARG1, a1);
    MDB_AddLink(n, MDB_ARG2, a2);
    return n;
}
ND A2(SK s, ND o, ND a0, ND a1) {
    ND n = MDB_DraftNode(s, MDB_FORM);
    MDB_AddLink(n, MDB_APPLY, o);
    MDB_AddLink(n, MDB_ARG0, a0);
    MDB_AddLink(n, MDB_ARG1, a1);
    return n;
}
ND A1(SK s, ND o, ND a0) {
    ND n = MDB_DraftNode(s, MDB_FORM);
    MDB_AddLink(n, MDB_APPLY, o);
    MDB_AddLink(n, MDB_ARG0, a0);
    return n;
}

#define con MDB_CreateConst
#define draft MDB_StartDraft
#define sfree MDB_DiscardDraft
#define g MDB_CreateGraph
#define gfree MDB_FreeGraph
#define root MDB_SetDraftRoot
#define commit MDB_CommitDraft
#define mfree MDB_FreeNodeMap
#define node MDB_DraftNode
#define link MDB_AddLink
#define lookup MDB_MapNode

int Test(int i, int fn, char const** name);
int RunUnitTest(int id);

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
            MDB_DRAFT s = MDB_StartDraft();
            MDB_NODE plus = MDB_CreateConst("+");
            MDB_NODE eq = MDB_CreateConst("=");
            MDB_NODE one = MDB_CreateConst("1");
            MDB_NODE two = MDB_CreateConst("2");
            MDB_NODE lhs = MDB_DraftNode(s, MDB_FORM);
            MDB_NODE eqn = MDB_DraftNode(s, MDB_FORM);
            MDB_SetDraftRoot(s, eqn);
            MDB_AddLink(lhs, MDB_APPLY, plus);
            MDB_AddLink(lhs, MDB_ARG0, one);
            MDB_AddLink(lhs, MDB_ARG1, one);
            MDB_AddLink(eqn, MDB_APPLY, eq);
            MDB_AddLink(eqn, MDB_ARG0, lhs);
            MDB_AddLink(eqn, MDB_ARG1, two);
            MDB_CommitDraft(s);
            MDB_error e;
            check(!MDB_GetError(&e,1));
            check(!strcmp("=", MDB_NodeName(eq)) && MDB_Type(eq) == MDB_CONST && MDB_ChildCount(eq) == 0);
            check(!strcmp("+", MDB_NodeName(plus)) && MDB_Type(plus) == MDB_CONST && MDB_ChildCount(plus) == 0);
            check(!strcmp("1", MDB_NodeName(one)) && MDB_Type(one) == MDB_CONST && MDB_ChildCount(one) == 0);
            check(!strcmp("2", MDB_NodeName(two)) && MDB_Type(two) == MDB_CONST && MDB_ChildCount(two) == 0);
            check(!MDB_NodeName(lhs) && MDB_Type(lhs) == MDB_FORM && MDB_ChildCount(lhs) == 3 &&
                MDB_Child(lhs,0) == plus && MDB_Child(lhs,1) == one && MDB_Child(lhs,2) == one);
            check(!MDB_NodeName(eqn) && MDB_Type(eqn) == MDB_FORM && MDB_ChildCount(eqn) == 3 &&
                MDB_Child(eqn,0) == eq && MDB_Child(eqn,1) == lhs && MDB_Child(eqn,2) == two);
            check(!MDB_GetError(&e,1));
            MDB_FreeGraph();
            return PASS;
        }
        case 2: { *name = "simple cycle";
            if (fn == NAME) return -1;
            MDB_error e;
            MDB_CreateGraph();
            MDB_DRAFT s = MDB_StartDraft();
            MDB_NODE x = MDB_CreateConst("x");
            MDB_NODE y = MDB_DraftNode(s, MDB_FORM);
            MDB_NODE z = MDB_DraftNode(s, MDB_POCKET);
            MDB_SetDraftRoot(s, y);
            MDB_AddLink(y, MDB_APPLY, x);
            MDB_AddLink(y, MDB_ARG0, z);
            MDB_AddLink(z, MDB_ELEM, y);
            MDB_CommitDraft(s);
            check(!MDB_GetError(&e,1));
            check(!strcmp("x", MDB_NodeName(x)) && MDB_Type(x) == MDB_CONST && MDB_ChildCount(x) == 0);
            check(!MDB_NodeName(y) && MDB_Type(y) == MDB_FORM && MDB_ChildCount(y) == 2 &&
                MDB_Child(y,0) == x && MDB_Child(y,1) == z);
            check(!MDB_NodeName(z) && MDB_Type(z) == MDB_POCKET && MDB_ChildCount(z) == 1 &&
                MDB_Child(z,0) == y);
            check(!MDB_GetError(&e,1));
            MDB_FreeGraph();
            return PASS;
        }
        case 3: { *name = "simple world";
            if (fn == NAME) return -1;
            MDB_error e;
            MDB_CreateGraph();
            MDB_DRAFT s = MDB_StartDraft();
            MDB_NODE w = MDB_DraftNode(s, MDB_WORLD);
            MDB_SetDraftRoot(s, w);
            MDB_CommitDraft(s);
            s = MDB_StartDraft();
            MDB_NODE k = MDB_CreateConst("k");
            MDB_NODE v0 = MDB_CreateConst("v0");
            MDB_NODE v1 = MDB_DraftNode(s, MDB_FORM);
            MDB_NODE v2 = MDB_DraftNode(s, MDB_FORM);
            MDB_AddLink(v1, MDB_APPLY, k);
            MDB_AddLink(v1, MDB_ARG0, v0);
            MDB_AddLink(v2, MDB_APPLY, k);
            MDB_AddLink(v2, MDB_ARG0, v1);
            MDB_SetDraftRoot(s, v2);
            MDB_CommitDraft(s);
            MDB_AddLink(w, MDB_ELEM, v0);
            MDB_AddLink(w, MDB_ELEM, v1);
            MDB_AddLink(w, MDB_ELEM, v2);
            check(!MDB_GetError(&e,1));
            check(!strcmp("k", MDB_NodeName(k)));
            check(!strcmp("v0", MDB_NodeName(v0)));
            check(!MDB_NodeName(v1) && MDB_Type(v1) == MDB_FORM && MDB_ChildCount(v1) == 2 &&
                MDB_Child(v1,0) == k && MDB_Child(v1,1) == v0);
            check(!MDB_NodeName(v2) && MDB_Type(v2) == MDB_FORM && MDB_ChildCount(v2) == 2 &&
                MDB_Child(v2,0) == k && MDB_Child(v2,1) == v1);
            check(!MDB_NodeName(w) && MDB_Type(w) == MDB_WORLD && MDB_ChildCount(w) == 3 &&
                MDB_Child(w,0) == v0 && MDB_Child(w,1) == v1 && MDB_Child(w,2) == v2);
            check(!MDB_GetError(&e,1));
            MDB_FreeGraph();
            return PASS;
        }
        case 4: { *name = "discard s node";
            if (fn == NAME) return -1;
            MDB_error e;
            MDB_CreateGraph();
            MDB_DRAFT s = MDB_StartDraft();
            MDB_NODE n1 = MDB_DraftNode(s, MDB_WORLD);
            MDB_NODE n2 = MDB_DraftNode(s, MDB_FORM);
            MDB_SetDraftRoot(s, n1);
            MDB_DiscardDraftNode(n1);
            MDB_NODE n3 = MDB_DraftNode(s, MDB_POCKET);
            MDB_SetDraftRoot(s, n3);
            MDB_CommitDraft(s);
            check(!MDB_GetError(&e,1));
            check(MDB_Type(n2) == MDB_FORM);
            check(MDB_Type(n3) == MDB_POCKET);
            MDB_FreeGraph();
            return PASS;
        }
        case 5: { *name = "discard draft";
            if (fn == NAME) return -1;
            MDB_error e;
            MDB_CreateGraph();
            MDB_DRAFT s = MDB_StartDraft();
            MDB_DiscardDraft(s);
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
            //MDB_DRAFT s = MDB_StartDraft();
            //MDB_NODE w = MDB_DraftNode(s, MDB_WORLD);
            //MDB_SetDraftRoot(s, w);
            //MDB_CommitDraft(s);
            MDB_StartDraft();
            MDB_FreeGraph();
            return PASS;
        }
        case 8: { *name = "segfault";
            if (fn == NAME) return -1;
            MDB_CreateGraph();
            MDB_FreeGraph();
            MDB_CreateGraph();
            MDB_DRAFT s = MDB_StartDraft();
            MDB_NODE n1 = MDB_DraftNode(s, MDB_WORLD);
            MDB_DraftNode(s, MDB_FORM);
            MDB_SetDraftRoot(s, n1);
            MDB_DiscardDraftNode(n1);
            MDB_NODE n3 = MDB_DraftNode(s, MDB_POCKET);
            MDB_SetDraftRoot(s, n3);
            MDB_CommitDraft(s);
            MDB_FreeGraph();
            return PASS;
        }
        case 9: { *name = "segfault 2";
            if (fn == NAME) return -1;
            MDB_CreateGraph();
            MDB_DRAFT s = MDB_StartDraft();
            MDB_NODE n1 = MDB_DraftNode(s, MDB_WORLD);
            MDB_DraftNode(s, MDB_FORM);
            MDB_SetDraftRoot(s, n1);
            MDB_DiscardDraftNode(n1);
            MDB_NODE n3 = MDB_DraftNode(s, MDB_POCKET);
            MDB_SetDraftRoot(s, n3);
            MDB_CommitDraft(s);
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
    case 11: *name = "simple formation match";
        {
            if (fn == NAME) return -1;
            g();
            SK s = draft();
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
            check(MDB_MapNode(map, va) == one);
            check(MDB_MapNode(map, vb) == one);
            mfree(map);
            gfree();
            return PASS;
        }
    case 12: *name = "simple pattern search";
        {
            if (fn == NAME) return -1;
            g();
            SK s = draft();
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
            check(MDB_ChildCount(w) == 2 && !MDB_NodeName(w));
            check(MDB_Type(w) == MDB_POCKET || MDB_Type(w) == MDB_WORLD);//TODO: which one is correct (or should we allow both)?
            check((MDB_Child(w,0)==opt && MDB_Child(w,1)==tpt) ||
                (MDB_Child(w,1)==opt && MDB_Child(w,0)==tpt));
            gfree();
            return PASS;
        }
    case 13: *name = "discard non-empty draft";
        {
            if (fn == NAME) return -1;
            g();
            SK s = draft();
            node(s, MDB_POCKET);
            sfree(s);
            gfree();
            return PASS;
        }
    case 14: *name = "creating node after discarding draft";
        {
            if (fn == NAME) return -1;
            g();
            con("a");
            SK s = draft();
            node(s,MDB_WORLD);
            con("b");
            sfree(s);
            con("c");
            con("d");
            gfree();
            return PASS;
        }
    case 15: *name = "trigger error on linking two drafts (simple)";
        {
            if (fn == NAME) return -1;
            MDB_CreateGraph();
            SK s1 = MDB_StartDraft();
            SK s2 = MDB_StartDraft();
            ND n1 = MDB_DraftNode(s1, MDB_POCKET);
            ND n2 = MDB_DraftNode(s2, MDB_POCKET);
            MDB_AddLink(n1, MDB_ELEM, n2);
            MDB_error e;
            check(MDB_GetError(&e,1) && e.id == MDB_EINVARG);
            check(!MDB_GetError(&e,0));
            MDB_FreeGraph();
            return PASS;
        }
    case 16: *name = "trigger error on linking two drafts (complex)";
        {
            if (fn == NAME) return -1;
            MDB_CreateGraph();
            ND n2,n3,n4;
            SK s1,s2,s3;
            s1 = MDB_StartDraft();
            MDB_CreateConst("a");
            n2=MDB_DraftNode(s1, MDB_FORM);
            s2=MDB_StartDraft();
            n3=MDB_DraftNode(s2, MDB_FORM);
            MDB_AddLink(n2, MDB_APPLY, n3);
            MDB_DraftNode(s1, MDB_POCKET);
            s3=MDB_StartDraft();
            MDB_CreateConst("b");
            MDB_DiscardDraft(s1);
            MDB_DiscardDraft(s3);
            n4=MDB_DraftNode(s2, MDB_POCKET);
            MDB_DraftNode(s2, MDB_WORLD);
            MDB_DiscardDraftNode(n4);
            MDB_FreeGraph();
            return PASS;
        }
    case 17: *name = "discard root node";
        {
            if (fn==NAME) return -1;
            ND n1;
            SK s;
            MDB_CreateGraph();
            s = MDB_StartDraft();
            n1 = MDB_DraftNode(s, MDB_WORLD);
            MDB_SetDraftRoot(s, n1);
            MDB_DraftNode(s, MDB_WORLD);
            MDB_DiscardDraftNode(n1);
            MDB_FreeGraph();
            return PASS;
        }
        default: {
            if (fn == COUNT) return 18;
            fprintf(stderr, "\nInvalid test id.\n");
        } return FAIL;
    }
}

// returns 1 iff the test succeeded
int RunUnitTest(int id) {
    char const* s;
    Test(id,NAME,&s);
    fprintf(stderr, "Test %d (%s): ", id, s);
    UP startAllocCount = MDB_GetAllocatedBlockCount();
    int result = Test(id,RUN,&s);
    if (startAllocCount != MDB_GetAllocatedBlockCount()) result = FAIL;
    MDB_error e;
    if (MDB_GetError(&e,1)) result = FAIL;
    while (MDB_GetError(&e,1)); // Clear errors
    fprintf(stderr, "%s\n", result ? "passed" : "FAILED");
    return result;
}

int main(int argc, char const* const* argv) {
    int failCount = 0;
    char const* s;
    int testCount = Test(-1,COUNT,&s);
    if (argc > 1) { // args are a list of test numbers
        testCount = argc-1;
        for (int i = 1; i < argc; i++) {
            int id = (int)strtol(argv[i], 0, 0);
            failCount += !RunUnitTest(id);
        }
    } else { // if no args supplied run all tests
        for (int i = 0; i < testCount; i++) {
            failCount += !RunUnitTest(i);
        }
    }
    if (testCount > 1) { // only print a summary if we ran multiple tests
        if (failCount == 0)
            fprintf(stderr, "All tests passed");
        else fprintf(stderr, "*** %d/%d TESTS FAILED ***", failCount, testCount);
        fprintf(stderr, "\n");
    }
}
