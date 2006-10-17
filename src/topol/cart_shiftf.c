/* cart_shift.c */
/* Custom Fortran interface file */
/*
 *  
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_CART_SHIFT = PMPI_CART_SHIFT
EXPORT_MPI_API void MPI_CART_SHIFT ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_cart_shift__ = pmpi_cart_shift__
EXPORT_MPI_API void mpi_cart_shift__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_cart_shift = pmpi_cart_shift
EXPORT_MPI_API void mpi_cart_shift ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_cart_shift_ = pmpi_cart_shift_
EXPORT_MPI_API void mpi_cart_shift_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_CART_SHIFT ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_CART_SHIFT")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_cart_shift__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_cart_shift__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_cart_shift ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_cart_shift")));
#else
EXPORT_MPI_API void mpi_cart_shift_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_cart_shift_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_CART_SHIFT  MPI_CART_SHIFT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_cart_shift__  mpi_cart_shift__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_cart_shift  mpi_cart_shift
#else
#pragma _HP_SECONDARY_DEF pmpi_cart_shift_  mpi_cart_shift_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_CART_SHIFT as PMPI_CART_SHIFT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_cart_shift__ as pmpi_cart_shift__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_cart_shift as pmpi_cart_shift
#else
#pragma _CRI duplicate mpi_cart_shift_ as pmpi_cart_shift_
#endif

/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#include "mpiprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif

#ifdef FORTRANCAPS
#define mpi_cart_shift_ PMPI_CART_SHIFT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_shift_ pmpi_cart_shift__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_shift_ pmpi_cart_shift
#else
#define mpi_cart_shift_ pmpi_cart_shift_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_cart_shift_ MPI_CART_SHIFT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_shift_ mpi_cart_shift__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_shift_ mpi_cart_shift
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_cart_shift_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                 MPI_Fint *, MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_cart_shift_( MPI_Fint *comm, MPI_Fint *direction, MPI_Fint *shift, MPI_Fint *source, MPI_Fint *dest, MPI_Fint *ierr )
{
    int lsource;
    int ldest;

    *ierr =     MPI_Cart_shift( MPI_Comm_f2c(*comm), (int)*direction, 
                                (int)*shift, &lsource, &ldest );
    *source = (MPI_Fint)lsource;
    *dest = (MPI_Fint)ldest;
}
