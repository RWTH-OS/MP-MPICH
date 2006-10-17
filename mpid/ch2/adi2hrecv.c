/*
 *  $Id$
 *
 *  (C) 1995 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#include "mpid_common.h"
#include "mpid.h"
#include "mpiddev.h"
#include "mpid_debug.h"
#include "reqalloc.h"
#include "../util/queue.h"

/***************************************************************************/
/*
 * Multi-protocol, Multi-device support for 2nd generation ADI.
 * This file has support for noncontiguous sends for systems that do not 
 * have native support for complex datatypes.
 */
/***************************************************************************/

void MPID_RecvDatatype( comm_ptr, buf, count, dtype_ptr, src_comm_lrank, tag, 
			context_id, status, error_code )
struct MPIR_COMMUNICATOR *comm_ptr;
void         *buf;
int          count, src_comm_lrank, tag, context_id, *error_code;
struct MPIR_DATATYPE * dtype_ptr;
MPI_Status   *status;
{
    MPIR_RHANDLE rhandle;
    MPI_Request  request = (MPI_Request)&rhandle;

    DEBUG_INIT_STRUCT(request,sizeof(rhandle));
    MPID_Recv_init( &rhandle );
    /* rhandle.finish = 0; gets set in IrecvDatatype */
    *error_code = 0;
    MPID_IrecvDatatype( comm_ptr, buf, count, dtype_ptr, src_comm_lrank, tag, 
			context_id, request, error_code );
    if (!*error_code) {
	MPID_RecvComplete( request, status, error_code );
    }
}


void MPID_IrecvDatatype( comm_ptr, buf, count, dtype_ptr, src_comm_lrank, tag, 
			 context_id, request, error_code )
struct MPIR_COMMUNICATOR *comm_ptr;
void         *buf;
int          count, src_comm_lrank, tag, context_id, *error_code;
struct MPIR_DATATYPE * dtype_ptr;
MPI_Request  request;
{
    MPIR_RHANDLE    *dmpi_unexpected, *rhandle = &request->rhandle;
    int             len;
    MPID_Msgrep_t   msgrep = MPID_MSGREP_RECEIVER;
    MPID_DO_HETERO(MPID_Msg_pack_t msgact = MPID_MSG_OK;)
    void            *mybuf;
    int             contig_size;
    int             src_grank;
    MPID_Device *dev = NULL;

    DEBUG_PRINT_ARGS2(src_comm_lrank, "R starting IrecvDatatype");

    /* Just in case; make sure that finish is 0 */
    rhandle->finish = 0;
    
    /* See if this is really contiguous */
    contig_size = MPIR_GET_DTYPE_SIZE(datatype,dtype_ptr);

    /* MPI_ANY_SOURCE and other special ranks are < 0 */
    src_grank = (src_comm_lrank >= 0) ? comm_ptr->lrank_to_grank[src_comm_lrank] : src_comm_lrank;

    MPID_DO_HETERO(MPID_Msg_rep( comm_ptr, src_grank, dtype_ptr, &msgrep, &msgact ));

    if (contig_size > 0	MPID_DO_HETERO(&& msgact == MPID_MSG_OK)) {
	/* Just drop through into the contiguous send routine.
	   For packed data, the representation format is that in the  communicator. */
	len = contig_size * count;

	MPID_IrecvContig( buf, len, src_comm_lrank, tag, context_id, 
			  request, error_code );
	return;
    }

    /* For non-contignous recv, we need to know the device to use. For multi-device
       configurations, this is only possible  for explicit ranks; single-device
       configuration can be used in any case. */
    dev = MPID_GET_DEV(src_grank);

    /* At this time, we check to see if the message has already been received.
       Note that we cannot have any thread receiving a message while 
       checking the queues.   In case we do enqueue the message, we set
       the fields that will need to be valid BEFORE calling this routine
       (this is extra overhead ONLY in the case that the message was
       unexpected, which is already the higher-overhead case).
     */
    rhandle->datatype    = dtype_ptr;
    MPIR_REF_INCR(dtype_ptr);
    rhandle->start       = buf;
    rhandle->count       = count;
    rhandle->is_complete = 0;
    rhandle->wait        = 0;
    rhandle->test        = 0;
    /* We still need to decrease the refcount for the datatype */
    rhandle->finish      = MPID_UnpackMessageComplete;
    rhandle->msgrep      = msgrep;

    /*  Check if the device supports non-contig receive without an 
	intermediate buffer. We don't do the direct pack for short messages.
	Test:
	1) we have a device structure and the datatype is non-contig
	2) for nc_enable == 2 we always do ff for nc-datatypes
	3) for nc_enable == 1 we only do ff for datatypes with 1 stack
	4) no direct Pack for short messages
	5) no basic type conversion in direct Pack, so test for hetero */
    if (dev && !contig_size &&  
	((dev->nc_enable == 2) || 
	 ((dev->nc_enable == 1) && (MPIR_Datatype_numFFStacks(dtype_ptr) == 1))) &&
	(dtype_ptr->size*count >= MPID_Device_call_long_len( (src_grank >= 0) ?  dev->grank_to_devlrank[src_grank] : src_grank, dev ) )
	MPID_DO_HETERO(&& (msgact == MPID_MSG_OK))) { 

	MPID_Pack_size( count, dtype_ptr, MPID_MSG_XDR, &len );
	rhandle->len = len;
	/* We don't set up an intermediate buffer but set rhandle->buf to 0, 
	   the related routines know that this means direct packing. */
	rhandle->buf = 0;
	
    } else {
	/* Here we need to set up a different, special buffer if NOT 
	   contiguous/homogeneous. */
	MPID_UnpackMessageSetup( count, dtype_ptr, comm_ptr, src_comm_lrank, msgrep,
				 (void **)&mybuf, &len, error_code );
	if (*error_code) 
	    return;
	rhandle->len = len;
	rhandle->buf = mybuf;
    }      
    
    MPID_Search_unexpected_queue_and_post( src_comm_lrank, tag, context_id,  
					   rhandle, &dmpi_unexpected );
    if (dmpi_unexpected) {
	DEBUG_PRINT_MSG("R Found in unexpected queue");
	while (!dmpi_unexpected->is_valid)
	    ;
	DEBUG_TEST_FCN(dmpi_unexpected->push,"req->push");
	*error_code = (*dmpi_unexpected->push)( rhandle, dmpi_unexpected );
	DEBUG_PRINT_MSG("R Exiting IrecvDatatype");
	/* This may or may not complete the message */
	return;
    }

    /* If we got here, the message is not yet available */
    DEBUG_PRINT_MSG("R Exiting IrecvDatatype");
    return;
}
