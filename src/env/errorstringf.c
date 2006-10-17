/* error_string.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#ifdef _CRAY
#include "fortran.h"
#endif


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_ERROR_STRING = PMPI_ERROR_STRING
EXPORT_MPI_API void MPI_ERROR_STRING ( MPI_Fint *, char *, MPI_Fint *, MPI_Fint *, MPI_Fint );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_error_string__ = pmpi_error_string__
EXPORT_MPI_API void mpi_error_string__ ( MPI_Fint *, char *, MPI_Fint *, MPI_Fint *, MPI_Fint );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_error_string = pmpi_error_string
EXPORT_MPI_API void mpi_error_string ( MPI_Fint *, char *, MPI_Fint *, MPI_Fint *, MPI_Fint );
#else
#pragma weak mpi_error_string_ = pmpi_error_string_
EXPORT_MPI_API void mpi_error_string_ ( MPI_Fint *, char *, MPI_Fint *, MPI_Fint *, MPI_Fint );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_ERROR_STRING ( MPI_Fint *, char *, MPI_Fint *,
	MPI_Fint *, MPI_Fint ) __attribute__ ((weak, alias ("PMPI_ERROR_STRING")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_error_string__ ( MPI_Fint *, char *, MPI_Fint *,
	MPI_Fint *, MPI_Fint ) __attribute__ ((weak, alias ("pmpi_error_string__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_error_string ( MPI_Fint *, char *, MPI_Fint *,
	MPI_Fint *, MPI_Fint ) __attribute__ ((weak, alias ("pmpi_error_string")));
#else
EXPORT_MPI_API void mpi_error_string_ ( MPI_Fint *, char *, MPI_Fint *,
	MPI_Fint *, MPI_Fint ) __attribute__ ((weak, alias ("pmpi_error_string_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_ERROR_STRING  MPI_ERROR_STRING
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_error_string__  mpi_error_string__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_error_string  mpi_error_string
#else
#pragma _HP_SECONDARY_DEF pmpi_error_string_  mpi_error_string_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_ERROR_STRING as PMPI_ERROR_STRING
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_error_string__ as pmpi_error_string__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_error_string as pmpi_error_string
#else
#pragma _CRI duplicate mpi_error_string_ as pmpi_error_string_
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
#define mpi_error_string_ PMPI_ERROR_STRING
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_error_string_ pmpi_error_string__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_error_string_ pmpi_error_string
#else
#define mpi_error_string_ pmpi_error_string_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_error_string_ MPI_ERROR_STRING
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_error_string_ mpi_error_string__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_error_string_ mpi_error_string
#endif
#endif


#define LOCAL_MIN(a,b) ((a) < (b) ? (a) : (b))

#ifdef _CRAY
void mpi_error_string_( errorcode, string_fcd, resultlen, __ierr )
int*errorcode, *resultlen;
_fcd string_fcd;
int *__ierr;
{
  char cres[MPI_MAX_ERROR_STRING];
  *__ierr = MPI_Error_string(*errorcode,cres,resultlen);
 
  /* Assign the result to the Fortran string doing blank padding as required */
  MPIR_cstr2fstr(_fcdtocp(string_fcd), _fcdlen(string_fcd), cres);
  
  *resultlen = LOCAL_MIN(_fcdlen(string_fcd), *resultlen);
}
#else

#ifdef VISUAL_FORTRAN
EXPORT_MPI_API void FORTRAN_API mpi_error_string_( MPI_Fint *errorcode, char *string, MPI_Fint d, MPI_Fint *resultlen, MPI_Fint *__ierr )
#else
/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_error_string_ ANSI_ARGS(( MPI_Fint *, char *, MPI_Fint *, 
                                   MPI_Fint *, MPI_Fint ));

EXPORT_MPI_API void FORTRAN_API mpi_error_string_( MPI_Fint *errorcode, char *string, MPI_Fint *resultlen, MPI_Fint *__ierr, MPI_Fint d )
#endif
{
  char cres[MPI_MAX_ERROR_STRING];
  int l_resultlen;

  *__ierr = MPI_Error_string((int)*errorcode, cres, &l_resultlen);

  /* Assign the result to the Fortran string doing blank padding as required */
  MPIR_cstr2fstr(string, (int)d, cres);
  *resultlen = LOCAL_MIN((MPI_Fint)l_resultlen, (int)d);
}
#endif
