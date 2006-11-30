/*
 *  $Id$
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Ibsend = PMPI_Ibsend
#elif defined(HAVE_ATTRIBUTE_WEAK)
EXPORT_MPI_API int MPI_Ibsend( void *buf, int count, MPI_Datatype datatype, int dest, int tag,
	MPI_Comm comm, MPI_Request *request) __attribute__ ((weak, alias ("PMPI_Ibsend")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Ibsend  MPI_Ibsend
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Ibsend as PMPI_Ibsend
/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#define MPI_BUILD_PROFILING
#include "mpiprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif
#include "reqalloc.h"

/*@
    MPI_Ibsend - Starts a nonblocking buffered send

Input Parameters:
+ buf - initial address of send buffer (choice) 
. count - number of elements in send buffer (integer) 
. datatype - datatype of each send buffer element (handle) 
. dest - rank of destination (integer) 
. tag - message tag (integer) 
- comm - communicator (handle) 

Output Parameter:
. request - communication request (handle) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_TAG
.N MPI_ERR_RANK
.N MPI_ERR_BUFFER

@*/
EXPORT_MPI_API int MPI_Ibsend( void *buf, int count, MPI_Datatype datatype, int dest, int tag,
		MPI_Comm comm, MPI_Request *request )
{
    int         mpi_errno = MPI_SUCCESS;
    static char myname[] = "MPI_IBSEND";
    int         psize;
    void        *bufp;
    MPIR_SHANDLE *shandle;
    struct MPIR_COMMUNICATOR *comm_ptr;
    struct MPIR_DATATYPE     *dtype_ptr;

    TR_PUSH(myname);
    comm_ptr = MPIR_GET_COMM_PTR(comm);
    MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

    dtype_ptr = MPIR_GET_DTYPE_PTR(datatype);

    MPIR_TEST_DTYPE(datatype,dtype_ptr,comm_ptr,myname);

#ifndef MPIR_NO_ERROR_CHECKING
    MPIR_TEST_COUNT(count);
    MPIR_TEST_SEND_TAG(tag);
    MPIR_TEST_SEND_RANK(comm_ptr,dest);
    if (mpi_errno)
	return MPIR_ERROR(comm_ptr, mpi_errno, myname );
#endif

    MPIR_ALLOCFN(shandle,MPID_Send_alloc,comm_ptr, MPI_ERR_EXHAUSTED,myname);
    MPID_Request_init((MPI_Request)shandle, MPIR_SEND );

    /* we need the rank of dest in MPI_COMM_ALL in MPID_Gateway_SendCancelPacket(),
       so we save it here */
    shandle->partner_grank = comm_ptr->lrank_to_grank[dest];

    *request = (MPI_Request)shandle;

    MPIR_REMEMBER_SEND(shandle,buf, count, datatype, dest, tag, comm_ptr);

    if (dest == MPI_PROC_NULL) {
	shandle->is_complete = 1;
	return MPI_SUCCESS;
    }

    /* Allocate space if needed */
    MPI_Pack_size( count, datatype, comm, &psize );
    MPIR_CALL(MPIR_BsendAlloc( psize, *request, &bufp ),comm_ptr, myname );
    /* Information stored in the bsend part by BsendAlloc */

    MPIR_IbsendDatatype( comm_ptr, buf, count, dtype_ptr, 
			 comm_ptr->local_rank, tag, comm_ptr->send_context, 
			 comm_ptr->lrank_to_grank[dest], *request, 
			 &mpi_errno, 1 );

    TR_POP;
    MPIR_RETURN( comm_ptr, mpi_errno, myname );
}
