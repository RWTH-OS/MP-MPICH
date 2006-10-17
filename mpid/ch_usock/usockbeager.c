/*
 *  $Id$
 *
 *  (C) 1995 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#include "mpid.h"
#include "usockdev.h"
#include "mpimem.h"
#include "reqalloc.h"
/* flow.h includs the optional flow control for eager delivery */
#include "flow.h"


#define _DEBUG_EXTERN_REC
#include "mydebug.h"

/*
   Blocking, eager send/recv.
   These are ALWAYS for long messages.  Short messages are always
   handled in eager mode.
 */

/* Prototype definitions */
int MPID_USOCK_Eagerb_recv ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
int MPID_USOCK_Eagerb_irecv ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
int MPID_USOCK_Eagerb_save ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
int MPID_USOCK_Eagerb_unxrecv_start ANSI_ARGS(( MPIR_RHANDLE *, void * ));
int MPID_USOCK_Eagern_WaitReceive ANSI_ARGS((MPIR_RHANDLE*));
int MPID_USOCK_Eagern_TestReceive ANSI_ARGS((MPIR_RHANDLE*));

int MPID_USOCK_Eagerb_cancel_recv ANSI_ARGS((MPIR_RHANDLE *)) ;


/*
 * Definitions of the actual functions
 */

/*
 * This is the routine called when a packet of type MPID_PKT_LONG is
 * seen.  It receives the data as shown (final interface not set yet)
 */
int MPID_USOCK_Eagerb_recv( rhandle, from, in_pkt )
MPIR_RHANDLE *rhandle;
int          from;
void         *in_pkt;
{
    DSECTION("MPID_USOCK_Eagerb_recv");
    MPID_PKT_LONG_T   *pkt = (MPID_PKT_LONG_T *)in_pkt;
    int   msglen, err = MPI_SUCCESS;
    int  remain;

    DSECTENTRYPOINT;

    MPID_USOCK_Test_device(MPID_devset->active_dev, "Eagerb_recv");

    msglen = pkt->len;

    MPID_DO_HETERO(rhandle->msgrep = (MPID_Msgrep_t)pkt->msgrep);
    MPID_FLOW_MEM_GET(pkt,from);
    /* Check for truncation */
    MPID_CHK_MSGLEN(rhandle,msglen,err)

    MPID_FLOW_MEM_READ(msglen,from);
    MPID_FLOW_MEM_RECV(msglen,from);
    rhandle->s.count	 = msglen;
    rhandle->s.MPI_ERROR = err;
    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->src_comm_lrank;
    
    MPID_RecvFromChannel( rhandle->buf, msglen, from );
    if (rhandle->finish) {
      (rhandle->finish)( rhandle );
    }
    rhandle->is_complete = 1;
    
    /* In case we truncated the incoming message, 
       discard the remaining part of it...*/
    if(msglen<pkt->len) {
	remain = pkt->len-msglen;
	MPID_USOCK_ConsumeData(remain,from);
    }

    DSECTLEAVE
	return err;
}

/* 
* This routine is called when it is time to receive an unexpected
* message
*/
int MPID_USOCK_Eagerb_unxrecv_start( rhandle, in_runex )
MPIR_RHANDLE *rhandle;
void         *in_runex;
{
    DSECTION("MPID_USOCK_Eagerb_unxrecv_start");
    MPIR_RHANDLE *runex = (MPIR_RHANDLE *)in_runex;
    int          msglen, err = 0;
	
    DSECTENTRYPOINT;

    /* get the device of this handle: (may be, there a multiple usock entities) */
    if( runex->dev!=0 )
      MPID_devset->active_dev = runex->dev;

    MPID_USOCK_Test_device(MPID_devset->active_dev, "Eagerb_unxrecv_start");

    msglen = runex->s.count;
    MPID_CHK_MSGLEN(rhandle,msglen,err);
    /* Copy the data from the local area and free that area */
    if(!runex->is_complete)
		MPID_USOCK_Eagern_WaitReceive(runex);

    if (runex->s.count > 0) {
		MEMCPY( rhandle->buf, runex->start, msglen );
		FREE( runex->start );
    }
    MPID_FLOW_MEM_RECV(msglen,runex->from);
    MPID_DO_HETERO(rhandle->msgrep = runex->msgrep);
    rhandle->s		 = runex->s;
    rhandle->s.count     = msglen;
    rhandle->s.MPI_ERROR = err;
/*  MPID_RecvFree( runex );     */
    rhandle->wait	 = 0;
    rhandle->test	 = 0;
    rhandle->push	 = 0;
    rhandle->cancel      = 0;
    rhandle->is_complete = 1;

    if (rhandle->finish) 
		(rhandle->finish)( rhandle );
	
    DSECTLEAVE
	return err;
}

int MPID_USOCK_Eagerb_cancel_recv(runex) 
MPIR_RHANDLE *runex;
{
  /* get the device of this handle: (may be, there a multiple usock entities) */
  if( runex->dev!=0 )
    MPID_devset->active_dev = runex->dev;

  MPID_USOCK_Test_device(MPID_devset->active_dev, "cancel_recv");
  
  if(!runex->is_complete) MPID_USOCK_WaitTransfer(runex->rid);
  if (runex->s.count > 0) {
    FREE( runex->start );
  }

/*MPID_RecvFree( runex );  */

  return 0;
}

/* Save an unexpected message in rhandle */
int MPID_USOCK_Eagerb_save( rhandle, from, in_pkt )
MPIR_RHANDLE *rhandle;
int          from;
void         *in_pkt;
{
    DSECTION("MPID_USOCK_Eagerb_save");
    MPID_PKT_T *pkt = (MPID_PKT_T *)in_pkt;

    DSECTENTRYPOINT;

    MPID_USOCK_Test_device(MPID_devset->active_dev, "Eagerb_save");

    rhandle->finish=0;
    rhandle->s.MPI_TAG	  = pkt->head.tag;
    rhandle->s.MPI_SOURCE = pkt->head.src_comm_lrank;
    rhandle->s.MPI_ERROR  = MPI_SUCCESS;
    rhandle->s.count      = pkt->head.len;
    rhandle->from         = from; /* Needed for flow control */
    rhandle->partner	  = from;
    rhandle->cancel	  = MPID_USOCK_Eagerb_cancel_recv;
    rhandle->dev          = MPID_devset->active_dev; /* <-- remember this device! */
  
    /* Need to save msgrep for heterogeneous systems */
    MPID_DO_HETERO(rhandle->msgrep = (MPID_Msgrep_t)pkt->head.msgrep);
    if (pkt->head.len > 0) {
	rhandle->start	  = (void *)MALLOC( pkt->head.len );
	if (!rhandle->start) {
	    rhandle->s.MPI_ERROR = MPI_ERR_NOMEM;
	    rhandle->s.count = 0;
	    /* This is really pretty fatal. We could leave the message in the
	       system, which would cause a fatal error on the next receive.
	       So we just discard the message...*/
	    MPID_USOCK_ConsumeData(pkt->head.len,from);
	    rhandle->push = MPID_USOCK_Eagerb_unxrecv_start;
	    DSECTLEAVE
		return 1;
	}
	MPID_FLOW_MEM_READ(pkt->head.len,from);
    }
    if(MPID_USOCK_RecvFromChannelAsync( rhandle->start, pkt->head.len, from,rhandle->rid)) {
	if (rhandle->finish)
	    (rhandle->finish)( rhandle );
	rhandle->is_complete  = 1;
    } else	rhandle->is_complete  = 0;
    rhandle->push = MPID_USOCK_Eagerb_unxrecv_start;
    
    DSECTLEAVE
	return 0;
}


/* This routine is called when a message arrives and was expected */
int MPID_USOCK_Eagerb_irecv( rhandle, from, in_pkt )
MPIR_RHANDLE *rhandle;
int          from;
void         *in_pkt;
{
    DSECTION("MPID_USOCK_Eagerb_irecv");
    MPID_PKT_LONG_T *pkt = (MPID_PKT_LONG_T *)in_pkt;
    int    msglen, err = MPI_SUCCESS,remain;

    DSECTENTRYPOINT;

    MPID_USOCK_Test_device(MPID_devset->active_dev, "Eagerb_irecv");
    
    msglen = pkt->len;
    
    /* Check for truncation */
    MPID_CHK_MSGLEN(rhandle,msglen,err);
    MPID_FLOW_MEM_GET(pkt,from);
    MPID_FLOW_MEM_READ(msglen,from);
    MPID_FLOW_MEM_RECV(msglen,from);
    rhandle->from	  = from;
    rhandle->s.count	  = msglen;
    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->src_comm_lrank;
    rhandle->s.MPI_ERROR  = err;

    /*if(MPID_USOCK_RecvFromChannelAsync( rhandle->buf, msglen, from,rhandle->rid)) {*/
    MPID_USOCK_RecvFromChannel( rhandle->buf, msglen, from);
    if (rhandle->finish)
	(rhandle->finish)( rhandle );
    rhandle->is_complete = 1;
    rhandle->wait	 = 0;
    rhandle->test	 = 0;
    rhandle->cancel      = 0;
    /*} else {
      
      rhandle->is_complete = 0;
      rhandle->wait	 = MPID_USOCK_Eagern_WaitReceive;
      rhandle->test	 = MPID_USOCK_Eagern_TestReceive;
    }*/
    rhandle->push	 = 0;
    
    /*Discard remaining part of the message in the case of truncation*/
    if(msglen<pkt->len) {
	remain = pkt->len-msglen;
	MPID_USOCK_ConsumeData(remain,from);
    }
    
    DSECTLEAVE
	return err;
}

int MPID_USOCK_Eagern_WaitReceive (rhandle) 
MPIR_RHANDLE *rhandle;
{
    DSECTION("MPID_USOCK_Eagern_WaitReceive");

    DSECTENTRYPOINT;    
    
    /* get the device of this handle: (may be, there a multiple usock entities) */
    if( rhandle->dev!=0 )
      MPID_devset->active_dev = rhandle->dev;
    
    MPID_USOCK_Test_device(MPID_devset->active_dev, "Eagern_WaitReceive");

    MPID_USOCK_WaitTransfer(rhandle->rid);
    rhandle->is_complete = 1;
    if (rhandle->finish) 
	(rhandle->finish)( rhandle );
    
    DSECTLEAVE
	return MPI_SUCCESS;
}

int MPID_USOCK_Eagern_TestReceive (rhandle) 
MPIR_RHANDLE *rhandle;

{
    DSECTION("MPID_USOCK_Eagern_TestReceive");

    DSECTENTRYPOINT;

    /* get the device of this handle: (may be, there a multiple usock entities) */
    if( rhandle->dev!=0 )
      MPID_devset->active_dev = rhandle->dev;
    
    MPID_USOCK_Test_device(MPID_devset->active_dev, "Eagern_TestReceive");
    
    if (MPID_USOCK_TestTransfer( rhandle->rid )) {
	rhandle->is_complete = 1;
	if (rhandle->finish) 
	    (rhandle->finish)( rhandle );
	MPID_USOCK_ReceiveTransfer(-1,rhandle->rid);
    }
    
    DSECTLEAVE
	return MPI_SUCCESS;
}
