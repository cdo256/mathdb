#include "mdb_global.h"
#include "mdb_all_multiset.h" //IMPLEMENTS
#include <assert.h>
#include <stdlib.h>

#define check_inv(ms) assert((ms)->s <= (ms)->c)

// return index of new x or ~0U on fail
UP MDB_MSetAdd(MDB_mset* s, UP x) {
    check_inv(s);
    if (s->s == s->c) {
        UP c = s->c ? s->c*2 : 1;
        if (c <= s->c) return ~0ULL;//int of
        UP* a = realloc(s->a, c*PS);
        if (!a) return ~0ULL;
        s->a = a; s->c = c;
    }
    s->a[s->s++]=x;
    return s->s-1;
}
// remove last occurance only, returns index x used to occupy or ~0U if not found
UP MDB_MSetRemove(MDB_mset* s, UP x) {
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
UP MDB_MSetContains(MDB_mset* s, UP x) {
    check_inv(s);
    for (UP i = s->s-1; ~i; i--)
        if (s->a[i]==x) return i;
    return ~0ULL;
}
void MDB_FreeMSet(MDB_mset* s) {
    check_inv(s);
    free(s->a); s->a=0;s->c=s->s=0;
}
