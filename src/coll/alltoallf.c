/* alltoall.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef _CRAY
#include <fortran.h>
#include <stdarg.h>
#endif


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_ALLTOALL = PMPI_ALLTOALL
EXPORT_MPI_API void MPI_ALLTOALL ( void *, MPI_Fint *, MPI_Fint *, void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_alltoall__ = pmpi_alltoall__
EXPORT_MPI_API void mpi_alltoall__ ( void *, MPI_Fint *, MPI_Fint *, void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_alltoall = pmpi_alltoall
EXPORT_MPI_API void mpi_alltoall ( void *, MPI_Fint *, MPI_Fint *, void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_alltoall_ = pmpi_alltoall_
EXPORT_MPI_API void mpi_alltoall_ ( void *, MPI_Fint *, MPI_Fint *, void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_ALLTOALL ( void *, MPI_Fint *, MPI_Fint *, void *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_ALLTOALL")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_alltoall__ ( void *, MPI_Fint *, MPI_Fint *, void *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_alltoall__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_alltoall ( void *, MPI_Fint *, MPI_Fint *, void *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_alltoall")));
#else
EXPORT_MPI_API void mpi_alltoall_ ( void *, MPI_Fint *, MPI_Fint *, void *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_alltoall_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_ALLTOALL  MPI_ALLTOALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_alltoall__  mpi_alltoall__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_alltoall  mpi_alltoall
#else
#pragma _HP_SECONDARY_DEF pmpi_alltoall_  mpi_alltoall_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_ALLTOALL as PMPI_ALLTOALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_alltoall__ as pmpi_alltoall__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_alltoall as pmpi_alltoall
#else
#pragma _CRI duplicate mpi_alltoall_ as pmpi_alltoall_
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
#define mpi_alltoall_ PMPI_ALLTOALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_alltoall_ pmpi_alltoall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_alltoall_ pmpi_alltoall
#else
#define mpi_alltoall_ pmpi_alltoall_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_alltoall_ MPI_ALLTOALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_alltoall_ mpi_alltoall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_alltoall_ mpi_alltoall
#endif
#endif


#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 8

 void mpi_alltoall_( void *unknown, ...)
{
void             *sendbuf;
int		*sendcount;
MPI_Datatype     * sendtype;
void             *recvbuf;
int		*recvcount;
MPI_Datatype     * recvtype;
MPI_Comm         * comm;
int 		*__ierr;
int             buflen;
va_list         ap;

va_start(ap, unknown);
sendbuf = unknown;
if (_numargs() == NUMPARAMS+1) {
    /* Note that we can't set __ierr because we don't know where it is! */
    (void) MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_ONE_CHAR, "MPI_ALLTOALL" );
}
if (_numargs() == NUMPARAMS+2) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
sendcount =     va_arg (ap, int *);
sendtype =      va_arg(ap, MPI_Datatype *);
recvbuf =       va_arg(ap, void *);
if (_numargs() == NUMPARAMS+2) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
recvcount =     va_arg (ap, int *);
recvtype =      va_arg(ap, MPI_Datatype *);
comm =          va_arg(ap, MPI_Comm *);
__ierr =        va_arg(ap, int *);

*__ierr = MPI_Alltoall(MPIR_F_PTR(sendbuf),*sendcount,*sendtype,
        MPIR_F_PTR(recvbuf),*recvcount,*recvtype,*comm);
}

#else

 void mpi_alltoall_( sendbuf, sendcount, sendtype, 
                  recvbuf, recvcnt, recvtype, comm, __ierr )
void             *sendbuf;
int*sendcount;
MPI_Datatype     *sendtype;
void             *recvbuf;
int*recvcnt;
MPI_Datatype     *recvtype;
MPI_Comm         *comm;
int *__ierr;
{
_fcd            temp;
if (_isfcd(sendbuf)) {
        temp = _fcdtocp(sendbuf);
        sendbuf = (void *)temp;
}
if (_isfcd(recvbuf)) {
        temp = _fcdtocp(recvbuf);
        recvbuf = (void *)temp;
}

*__ierr = MPI_Alltoall(MPIR_F_PTR(sendbuf),*sendcount,*sendtype,
        MPIR_F_PTR(recvbuf),*recvcnt,*recvtype,*comm);
}

#endif
#else
/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_alltoall_ ANSI_ARGS(( void *, MPI_Fint *, MPI_Fint *, void *, 
                               MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                               MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_alltoall_( void *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, 
                  void *recvbuf, MPI_Fint *recvcnt, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *__ierr )
{
    *__ierr = MPI_Alltoall(MPIR_F_PTR(sendbuf), (int)*sendcount,
                           MPI_Type_f2c(*sendtype), MPIR_F_PTR(recvbuf),
                           (int)*recvcnt, MPI_Type_f2c(*recvtype),
                           MPI_Comm_f2c(*comm) );
}
#endif



