/*
 *  $Id$
 *
 * Multi-protocol, Multi-device support for 2nd generation ADI.
 * We start with support for blocking, contiguous sends.
 * Note the 'msgrep' field; this gives a hook for heterogeneous systems
 * which can be ignored on homogeneous systems.
 * 
 * For the synchronous send, we always use a Rendezvous send.
 */

#include "mpid.h"
#include "mpiddev.h"


void MPID_SsendContig( buf, len, src_comm_lrank, tag, context_id, 
		       dest_grank, msgrep, error_code )
void          *buf;
int           len, src_comm_lrank, tag, context_id, dest_grank, *error_code;
MPID_Msgrep_t msgrep;
{
    MPID_Device *dev = MPID_devset->dev[dest_grank];

    /* The one error test that makes sense here */
    if (buf == 0 && len > 0) {
	*error_code = MPI_ERR_BUFFER;
	return;
    }

    /* call function from device with device ranks for src and dest */
    *error_code = MPID_Protocol_call_send( dev->rndv->send,
			     buf, len, src_comm_lrank, tag, context_id, MPID_GET_DEVRANK(dest_grank), msgrep, NULL,
			     dev );
    return;
}

void MPID_SsendNonContig( buf, len, src_comm_lrank, tag, context_id, 
			  dest_grank, msgrep, error_code, dtypeptr)
void          *buf;
int           len, src_comm_lrank, tag, context_id, dest_grank, *error_code;
MPID_Msgrep_t msgrep;
struct MPIR_DATATYPE* dtypeptr;
{
    MPID_Device *dev = MPID_devset->dev[dest_grank];

    /* The one error test that makes sense here */
    if (buf == 0 && len > 0) {
	*error_code = MPI_ERR_BUFFER;
	return;
    }

    /* call function from device with device ranks for src and dest */
    *error_code = MPID_Protocol_call_send( dev->rndv->send,
			     buf, len, src_comm_lrank, tag, context_id, MPID_GET_DEVRANK(dest_grank), msgrep, dtypeptr,
			     dev );
    return;
}


void MPID_IssendContig( buf, len, src_comm_lrank, tag, context_id, 
			dest_grank, msgrep, request, error_code )
void          *buf;
int           len, src_comm_lrank, tag, context_id, dest_grank, *error_code;
MPID_Msgrep_t msgrep;
MPI_Request   request;
{
    MPID_Device *dev = MPID_devset->dev[dest_grank];
    
    /* The one error test that makes sense here */
    if (buf == 0 && len > 0) {
	*error_code = MPI_ERR_BUFFER;
	return;
    }
    
    /* call function from device with device ranks for src and dest */
    *error_code =  MPID_Protocol_call_isend( dev->rndv->isend,
					     buf, len, src_comm_lrank, tag, context_id, MPID_GET_DEVRANK(dest_grank), msgrep, (MPIR_SHANDLE *)request, NULL,
					     dev );
    return;
}

void MPID_IssendNonContig( buf, len, src_comm_lrank, tag, context_id, 
			      dest_grank, msgrep, request, error_code, dtypeptr)
void          *buf;
int           len, src_comm_lrank, tag, context_id, dest_grank, *error_code;
MPID_Msgrep_t msgrep;
MPI_Request   request;
struct MPIR_DATATYPE* dtypeptr;
{
    MPID_Device *dev = MPID_devset->dev[dest_grank];

    /* The one error test that makes sense here */
    if (buf == 0 && len > 0) {
	*error_code = MPI_ERR_BUFFER;
	return;
    }

    /* call function from device with device ranks for src and dest */
    *error_code = MPID_Protocol_call_isend( dev->rndv->isend,
			      buf, len, src_comm_lrank, tag, context_id, MPID_GET_DEVRANK(dest_grank), msgrep, (MPIR_SHANDLE *)request, dtypeptr,
			      dev );
    return;
}
