/*
 *  $Id$
 *
 * 
 * For the ready send, we use the special ready protocol if available. If it is not
 * available, we just use the usual protocol according to the message size. This causes
 * no problems because ready send does not promise anything to the user.
 */

#include "mpid.h"
#include "mpiddev.h"


void MPID_RsendContig( buf, len, src_comm_lrank, tag, context_id, 
		       dest_grank, msgrep, error_code )
void          *buf;
int           len, src_comm_lrank, tag, context_id, dest_grank, *error_code;
MPID_Msgrep_t msgrep;
{
    MPID_Device *dev = MPID_devset->dev[dest_grank];

    if (dev->ready != NULL) {
	/* The one error test that makes sense here */
	if (buf == 0 && len > 0) {
	    *error_code = MPI_ERR_BUFFER;
	    return;
	}
	
	*error_code = MPID_Protocol_call_send( dev->ready->send,
					       buf, len,src_comm_lrank, tag, context_id, MPID_GET_DEVRANK(dest_grank), msgrep, NULL,
					       dev );
    } else {
	MPID_SendContig( buf, len, src_comm_lrank, tag, context_id, 
			 dest_grank, msgrep, error_code);
    }

    return;
}

void MPID_RsendNonContig( buf, len, src_comm_lrank, tag, context_id, 
			  dest_grank, msgrep, error_code, dtypeptr)
void          *buf;
int           len, src_comm_lrank, tag, context_id, dest_grank, *error_code;
MPID_Msgrep_t msgrep;
struct MPIR_DATATYPE* dtypeptr;
{
    MPID_Device *dev = MPID_devset->dev[dest_grank];

    if (dev->ready != NULL) {
	/* The one error test that makes sense here */
	if (buf == 0 && len > 0) {
	    *error_code = MPI_ERR_BUFFER;
	    return;
	}

	/* call function from device with device ranks for src and dest */
	*error_code = MPID_Protocol_call_send( dev->ready->send,
					       buf, len, src_comm_lrank, tag, context_id, MPID_GET_DEVRANK(dest_grank), msgrep, dtypeptr,
					       dev );
    } else {
	MPID_SendNonContig( buf, len, src_comm_lrank, tag, context_id, 
			    dest_grank, msgrep, error_code, dtypeptr );
    }

    return;
}


void MPID_IrsendContig( buf, len, src_comm_lrank, tag, context_id, 
			      dest_grank, msgrep, request, error_code )
void          *buf;
int           len, src_comm_lrank, tag, context_id, dest_grank, *error_code;
MPID_Msgrep_t msgrep;
MPI_Request   request;
{
    MPID_Device *dev = MPID_devset->dev[dest_grank];

    if (dev->ready != NULL) {
	/* The one error test that makes sense here */
	if (buf == 0 && len > 0) {
	    *error_code = MPI_ERR_BUFFER;
	    return;
	}
	
	/* call function from device with device ranks for src and dest */
	*error_code = MPID_Protocol_call_isend( dev->ready->isend,
						buf, len, src_comm_lrank, tag, context_id, MPID_GET_DEVRANK(dest_grank), msgrep, (MPIR_SHANDLE *)request, NULL,
						dev );
    } else {
	MPID_IsendContig( buf, len, src_comm_lrank, tag, context_id, 
			  dest_grank, msgrep, request, error_code);
    }
	
    return;
}

void MPID_IrsendNonContig( buf, len, src_comm_lrank, tag, context_id, 
			      dest_grank, msgrep, request, error_code, dtypeptr)
void          *buf;
int           len, src_comm_lrank, tag, context_id, dest_grank, *error_code;
MPID_Msgrep_t msgrep;
MPI_Request   request;
struct MPIR_DATATYPE* dtypeptr;
{
    MPID_Device *dev = MPID_devset->dev[dest_grank];

    if (dev->ready != NULL) {
	/* The one error test that makes sense here */
	if (buf == 0 && len > 0) {
	    *error_code = MPI_ERR_BUFFER;
	    return;
	}

	/* call function from device with device ranks for src and dest */
	*error_code = MPID_Protocol_call_isend( dev->ready->isend,
						buf, len, src_comm_lrank, tag, context_id, MPID_GET_DEVRANK(dest_grank), msgrep, (MPIR_SHANDLE *)request, NULL,
						dev );
    } else {
	MPID_IsendNonContig( buf, len, src_comm_lrank, tag, context_id, 
			     dest_grank, msgrep, request, error_code, dtypeptr);
    }
	
    return;
}
