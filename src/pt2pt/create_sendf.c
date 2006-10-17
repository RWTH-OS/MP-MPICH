/* create_send.c */
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
#pragma weak MPI_SEND_INIT = PMPI_SEND_INIT
EXPORT_MPI_API void MPI_SEND_INIT ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_send_init__ = pmpi_send_init__
EXPORT_MPI_API void mpi_send_init__ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_send_init = pmpi_send_init
EXPORT_MPI_API void mpi_send_init ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_send_init_ = pmpi_send_init_
EXPORT_MPI_API void mpi_send_init_ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_SEND_INIT ( void *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_SEND_INIT")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_send_init__ ( void *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_send_init__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_send_init ( void *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_send_init")));
#else
EXPORT_MPI_API void mpi_send_init_ ( void *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_send_init_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_SEND_INIT  MPI_SEND_INIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_send_init__  mpi_send_init__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_send_init  mpi_send_init
#else
#pragma _HP_SECONDARY_DEF pmpi_send_init_  mpi_send_init_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_SEND_INIT as PMPI_SEND_INIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_send_init__ as pmpi_send_init__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_send_init as pmpi_send_init
#else
#pragma _CRI duplicate mpi_send_init_ as pmpi_send_init_
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
#define mpi_send_init_ PMPI_SEND_INIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_send_init_ pmpi_send_init__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_send_init_ pmpi_send_init
#else
#define mpi_send_init_ pmpi_send_init_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_send_init_ MPI_SEND_INIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_send_init_ mpi_send_init__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_send_init_ mpi_send_init
#endif
#endif


#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 8

void mpi_send_init_( void *unknown, ...)
{
void          *buf;
int*count;
MPI_Datatype  *datatype;
int*dest;
int*tag;
MPI_Comm      *comm;
MPI_Request   *request;
int *__ierr;
MPI_Request lrequest;
int buflen;
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
request =       va_arg(ap, MPI_Request *);
__ierr =        va_arg(ap, int *);

*__ierr = MPI_Send_init(MPIR_F_PTR(buf),*count,
	*datatype,*dest,*tag,*comm,&lrequest);
*(int*)request = MPIR_FromPointer( lrequest );
}

#else

void mpi_send_init_( buf, count, datatype, dest, tag, comm, request, __ierr )
void          *buf;
int*count;
MPI_Datatype  *datatype;
int*dest;
int*tag;
MPI_Comm      *comm;
MPI_Request   *request;
int *__ierr;
{
MPI_Request lrequest;
_fcd	temp;
if (_isfcd(buf)) {
	temp = _fcdtocp(buf);
	buf = (void *) temp;
}

*__ierr = MPI_Send_init(MPIR_F_PTR(buf),*count,	*datatype,*dest,*tag,
			*comm,&lrequest);
*(int*)request = MPIR_FromPointer( lrequest );
}

#endif
#else
/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_send_init_ ANSI_ARGS(( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_send_init_( void *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *__ierr )
{
    MPI_Request lrequest;
    *__ierr = MPI_Send_init(MPIR_F_PTR(buf),(int)*count, 
                            MPI_Type_f2c(*datatype),(int)*dest,(int)*tag,
                            MPI_Comm_f2c(*comm),&lrequest);
    *request = MPI_Request_c2f( lrequest );
}
#endif



