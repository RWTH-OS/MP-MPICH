/* recv.c */
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
#pragma weak MPI_RECV = PMPI_RECV
EXPORT_MPI_API void MPI_RECV ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_recv__ = pmpi_recv__
EXPORT_MPI_API void mpi_recv__ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_recv = pmpi_recv
EXPORT_MPI_API void mpi_recv ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_recv_ = pmpi_recv_
EXPORT_MPI_API void mpi_recv_ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_RECV ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_RECV")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_recv__ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_recv__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_recv ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_recv")));
#else
EXPORT_MPI_API void mpi_recv_ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_recv_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_RECV  MPI_RECV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_recv__  mpi_recv__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_recv  mpi_recv
#else
#pragma _HP_SECONDARY_DEF pmpi_recv_  mpi_recv_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_RECV as PMPI_RECV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_recv__ as pmpi_recv__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_recv as pmpi_recv
#else
#pragma _CRI duplicate mpi_recv_ as pmpi_recv_
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
#define mpi_recv_ PMPI_RECV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_recv_ pmpi_recv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_recv_ pmpi_recv
#else
#define mpi_recv_ pmpi_recv_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_recv_ MPI_RECV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_recv_ mpi_recv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_recv_ mpi_recv
#endif
#endif


#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 8

 void mpi_recv_( void *unknown, ...)
{
void             *buf;
int*count,*source,*tag;
MPI_Datatype     *datatype;
MPI_Comm         *comm;
MPI_Status       *status;
int *__ierr;
int		buflen;
va_list ap;

va_start(ap, unknown);
buf = unknown;
if (_numargs() == NUMPARAMS+1) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
count =         va_arg (ap, int *);
datatype =      va_arg(ap, MPI_Datatype*);
source =          va_arg(ap, int *);
tag =           va_arg(ap, int *);
comm =          va_arg(ap, MPI_Comm *);
status =        va_arg(ap, MPI_Status *);
__ierr =        va_arg(ap, int *);

*__ierr = MPI_Recv(MPIR_F_PTR(buf),*count,*datatype,*source,*tag,*comm,
		   status);
}

#else

 void mpi_recv_( buf, count, datatype, source, tag, comm, status, __ierr )
void             *buf;
int*count,*source,*tag;
MPI_Datatype     *datatype;
MPI_Comm         *comm;
MPI_Status       *status;
int *__ierr;
{
_fcd temp;
if (_isfcd(buf)) {
	temp = _fcdtocp(buf);
	buf = (void *)temp;
}
*__ierr = MPI_Recv(MPIR_F_PTR(buf),*count,*datatype,*source,*tag,*comm,
		   status);
}

#endif
#else
/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_recv_ ANSI_ARGS(( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                           MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                           MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_recv_( void *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *__ierr )
{
    MPI_Status c_status;

    *__ierr = MPI_Recv(MPIR_F_PTR(buf), (int)*count,MPI_Type_f2c(*datatype),
                       (int)*source, (int)*tag,
		       MPI_Comm_f2c(*comm), &c_status);
    MPI_Status_c2f(&c_status, status);
}
#endif
