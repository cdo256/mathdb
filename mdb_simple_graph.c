#include "mdb_global.h"
#include "mdb_graph.h" //IMPLEMENTS
#include "mdb_all_multiset.h"
#include "mdb_error.h"
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#define log(...) fprintf(stderr, __VA_ARGS__)

typedef struct mdb_graph {
    s32 init, badbit;
    MDB_mset nodes,drafts;
} mdb_graph;
static mdb_graph g = {.init = 0,.badbit = 1};

typedef struct mdb_node mdb_node;

typedef struct mdb_draft {
    MDB_mset n;
    mdb_node* root;
} mdb_draft;

typedef struct mdb_node {
    MDB_NODETYPE type;
    mdb_draft* draft;
    char* name;
    MDB_mset in;
    MDB_vector out;
} mdb_node;

void MDB_stdcall MDB_CreateGraph(void) {
    log("MDB_CreateGraph(): ");
    assert(!g.init);
    g = (mdb_graph){
        .init = 1,
        .badbit = 0,
        .nodes = {0},
        .drafts = {0},
    };
    log("done\n");
}

static void mdb_FreeNode(mdb_node* n) {
    if (n) {
        verify(~MDB_MSetRemove(&g.nodes, n));
        assert(!~MDB_MSetContains(&g.nodes, n));
        MDB_FreeVector(&n->out);
        MDB_FreeMSet(&n->in);
        free(n->name);
        free(n);
    }
}

void MDB_stdcall MDB_FreeGraph(void) {
    log("MDB_FreeGraph(): ");
    g.init = 0;
    // go backwards since removing from the end doesn't affect the other elems
    for (UP i = g.nodes.s - 1; ~i; i--) mdb_FreeNode(g.nodes.a[i]);
    for (UP i = g.drafts.s-1; ~i; i--) {
        mdb_draft* d = g.drafts.a[i];
        MDB_FreeMSet(&d->n);
        free(d);
    }
    MDB_FreeMSet(&g.nodes);
    MDB_FreeMSet(&g.drafts);
    log("done\n");
}

MDB_DRAFT MDB_stdcall
MDB_StartDraft(void) {
    if (g.badbit) return 0;
    log("MDB_StartDraft(): ");
    mdb_draft* d = malloc(sizeof(mdb_draft));
    if (!d) {g.badbit = 1; return 0;}
    *d = (mdb_draft){
        .n = {0},
    };
    MDB_DRAFT draft = d;
    if (~MDB_MSetAdd(&g.drafts, draft)) {
        log("%p\n", draft);
        return draft;
    }
    g.badbit = 1;
    return 0;
}
MDB_NODE MDB_stdcall
MDB_DraftNode(MDB_DRAFT draft, MDB_NODETYPE type) {
    if (g.badbit) return 0;
    log("MDB_DraftNode(draft: %p, type: %x): ", draft, (u32)type);
    mdb_draft* d = draft;
    mdb_node* n = malloc(sizeof(mdb_node));
    if (!n) {g.badbit = 1; return 0;}
    *n = (mdb_node){
        .type = type,
        .name = 0,
        .draft = d,
    };
    MDB_NODE node = (MDB_NODE)n;
    if (~MDB_MSetAdd(&d->n, node)) {
        if (~MDB_MSetAdd(&g.nodes, node)) {
            log("%p\n", node);
            return node;
        }
        MDB_MSetRemove(&d->n, node);
    }
    g.badbit = 1;
    free(n);
    return 0;
}
MDB_NODE MDB_stdcall
MDB_CreateConst(const char* name) {
    if (g.badbit) return 0;
    log("MDB_CreateConst(name: %s): ", name);
    UP len = strlen(name)+1;
    mdb_node* n = malloc(sizeof(mdb_node));
    if (!n) {g.badbit = 1; return 0;}
    *n = (mdb_node){
        .type = MDB_CONST,
        .name = malloc(len),
    };
    if (!n->name) {free(n); g.badbit = 1; return 0;}
    strcpy(n->name, name);
    MDB_NODE node = (MDB_NODE)n;
    if (~MDB_MSetAdd(&g.nodes, node)) {
        log("%p\n", node);
        return node;
    }
    g.badbit = 1;
    free(n);
    return 0;
}
void MDB_stdcall
MDB_SetDraftRoot(MDB_DRAFT draft, MDB_NODE node) {
    if (g.badbit) return;
    log("MDB_SetDraftRoot(draft: %p, node: %p): ", draft, node);
    mdb_draft* d = draft;
    mdb_node* n = node;
    assert(~MDB_MSetContains(&d->n,node));
    d->root = n;
    log("done\n");
}

static UP mdb_LinkToIndex(UP end, MDB_LINKDESC link) {
    if (link & MDB_ARG) return (link & ~MDB_ARG) + 1;
    if (link == MDB_APPLY) return 0;
    if (link == MDB_ELEM) return end;
    assert(0);
    return ~0ULL;
}

// a world elem may link anywhere
// otherwise, s must be in a draft and d is not in a different draft
static s32 mdb_ValidLink(mdb_node* s, MDB_LINKDESC link, mdb_node* d) {
    if ((s->draft && (!d->draft || d->draft == s->draft)) || (s->type == MDB_WORLD && link == MDB_ELEM)) {
        if (link & MDB_ARG || link == MDB_APPLY) return (s->type == MDB_FORM);
        else if (link == MDB_ELEM) return (s->type == MDB_WORLD || s->type == MDB_POCKET);
        return 1;
    } else return 0;
}

void MDB_stdcall
MDB_AddLink(MDB_NODE src, MDB_LINKDESC link, MDB_NODE dst) {
    if (g.badbit) return;
    log("MDB_AddLink(src: %p, link: %x, dst: %p): ", src, (u32)link, dst);
    mdb_node* s = src;
    mdb_node* d = dst;
    assert(mdb_ValidLink(s,link,d));
    UP idx = mdb_LinkToIndex(s->out.s, link);
    if (s->draft || ~MDB_MSetAdd(&d->in, src)) {
        if (~MDB_GrowVector(&s->out, idx+1, 0ULL)) {
            assert(!s->out.a[idx]);
            s->out.a[idx] = dst;
            s->out.s++;
            log("done\n");
            return;
        } else if (!s->draft) MDB_MSetRemove(&d->in, src);
    }
    g.badbit = 1;
}

void MDB_stdcall
MDB_DiscardDraftNode(MDB_NODE node) {
    if (!node) return;
    log("MDB_DiscardDraftNode(node: %p): ", node);
    mdb_node* n = node;
    mdb_draft* d = n->draft;
    verify(~MDB_MSetRemove(&d->n, node));
    assert(!~MDB_MSetContains(&d->n, node));
    mdb_FreeNode(n);
    log("done\n");
}
void MDB_stdcall
MDB_DiscardDraft(MDB_DRAFT draft) {
    if (draft == 0) return;
    log("MDB_DiscardDraft(draft: %p): ", draft);
    mdb_draft* d = draft;
    for (UP i = 0; i < d->n.s; i++)
        mdb_FreeNode(d->n.a[i]);
    MDB_FreeMSet(&d->n);
    free(d);
    MDB_MSetRemove(&g.drafts,draft);
    log("done\n");
}
void MDB_stdcall
MDB_DiscardLink(MDB_NODE src, MDB_LINKDESC link, MDB_NODE dst) {
    if (!src || !dst) return;
    log("MDB_DiscardLink(src: %p, link: %x, dst: %p): ", src, (u32)link, dst);
    mdb_node* s = src;
    mdb_node* d = dst;
    assert(mdb_ValidLink(s,link,d));
    UP idx = mdb_LinkToIndex(s->out.s, link);
    s->out.a[idx] = 0;
    verify(~MDB_MSetRemove(&d->in, src));
    log("done\n");
}
int32_t MDB_stdcall
MDB_CommitDraft(MDB_DRAFT draft) {
    if (g.badbit) return 0;
    log("MDB_CommitDraft(draft: %p): ", draft);
    mdb_draft* d = draft;
    assert(d->root);
    for (UP i = 0; i < d->n.s; i++) {
        mdb_node* n = d->n.a[i];
        assert(n->draft == d);
        n->draft = 0;
        for (UP j = 0; j < n->out.s; j++) {
            mdb_node* c = n->out.a[j];
            if (!c) {log("0\n");return 0;} // gap
            verify(~MDB_MSetAdd(&c->in, n));
        }
    }
    MDB_FreeMSet(&d->n);
    free(d);
    log("1\n");
    return 1;
}

MDB_NODETYPE MDB_stdcall MDB_Type(MDB_NODE node) {
    return node->type;
}
uintptr_t MDB_stdcall MDB_ChildCount(MDB_NODE node) {
    return node->out.s;
}
char const* MDB_stdcall MDB_NodeName(MDB_NODE node) {
    return node->name;
}
MDB_NODE MDB_stdcall MDB_Child(MDB_NODE node, uintptr_t idx) {
    assert(idx < node->out.s);
    return node->out.a[idx];
}

int32_t MDB_stdcall MDB_GetError(MDB_error* error, int32_t clear) {
    //using asserts instead so this is a nop
    (void)error; (void)clear;
    return 0;
}
