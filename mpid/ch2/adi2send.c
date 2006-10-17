/*
 *  $Id$
 *
 * Multi-protocol, Multi-device support for 2nd generation ADI.
 * We start with support for blocking, contiguous sends.
 * Note the 'msgrep' field; this gives a hook for heterogeneous systems
 * which can be ignored on homogeneous systems.
 */

#include "mpid.h"
#include "mpiddev.h"
#include "mpid_debug.h"
/* flow.h includes the optional flow control for eager delivery */
#include "flow.h"


void MPID_SendContig( buf, len, src_comm_lrank, tag, context_id, 
			      dest_grank, msgrep, error_code )
void     *buf;
int      len, src_comm_lrank, tag, context_id, dest_grank, *error_code;
MPID_Msgrep_t msgrep;
{
    MPID_Device *dev = MPID_devset->dev[dest_grank];
    int (*fcn) ( void *, int, int, int, int, int, MPID_Msgrep_t,struct MPIR_DATATYPE*);

    /* The one error test that makes sense here */
    if (buf == 0 && len > 0) {
	*error_code = MPI_ERR_BUFFER;
	return;
    }

    /* Choose the function based on the message length in bytes */
    if (len < MPID_Device_call_long_len( dev->grank_to_devlrank[dest_grank], dev ) )
	fcn = dev->short_msg->send;
    else 
      if (len < MPID_Device_call_vlong_len( dev->grank_to_devlrank[dest_grank], dev ) && MPID_FLOW_MEM_OK(len,dest_grank)) 
	fcn = dev->long_msg->send;
      else
	fcn = dev->vlong_msg->send;

    DEBUG_TEST_FCN(fcn,"dev->proto->send");

    /* call function from device with device ranks for src and dest */
    *error_code = MPID_Protocol_call_send( fcn, 
					   buf, len, src_comm_lrank, tag, context_id, MPID_GET_DEVRANK(dest_grank), msgrep, NULL,
					   dev ); 
    return;
}


void MPID_IsendContig( buf, len, src_comm_lrank, tag, context_id,
		       dest_grank, msgrep, request, error_code )
void        *buf;
int         len, src_comm_lrank, tag, context_id, dest_grank, *error_code;
MPID_Msgrep_t msgrep;
MPI_Request request;
{
    MPID_Device *dev = MPID_devset->dev[dest_grank];
    int (*fcn) ( void *, int, int, int, int, int, MPID_Msgrep_t, MPIR_SHANDLE *,struct MPIR_DATATYPE* );
    
    /* The one error test that makes sense here */
    if (buf == 0 && len > 0) {
	*error_code = MPI_ERR_BUFFER;
	return;
    }

    /* Just in case; make sure that finish is 0 */
    request->shandle.finish = 0;

    /* Choose the function based on the message length in bytes */ /*//SI Error dev->grank_to_devlrank invalid! */
    if (len < MPID_Device_call_long_len( dev->grank_to_devlrank[dest_grank], dev ))
	fcn = dev->short_msg->isend;
    else if (len < MPID_Device_call_vlong_len( dev->grank_to_devlrank[dest_grank], dev ) && MPID_FLOW_MEM_OK(len,dest_grank)) 
	fcn = dev->long_msg->isend;
    else
	fcn = dev->vlong_msg->isend;

    DEBUG_TEST_FCN(fcn,"dev->proto->isend");
    
    /* call function from device with device ranks for src and dest */
    *error_code = MPID_Protocol_call_isend( fcn,
					    buf, len, src_comm_lrank, tag, context_id, MPID_GET_DEVRANK(dest_grank), msgrep, (MPIR_SHANDLE *)request,NULL,
					    dev );
    return;
}


/* Bsend is just a test for short send */
void MPID_BsendContig( buf, len, src_comm_lrank, tag, context_id, 
		       dest_grank, msgrep, error_code )
void     *buf;
int      len, src_comm_lrank, tag, context_id, dest_grank, *error_code;
MPID_Msgrep_t msgrep;
{
    MPID_Device *dev = MPID_devset->dev[dest_grank];
    int rc  ;
    
    if (len < MPID_Device_call_long_len( dev->grank_to_devlrank[dest_grank], dev )) {

	DEBUG_TEST_FCN(dev->short_msg->send,"dev->short->send");

	/* call function from device with device ranks for src and dest */
	rc = MPID_Protocol_call_send( *dev->short_msg->send,
				 buf, len, src_comm_lrank, tag, context_id,
				 MPID_GET_DEVRANK(dest_grank), msgrep, NULL,
				 dev );
    }
    else
	rc = MPIR_ERR_MAY_BLOCK;
    
    *error_code = rc;

    return;
}


int MPID_SendIcomplete( request, error_code )
MPI_Request request;
int         *error_code;
{
    MPIR_SHANDLE *shandle = &request->shandle;
    int lerr;

    if (shandle->is_complete) {
	if (shandle->finish) 
	    (shandle->finish)( shandle );
	return 1;
    }

    if (shandle->test) 
	*error_code = (*shandle->test)( shandle );
    else {
	/* The most common case is a check device loop */
	MPID_Device *dev;

	dev = MPID_devset->dev_list;
	while (dev) {
	    lerr = MPID_Device_call_check_device ( dev, MPID_NOTBLOCKING );
	    if (lerr > 0) {
		*error_code = lerr;
		return 0;
	    }
	    dev = dev->next;
	}
    }
    if (shandle->is_complete && shandle->finish) 
	(shandle->finish)( shandle );
    return shandle->is_complete;
}


void MPID_SendComplete( request, error_code )
MPI_Request request;
int         *error_code;
{
    MPIR_SHANDLE *shandle = &request->shandle;
    int          lerr;
    MPID_BLOCKING_TYPE check_mode;

    /* The 'while' is at the top in case the 'wait' routine is changed
       by one of the steps.  This happens, for example, in the Rendezvous
       Protocol */
#ifdef MPID_USE_DEVTHREADS
    /* If threads are used in the device, they will take care of 
       completing the message, and *this* thread has to block! */
    /* XXX This is not multi-device safe! Only used by ch_smi, so far. */
    check_mode = MPID_BLOCKING;
#else
    check_mode = MPID_NOTBLOCKING;
#endif

    while (!shandle->is_complete) {
	if (shandle->wait) 
	    *error_code = (*shandle->wait)( shandle );
	else {
	    /* The most common case is a check device loop until it is
	       complete. */
	    MPID_Device *dev;

	    if (!shandle->is_complete) {
		dev = MPID_devset->dev_list;
		while (dev) {
		    lerr = MPID_Device_call_check_device( dev, check_mode );
		    if (lerr > 0) {
			*error_code = lerr;
			return;
		    }
		    dev = dev->next;
		}
	    }
	}
    }

    if (shandle->finish) 
	(shandle->finish)( shandle );

    return;
}

void MPID_SendNonContig( buf, len, src_comm_lrank, tag, context_id, 
			 dest_grank, msgrep, error_code, dtypeptr)
void     *buf;
int      len, src_comm_lrank, tag, context_id, dest_grank, *error_code;
struct MPIR_DATATYPE* dtypeptr;
MPID_Msgrep_t msgrep;
{
    int blen;
    MPID_Device *dev = MPID_devset->dev[dest_grank];
    int (*fcn) ( void *, int, int, int, int, int, MPID_Msgrep_t,struct MPIR_DATATYPE*);

    blen = len;

    /* Choose the function based on the message length in bytes */
    if (blen < MPID_Device_call_long_len( dev->grank_to_devlrank[dest_grank], dev ))
	fcn = dev->short_msg->send;
    else if (blen < MPID_Device_call_vlong_len( dev->grank_to_devlrank[dest_grank], dev ) && MPID_FLOW_MEM_OK(blen,dest_grank)) 
	fcn = dev->long_msg->send;
    else
	fcn = dev->vlong_msg->send;
   
    DEBUG_TEST_FCN(fcn,"dev->proto->send");

    /* Have to use blen at call to Send function!!! */
    /* call function from device with device ranks for src and dest */
    *error_code = MPID_Protocol_call_send( fcn,
					   buf, blen, src_comm_lrank, tag, context_id, MPID_GET_DEVRANK(dest_grank), msgrep, dtypeptr,
					   dev );
    return;
}


void MPID_IsendNonContig( buf, len, src_comm_lrank, tag, context_id,
			  dest_grank, msgrep, request, error_code, dtypeptr )
void        *buf;
int         len, src_comm_lrank, tag, context_id, dest_grank, *error_code;
MPID_Msgrep_t msgrep;
MPI_Request request;
struct MPIR_DATATYPE* dtypeptr;
{
    MPID_Device *dev = MPID_devset->dev[dest_grank];
    int (*fcn) ( void *, int, int, int, int, int, MPID_Msgrep_t, MPIR_SHANDLE *,struct MPIR_DATATYPE* );

    /* Testing for buf == 0 does not make sense here because MPI_BOTTOM may be passed */

    /* Just in case; make sure that finish is 0 */
    request->shandle.finish = 0;

    /* Choose the function based on the message length in bytes */
    if (len < MPID_Device_call_long_len( dev->grank_to_devlrank[dest_grank], dev ))
	fcn = dev->short_msg->isend;
    else if (len < MPID_Device_call_vlong_len( dev->grank_to_devlrank[dest_grank], dev ) && MPID_FLOW_MEM_OK(len,dest_grank)) 
	fcn = dev->long_msg->isend;
    else
	fcn = dev->vlong_msg->isend;

    DEBUG_TEST_FCN(fcn,"dev->proto->isend");

    /* call function from device with device ranks for src and dest */
    *error_code = MPID_Protocol_call_isend(fcn,
					   buf, len, src_comm_lrank, tag, context_id, MPID_GET_DEVRANK(dest_grank), msgrep, (MPIR_SHANDLE *)request, dtypeptr,
					   dev );
    return;
}

