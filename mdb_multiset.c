#include <stdlib.h>
#include "mdb_global.h"
#include "mdb_all_multiset.h" //IMPLEMENTS
#define check_inv(ms) assert((ms)->s <= (ms)->c)

// return index of new x or ~0U on fail
UP MDB_MSetAdd(MDB_mset* s, VAL x) {
    check_inv(s);
    return MDB_VectorPush((MDB_vector*)s, x);
}
// remove last occurrence only, returns index x used to occupy or ~0U if not found
UP MDB_MSetRemove(MDB_mset* s, VAL x) {
    check_inv(s);
    s->s--;
    for (UP i = s->s; ~i; i--) {
        if (s->a[i]==x) {
            s->a[i] = s->a[s->s];
            return i;
        }
    }
    s->s++;
    return ~0ULL;
}
// returns last index of x or ~0U
UP MDB_MSetContains(MDB_mset* s, VAL x) {
    check_inv(s);
    for (UP i = s->s-1; ~i; i--)
        if (s->a[i]==x) return i;
    return ~0ULL;
}
void MDB_FreeMSet(MDB_mset* s) {
    check_inv(s);
    free(s->a); s->a=0;s->c=s->s=0;
}
