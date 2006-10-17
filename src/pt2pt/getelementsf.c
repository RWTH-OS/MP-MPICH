/* get_elements.c */
/* Fortran interface file */
#include "mpiimpl.h"

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_GET_ELEMENTS = PMPI_GET_ELEMENTS
EXPORT_MPI_API void MPI_GET_ELEMENTS ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_get_elements__ = pmpi_get_elements__
EXPORT_MPI_API void mpi_get_elements__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_get_elements = pmpi_get_elements
EXPORT_MPI_API void mpi_get_elements ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_get_elements_ = pmpi_get_elements_
EXPORT_MPI_API void mpi_get_elements_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_GET_ELEMENTS ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_GET_ELEMENTS")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_get_elements__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_get_elements__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_get_elements ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_get_elements")));
#else
EXPORT_MPI_API void mpi_get_elements_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_get_elements_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_GET_ELEMENTS  MPI_GET_ELEMENTS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_get_elements__  mpi_get_elements__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_get_elements  mpi_get_elements
#else
#pragma _HP_SECONDARY_DEF pmpi_get_elements_  mpi_get_elements_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_GET_ELEMENTS as PMPI_GET_ELEMENTS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_get_elements__ as pmpi_get_elements__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_get_elements as pmpi_get_elements
#else
#pragma _CRI duplicate mpi_get_elements_ as pmpi_get_elements_
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
#define mpi_get_elements_ PMPI_GET_ELEMENTS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_get_elements_ pmpi_get_elements__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_get_elements_ pmpi_get_elements
#else
#define mpi_get_elements_ pmpi_get_elements_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_get_elements_ MPI_GET_ELEMENTS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_get_elements_ mpi_get_elements__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_get_elements_ mpi_get_elements
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_get_elements_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                   MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_get_elements_ ( MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *elements, MPI_Fint *__ierr )
{
    int lelements;
    MPI_Status c_status;

    MPI_Status_f2c(status, &c_status);
    *__ierr = MPI_Get_elements(&c_status,MPI_Type_f2c(*datatype),
                               &lelements);
    *elements = (MPI_Fint)lelements;
}
