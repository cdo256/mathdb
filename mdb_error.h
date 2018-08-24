#include "mdb_base.h"

typedef struct MDB_error {
    unsigned id;
    char const* str;
} MDB_error;

#define MDB_EMEMBIT 0x0001U
#define MDB_EUNINITBIT 0x0002U
#define MDB_EINVCALLBIT 0x0003U

#define MDB_ENONE 0x0000U
#define MDB_EMEM 0x0002U
#define MDB_EUNINIT 0x0004U
#define MDB_EINVCALL 0x0008U
