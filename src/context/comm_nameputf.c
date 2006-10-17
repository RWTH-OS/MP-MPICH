/*
 *  $Id: comm_nameputf.c,v 1.4 2003/06/13 09:39:35 rainer Exp $
 *
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */
/* Update log
 *
 * Nov 29 1996 jcownie@dolphinics.com: Implement MPI-2 Fortran binding for 
 *        communicator naming function.
 */

#include "mpiimpl.h"
#ifdef _CRAY
#include "fortran.h"
#endif



#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_COMM_SET_NAME = PMPI_COMM_SET_NAME
EXPORT_MPI_API void MPI_COMM_SET_NAME ( MPI_Fint *, char *, MPI_Fint *, MPI_Fint );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_comm_set_name__ = pmpi_comm_set_name__
EXPORT_MPI_API void mpi_comm_set_name__ ( MPI_Fint *, char *, MPI_Fint *, MPI_Fint );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_comm_set_name = pmpi_comm_set_name
EXPORT_MPI_API void mpi_comm_set_name ( MPI_Fint *, char *, MPI_Fint *, MPI_Fint );
#else
#pragma weak mpi_comm_set_name_ = pmpi_comm_set_name_
EXPORT_MPI_API void mpi_comm_set_name_ ( MPI_Fint *, char *, MPI_Fint *, MPI_Fint );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_COMM_SET_NAME ( MPI_Fint *, char *, MPI_Fint *,
	MPI_Fint ) __attribute__ ((weak, alias ("PMPI_COMM_SET_NAME")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_comm_set_name__ ( MPI_Fint *, char *, MPI_Fint *,
	MPI_Fint ) __attribute__ ((weak, alias ("pmpi_comm_set_name__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_comm_set_name ( MPI_Fint *, char *, MPI_Fint *,
	MPI_Fint ) __attribute__ ((weak, alias ("pmpi_comm_set_name")));
#else
EXPORT_MPI_API void mpi_comm_set_name_ ( MPI_Fint *, char *, MPI_Fint *,
	MPI_Fint ) __attribute__ ((weak, alias ("pmpi_comm_set_name_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_COMM_SET_NAME  MPI_COMM_SET_NAME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_comm_set_name__  mpi_comm_set_name__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_comm_set_name  mpi_comm_set_name
#else
#pragma _HP_SECONDARY_DEF pmpi_comm_set_name_  mpi_comm_set_name_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_COMM_SET_NAME as PMPI_COMM_SET_NAME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_comm_set_name__ as pmpi_comm_set_name__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_comm_set_name as pmpi_comm_set_name
#else
#pragma _CRI duplicate mpi_comm_set_name_ as pmpi_comm_set_name_
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
#define mpi_comm_set_name_ PMPI_COMM_SET_NAME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_set_name_ pmpi_comm_set_name__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_set_name_ pmpi_comm_set_name
#else
#define mpi_comm_set_name_ pmpi_comm_set_name_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_comm_set_name_ MPI_COMM_SET_NAME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_set_name_ mpi_comm_set_name__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_set_name_ mpi_comm_set_name
#endif
#endif


#ifdef _CRAY

void mpi_comm_set_name_( comm, string_fcd, __ierr )
MPI_Comm *comm;
_fcd string_fcd;
int *__ierr;
{
  char cres[MPI_MAX_NAME_STRING];

  /* Convert to a C string */
  MPIR_fstr2cstr( cres, MPI_MAX_NAME_STRING, _fcdtocp(string_fcd), _fcdlen(string_fcd));

  *__ierr = MPI_Comm_set_name( *comm, cres );
}
#else

#ifdef VISUAL_FORTRAN
EXPORT_MPI_API void FORTRAN_API mpi_comm_set_name_( MPI_Fint *comm, char *string, MPI_Fint d, MPI_Fint *__ierr )
#else
/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_comm_set_name_ ANSI_ARGS(( MPI_Fint *, char *, MPI_Fint *, 
                                    MPI_Fint ));

EXPORT_MPI_API void FORTRAN_API mpi_comm_set_name_( MPI_Fint *comm, char *string, MPI_Fint *__ierr, MPI_Fint d )
#endif
{
  char cres[MPI_MAX_ERROR_STRING];

  /* Assign the result to the Fortran string doing blank padding as required */
  MPIR_fstr2cstr(cres, MPI_MAX_ERROR_STRING, string, (long)d);
  *__ierr = MPI_Comm_set_name(MPI_Comm_f2c(*comm), cres);
}
#endif
