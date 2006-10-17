/*
 *  $Id: bsend_init.c,v 1.6 2003/06/05 11:50:37 rainer Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Bsend_init = PMPI_Bsend_init
#elif defined(HAVE_ATTRIBUTE_WEAK)
EXPORT_MPI_API int MPI_Bsend_init( void *buf, int count, MPI_Datatype datatype, int dest, 
		    int tag, MPI_Comm comm, MPI_Request *request ) __attribute__ ((weak, alias ("PMPI_Bsend_init")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Bsend_init  MPI_Bsend_init
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Bsend_init as PMPI_Bsend_init
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
    MPI_Bsend_init - Builds a handle for a buffered send

Input Parameters:
+ buf - initial address of send buffer (choice) 
. count - number of elements sent (integer) 
. datatype - type of each element (handle) 
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
.N MPI_ERR_RANK
.N MPI_ERR_TAG
@*/
EXPORT_MPI_API int MPI_Bsend_init( void *buf, int count, MPI_Datatype datatype, int dest, 
		    int tag, MPI_Comm comm, MPI_Request *request )
{
    int         mpi_errno = MPI_SUCCESS;
    int         psize;
    void        *bufp;
    struct MPIR_DATATYPE *dtype_ptr;
    struct MPIR_COMMUNICATOR *comm_ptr;
    MPIR_PSHANDLE *shandle;
    MPIR_ERROR_DECL;
    static char myname[] = "MPI_BSEND_INIT";

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

    MPIR_ALLOCFN(shandle,MPID_PSend_alloc,comm_ptr,MPI_ERR_EXHAUSTED,myname);
    MPIR_REF_INCR(dtype_ptr);
    MPIR_REF_INCR(comm_ptr);

    /* Save the information about the operation, being careful with
       ref-counted items */
    shandle->perm_datatype = dtype_ptr;
    shandle->perm_tag	   = tag;
    shandle->perm_dest	   = dest;
    shandle->perm_count	   = count;
    shandle->perm_buf	   = buf;
    shandle->perm_comm	   = comm_ptr;
    shandle->active	   = 0;
    /* we need the rank of dest in MPI_COMM_ALL in MPID_Gateway_SendCancelPacket(),
       so we save it here */
    shandle->shandle.partner_grank = comm_ptr->lrank_to_grank[dest];
    
    shandle->send          = MPIR_IbsendDatatype;

    MPID_Request_init((MPI_Request)&(shandle->shandle), MPIR_PERSISTENT_SEND );
    *request = (MPI_Request)shandle;

    if (dest != MPI_PROC_NULL) {
	MPIR_ERROR_PUSH(comm_ptr);
	/* Allocate space if needed */
	MPIR_CALL_POP(MPI_Pack_size( count, datatype, comm, &psize ),
		      comm_ptr,myname);
	MPIR_CALL_POP(MPIR_BsendAlloc( psize, *request, &bufp ),comm_ptr,myname);
	/* Information stored in the bsend part by BsendAlloc */
/* 	shandle->shandle.start = bufp;
	shandle->shandle.bytes_as_contig = psize; */
	MPIR_ERROR_POP(comm_ptr);
    }
    else 
	/* Rest of dest of MPI_PROC_NULL handled in start */
	shandle->shandle.start = 0;

    TR_POP;
    return MPI_SUCCESS;
}
