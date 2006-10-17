/* irecv.c */
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
#pragma weak MPI_IRECV = PMPI_IRECV
EXPORT_MPI_API void MPI_IRECV ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_irecv__ = pmpi_irecv__
EXPORT_MPI_API void mpi_irecv__ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_irecv = pmpi_irecv
EXPORT_MPI_API void mpi_irecv ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_irecv_ = pmpi_irecv_
EXPORT_MPI_API void mpi_irecv_ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_IRECV ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_IRECV")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_irecv__ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_irecv__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_irecv ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_irecv")));
#else
EXPORT_MPI_API void mpi_irecv_ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_irecv_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_IRECV  MPI_IRECV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_irecv__  mpi_irecv__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_irecv  mpi_irecv
#else
#pragma _HP_SECONDARY_DEF pmpi_irecv_  mpi_irecv_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_IRECV as PMPI_IRECV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_irecv__ as pmpi_irecv__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_irecv as pmpi_irecv
#else
#pragma _CRI duplicate mpi_irecv_ as pmpi_irecv_
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
#define mpi_irecv_ PMPI_IRECV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_irecv_ pmpi_irecv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_irecv_ pmpi_irecv
#else
#define mpi_irecv_ pmpi_irecv_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_irecv_ MPI_IRECV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_irecv_ mpi_irecv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_irecv_ mpi_irecv
#endif
#endif


#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 8

 void mpi_irecv_( void *unknown, ...)
{
void             *buf;
int*count;
MPI_Datatype     *datatype;
int*source;
int*tag;
MPI_Comm         *comm;
MPI_Request      *request;
int *__ierr;
MPI_Request lrequest;
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
comm =          va_arg(ap, MPI_Comm*);
request =       va_arg(ap, MPI_Request *);
__ierr =        va_arg(ap, int *);

*__ierr = MPI_Irecv(MPIR_F_PTR(buf),*count,*datatype,*source,*tag,*comm,
		    &lrequest);
*(int*)request = MPIR_FromPointer(lrequest);
}

#else
void mpi_irecv_( buf, count, datatype, source, tag, comm, request, __ierr )
void             *buf;
int*count;
MPI_Datatype    * datatype;
int*source;
int*tag;
MPI_Comm         *comm;
MPI_Request      *request;
int *__ierr;
{
MPI_Request lrequest;
_fcd temp;
if (_isfcd(buf)) {
	temp = _fcdtocp(buf);
	buf = (void *)temp;
}
*__ierr = MPI_Irecv(MPIR_F_PTR(buf),*count,*datatype,*source,*tag,*comm,
	&lrequest);
*(int*)request = MPIR_FromPointer(lrequest);
}

#endif
#else
/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_irecv_ ANSI_ARGS(( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                            MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_irecv_( void *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *__ierr )
{
    MPI_Request lrequest;
    *__ierr = MPI_Irecv(MPIR_F_PTR(buf),(int)*count,MPI_Type_f2c(*datatype),
			(int)*source,(int)*tag,
                        MPI_Comm_f2c(*comm),&lrequest);
    *request = MPI_Request_c2f(lrequest);
}
#endif
