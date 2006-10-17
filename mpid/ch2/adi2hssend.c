/*
 *  $Id$
 *
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "mpid_common.h"

/***************************************************************************/
/*
 * Multi-protocol, Multi-device support for 2nd generation ADI.
 * This file has support for noncontiguous sends for systems that do not 
 * have native support for complex datatypes.
 */
/***************************************************************************/

void MPID_SsendDatatype( comm_ptr, buf, count, dtype_ptr, src_comm_lrank, tag, 
			context_id, dest_grank, error_code )
struct MPIR_COMMUNICATOR *    comm_ptr;
struct MPIR_DATATYPE *dtype_ptr;
void         *buf;
int          count, src_comm_lrank, tag, context_id, dest_grank, *error_code;
{
    MPID_Msgrep_t   msgrep = MPID_MSGREP_RECEIVER;
    MPID_Msg_pack_t msgact = MPID_MSG_OK;
    MPID_Device *dev = MPID_devset->dev[dest_grank];
    int             len, contig_size;
    void            *mybuf;

    /*
     * Alogrithm:
     * First, see if we can just send the data (contiguous or, for
     * heterogeneous, packed).
     * Otherwise, 
     * Create a local buffer, use SendContig, and then free the buffer.
     */

    contig_size = MPIR_GET_DTYPE_SIZE(datatype,dtype_ptr);
    MPID_DO_HETERO(MPID_Msg_rep( comm_ptr, dest_grank, dtype_ptr, 
				 &msgrep, &msgact ));
	
    if (contig_size > 0
	MPID_DO_HETERO(&& msgact == MPID_MSG_OK)) {
	/* Just drop through into the contiguous send routine 
	   For packed data, the representation format is that in the
	   communicator.
	 */
	len = contig_size * count;
	MPID_SsendContig( buf, len, src_comm_lrank, tag, context_id, 
			  dest_grank, msgrep, error_code );
	return;
    }

    MPID_Pack_size( count, dtype_ptr, MPID_MSG_XDR, &len );

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
	(dtype_ptr->size*count >= MPID_Device_call_long_len( dev->grank_to_devlrank[dest_grank], dev ))
	MPID_DO_HETERO(&& (msgact == MPID_MSG_OK))) { 

	MPID_SsendNonContig( buf, dtype_ptr->size*count, src_comm_lrank, tag, context_id, 
			     dest_grank, msgrep, error_code, dtype_ptr);
	return;
    }
    
    /* Just use the generic Pack-first and Send-contig if we got here */
    mybuf = 0;
    MPID_PackMessage( buf, count, dtype_ptr, comm_ptr, dest_grank, 
		      msgrep, msgact, (void **)&mybuf, &len, error_code );
    if (*error_code) return;

    MPID_SsendContig( mybuf, len, src_comm_lrank, tag, context_id, 
		     dest_grank, msgrep, error_code );
    if (mybuf) {
	FREE( mybuf );
    }
}

/*
 * Noncontiguous datatype issend
 * This is a simple implementation.  Note that in the rendezvous case, the
 * "pack" could be deferred until the "ok to send" message arrives.  To
 * implement this, the individual "send" routines would have to know how to
 * handle general datatypes.  We'll leave that for later.
 */
void MPID_IssendDatatype( comm_ptr, buf, count, dtype_ptr, src_comm_lrank, tag, 
			  context_id, dest_grank, request, error_code, do_hetero_flag )
struct MPIR_COMMUNICATOR *    comm_ptr;
struct MPIR_DATATYPE *dtype_ptr;
void         *buf;
int          count, src_comm_lrank, tag, context_id, dest_grank, *error_code;
MPI_Request  request;
int do_hetero_flag;
{
    MPID_Msgrep_t   msgrep = MPID_MSGREP_RECEIVER;
    MPID_Msg_pack_t msgact = MPID_MSG_OK;
    MPID_Device *dev = MPID_devset->dev[dest_grank];
    int             len, contig_size;
    char            *mybuf;

    /*
     * Alogrithm:
     * First, see if we can just send the data (contiguous or, for
     * heterogeneous, packed).
     * Otherwise, 
     * Create a local buffer, use SendContig, and then free the buffer.
     */

    if( do_hetero_flag )
	MPID_DO_HETERO(MPID_Msg_rep( comm_ptr, dest_grank, dtype_ptr, 
				     &msgrep, &msgact ));
    else {
	msgrep = MPID_MSGREP_RECEIVER;
	msgact = MPID_MSG_OK;
    }

    contig_size = MPIR_GET_DTYPE_SIZE(datatype,dtype_ptr);
    if (contig_size > 0 
	MPID_DO_HETERO(&& msgact == MPID_MSG_OK)) {
	/* Just drop through into the contiguous send routine 
	   For packed data, the representation format is that in the
	   communicator.
	 */
	len = contig_size * count;
	MPID_IssendContig( buf, len, src_comm_lrank, tag, context_id, 
			  dest_grank, msgrep, request, error_code );
	return;
    }

    MPID_Pack_size( count, dtype_ptr, MPID_MSG_XDR, &len );

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
	(dtype_ptr->size*count >= MPID_Device_call_long_len( dev->grank_to_devlrank[dest_grank], dev ))
	MPID_DO_HETERO(&& (msgact == MPID_MSG_OK))) { 
	
	MPID_IsendNonContig( buf, dtype_ptr->size*count, src_comm_lrank, tag, context_id, 
			     dest_grank, msgrep, request, error_code, dtype_ptr);
	return;
    }
    
    /* Just use the generic Pack-first and Send-contig if we got here*/
    mybuf = 0;
    MPID_PackMessage( buf, count, dtype_ptr, comm_ptr, dest_grank, 
		      msgrep, msgact, (void **)&mybuf, &len, error_code );
    if (*error_code) return;

    MPID_IssendContig( mybuf, len, src_comm_lrank, tag, context_id, 
		      dest_grank, msgrep, request, error_code );
    if (request->shandle.is_complete) {
	if (mybuf) { FREE( mybuf ); }
	}
    else {
	request->shandle.start  = mybuf;
	request->shandle.finish = MPID_PackMessageFree;
    }

    /* Note that, from the users perspective, the message is now complete
       (!) since the data is out of the input buffer (!) */
}
