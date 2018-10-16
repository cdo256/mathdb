#include "mdb_global.h"
#include "mdb_all_vector.h" //IMPLEMENTS
#include "mdb_util.h"
#include <stdlib.h>
#include <string.h>

void MDB_stdcall MDB_FreeVector(MDB_vector* v) {
    free(v->a); *v=(MDB_vector){0};
}
// returns v->c or ~0ULL
UP MDB_stdcall MDB_SetVectorSize(MDB_vector* v, UP size, UP fill) {
    if (size == 0) {
        MDB_FreeVector(v);
        return 0;
    }
    UP* a = realloc(v->a, size*PS);
    if (a) {
        v->a=a;
        if ((fill>>8U)<<8U == fill>>8U)
            memset(a+v->c,(int)(fill&255U),(size-v->c)*PS);
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
        return MDB_SetVectorSize(v, MDB_Max(v->c*2,size), fill);
    else return v->c;
}
// returns index of x or ~0ULL
UP MDB_stdcall MDB_VectorPush(MDB_vector* v, UP x) {
    if (~MDB_GrowVector(v, v->s+1, 0ULL)) {
        v->a[v->s++] = x;
        return v->s-1;
    } else return ~0ULL;
}
