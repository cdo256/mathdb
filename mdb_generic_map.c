#include <stdlib.h>
#include <string.h>
#include "mdb_global.h"
#include "mdb_all_generic_map.h" //IMPLEMENTS
#include "mdb_alloc_mem.h"

#define calloc MDB_CAlloc
#define malloc MDB_Alloc
#define free MDB_Free
#define realloc MDB_Realloc

MDB_generic_map* MDB_stdcall MDB_CreateGMap(uintptr_t size) {
    MDB_generic_map* m = malloc(sizeof(MDB_generic_map));
    if (!m) return 0;
    m->c = 0; m->s = size;m->d=0;
    m->a = calloc(m->s*2,PS);
    if (!m->a) {free(m);return 0;}
    return m;
}
void MDB_stdcall MDB_FreeGMap(MDB_generic_map* map) {
    free(map->a);
    free(map);
}

// the map is stored as an array,
// all even index are the lhs
// all odd cells are the rhs
// the pattern [0,0] means that a cell hasn't been inited
// the pattern [0,~0] means that a cell has been delete
uintptr_t* MDB_stdcall MDB_GLookup(MDB_generic_map* map, uintptr_t n) {
    uintptr_t h = 0;
    MDB_generic_map* m = map;
    for (int i =0;i<PS;i++)
        h=h*65599U+((n>>(i*8U))&255U);
    h%=m->s;
    uintptr_t* p = &m->a[h*2];
    uintptr_t* d = 0;
    while ((p[0]||p[1])&&p[0]!=n) {
        if (!p[0]&&p[1]) d= p;
        p+=2;
        if (p >= m->a+m->s*2) p=m->a;
    }
    return p[0]==n?p:(d?d:p);
}

// returns 1 if the map grew successfully, 2 if no map grow was required and 0 on failure to grow
// map is left in-tact on failure.
int32_t MDB_stdcall MDB_GrowGMap(MDB_generic_map* m, uintptr_t c) {
    m->c+=c;
    if (m->c*3 >= 2*m->s) {
        MDB_generic_map nm = *m;
        nm.s*=1+((m->c-m->d)*2>m->s);//no need to double if we're full of deleted
        nm.a = malloc(2*nm.s*PS);
        if (!nm.a) return 0;
        memset(nm.a,0,2*nm.s*PS);
        for (uintptr_t i = 0; i < m->s*2; i+=2) {
            if (m->a[i]) memmove(MDB_GLookup(&nm, m->a[i]), &m->a[i], 2*PS);
        }
        free(m->a);
        nm.c -= nm.d;
        nm.d = 0;
        *m = nm;
        return 1;
    }
    return 2;
}
