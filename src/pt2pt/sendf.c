/* send.c */
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
#pragma weak MPI_SEND = PMPI_SEND
EXPORT_MPI_API void MPI_SEND ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint*, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_send__ = pmpi_send__
EXPORT_MPI_API void mpi_send__ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint*, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_send = pmpi_send
EXPORT_MPI_API void mpi_send ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint*, MPI_Fint * );
#else
#pragma weak mpi_send_ = pmpi_send_
EXPORT_MPI_API void mpi_send_ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint*, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_SEND ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint*,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_SEND")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_send__ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint*,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_send__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_send ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint*,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_send")));
#else
EXPORT_MPI_API void mpi_send_ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint*,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_send_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_SEND  MPI_SEND
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_send__  mpi_send__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_send  mpi_send
#else
#pragma _HP_SECONDARY_DEF pmpi_send_  mpi_send_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_SEND as PMPI_SEND
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_send__ as pmpi_send__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_send as pmpi_send
#else
#pragma _CRI duplicate mpi_send_ as pmpi_send_
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
#define mpi_send_ PMPI_SEND
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_send_ pmpi_send__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_send_ pmpi_send
#else
#define mpi_send_ pmpi_send_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_send_ MPI_SEND
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_send_ mpi_send__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_send_ mpi_send
#endif
#endif


#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 7

 void mpi_send_( void *unknown, ...)
{
void             *buf;
int*count,*dest,*tag;
MPI_Datatype     *datatype;
MPI_Comm         *comm;
int *__ierr;
int		buflen;
va_list ap;

va_start(ap, unknown);
buf = unknown;
if (_numargs() == NUMPARAMS+1) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
count =         va_arg (ap, int *);
datatype =      va_arg(ap, MPI_Datatype *);
dest =          va_arg(ap, int *);
tag =           va_arg(ap, int *);
comm =          va_arg(ap, MPI_Comm *);
__ierr =        va_arg(ap, int *);

*__ierr = MPI_Send(MPIR_F_PTR(buf),*count,*datatype,*dest,*tag,*comm);
}

#else

 void mpi_send_( buf, count, datatype, dest, tag, comm, __ierr )
void             *buf;
int*count,*dest,*tag;
MPI_Datatype     *datatype;
MPI_Comm         *comm;
int *__ierr;
{
_fcd temp;
if (_isfcd(buf)) {
        temp = _fcdtocp(buf);
        buf = (void *)temp;
}
*__ierr = MPI_Send(MPIR_F_PTR(buf),*count,*datatype,*dest,*tag,*comm);
}

#endif
#else
/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_send_ ANSI_ARGS(( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                           MPI_Fint *, MPI_Fint*, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_send_( void *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *__ierr )
{
    *__ierr = MPI_Send(MPIR_F_PTR(buf), (int)*count, MPI_Type_f2c(*datatype),
                       (int)*dest, (int)*tag, MPI_Comm_f2c(*comm));
}
#endif
