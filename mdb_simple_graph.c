#include "mdb_global.h"
#if 1
#include "mdb_graph.h" //IMPLEMENTS
#include "mdb_error.h"
#include "mdb_alloc_mem.h"
#include "mdb_all_multiset.h"
#include "mdb_all_vector.h"
#include "mdb_get_error_string.h"
#include <string.h>
#include <stdio.h>

#define log(...) fprintf(stderr, __VA_ARGS__)

typedef struct mdb_graph {
    u32 error;
    MDB_mset nodes,drafts;
} mdb_graph;
static mdb_graph g = {.error = MDB_EUNINIT};

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
    assert(g.error & MDB_EUNINIT);
    g = (mdb_graph){
        .error = MDB_ENONE,
        .nodes = {0},
        .drafts = {0},
    };
    log("done\n");
}

static void mdb_FreeNode(mdb_node* n) {
    if (n) {
        verify(~MDB_MSetRemove(&g.nodes, (UP)n));
        assert(!~MDB_MSetContains(&g.nodes, (UP)n));
        MDB_FreeVector(&n->out);
        MDB_FreeMSet(&n->in);
        MDB_Free(n->name);
        MDB_Free(n);
    }
}

void MDB_stdcall MDB_FreeGraph(void) {
    log("MDB_FreeGraph(): ");
    g.error |= MDB_EUNINIT;
    // go backwards since removing from the end doesn't affect the other elems
    for (UP i = g.nodes.s-1; ~i; i--) mdb_FreeNode((mdb_node*)g.nodes.a[i]);
    for (UP i = g.drafts.s-1; ~i; i--) {
        mdb_draft* d = (mdb_draft*)g.drafts.a[i];
        MDB_FreeMSet(&d->n);
        MDB_Free(d);
    }
    MDB_FreeMSet(&g.nodes);
    MDB_FreeMSet(&g.drafts);
    log("done\n");
}

MDB_DRAFT MDB_stdcall
MDB_StartDraft(void) {
    if (g.error) return 0;
    log("MDB_StartDraft(): ");
    mdb_draft* d = MDB_Alloc(sizeof(mdb_draft));
    if (!d) {
        g.error |= MDB_EMEM;
        return 0;
    }
    *d = (mdb_draft){
        .n = {0},
    };
    MDB_DRAFT draft = (UP)d;
    if (~MDB_MSetAdd(&g.drafts, draft)) {
        log("%x\n", (u32)draft);
        return draft;
    } else {
        g.error |= MDB_EMEM;
        return 0;
    }
}
MDB_NODE MDB_stdcall
MDB_DraftNode(MDB_DRAFT draft, MDB_NODETYPE type) {
    if (g.error) return 0;
    log("MDB_DraftNode(draft: %x, type: %x): ", (u32)draft, (u32)type);
    mdb_draft* d = (mdb_draft*)draft;
    mdb_node* n = MDB_Alloc(sizeof(mdb_node));
    if (!n) {g.error |= MDB_EMEM; return 0;}
    *n = (mdb_node){
        .type = type,
        .name = 0,
        .draft = d,
    };
    MDB_NODE node = (MDB_NODE)n;
    if (~MDB_MSetAdd(&d->n, node)) {
        if (~MDB_MSetAdd(&g.nodes, node)) {
            log("%x\n",(u32)node);
            return node;
        } else {
            g.error |= MDB_EMEM;
            MDB_MSetRemove(&d->n, node);
            MDB_Free(n);
            return 0;
        }
    } else {
        g.error |= MDB_EMEM;
        MDB_Free(n);
        return 0;
    }
}
MDB_NODE MDB_stdcall
MDB_CreateConst(const char* name) {
    if (g.error) return 0;
    log("MDB_CreateConst(name: %s): ", name);
    UP len = strlen(name)+1;
    mdb_node* n = MDB_Alloc(sizeof(mdb_node));
    if (!n) {g.error |= MDB_EMEM; return 0;}
    *n = (mdb_node){
        .type = MDB_CONST,
        .name = MDB_Alloc(len),
    };
    if (!n->name) {
        MDB_Free(n); g.error |= MDB_EMEM; return 0;
    }
    strcpy(n->name, name);
    MDB_NODE node = (MDB_NODE)n;
    if (~MDB_MSetAdd(&g.nodes, node)) {
        log("%x\n",(u32)node);
        return node;
    } else {
        g.error |= MDB_EMEM;
        MDB_Free(n);
        return 0;
    }
}
void MDB_stdcall
MDB_SetDraftRoot(MDB_DRAFT draft, MDB_NODE node) {
    if (g.error) return;
    log("MDB_SetDraftRoot(draft: %x, node: %x): ", (u32)draft, (u32)node);
    mdb_draft* d = (mdb_draft*)draft;
    mdb_node* n = (mdb_node*)node;
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
static s32 mdb_ValidateLink(mdb_node* s, MDB_LINKDESC link, mdb_node* d) {
    if (s->type == MDB_WORLD) {
        if (link != MDB_ELEM) {
            return g.error |= MDB_EINVARG;
        }
    } else {
        if (!s->draft) {
            g.error |= MDB_EINVARG;
        }
        if (s->draft != d->draft) {
            g.error |= MDB_ECROSSDRAFT;
        }
        if (s->type == MDB_FORM && !(link & MDB_ARG) && link != MDB_APPLY) {
            g.error |= MDB_EINVARG;
        }
        if ((s->type == MDB_WORLD || s->type == MDB_POCKET) && link != MDB_ELEM) {
            g.error |= MDB_EINVARG;
        }
        if (s->type != MDB_FORM && s->type != MDB_WORLD && s->type != MDB_POCKET) {
            g.error |= MDB_EINVARG;
        }
    }
    return g.error != 0;
}

void MDB_stdcall
MDB_AddLink(MDB_NODE src, MDB_LINKDESC link, MDB_NODE dst) {
    if (g.error) return;
    log("MDB_AddLink(src: %x, link: %x, dst: %x): ",(u32)src,(u32)link,(u32)dst);
    mdb_node* s = (mdb_node*)src;
    mdb_node* d = (mdb_node*)dst;
    if (!mdb_ValidateLink(s,link,d)) {
        return;
    }
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
    g.error |= MDB_EMEM;
}

void MDB_stdcall
MDB_DiscardDraftNode(MDB_NODE node) {
    if (g.error) return;
    if (!node) return;
    log("MDB_DiscardDraftNode(node: %x): ", (u32)node);
    mdb_node* n = (mdb_node*)node;
    mdb_draft* d = n->draft;
    verify(~MDB_MSetRemove(&d->n, node));
    assert(!~MDB_MSetContains(&d->n, node));
    mdb_FreeNode(n);
    log("done\n");
}
void MDB_stdcall
MDB_DiscardDraft(MDB_DRAFT draft) {
    if (g.error) return;
    if (draft == 0) return;
    log("MDB_DiscardDraft(draft: %x): ", (u32)draft);
    mdb_draft* d = (mdb_draft*)draft;
    for (UP i = 0; i < d->n.s; i++)
        mdb_FreeNode((mdb_node*)d->n.a[i]);
    MDB_FreeMSet(&d->n);
    MDB_Free(d);
    MDB_MSetRemove(&g.drafts,draft);
    log("done\n");
}
void MDB_stdcall
MDB_DiscardLink(MDB_NODE src, MDB_LINKDESC link, MDB_NODE dst) {
    if (g.error) return;
    if (!src || !dst) return;
    log("MDB_DiscardLink(src: %x, link: %x, dst: %x): ",(u32)src,(u32)link,(u32)dst);
    mdb_node* s = (mdb_node*)src;
    mdb_node* d = (mdb_node*)dst;
    if (mdb_ValidateLink(s,link,d)) {
       UP idx = mdb_LinkToIndex(s->out.s, link);
       s->out.a[idx] = 0;
       verify(~MDB_MSetRemove(&d->in, src));
    }
    log("done\n");
}
int32_t MDB_stdcall
MDB_CommitDraft(MDB_DRAFT draft) {
    if (g.error) return 0;
    log("MDB_CommitDraft(draft: %x): ", (u32)draft);
    mdb_draft* d = (mdb_draft*)draft;
    assert(d->root);
    for (UP i = 0; i < d->n.s; i++) {
        mdb_node* n = (mdb_node*)d->n.a[i];
        assert(n->draft == d);
        n->draft = 0;
        for (UP j = 0; j < n->out.s; j++) {
            mdb_node* c = (mdb_node*)n->out.a[j];
            if (!c) {log("0\n");return 0;} // gap
            verify(~MDB_MSetAdd(&c->in, (UP)n));
        }
    }
    MDB_FreeMSet(&d->n);
    MDB_Free(d);
    MDB_MSetRemove(&g.drafts,draft);
    log("1\n");
    return 1;
}

MDB_NODETYPE MDB_stdcall MDB_Type(MDB_NODE node) {
    return ((mdb_node*)node)->type;
}
uintptr_t MDB_stdcall MDB_ChildCount(MDB_NODE node) {
    return ((mdb_node*)node)->out.s;
}
char const* MDB_stdcall MDB_NodeName(MDB_NODE node) {
    return ((mdb_node*)node)->name;
}
uintptr_t MDB_stdcall MDB_Child(MDB_NODE node, uintptr_t idx) {
    assert(idx < ((mdb_node*)node)->out.s);
    return ((mdb_node*)node)->out.a[idx];
}

int32_t MDB_stdcall MDB_GetError(MDB_error* error, int32_t clear) {
    error->id = g.error & ~MDB_EUNINIT;
    error->str = mdb_GetErrorString(error->id);
    if (clear) {
        g.error &= MDB_EUNINIT; // Don't erase uninit bit
    }
    return error->id;
}

#endif
