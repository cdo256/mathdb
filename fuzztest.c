#include "mdb_global.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "mdb_manage.h"
#include "mdb_all_generic_map.h"
#include "mdb_read_graph.h"
#include "mdb_edit_graph.h"
#include "mdb_util.h"
#define MAX_DRAFT 10
#define MAX_NODE 40
#define MAX_MAP 11

typedef struct id_array {
    UP c, s;
    VAL* a;
} id_array;

uintptr_t* FirstBlank(id_array* a) {
    for (UP i =0;i<a->s;i++)if (a->a[i]==0)return &a->a[i];
    return 0;
}

uintptr_t PickUniform(id_array* a, int zero) {
    if (a->c==0)return 0;
    UP r = (UP)rand()%(a->c+zero);
    if (r == a->c) return 0;
    UP j = 0;
    for (UP i = 0; i < a->s; i++)
        if (a->a[i]&&j++==r)return a->a[i];
    assert(0);
    return 0;
}

typedef struct node_info {
    UP childCount;
    UP childEnd;
    MDB_NODETYPE type;
    MDB_DRAFT draft;
    u32 childBits;
    UP rc;
} node_info;

typedef struct draft_info {
    MDB_NODE root;
    UP size;
    UP nonSaturated; // number of nodes that don't have consecutive children
} draft_info;

void PrepareNodeRemove(MDB_NODE nd, MDB_generic_map* sm, MDB_generic_map* nm, s32* sNonEmpty, id_array* n, s32* nUnreferenced, s32* sNonEmptyUnrooted) {
    node_info** k = (node_info**)MDB_GLookup(nm, nd);
    node_info* ni = k[1];
    draft_info* si = MDB_GLookup(sm, ni->draft)[1];
    si->size--;
    if (si->size!=0)--*sNonEmpty;
    if (si->root==nd)*sNonEmptyUnrooted+=(si->size!=0),si->root=0;
    else if (si->size!=0) --*sNonEmptyUnrooted;
    UP notSat = 0;
    UP childCount = MDB_ChildCount(nd);
    if (childCount > 0) {
        for (UP i = 0; i < childCount; i++) {
            if (!MDB_Child(nd,i)) {notSat = 1; continue;}
            //TODO: do I need to have a special case for self-loops?
            // do I need to handle all cycles specially?
            node_info* dni = MDB_GLookup(nm, MDB_Child(nd, i))[1];
            if (~(UP)dni && MDB_Child(nd, i) != nd && dni->rc-- == 1)
                --*nUnreferenced;
        }
        si->nonSaturated -= notSat;
        assert(~si->nonSaturated);
    }
    for (UP i = 0;i < n->s;i++)
        if (n->a[i] == nd) {
            n->a[i] = 0;
            n->c--;
        }
    free(ni);
    k[0]=0;k[1]=~0ULL;nm->d++;
}

void UniformRandomFuzz(void) {
    s32 count = 0;
    bool graphCreated = false;
    id_array nodeTypes = {3,3,calloc(3,PS)};
    memcpy(nodeTypes.a,(MDB_NODETYPE[]){MDB_FORM,MDB_POCKET,MDB_WORLD},3*PS);
    MDB_generic_map* sm = MDB_CreateGMap(MAX_MAP);
    s32 sNonEmpty = 0;
    s32 sNonEmptyUnrooted = 0;
    s32 nUnreferenced = 0;
    MDB_generic_map* nm = MDB_CreateGMap(MAX_MAP);
    id_array s = {.c=0,.s=MAX_DRAFT,.a=calloc(MAX_DRAFT,PS)};
    id_array n = {.c=0,.s=MAX_NODE,.a=calloc(MAX_NODE,PS)};
    bool exit = false;
    UP steps = 0;
    while (!exit && steps++ < 4) {
        if (!graphCreated) {
            fprintf(stderr, "------------------\n fuzz: %d\n",++count);
            MDB_CreateGraph(),graphCreated = true;continue;}
        cont: switch (rand()%9) {
            case 0: MDB_FreeGraph();
                memset(n.a,0,n.s*PS);n.c=0;
                memset(s.a,0,s.s*PS);s.c=0;
                for (UP i = 0; i < 2*nm->s; i+=2) {
                    if (nm->a[i]) free((void*)nm->a[i+1]);
                }
                for (UP i = 0; i < 2*sm->s; i+=2) {
                    if (sm->a[i]) free((void*)sm->a[i+1]);
                }
                MDB_FreeGMap(sm);
                MDB_FreeGMap(nm);
                sm = MDB_CreateGMap(MAX_MAP);
                nm = MDB_CreateGMap(MAX_MAP);
                sNonEmpty=nUnreferenced=sNonEmptyUnrooted = 0;
                graphCreated = false; exit = true; break;
            case 1: if (s.c < s.s) { // MDB_StartDraft
                MDB_DRAFT sk = *FirstBlank(&s) = MDB_StartDraft();s.c++;
                UP* k = MDB_GLookup(sm, sk);
                k[0] = sk; k[1] = (UP)malloc(sizeof(draft_info));
                ((draft_info*)k[1])->root = 0;
                ((draft_info*)k[1])->size = 0;
                ((draft_info*)k[1])->nonSaturated = 0;
                MDB_GrowGMap(sm, 1);
            } else goto cont; break;
            case 2: if (n.c < n.s && s.c > 0) { // MDB_DraftNode
                MDB_DRAFT sk = PickUniform(&s,0);
                MDB_NODETYPE type = PickUniform(&nodeTypes,0);
                MDB_NODE nd = *FirstBlank(&n) = MDB_DraftNode(sk,type);n.c++;
                UP* k = MDB_GLookup(nm,nd);
                k[0] = nd;k[1] = (UP)malloc(sizeof(node_info));
                *(node_info*)k[1] = (node_info){
                        .draft = sk,
                        .childCount = 0,
                        .type = type,
                        .childBits = 0,
                        .childEnd = 0,
                        .rc = 0,
                };
                MDB_GrowGMap(nm,1);
                nUnreferenced++;
                k = MDB_GLookup(sm,sk);
                k[0]=sk;
                ((draft_info*)k[1])->nonSaturated += (type == MDB_FORM);
                if (((draft_info*)k[1])->size++ == 0) sNonEmpty++,sNonEmptyUnrooted++;
            } else goto cont; break;
            case 3: if (n.c < n.s) { // MDB_CreateConst
                char name[3];
                name[0] = (u8)(rand()%(127-32)+32);
                name[1] = (u8)(rand()%(127-32)+32);
                name[2] = 0;
                MDB_NODE nd = MDB_CreateConst(name);
                *FirstBlank(&n) = nd;
                n.c++;
                UP* k = MDB_GLookup(nm,nd);
                k[0] = nd;k[1] = (UP)malloc(sizeof(node_info));
                *(node_info*)k[1] = (node_info){
                        .draft = 0,
                        .childCount = 0,
                        .type = MDB_CONST,
                        .childBits = 0,
                        .childEnd = 0,
                        .rc = 0,
                };
                MDB_GrowGMap(nm,1);
                nUnreferenced++;
            } else goto cont; break;
            case 4: if (n.c>0&&s.c>0&&sNonEmptyUnrooted) { // MDB_SetDraftRoot
                MDB_DRAFT sk;
                draft_info* si;
                do sk = PickUniform(&s,0),
                    si = (draft_info*)MDB_GLookup(sm,sk)[1];
                while (si->size == 0 || si->root);
                MDB_NODE nd;
                do nd = PickUniform(&n,0);
                while (((node_info*)MDB_GLookup(nm,nd)[1])->draft != sk);
                MDB_SetDraftRoot(sk,nd);
                UP* k = MDB_GLookup(sm,sk);
                assert(k[0]);((draft_info*)k[1])->root = nd;
                sNonEmptyUnrooted--;
            } else goto cont; break;
            case 5: if (n.c>0) { // MDB_AddLink
                MDB_NODE src,dst=PickUniform(&n,0);
                node_info* ni,*dni = (node_info*)MDB_GLookup(nm,dst)[1];
                //check there is a node
                for (UP i = 0; i < n.s; i++) {
                    if (!n.a[i]) continue;
                    ni = (node_info*)MDB_GLookup(nm,n.a[i])[1];
                    if (ni->type != MDB_CONST &&
                        (ni->type == MDB_WORLD || ni->draft) &&
                        (ni->draft == dni->draft || !dni->draft) &&
                        ni->childCount < 32) goto l5;
                }
                goto cont;
            l5: //generate the actual node
                do {
                    src = PickUniform(&n,0);
                    ni = (node_info*)MDB_GLookup(nm,src)[1];
                } while (
                    ni->type == MDB_CONST ||
                    (ni->type != MDB_WORLD && !ni->draft) ||
                    (ni->draft != dni->draft && dni->draft) ||
                    ni->childCount >= 32
                );
                draft_info* si = (draft_info*)MDB_GLookup(sm, ni->draft)[1];
                if (ni->type != MDB_FORM) {
                    MDB_AddLink(src,MDB_ELEM,dst);
                    ni->childBits |= (1U << ni->childEnd++);
                    ni->childCount++;
                } else if (ni->childCount == 0) {
                    UP i = (UP)rand()%(rand()%3 ? 1 : rand()%3 ? 2 :
                        rand()%3 ? 4 : rand()%3 ? 8 : rand()%3 ? 16 : 32);
                    MDB_AddLink(src, i?MDB_ARG+i-1:MDB_APPLY, dst);
                    ni->childCount++;
                    ni->childEnd = i+1;
                    ni->childBits |= (1U<<i);
                    if (ni->draft && i == 0) si->nonSaturated--;
                } else {
                    UP i;
                    // a little past the end
                    do i = (UP)rand()%MDB_Min(ni->childEnd+3,31);
                    while ((1U << i) & ni->childBits);
                    MDB_AddLink(src, i?MDB_ARG+i-1:MDB_APPLY, dst);
                    ni->childCount++;
                    if (i >= ni->childEnd) {
                        ni->childEnd = i+1;
                        si->nonSaturated++;
                    } else if (i+1 < ni->childEnd && ni->childEnd == ni->childCount)
                        si->nonSaturated--;
                    ni->childBits |= (1U<<i);
                }
                if (dst != src && ((node_info*)MDB_GLookup(nm,dst)[1])->rc++==0) nUnreferenced--;
            } else goto cont; break;
            case 6: if (sNonEmpty > 0 && nUnreferenced > 0) { // MDB_DiscardDraftNode
                node_info* ni;
                MDB_NODE nd;
                UP* k;
                //check there is a node
                for (UP i = 0; i < n.s; i++) {
                    if (!n.a[i]) continue;
                    ni = (node_info*)MDB_GLookup(nm,n.a[i])[1];
                    if (ni->rc == 0 && ni->draft) goto l6;
                }
                goto cont;
            l6: //generate the actual node
                do {
                    nd = PickUniform(&n,0);
                    k = MDB_GLookup(nm,nd);
                    ni = (node_info*)k[1];
                } while (!ni->draft || ni->rc != 0);
                PrepareNodeRemove(nd, sm, nm, &sNonEmpty, &n, &nUnreferenced,&sNonEmptyUnrooted);
                MDB_DiscardDraftNode(nd);
            } else goto cont; break;
            case 7: if (s.c > 0) { // MDB_DiscardDraft
                MDB_DRAFT sk;
                sk = PickUniform(&s, 0);
                UP* k = MDB_GLookup(sm, sk);
                draft_info* si = (draft_info*)k[1];
                for (UP i = 0; i < n.s; i++) {
                    MDB_NODE nd;
                    node_info* ni;
                    nd = n.a[i];
                    if (!nd) continue;
                    ni = (node_info*)MDB_GLookup(nm, nd)[1];
                    if (ni->draft == sk)
                        PrepareNodeRemove(nd, sm, nm, &sNonEmpty, &n, &nUnreferenced,&sNonEmptyUnrooted);
                }
                assert(si->size == 0);
                assert(si->root == 0);
                free(si);
                k[0]=0;k[1] = ~0ULL;sm->d++;
                s.c--;

                for (UP i = 0; i < s.s; i++)
                    if (s.a[i] == sk) s.a[i] = 0;
                MDB_DiscardDraft(sk);
            } else goto cont; break;
            case 8: if (sNonEmpty > sNonEmptyUnrooted) { // MDB_CommitDraft
                MDB_DRAFT sk; node_info* ni;
                draft_info* si;
                for (UP i = 0; i < s.s; i++) {
                    if (!s.a[i]) continue;
                    si = (draft_info*)MDB_GLookup(sm,s.a[i])[1];
                    if (si->root && si->nonSaturated == 0) goto l8;
                }
                goto cont;
            l8:
                do sk = PickUniform(&s, 0);
                while (!((draft_info*)MDB_GLookup(sm, sk)[1])->root ||
                    ((draft_info*)MDB_GLookup(sm, sk)[1])->nonSaturated != 0);
                UP* k = MDB_GLookup(sm, sk);
                for (UP i = 0; i < n.s; i++) {
                    if (n.a[i]) {
                        ni = (node_info*)MDB_GLookup(nm, n.a[i])[1];
                        if (ni->draft == sk) ni->draft = 0;
                    }
                }
                s.c--;
                for (UP i = 0; i < s.s; i++)
                    if (s.a[i] == sk) s.a[i] = 0;
                free((void*)k[1]);
                k[0] = 0; k[1] = ~0ULL;sm->d++;
                MDB_CommitDraft(sk);
                sNonEmpty--;
            } else goto cont; break;
            default: assert(0);
        }
    }
    free(nodeTypes.a);
    free(s.a); free(n.a);
    if (graphCreated) MDB_FreeGraph();
    for (UP i = 0; i < nm->s*2; i+=2) {
        if (nm->a[i]) free((void*)nm->a[i+1]);
    }
    for (UP i = 0; i < sm->s*2; i+=2) {
        if (sm->a[i]) free((void*)sm->a[i+1]);
    }
    MDB_FreeGMap(sm);
    MDB_FreeGMap(nm);
}
#if 0
int main(void) {
    //MDB_InitDebug();
    u32 seed = 477526;//(int)time(NULL);
    srand(seed);
    seed = (u32)rand() ^ ((u32)rand() << 16U);
    seed =  182574442;
    for (int i = 0 ;i<100;i++) {
        srand((int)seed);
        //s32 allocations = MDB_GetAllocatedBlockCount();
        fprintf(stderr, "testing with seed %d.\n", seed);
        UniformRandomFuzz();
        //assert(allocations == MDB_GetAllocatedBlockCount() );
        fprintf(stderr,"done\n");
        seed++;
    }
}
#endif
