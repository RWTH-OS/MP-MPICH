/*
 *  $Id$
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

#ifndef DBG
#ifdef _DEBUG
#include <stdio.h>
#define DBG(m) fprintf(stderr,"%s\n",m);fflush(stderr);
#else
#define DBG(m)
#endif
#endif

#ifdef HAVE_WEAK_SYMBOLS
#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Reduce = PMPI_Reduce
#elif defined(HAVE_ATTRIBUTE_WEAK)
EXPORT_MPI_API int MPI_Reduce ( void *sendbuf, void *recvbuf, int count, 
		 MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm ) __attribute__ ((weak, alias ("PMPI_Reduce")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Reduce  MPI_Reduce
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Reduce as PMPI_Reduce
/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#define MPI_BUILD_PROFILING
#include "mpiprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif
#include "coll.h"
#include "mpiops.h"
#include "x86_ops.h"

/*@

MPI_Reduce - Reduces values on all processes to a single value

Input Parameters:
+ sendbuf - address of send buffer (choice) 
. count - number of elements in send buffer (integer) 
. datatype - data type of elements of send buffer (handle) 
. op - reduce operation (handle) 
. root - rank of root process (integer) 
- comm - communicator (handle) 

Output Parameter:
. recvbuf - address of receive buffer (choice, 
significant only at 'root') 

Algorithm:
This implementation currently uses a simple tree algorithm.

.N fortran

.N collops

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_BUFFER
.N MPI_ERR_BUFFER_ALIAS
@*/
EXPORT_MPI_API int MPI_Reduce ( void *sendbuf, void *recvbuf, int count, 
		 MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm )
{
    int mpi_errno = MPI_SUCCESS;
    struct MPIR_COMMUNICATOR *comm_ptr;
    struct MPIR_DATATYPE     *dtype_ptr;
    MPIR_ERROR_DECL;
    static char myname[] = "MPI_REDUCE";
#ifdef RED_DEBUG
	char zahl[10];
	static int callcount=0;
	callcount++;

	DBG("Entering Reduce()");
	DBG(itoa(callcount,zahl,10));
#endif
    TR_PUSH(myname);

    comm_ptr = MPIR_GET_COMM_PTR(comm);

    dtype_ptr = MPIR_GET_DTYPE_PTR(datatype);

    /* Check for invalid arguments */

#ifndef MPIR_NO_ERROR_CHECKING
    MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);
    MPIR_TEST_DTYPE(datatype,dtype_ptr,comm_ptr,myname);
    MPIR_TEST_ALIAS(sendbuf,recvbuf);
	if (mpi_errno){
#ifdef RED_DEBUG
		DBG("Leaving Reduce Error No");
		DBG(itoa(mpi_errno,zahl,10));
#endif
		return MPIR_ERROR(comm_ptr, mpi_errno, myname );
	}	
#endif
    MPIR_ERROR_PUSH(comm_ptr);
    mpi_errno = comm_ptr->collops->Reduce(sendbuf, recvbuf, count, dtype_ptr, 
					  op, root, comm_ptr );
    MPIR_ERROR_POP(comm_ptr);
    TR_POP;
#ifdef RED_DEBUG
	DBG("Leaving Reduce");
#endif
    MPIR_RETURN(comm_ptr,mpi_errno,myname);
}


void MPIR_Setup_Reduce_Ops(void) {
/* XXX quick fix! */
#if CPU_ARCH_IS_X86 && !defined MPI_solaris86
    MPIR_Setup_x86_reduce_ops();
#else
    /* these are the standard Ops in C */
    MPIR_Op_setup( MPIR_MAXF,   1, 1, MPI_MAX );
    MPIR_Op_setup( MPIR_MINF,   1, 1, MPI_MIN );
    MPIR_Op_setup( MPIR_SUM,    1, 1, MPI_SUM );
    MPIR_Op_setup( MPIR_PROD,   1, 1, MPI_PROD );
    MPIR_Op_setup( MPIR_LAND,   1, 1, MPI_LAND );
    MPIR_Op_setup( MPIR_BAND,   1, 1, MPI_BAND );
    MPIR_Op_setup( MPIR_LOR,    1, 1, MPI_LOR );
    MPIR_Op_setup( MPIR_BOR,    1, 1, MPI_BOR );
    MPIR_Op_setup( MPIR_LXOR,   1, 1, MPI_LXOR );
    MPIR_Op_setup( MPIR_BXOR,   1, 1, MPI_BXOR );
    MPIR_Op_setup( MPIR_MAXLOC, 1, 1, MPI_MAXLOC );
    MPIR_Op_setup( MPIR_MINLOC, 1, 1, MPI_MINLOC );
#endif

    return;
}
