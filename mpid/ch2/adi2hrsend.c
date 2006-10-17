/*
 *  $Id$
 *
 */

#include "mpid_common.h"
#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "type_ff.h"


/*
 * Ready-send routines (MPI_Rsend() )
 *
 */


void MPID_RsendDatatype( comm_ptr, buf, count, dtype_ptr, src_comm_lrank, tag, 
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

    /* Alogrithm:
     * First, see if we can just send the data (contiguous or, for
     * heterogeneous, packed).
     * Otherwise, create a local buffer, use SendContig, and then free the buffer.
     */

    contig_size = MPIR_GET_DTYPE_SIZE(datatype,dtype_ptr);
    MPID_DO_HETERO(MPID_Msg_rep( comm_ptr, dest_grank, dtype_ptr, &msgrep, &msgact ));
    
    if (contig_size > 0
	MPID_DO_HETERO(&& msgact == MPID_MSG_OK)) {
	/* Just drop through into the contiguous send routine 
	   For packed data, the representation format is that in the
	   communicator. */
	len = contig_size * count;
	MPID_RsendContig( buf, len, src_comm_lrank, tag, context_id, 
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
	(dtype_ptr->size*count >= MPID_Device_call_long_len( dev->grank_to_devlrank[dest_grank], dev ) )
	MPID_DO_HETERO(&& (msgact == MPID_MSG_OK))) { 

	MPID_RsendNonContig( buf, dtype_ptr->size*count, src_comm_lrank, tag, context_id, 
			     dest_grank, msgrep, error_code, dtype_ptr);
	return;
    }
    
    /* We need to do the conventional pack & send */
    mybuf = 0;
    MPID_PackMessage( buf, count, dtype_ptr, comm_ptr, dest_grank, 
		      msgrep, msgact, (void **)&mybuf, &len, error_code );
    if (*error_code) 
	return;

    MPID_RsendContig( mybuf, len, src_comm_lrank, tag, context_id, 
		      dest_grank, msgrep, error_code );
    if (mybuf) {
	FREE( mybuf );
    }
}

/*
 * Noncontiguous datatype irsend
 * This is a simple implementation.  Note that in the rendezvous case, the
 * "pack" could be deferred until the "ok to send" message arrives.  To
 * implement this, the individual "send" routines would have to know how to
 * handle general datatypes.  We'll leave that for later.
 */
void MPID_IrsendDatatype( comm_ptr, buf, count, dtype_ptr, src_comm_lrank, tag, 
   			  context_id, dest_grank, request, error_code, dummy )
struct MPIR_COMMUNICATOR *    comm_ptr;
struct MPIR_DATATYPE *dtype_ptr;
void         *buf;
int          count, src_comm_lrank, tag, context_id, dest_grank, *error_code;
MPI_Request  request;
int dummy;
{
    MPID_Device *dev = MPID_devset->dev[dest_grank];
    MPID_Msgrep_t   msgrep = MPID_MSGREP_RECEIVER;
    MPID_Msg_pack_t msgact = MPID_MSG_OK;
    int             len, contig_size;
    char            *mybuf;

    /*
     * Alogrithm:
     * First, see if we can just send the data (contiguous or, for
     * heterogeneous, packed).
     * Otherwise, 
     * Create a local buffer, use SendContig, and then free the buffer.
     */

    MPID_DO_HETERO(MPID_Msg_rep( comm_ptr, dest_grank, dtype_ptr, 
				 &msgrep, &msgact ));
    contig_size = MPIR_GET_DTYPE_SIZE(datatype,dtype_ptr);
    if (contig_size > 0 
	MPID_DO_HETERO(&& msgact == MPID_MSG_OK)) {
	/* Just drop through into the contiguous send routine 
	   For packed data, the representation format is that in the
	   communicator.
	 */
	len = contig_size * count;
	MPID_IrsendContig( buf, len, src_comm_lrank, tag, context_id, 
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

	MPID_IrsendNonContig( buf, dtype_ptr->size*count, src_comm_lrank, tag, context_id, 
			     dest_grank, msgrep, request, error_code, dtype_ptr);
	return;
    }
    
    /* We need to do the conventional pack & send */
    mybuf = 0;
    MPID_PackMessage( buf, count, dtype_ptr, comm_ptr, dest_grank, 
		      msgrep, msgact, (void **)&mybuf, &len, error_code );
    if (*error_code) 
	return;
    
    MPID_IrsendContig( mybuf, len, src_comm_lrank, tag, context_id, 
		       dest_grank, msgrep, request, error_code );
    if (request->shandle.is_complete) {
	if (mybuf) {
	    FREE( mybuf ); 
	}
    } else {
	request->shandle.start  = mybuf;
	request->shandle.finish = MPID_PackMessageFree;
    }

    /* Note that, from the users perspective, the message is now complete
       (!) since the data is out of the input buffer (!) */
}
