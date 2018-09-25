#include "mdb_global.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "mdb_manage.h"
#include "mdb_edit_graph.h"
#include "mdb_all_generic_map.h"
#include "mdb_read_graph.h"

#define MAX_SKETCH 50
#define MAX_NODE 300
#define MAX_MAP 50
//#define PS (sizeof(void*))

const char* longAssString =
	"###############################################################################################################################################################################################################################################################";

typedef struct id_array {
	UP c, s;
	UP* a;
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
	fail();
	return 0;
}

typedef struct node_info {
	UP childCount;
	UP childEnd;
	MDB_NODETYPE type;
	MDB_SKETCH sketch;
	u32 childBits;
	UP rc;
} node_info;

typedef struct sketch_info {
	MDB_NODE root;
	UP size;
} sketch_info;

void PrepareNodeRemove(MDB_NODE nd, MDB_generic_map* sm, MDB_generic_map* nm, s32* sNonEmpty, id_array* n, s32* nUnreferenced) {
	UP* k = MDB_GLookup(nm,nd);
	node_info* ni = (node_info*)k[1];
	sketch_info* si = (sketch_info*)MDB_GLookup(sm,ni->sketch)[1];
	if (si->size--==1)sNonEmpty--;
	MDB_NODETYPE type; UP childCount; char* str;
	MDB_GetNodeInfo(nd, &type, &childCount, &str);
	MDB_NODE* children = malloc(PS*childCount);
	assert(childCount == MDB_GetChildren(nd, 0, childCount, children));
	for (UP i = 0; i < childCount; i++) nUnreferenced-=(((node_info*)MDB_GLookup(nm,children[i]))->rc--==1);
	free(children);
	for (UP i = 0;i < n->s;i++) if (n->a[i] == nd) n->a[i] = 0;
	free(ni);
	k[0]=0;k[1]=~0ULL;nm->d++;
}

void UniformRandomFuzz(void) {
	bool graphCreated = false;
	id_array nodeTypes = {4,4,calloc(4,PS)};
	memcpy(nodeTypes.a,(MDB_NODETYPE[]){MDB_CONST,MDB_FORM,MDB_POCKET,MDB_WORLD},4*PS);
	MDB_generic_map* sm = MDB_CreateGMap(64);
	int sNonEmpty = 0;
	int nUnreferenced = 0;
	MDB_generic_map* nm = MDB_CreateGMap(64);
	id_array s = {.c=0,.s=MAX_SKETCH,.a=calloc(MAX_SKETCH,PS)};
	id_array n = {.c=0,.s=MAX_NODE,.a=calloc(MAX_NODE,PS)};
	while (rand()%50) {
		if (!graphCreated) {MDB_CreateGraph(),graphCreated = true;continue;}
		cont: switch (rand()%9) {
			case 0: MDB_FreeGraph();
				memset(n.a,0,n.s*PS);n.c=0;
				memset(s.a,0,s.s*PS);s.c=0;
				for (UP i = 0; i < nm->s; i+=2) {
					if (nm->a[i]) free((void*)nm->a[i+1]);
				}
				for (UP i = 0; i < sm->s; i+=2) {
					if (sm->a[i]) free((void*)sm->a[i+1]);
				}
				sNonEmpty=nUnreferenced = 0;
				graphCreated = false; break;
			case 1: if (s.c < s.s) {
				MDB_SKETCH sk = *FirstBlank(&s) = MDB_StartSketch();s.c++;
				UP* k = MDB_GLookup(sm, sk);
				k[0] = sk; k[1] = (UP)malloc(sizeof(sketch_info));
				((sketch_info*)k[1])->root = 0;
				((sketch_info*)k[1])->size = 0;
				MDB_GrowGMap(nm, 1);
			} else goto cont; break;
			case 2: if (n.c < n.s && s.c > 0) {
				MDB_NODE sk = PickUniform(&s,0);
				MDB_NODETYPE type = PickUniform(&nodeTypes,0);
				MDB_NODE nd = *FirstBlank(&n) = MDB_SketchNode(sk,type);n.c++;
				UP* k = MDB_GLookup(nm,nd);
				k[0] = nd;k[1] = (UP)malloc(sizeof(node_info));
				*(node_info*)k[1] = (node_info){
						.sketch = sk,
						.childCount = 0,
						.type = type,
						.childBits = 0,
						.childEnd = 0,
						.rc = 0,
				};
				MDB_GrowGMap(nm,1);
				nUnreferenced++;
				k = MDB_GLookup(sm,sk);
				k[0]=sk;((sketch_info*)k[1])->size++;
				if (k[1] == 1) sNonEmpty++;
			} else goto cont; break;
			case 3: if (n.c < n.s) {
				MDB_NODE nd = *FirstBlank(&n) = MDB_CreateConst(longAssString+rand()%256);n.c++;
				UP* k = MDB_GLookup(nm,nd);
				k[0] = nd;k[1] = (UP)malloc(sizeof(node_info));
				*(node_info*)k[1] = (node_info){
						.sketch = 0,
						.childCount = 0,
						.type = MDB_CONST,
						.childBits = 0,
						.childEnd = 0,
						.rc = 0,
				};
				MDB_GrowGMap(nm,1);
				nUnreferenced++;
			} else goto cont; break;
			case 4: if (n.c>0&&s.c>0&&sNonEmpty) {
				MDB_SKETCH sk;
				do sk = PickUniform(&s,0);
				while (((sketch_info*)MDB_GLookup(sm,sk)[1])->size == 0);
				MDB_NODE nd;
				do nd = PickUniform(&n,0);
				while (((node_info*)MDB_GLookup(nm,nd)[1])->sketch != sk);
				MDB_SetSketchRoot(sk,nd);
				UP* k = MDB_GLookup(sm,sk);
				k[0]=sk;((sketch_info*)k[1])->root = nd;
				MDB_GrowGMap(sm,1);
			} else goto cont; break;
			case 5: if (n.c>0) {
				MDB_NODE src,dst=PickUniform(&n,0);
				node_info* ni;
				//check there is a node
				for (UP i = 0; i < n.s; i++) {
					if (!n.a[i]) continue;
					ni = (node_info*)MDB_GLookup(nm,n.a[i])[1];
					if (ni->type != MDB_CONST &&
						(ni->type == MDB_WORLD || ni->sketch) &&
						ni->childCount < 32) goto l5;
				}
				goto cont;
			l5: //generate the actual node
				do {
					src = PickUniform(&n,0);
					ni = (node_info*)MDB_GLookup(nm,src)[1];
				} while (
					ni->type == MDB_CONST ||
					(ni->type != MDB_WORLD && !ni->sketch) ||
					ni->childCount >= 32
				);
				if (ni->type != MDB_FORM) {
					MDB_AddLink(src,MDB_ELEM,dst);
					ni->childBits |= (1U << ni->childEnd++);
					ni->childCount++;
				} else if (ni->childCount == 0) {
					UP i = rand()%(rand()%3 ? 1 : rand()%3 ? 2 :
						rand()%3 ? 4 : rand()%3 ? 8 : rand()%3 ? 16 : 32);
					MDB_AddLink(src, i?MDB_ARG+i-1:MDB_APPLY, dst);
					ni->childCount++;
					ni->childEnd = i+1;
					ni->childBits |= (1U<<i);
				} else {
					UP i;
					do i = rand()%min(ni->childEnd+1,31);
					while ((1U << i) & ni->childBits);
					MDB_AddLink(src, i?MDB_ARG+i-1:MDB_APPLY, dst);
					ni->childCount++;
					ni->childEnd = max(i,ni->childEnd);
					ni->childBits |= (1U<<i);
				}
				if (((node_info*)MDB_GLookup(nm,dst)[1])->rc++==0) nUnreferenced--;
			} else goto cont; break;
			case 6: if (sNonEmpty > 0 && nUnreferenced > 0) {
				node_info* ni;
				MDB_NODE nd;
				UP* k;
				//check there is a node
				for (UP i = 0; i < n.s; i++) {
					if (!n.a[i]) continue;
					ni = (node_info*)MDB_GLookup(nm,n.a[i]);
					if (ni->rc == 0 && ni->sketch) goto l6;
				}
				goto cont;
			l6: //generate the actual node
				do {
					nd = PickUniform(&s,0);
					k = MDB_GLookup(nm,nd);
					ni = (node_info*)k[1];
				} while (!ni->sketch || ni->rc != 0);
				PrepareNodeRemove(nd, sm, nm, &sNonEmpty, &n, &nUnreferenced);
				MDB_DiscardSketchNode(nd);
			} else goto cont; break;
			case 7: if (s.c > 0) {
				MDB_SKETCH sk;
				sk = PickUniform(&s, 0);
				UP* k = MDB_GLookup(sm, sk);
				sketch_info* si = (sketch_info*)k[1];
				for (UP i = 0; i < n.s; i++) {
					MDB_NODE nd;
					node_info* ni;
					nd = n.a[i];
					if (!nd) continue;
					ni = (node_info*)MDB_GLookup(nm, nd)[1];
					if (ni->sketch == sk) PrepareNodeRemove(nd, sm, nm, &sNonEmpty, &n, &nUnreferenced);
				}
				assert(si->size == 0);
				assert(si->root == 0);
				free(si);
				k[0]=0;k[1] = ~0ULL;
				MDB_DiscardSketch(sk);
			} else goto cont; break;
			case 8: if (s.c > 0) {
				MDB_SKETCH sk;sketch_info* si;
				for (UP i = 0; i < s.s; i++) {
					if (!s.a[i]) continue;
					si = (sketch_info*)MDB_GLookup(sm, s.a[i])[1];
					if (si->root) goto l8;
				}
				goto cont;
l8:
				do sk = PickUniform(&s, 0);
				while (!((sketch_info*)MDB_GLookup(sm, sk)[1])->root);
				UP* k = MDB_GLookup(sm, sk);
				for (UP i = 0; i < n.s; i++) {
					((node_info*)MDB_GLookup(nm, n.a[i])[1])->sketch = 0;
				}free((void*)k[1]);
				k[0] = 0; k[1] = ~0ULL;sm->d++;
				MDB_CommitSketch(sk);
			}
		}
	}
}

#if 1
int main(void) {
	int seed = 0;//(int)time(NULL);
	srand(seed);
	printf("testing with seed %d.\n", seed);
	UniformRandomFuzz();
	printf("done\n");
	while(1);
}
#endif