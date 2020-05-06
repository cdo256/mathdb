#pragma once
#include "mdb_base.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4820) // Ignore padding added to the struct
#endif
typedef struct MDB_error {
    unsigned id;
    char const* str;
} MDB_error;
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#define MDB_EMEMBIT             0x0001U
#define MDB_EINVCALLBIT         0x0003U
#define MDB_EINVARGBIT          0x0004U
#define MDB_EINCOMPLETEDRAFTBIT 0x0005U
#define MDB_EINTOVERFLOWBIT     0x0006U
#define MDB_EUNINITBIT          0x0007U

#define MDB_ENONE               0x00000000U
#define MDB_EMEM                0x00000002U
#define MDB_EINVCALL            0x00000008U
#define MDB_EINVARG             0x00000010U
#define MDB_EINCOMPLETEDRAFT    0x00000020U
#define MDB_EINTOVERFLOW        0x00000040U
#define MDB_EUNINIT             0x00000080U
