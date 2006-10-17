/* gatherv.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#include "mpimem.h"

#ifdef _CRAY
#include <fortran.h>
#include <stdarg.h>
#endif


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_GATHERV = PMPI_GATHERV
EXPORT_MPI_API void MPI_GATHERV ( void *, MPI_Fint *, MPI_Fint *, void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_gatherv__ = pmpi_gatherv__
EXPORT_MPI_API void mpi_gatherv__ ( void *, MPI_Fint *, MPI_Fint *, void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_gatherv = pmpi_gatherv
EXPORT_MPI_API void mpi_gatherv ( void *, MPI_Fint *, MPI_Fint *, void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_gatherv_ = pmpi_gatherv_
EXPORT_MPI_API void mpi_gatherv_ ( void *, MPI_Fint *, MPI_Fint *, void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_GATHERV ( void *, MPI_Fint *, MPI_Fint *, void *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_GATHERV")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_gatherv__ ( void *, MPI_Fint *, MPI_Fint *, void *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_gatherv__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_gatherv ( void *, MPI_Fint *, MPI_Fint *, void *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_gatherv")));
#else
EXPORT_MPI_API void mpi_gatherv_ ( void *, MPI_Fint *, MPI_Fint *, void *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_gatherv_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_GATHERV  MPI_GATHERV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_gatherv__  mpi_gatherv__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_gatherv  mpi_gatherv
#else
#pragma _HP_SECONDARY_DEF pmpi_gatherv_  mpi_gatherv_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_GATHERV as PMPI_GATHERV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_gatherv__ as pmpi_gatherv__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_gatherv as pmpi_gatherv
#else
#pragma _CRI duplicate mpi_gatherv_ as pmpi_gatherv_
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
#define mpi_gatherv_ PMPI_GATHERV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_gatherv_ pmpi_gatherv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_gatherv_ pmpi_gatherv
#else
#define mpi_gatherv_ pmpi_gatherv_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_gatherv_ MPI_GATHERV
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_gatherv_ mpi_gatherv__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_gatherv_ mpi_gatherv
#endif
#endif


#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 10

 void mpi_gatherv_ ( void *unknown, ...)
{
void             *sendbuf;
int		*sendcnt;
MPI_Datatype     *sendtype;
void             *recvbuf;
int              *recvcnts;
int              *displs;
MPI_Datatype     *recvtype;
int		*root;
MPI_Comm          *comm;
int 		*__ierr;
int             buflen;
va_list         ap;

va_start(ap, unknown);
sendbuf = unknown;
if (_numargs() == NUMPARAMS+1) {
    /* Note that we can't set __ierr because we don't know where it is! */
    (void) MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_ONE_CHAR, "MPI_GATHERV" );
    return;
}
if (_numargs() == NUMPARAMS+2) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
sendcnt =     	va_arg(ap, int *);
sendtype =      va_arg(ap, MPI_Datatype *);
recvbuf =       va_arg(ap, void *);
if (_numargs() == NUMPARAMS+2) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
recvcnts =     	va_arg(ap, int *);
displs = 	va_arg(ap, int *);
recvtype =      va_arg(ap, MPI_Datatype *);
root =		va_arg(ap, int *);
comm =          va_arg(ap, MPI_Comm *);
__ierr =        va_arg(ap, int *);

*__ierr = MPI_Gatherv(MPIR_F_PTR(sendbuf),*sendcnt,*sendtype,
        MPIR_F_PTR(recvbuf),recvcnts,displs,*recvtype,*root,*comm);
}

#else

 void mpi_gatherv_ ( sendbuf, sendcnt,  sendtype, 
                  recvbuf, recvcnts, displs, recvtype, 
                  root, comm, __ierr )
void             *sendbuf;
int*sendcnt;
MPI_Datatype     *sendtype;
void             *recvbuf;
int              *recvcnts;
int              *displs;
MPI_Datatype     *recvtype;
int*root;
MPI_Comm          *comm;
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

*__ierr = MPI_Gatherv(MPIR_F_PTR(sendbuf),*sendcnt,*sendtype,
        MPIR_F_PTR(recvbuf),recvcnts,displs,*recvtype,*root,*comm);
}

#endif
#else
/* Prototype to suppress warnings about missing prototypes */

EXPORT_MPI_API void FORTRAN_API mpi_gatherv_ ANSI_ARGS(( void *, MPI_Fint *, MPI_Fint *, void *, 
                              MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                              MPI_Fint *, MPI_Fint *, MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_gatherv_ ( void *sendbuf, MPI_Fint *sendcnt,  MPI_Fint *sendtype, 
                  void *recvbuf, MPI_Fint *recvcnts, MPI_Fint *displs, MPI_Fint *recvtype, 
                  MPI_Fint *root, MPI_Fint *comm, MPI_Fint *__ierr )
{

    if (sizeof(MPI_Fint) == sizeof(int)) 
        *__ierr = MPI_Gatherv(MPIR_F_PTR(sendbuf), *sendcnt,
                              MPI_Type_f2c(*sendtype), MPIR_F_PTR(recvbuf),
                              recvcnts, displs, 
                              MPI_Type_f2c(*recvtype), *root,
                              MPI_Comm_f2c(*comm));
    else {
	int size;
        int *l_recvcnts;
        int *l_displs;
	int i;

	MPI_Comm_size(MPI_Comm_f2c(*comm), &size);
 
	MPIR_FALLOC(l_recvcnts,(int*)MALLOC(sizeof(int)* size),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Gatherv");
	MPIR_FALLOC(l_displs,(int*)MALLOC(sizeof(int)* size),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Gatherv");
	for (i=0; i<size; i++) {
	    l_recvcnts[i] = (int)recvcnts[i];
	    l_displs[i] = (int)displs[i];
	}    
        *__ierr = MPI_Gatherv(MPIR_F_PTR(sendbuf), (int)*sendcnt,
                              MPI_Type_f2c(*sendtype), MPIR_F_PTR(recvbuf),
                              l_recvcnts, l_displs, 
                              MPI_Type_f2c(*recvtype), (int)*root,
                              MPI_Comm_f2c(*comm));
	FREE( l_recvcnts );
	FREE( l_displs );
    }

}
#endif

