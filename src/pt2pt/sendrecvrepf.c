/* sendrecv_rep.c */
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
#pragma weak MPI_SENDRECV_REPLACE = PMPI_SENDRECV_REPLACE
EXPORT_MPI_API void MPI_SENDRECV_REPLACE ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_sendrecv_replace__ = pmpi_sendrecv_replace__
EXPORT_MPI_API void mpi_sendrecv_replace__ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_sendrecv_replace = pmpi_sendrecv_replace
EXPORT_MPI_API void mpi_sendrecv_replace ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_sendrecv_replace_ = pmpi_sendrecv_replace_
EXPORT_MPI_API void mpi_sendrecv_replace_ ( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_SENDRECV_REPLACE ( void *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_SENDRECV_REPLACE")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_sendrecv_replace__ ( void *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_sendrecv_replace__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_sendrecv_replace ( void *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_sendrecv_replace")));
#else
EXPORT_MPI_API void mpi_sendrecv_replace_ ( void *, MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_sendrecv_replace_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_SENDRECV_REPLACE  MPI_SENDRECV_REPLACE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_sendrecv_replace__  mpi_sendrecv_replace__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_sendrecv_replace  mpi_sendrecv_replace
#else
#pragma _HP_SECONDARY_DEF pmpi_sendrecv_replace_  mpi_sendrecv_replace_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_SENDRECV_REPLACE as PMPI_SENDRECV_REPLACE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_sendrecv_replace__ as pmpi_sendrecv_replace__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_sendrecv_replace as pmpi_sendrecv_replace
#else
#pragma _CRI duplicate mpi_sendrecv_replace_ as pmpi_sendrecv_replace_
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
#define mpi_sendrecv_replace_ PMPI_SENDRECV_REPLACE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_sendrecv_replace_ pmpi_sendrecv_replace__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_sendrecv_replace_ pmpi_sendrecv_replace
#else
#define mpi_sendrecv_replace_ pmpi_sendrecv_replace_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_sendrecv_replace_ MPI_SENDRECV_REPLACE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_sendrecv_replace_ mpi_sendrecv_replace__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_sendrecv_replace_ mpi_sendrecv_replace
#endif
#endif


#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 10

void mpi_sendrecv_replace_( void *unknown, ...)
{
void         	*buf;
int		*count,*dest,*sendtag,*source,*recvtag;
MPI_Datatype  	*datatype;
MPI_Comm      	*comm;
MPI_Status   	*status;
int 		*__ierr;
va_list		ap;
int		buflen;

va_start(ap, unknown);
buf = unknown;
if (_numargs() == NUMPARAMS+1) {
	buflen = va_arg(ap, int) / 8;
}
count =         va_arg(ap, int *);
datatype =      va_arg(ap, MPI_Datatype*);
dest =          va_arg(ap, int *);
sendtag =       va_arg(ap, int *);
source =        va_arg(ap, int *);
recvtag =       va_arg(ap, int *);
comm =          va_arg(ap, MPI_Comm *);
status =        va_arg(ap, MPI_Status *);
__ierr =        va_arg(ap, int *);

*__ierr = MPI_Sendrecv_replace(MPIR_F_PTR(buf),*count,*datatype,*dest,
			       *sendtag,*source,*recvtag,*comm,status);
}

#else

void mpi_sendrecv_replace_( buf, count, datatype, dest, sendtag, 
     source, recvtag, comm, status, __ierr )
void         *buf;
int*count,*dest,*sendtag,*source,*recvtag;
MPI_Datatype  *datatype;
MPI_Comm     * comm;
MPI_Status   *status;
int *__ierr;
{
_fcd temp;
if (_isfcd(buf)) {
	temp = _fcdtocp(buf);
	buf = (void *) temp;
}
*__ierr = MPI_Sendrecv_replace(MPIR_F_PTR(buf),*count,
	*datatype,*dest,*sendtag,*source,*recvtag,*comm, status );
}

#endif
#else
/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_sendrecv_replace_ ANSI_ARGS(( void *, MPI_Fint *, MPI_Fint *, 
                                       MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                       MPI_Fint *, MPI_Fint *, MPI_Fint *,
                                       MPI_Fint * ));
EXPORT_MPI_API void FORTRAN_API mpi_sendrecv_replace_( void *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *sendtag, 
     MPI_Fint *source, MPI_Fint *recvtag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *__ierr )
{
    MPI_Status c_status;

    *__ierr = MPI_Sendrecv_replace(MPIR_F_PTR(buf), (int)*count,
			     MPI_Type_f2c(*datatype), (int)*dest, 
                             (int)*sendtag, (int)*source, (int)*recvtag,
				   MPI_Comm_f2c(*comm), &c_status );
    MPI_Status_c2f(&c_status, status);
}
#endif

