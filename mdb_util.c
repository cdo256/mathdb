#include "mdb_global.h"
#include "mdb_util.h" //IMPLEMENTS

UP MDB_Min(UP a, UP b) {
    return a < b ? a : b;
}
UP MDB_Max(UP a, UP b) {
    return a < b ? b : a;
}
