#ifndef __ERRHANDLER_H__
#define __ERRHANDLER_H__

#include "mpi.h"

typedef enum {
    MPIR_PREDEFINED_HANDLER,
    MPIR_COMM_HANDLER,
    MPIR_FILE_HANDLER,
    MPIR_WIN_HANDLER
} MPIR_ERRHANDLER_TYPE;

#ifndef MPIR_COOKIE
#include "cookie.h"
#endif

struct MPIR_Errhandler {
    MPIR_COOKIE       /* Cookie to help detect valid items */
    MPIR_ERRHANDLER_TYPE type;
#ifdef _WIN32
    void	*fortran_routine;
#endif
    MPI_Handler_function *routine;
    int                  ref_count;
    };
#define MPIR_ERRHANDLER_COOKIE 0xe443a2dd

#endif
