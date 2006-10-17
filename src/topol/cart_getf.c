/* cart_get.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpifort.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_CART_GET = PMPI_CART_GET
EXPORT_MPI_API void MPI_CART_GET ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_cart_get__ = pmpi_cart_get__
EXPORT_MPI_API void mpi_cart_get__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_cart_get = pmpi_cart_get
EXPORT_MPI_API void mpi_cart_get ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_cart_get_ = pmpi_cart_get_
EXPORT_MPI_API void mpi_cart_get_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_CART_GET ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_CART_GET")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_cart_get__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_cart_get__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_cart_get ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_cart_get")));
#else
EXPORT_MPI_API void mpi_cart_get_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_cart_get_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_CART_GET  MPI_CART_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_cart_get__  mpi_cart_get__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_cart_get  mpi_cart_get
#else
#pragma _HP_SECONDARY_DEF pmpi_cart_get_  mpi_cart_get_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_CART_GET as PMPI_CART_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_cart_get__ as pmpi_cart_get__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_cart_get as pmpi_cart_get
#else
#pragma _CRI duplicate mpi_cart_get_ as pmpi_cart_get_
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
#define mpi_cart_get_ PMPI_CART_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_get_ pmpi_cart_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_get_ pmpi_cart_get
#else
#define mpi_cart_get_ pmpi_cart_get_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_cart_get_ MPI_CART_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_get_ mpi_cart_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_get_ mpi_cart_get
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_cart_get_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                               MPI_Fint *, MPI_Fint *, MPI_Fint * );

EXPORT_MPI_API void FORTRAN_API mpi_cart_get_ ( MPI_Fint *comm, MPI_Fint *maxdims, MPI_Fint *dims, MPI_Fint *periods, MPI_Fint *coords, MPI_Fint *__ierr )
{
    struct MPIR_COMMUNICATOR *comm_ptr;
    int lperiods[20], i;
    int ldims[20];
    int lcoords[20];
    static char myname[] = "MPI_CART_GET";

    if ((int)*maxdims > 20) {
	comm_ptr = MPIR_GET_COMM_PTR(MPI_Comm_f2c(*comm));
	*__ierr = MPIR_Err_setmsg( MPI_ERR_DIMS, MPIR_ERR_DIMS_TOOLARGE,
				   myname, (char *)0, (char *)0, 
				   (int)*maxdims, 20 );
	*__ierr = MPIR_ERROR( comm_ptr, *__ierr, myname );
	return;
	}
    *__ierr = MPI_Cart_get( MPI_Comm_f2c(*comm), (int)*maxdims, ldims,
                            lperiods, lcoords);
    for (i=0; i<(int)*maxdims; i++) {
	   dims[i] = (MPI_Fint)ldims[i];
	   periods[i] = MPIR_TO_FLOG(lperiods[i]);
	   coords[i] = (MPI_Fint)lcoords[i];
    }
}



