#include "mdb_global.h"
#include "mdb_all_vector.h" //IMPLEMENTS
#include <stdlib.h>
#include <string.h>

void MDB_stdcall MDB_FreeVector(MDB_vector* v) {
    free(v->a); *v=(MDB_vector){0};
}
// returns v->c or ~0ULL
UP MDB_stdcall MDB_SetVectorSize(MDB_vector* v, UP size, UP fill) {
    UP* a = realloc(v->a, size*PS);
    if (a||size==0) {
        v->a=a;
        if (a && (fill>>8)<<8 == fill>>8)
            memset(a+v->c,fill&255,(size-v->c)*PS);
        else for (UP i = v->c; i < size; i++)
            a[i] = fill;
        v->c=size;
        return size;
    }
    else return ~0ULL;
}
// returns v->c or ~0ULL, geometric growth
UP MDB_stdcall MDB_GrowVector(MDB_vector* v, UP size, UP fill) {
    if (size > v->c) 
        return MDB_SetVectorSize(v, max(v->c*2,size), fill);
    else return v->c;
}
// returns index of x or ~0ULL
UP MDB_stdcall MDB_VectorPush(MDB_vector* v, UP x) {
    if (~MDB_GrowVector(v, v->s+1, 0ULL)) {
        v->a[v->s++] = x;
        return v->s-1;
    } else return ~0ULL;
}
