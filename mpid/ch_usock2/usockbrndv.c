/*
 *  $Id: usockbrndv.c 3189 2005-03-03 17:12:54Z carsten $
 *
 */
#include "mpid.h"
#include "usockdev.h"
#include "mpimem.h"
#include "reqalloc.h"
#include "usockdebug.h"


#define _DEBUG_EXTERN_REC
#include "mydebug.h"

/* Blocking Rendezvous */

/* Prototype definitions */
int MPID_USOCK_Rndvb_send ANSI_ARGS(( void *, int, int, int, int, int, 
					  MPID_Msgrep_t, struct MPIR_DATATYPE * ));
int MPID_USOCK_Rndvb_isend ANSI_ARGS(( void *, int, int, int, int, int, 
				    MPID_Msgrep_t, MPIR_SHANDLE *, struct MPIR_DATATYPE * ));
int MPID_USOCK_Rndvb_irecv ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
int MPID_USOCK_Rndvb_save ANSI_ARGS(( MPIR_RHANDLE *, int, void *));
int MPID_USOCK_Rndvb_unxrecv_start ANSI_ARGS(( MPIR_RHANDLE *, void * ));
int MPID_USOCK_Rndvb_unxrecv_end ANSI_ARGS(( MPIR_RHANDLE * ));
int MPID_USOCK_Rndvb_unxrecv_test_end ANSI_ARGS(( MPIR_RHANDLE * ));
int MPID_USOCK_Rndvb_ok_to_send  ANSI_ARGS(( MPID_Aint,int, MPID_RNDV_T, int ));
int MPID_USOCK_Rndvb_ack ANSI_ARGS(( void *, int ));
int MPID_USOCK_Rndvn_send_wait ANSI_ARGS(( MPIR_SHANDLE *));
int MPID_USOCK_Rndvn_send_test ANSI_ARGS(( MPIR_SHANDLE *));
int MPID_USOCK_Rndvn_send_wait_ack ANSI_ARGS(( MPIR_SHANDLE *));
int MPID_USOCK_Rndvn_send_test_ack ANSI_ARGS(( MPIR_SHANDLE *));



#if defined(MPID_RNDV_SELF)
int MPID_USOCK_Rndvb_save_self ANSI_ARGS(( MPIR_RHANDLE *, int, void *));
int MPID_USOCK_Rndvb_unxrecv_start_self ANSI_ARGS(( MPIR_RHANDLE *, void * ));
#endif

void MPID_USOCK_Rndvb_delete ANSI_ARGS(( MPID_Protocol * ));

/* Globals for this protocol */
/* This should be state in the protocol/device ?? */
static int CurTag    = 1024;
static int TagsInUse = 0;
/*
 * Notes
 * In the case of sending a rendezvous message to self (source==destination),
 * there can be problems because the code expects to be able to send a 
 * request and then receive the requested data.  The sequence
 *  Send rendezvous (to self)
 *  Receive as unexpected
 *  Post receive and send "ok to send" (to self)
 *  Wait by entering blocking receive for message (from self)
 * can fail if the process does not receive its ok "ok to send" message.
 * We can fix this in two ways
 *  Add a DeviceCheck before trying to complete the rendezvous.
 *  Use a different set of routines for handling sends to self in the
 *   rendezvous case.
 * While the second case looks like the obvious thing to do, one problem
 * with it is that some systems provide better self-to-self copy when using
 * their communication network.  We could take the position that these systems
 * have badly designed memory systems, but it could be a problem.  
 * See aditest12 for an example.
 */

/*
 * Definitions of the actual functions
 */

int MPID_USOCK_Rndvb_isend( buf, len, src_lrank, tag, context_id, dest,
			   msgrep, shandle, dtype )
    void          *buf;
int           len, tag, context_id, src_lrank, dest;
MPID_Msgrep_t msgrep;
MPIR_SHANDLE  *shandle;
struct MPIR_DATATYPE *dtype;
{
    DSECTION("MPID_USOCK_Rndvb_isend");
    MPID_PKT_REQUEST_SEND_T  pkt;
    
    DSECTENTRYPOINT;

    shandle->partner = dest;
    shandle->cancel  = 0;

    pkt.mode	   = MPID_PKT_REQUEST_SEND;
    pkt.context_id = context_id;
    pkt.lrank	   = src_lrank;
    pkt.tag	   = tag;
    pkt.len	   = len;
    MPID_AINT_SET(pkt.send_id,shandle);
    
    MPID_DO_HETERO(pkt.msgrep = (int)msgrep);
    
    
    
    /* Store info in the request for completing the message */
    shandle->is_complete     = 0;
    shandle->start	         = buf;
    shandle->bytes_as_contig = len;
    /* Set the test/wait functions */
    shandle->wait            = MPID_USOCK_Rndvn_send_wait_ack;
    shandle->test            = MPID_USOCK_Rndvn_send_test_ack;
    shandle->finish          = 0;
    
    
    /* shandle->finish must NOT be set here; it must be cleared/set
    when the request is created but it isn't AFAIS*/
    
    MPID_USOCK_DEBUG_PRINT_BASIC_SEND_PKT("S Sending rndv message",&pkt)
	MPID_PKT_PACK( &pkt, sizeof(pkt), dest );
    MPID_DRAIN_INCOMING_FOR_TINY(1);
    MPID_n_pending++;
    MPID_SendControlBlock( (MPID_PKT_T*)&pkt, sizeof(pkt), dest );
    
    DSECTLEAVE
	return MPI_SUCCESS;
}

int MPID_USOCK_Rndvb_send( buf, len, src_lrank, tag, context_id, dest,
			 msgrep, dtype )
void          *buf;
int           len, tag, context_id, src_lrank, dest;
MPID_Msgrep_t msgrep;
struct MPIR_DATATYPE *dtype;
{
    DSECTION("MPID_USOCK_Rndvb_send");
    MPIR_SHANDLE shandle;

    DSECTENTRYPOINT;

    MPID_USOCK_DEBUG_INIT_STRUCT(&shandle,sizeof(shandle));
    MPIR_SET_COOKIE((&shandle),MPIR_REQUEST_COOKIE);
    MPID_SendInit( &shandle );

    MPID_USOCK_Rndvb_isend( buf, len, src_lrank, tag, context_id, dest,
			 msgrep, &shandle, dtype );
	MPID_USOCK_DEBUG_PRINT_MSG2("Wait pointer is: %x",shandle.wait);
    if(!shandle.is_complete) {
		MPID_USOCK_DEBUG_TEST_FCN(shandle.wait,"req->wait");
		shandle.wait( &shandle );
	}

    DSECTLEAVE
	return MPI_SUCCESS;
}


/*
 * This is the routine called when a packet of type MPID_PKT_REQUEST_SEND is
 * seen and the receive has been posted.
 */
int MPID_USOCK_Rndvb_irecv( rhandle, from, in_pkt )
MPIR_RHANDLE *rhandle;
int          from;
void         *in_pkt;
{
    DSECTION("MPID_USOCK_Rndvb_irecv");
    MPID_PKT_REQUEST_SEND_T *pkt = (MPID_PKT_REQUEST_SEND_T *)in_pkt;
    int    msglen, err = MPI_SUCCESS;
#if defined(MPID_RNDV_SELF)
    MPIR_SHANDLE *shandle;
#endif
    MPID_RNDV_T rtag;
    
    DSECTENTRYPOINT;
    
    MPID_USOCK_DEBUG_PRINT_MSG("R Starting rndvb irecv");
    
    /* A request packet is a little larger than the basic packet size and 
    may need to be unpacked (in the heterogeneous case) */
    MPID_PKT_UNPACK( (MPID_PKT_HEAD_T *)in_pkt + 1, 
	sizeof(MPID_PKT_REQUEST_SEND_T) - sizeof(MPID_PKT_HEAD_T),
	from );
    
    msglen = pkt->len;
    
    /* Check for truncation */
    MPID_CHK_MSGLEN(rhandle,msglen,err)

    rhandle->s.count	  = msglen;
    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;
    rhandle->s.MPI_ERROR  = err;
#if defined(MPID_RNDV_SELF)
    if (from == MPID_USOCK_Data_global.MyWorldRank) {
	MPID_USOCK_DEBUG_PRINT_MSG("R Starting a receive transfer from self");
	MPID_AINT_GET(shandle,pkt->send_id);
#ifdef MPIR_HAS_COOKIES
	if (shandle->cookie != MPIR_REQUEST_COOKIE) {
	    fprintf( stderr, "shandle is %lx\n", (long)shandle );
	    fprintf( stderr, "shandle cookie is %lx\n", shandle->cookie );
	    MPID_USOCK_DEBUG_PRINT_SHANDLE( stderr, shandle )
	    MPID_Abort( (struct MPIR_COMMUNICATOR *)0, 1, "MPI internal", 
		"Bad address in Rendezvous send (irecv-self)" );
	}
#endif	
	/* Copy directly from the shandle */
	MEMCPY( rhandle->buf, shandle->start, shandle->bytes_as_contig );
	
	shandle->is_complete = 1;
	if (shandle->finish) 
	    (shandle->finish)( shandle );
	MPID_n_pending--;
	
	/* Update all of the rhandle information */
	rhandle->wait	 = 0;
	rhandle->test	 = 0;
	rhandle->push	 = 0;
	
	rhandle->is_complete = 1;
	if (rhandle->finish) 
	    (rhandle->finish)( rhandle );
	DSECTLEAVE
	    return err;
    }
#endif
    MPID_USOCK_DEBUG_PRINT_MSG("Starting a receive transfer in irecv");
    rhandle->from         = from;
    rhandle->send_id	  = pkt->send_id;
    /* KARSTEN: Added rhandle->buf and rhandle->s.count to CreateRecvTransfer */
    MPID_CreateRecvTransfer( rhandle->buf, rhandle->s.count, from, &rtag,rhandle->rid );
    MPID_USOCK_Rndvb_ok_to_send( rhandle->send_id,rhandle->s.count, rtag, from );
    rhandle->recv_handle = rtag;
    rhandle->wait	 = MPID_USOCK_Rndvb_unxrecv_end;
    rhandle->test	 = MPID_USOCK_Rndvb_unxrecv_test_end;
    rhandle->push	 = 0;
    rhandle->is_complete = 0;
    
    DSECTLEAVE
	return err;
}

int MPID_USOCK_Rndvb_cancel_recv(MPIR_RHANDLE*);

int MPID_USOCK_Rndvb_cancel_recv(runex)
MPIR_RHANDLE *runex;
{
/*	MPID_RecvFree( runex );  */
	return 0;
}

/* Save an unexpected message in rhandle */
int MPID_USOCK_Rndvb_save( rhandle, from, in_pkt )
MPIR_RHANDLE *rhandle;
int          from;
void         *in_pkt;
{
    DSECTION("MPID_USOCK_Rndvb_save");
    MPID_PKT_REQUEST_SEND_T   *pkt = (MPID_PKT_REQUEST_SEND_T *)in_pkt;
    
    DSECTENTRYPOINT;

    /* A request packet is a little larger than the basic packet size and 
    may need to be unpacked (in the heterogeneous case) */
    MPID_PKT_UNPACK( (MPID_PKT_HEAD_T *)in_pkt + 1, 
	sizeof(MPID_PKT_REQUEST_SEND_T) - sizeof(MPID_PKT_HEAD_T),
	from );
    
#if defined(MPID_RNDV_SELF)
    if (from == MPID_USOCK_Data_global.MyWorldRank) {
	DSECTLEAVE
	    return MPID_USOCK_Rndvb_save_self( rhandle, from, in_pkt );
    }
#endif
    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;
    rhandle->s.MPI_ERROR  = MPI_SUCCESS;
    rhandle->s.count      = rhandle->len = pkt->len;
    rhandle->is_complete  = 0;
    rhandle->from         = from;
    rhandle->send_id      = pkt->send_id;
    rhandle->partner	  = from;
    rhandle->cancel		  = MPID_USOCK_Rndvb_cancel_recv;
    MPID_DO_HETERO(rhandle->msgrep = (MPID_Msgrep_t)pkt->msgrep );
    /* Need to set the push etc routine to complete this transfer */
    rhandle->push = MPID_USOCK_Rndvb_unxrecv_start;
    MPID_USOCK_DEBUG_PRINT_MSG2("Saving Rhandle. s.count==%d:",rhandle->s.count);
    
    DSECTLEAVE
	return 0;
}

/*
 * This is an internal routine to return an OK TO SEND packet
 */
int MPID_USOCK_Rndvb_ok_to_send( send_id, len,rtag, from )
MPID_Aint   send_id;
int	    len;
MPID_RNDV_T rtag;
int         from;
{
    DSECTION("MPID_USOCK_Rndvb_ok_to_send");
    MPID_PKT_OK_TO_SEND_T pkt;

    DSECTENTRYPOINT;

    pkt.mode = MPID_PKT_OK_TO_SEND;
    /* MPID_AINT_SET(pkt.send_id,send_id); */
    pkt.len = len;
    pkt.send_id = send_id;
    pkt.recv_handle = rtag;

    MPID_USOCK_DEBUG_PRINT_BASIC_SEND_PKT("S Ok send", &pkt);
    MPID_PKT_PACK( &pkt, sizeof(pkt), from );
    MPID_SendControlBlock( (MPID_PKT_T*)&pkt, sizeof(pkt), from );
    
    DSECTLEAVE
	return MPI_SUCCESS;
}



/* 
 * This routine is called when it is time to receive an unexpected
 * message
 */
int MPID_USOCK_Rndvb_unxrecv_start( rhandle, in_runex )
MPIR_RHANDLE *rhandle;
void         *in_runex;
{
    DSECTION("MPID_USOCK_Rndvb_unxrecv_start");
    MPIR_RHANDLE *runex = (MPIR_RHANDLE *)in_runex;
    MPID_RNDV_T rtag;
    int msglen = runex->len,err=MPI_SUCCESS;

    DSECTENTRYPOINT;

    MPID_USOCK_DEBUG_PRINT_MSG2("Starting Rhandle. len==%u",rhandle->len);
    MPID_USOCK_DEBUG_PRINT_MSG2("runex len==%u",runex->len);
    /* Send a request back to the sender, then do the receive */
    
    MPID_CHK_MSGLEN(rhandle,msglen,err);

    /* KARSTEN: Added rhandle->buf and msglen to CreateRecvTransfer */
    MPID_CreateRecvTransfer( rhandle->buf, msglen, runex->from, &rtag,rhandle->rid);
    MPID_USOCK_Rndvb_ok_to_send( runex->send_id, msglen, rtag, runex->from );
    /* Now, we can either wait for the message to arrive here or
    wait until later (by testing for it in the "end" routine).
    If we wait for it here, we could deadlock if, for example,
    our "partner" is also starting the receive of an unexpected 
    message.
    
      Thus, we save the message tag and set the wait/test functions
      appropriately.
    */
    rhandle->s		 = runex->s;
    rhandle->s.count	 = msglen;
    rhandle->s.MPI_ERROR = err;
    rhandle->recv_handle = rtag;
    rhandle->wait	 = MPID_USOCK_Rndvb_unxrecv_end;
    rhandle->test	 = MPID_USOCK_Rndvb_unxrecv_test_end;
    rhandle->push	 = 0;
    rhandle->from    = runex->from;
    rhandle->cancel	 = 0;
    rhandle->is_complete = 0;
    
/*    MPID_RecvFree( runex );  */
    
    DSECTLEAVE
	return 0;
}

/* 
   This is the wait routine for a rendezvous message that was unexpected.
   A request for the message has already been sent.
 */
int MPID_USOCK_Rndvb_unxrecv_end( rhandle )
MPIR_RHANDLE *rhandle;
{
    DSECTION("MPID_USOCK_Rndvb_unxrecv_end");
    /* This is a blocking transfer */

    DSECTENTRYPOINT;
    
#if !defined(MPID_RNDV_SELF)
    MPID_DeviceCheck( MPID_NOTBLOCKING ); 
#endif
    MPID_USOCK_DEBUG_PRINT_MSG("Starting a receive transfer");
    /* We must guard against starting a blocking receive, particularly in
    a head-to-head rendezvous (each sends request to send, then ok to
    send, then waits in the RecvTransfer without either having RECEIVED
    the ok to send).  
    If p4 had a blocking probe, we could use that to keep from spinning 
    endlessly by waiting for any message; this handles the case of cycles 
    rather than head-to-head rendezvous */
    /*while (!MPID_USOCK_TestTransfer( rhandle->rid )) {
    MPID_DeviceCheck( MPID_NOTBLOCKING );
}*/
    MPID_USOCK_WaitTransfer(rhandle->rid);
    /*
    MPID_RecvTransfer( rhandle->buf, rhandle->s.count, 
    rhandle->from, 
    rhandle->recv_handle, rhandle->rid );
    */
    MPID_USOCK_DEBUG_PRINT_MSG("Completed receive transfer");
    rhandle->is_complete = 1;
    if (rhandle->finish) 
	(rhandle->finish)( rhandle );
    
    DSECTLEAVE
	return MPI_SUCCESS;
}

/* 
   This is the test routine for a rendezvous message that was unexpected.
   A request for the message has already been sent.
 */
int MPID_USOCK_Rndvb_unxrecv_test_end( rhandle )
MPIR_RHANDLE *rhandle;
{
    /* This is a blocking transfer */
    /* If the transfer is ready, do it, else just return */
    if (MPID_TestRecvTransfer( rhandle )) {
	MPID_USOCK_DEBUG_PRINT_MSG("Starting a receive transfer");
	MPID_RecvTransfer( rhandle->buf, rhandle->s.count, 
	    rhandle->from, 
	    rhandle->recv_handle, rhandle->rid );
	MPID_USOCK_DEBUG_PRINT_MSG("Completed receive transfer");
	rhandle->is_complete = 1;
	if (rhandle->finish) 
	    (rhandle->finish)( rhandle );
    }
    
    return MPI_SUCCESS;
}

/* This is the routine that is called when an "ok to send" packet is
   received. */
int MPID_USOCK_Rndvb_ack( in_pkt, from_grank )
void  *in_pkt;
int   from_grank;
{
    DSECTION("MPID_USOCK_Rndvb_ack");
    MPID_PKT_OK_TO_SEND_T *pkt = (MPID_PKT_OK_TO_SEND_T *)in_pkt;
    MPIR_SHANDLE *shandle=0;

    DSECTENTRYPOINT;

    /* A request packet is a little larger than the basic packet size and 
       may need to be unpacked (in the heterogeneous case) */
    MPID_PKT_UNPACK( (MPID_PKT_HEAD_T *)in_pkt + 1, 
		     sizeof(MPID_PKT_OK_TO_SEND_T) - sizeof(MPID_PKT_HEAD_T),
		     from_grank );

    MPID_AINT_GET(shandle,pkt->send_id);
    MPID_USOCK_DEBUG_PRINT_MSG2("OK_TO_SEND with len %u received",pkt->len);
  
    /*The receiver told us how many bytes to receive.
      We honor that...*/
    shandle->bytes_as_contig = pkt->len;

#ifdef MPIR_HAS_COOKIES
    if (shandle->cookie != MPIR_REQUEST_COOKIE) {
	fprintf( stderr, "shandle is %lx\n", (long)shandle );
	fprintf( stderr, "shandle cookie is %lx\n", shandle->cookie );
	MPID_USOCK_DEBUG_PRINT_SHANDLE( stderr, shandle )
	MPID_Abort( (struct MPIR_COMMUNICATOR *)0, 1, "MPI internal", 
		    "Bad address in Rendezvous send (ack)" );
    }
#endif	
    MPID_USOCK_DEBUG_PRINT_MSG("Sending data on channel");
#ifdef MPID_DEBUG_ALL
    if (MPID_DebugFlag) {
	FPRINTF( MPID_DEBUG_FILE, "[%d]S for ", MPID_USOCK_Data_global.MyWorldRank );
	MPID_USOCK_DEBUG_PRINT_SHANDLE( MPID_DEBUG_FILE, shandle )
    }
#endif
    MPID_n_pending--;
  
    if(MPID_USOCK_SendTransfer( shandle->start, shandle->bytes_as_contig, 
		from_grank, pkt->recv_handle,shandle->sid )<0) {
		MPID_USOCK_DEBUG_PRINT_MSG("Have to wait for completion");
		shandle->is_complete = 0;
		shandle->recv_handle = pkt->recv_handle; /* This is not needed, but anyway...*/
		shandle->wait	 = MPID_USOCK_Rndvn_send_wait;
		shandle->test	 = MPID_USOCK_Rndvn_send_test;
	} else {
		MPID_USOCK_DEBUG_PRINT_MSG("All data sent");
		shandle->wait	 = 0;
		shandle->test	 = 0;
		shandle->is_complete = 1;
		if (shandle->finish) 
			(shandle->finish)( shandle );
	}
    
    DSECTLEAVE
	return MPI_SUCCESS;
}

int MPID_USOCK_Rndvn_send_wait( shandle )
MPIR_SHANDLE *shandle;
{
    DSECTION("MPID_USOCK_Rndvn_send_wait");

    DSECTENTRYPOINT;

    MPID_USOCK_DEBUG_PRINT_MSG("Ending send transfer");
    MPID_USOCK_WaitSend(shandle->sid );
    shandle->is_complete = 1;
    if (shandle->finish) 
		(shandle->finish)( shandle );

    DSECTLEAVE
	return 0;
}

int MPID_USOCK_Rndvn_send_test( shandle )
MPIR_SHANDLE *shandle;
{
    DSECTION("MPID_USOCK_Rndvn_send_test");

    DSECTENTRYPOINT;
    
    MPID_USOCK_DEBUG_PRINT_MSG("Testing for end send transfer" );
    if (MPID_USOCK_TestSend( shandle->sid )) {
		shandle->is_complete = 1;
		if (shandle->finish) 
			(shandle->finish)( shandle );
    }
    
    DSECTLEAVE
	return 0;
}


/* These wait for the "ack" and then change the wait routine on the
   handle */
int MPID_USOCK_Rndvn_send_wait_ack( shandle )
MPIR_SHANDLE *shandle;
{
    DSECTION("MPID_USOCK_Rndvn_send_wait_ack");
    
    DSECTENTRYPOINT;

    MPID_USOCK_DEBUG_PRINT_MSG("Waiting for Rndvn ack");
    while (!shandle->is_complete && 
	   shandle->wait == MPID_USOCK_Rndvn_send_wait_ack) {
	MPID_DeviceCheck( MPID_BLOCKING );
    }
    if (!shandle->is_complete) {
	MPID_USOCK_DEBUG_TEST_FCN(shandle->wait,"shandle->wait");
	DSECTLEAVE
	    return (shandle->wait)( shandle );
    }

    DSECTLEAVE
	return 0;
}

int MPID_USOCK_Rndvn_send_test_ack( shandle )
MPIR_SHANDLE *shandle;
{
    DSECTION("MPID_USOCK_Rndvn_send_test_ack");

    DSECTENTRYPOINT;

    MPID_USOCK_DEBUG_PRINT_MSG("Testing for Rndvn ack" );
    if (!shandle->is_complete &&
	shandle->test == MPID_USOCK_Rndvn_send_test_ack) {
	MPID_DeviceCheck( MPID_NOTBLOCKING );
    }

    DSECTLEAVE
	return 0;
}


#if defined(MPID_RNDV_SELF)
/***************************************************************************/
/* This code handles rendezous messages to self                           
 */
/***************************************************************************/

/* Save an unexpected message in rhandle for sent to self */
int MPID_USOCK_Rndvb_save_self( rhandle, from, in_pkt )
MPIR_RHANDLE *rhandle;
int          from;
void         *in_pkt;
{
    DSECTION("MPID_USOCK_Rndvb_save_self");
    MPID_PKT_REQUEST_SEND_T   *pkt = (MPID_PKT_REQUEST_SEND_T *)in_pkt;

    DSECTENTRYPOINT;

    /* A request packet is a little larger than the basic packet size and 
       may need to be unpacked (in the heterogeneous case) */
    MPID_PKT_UNPACK( (MPID_PKT_HEAD_T *)in_pkt + 1, 
		     sizeof(MPID_PKT_REQUEST_SEND_T) - sizeof(MPID_PKT_HEAD_T),
		     from );
    
    MPID_USOCK_DEBUG_PRINT_MSG("Saving RNDV packet from self.");
    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;
    rhandle->s.MPI_ERROR  = 0;
    rhandle->s.count      = pkt->len;
    rhandle->is_complete  = 0;
    rhandle->from         = from;
    rhandle->send_id      = pkt->send_id;
    /* Note that the send_id is just the address of the sending handle
       IN OUR ADDRESS SPACE */
    /* Note that format MIGHT be different from native (e.g., packed data) */
    MPID_DO_HETERO(rhandle->msgrep = (MPID_Msgrep_t)pkt->msgrep );
    /* Need to set the push etc routine to complete this transfer */
    rhandle->push = MPID_USOCK_Rndvb_unxrecv_start_self;

    DSECTLEAVE
	return 0;
}

/* 
 * This routine is called when it is time to receive an unexpected
 * message.  This is simple; we can just copy the data with memcpy.
 * Once the memcpy is done, we mark the SEND as completed.
 */
int MPID_USOCK_Rndvb_unxrecv_start_self( rhandle, in_runex )
MPIR_RHANDLE *rhandle;
void         *in_runex;
{
    DSECTION("MPID_USOCK_Rndvb_unxrecv_start_self");
    MPIR_RHANDLE *runex = (MPIR_RHANDLE *)in_runex;
    MPIR_SHANDLE *shandle;

    DSECTENTRYPOINT;

    /* Get the source handle */
    MPID_AINT_GET(shandle,runex->send_id);
#ifdef MPIR_HAS_COOKIES
    if (shandle->cookie != MPIR_REQUEST_COOKIE) {
	fprintf( stderr, "shandle is %lx\n", (long)shandle );
	fprintf( stderr, "shandle cookie is %lx\n", shandle->cookie );
	MPID_USOCK_DEBUG_PRINT_SHANDLE( stderr, shandle )
	MPID_Abort( (struct MPIR_COMMUNICATOR *)0, 1, "MPI internal", 
		    "Bad address in Rendezvous send (unx_start_self)" );
    }
#endif	
    /* Copy directly from the shandle */
    MEMCPY( rhandle->buf, shandle->start, shandle->bytes_as_contig );

    shandle->is_complete = 1;
    if (shandle->finish) 
	(shandle->finish)( shandle );
    MPID_n_pending--;

    /* Update all of the rhandle information */
    rhandle->s		 = runex->s;
    rhandle->wait	 = 0;
    rhandle->test	 = 0;
    rhandle->push	 = 0;
    rhandle->from        = runex->from;

/*    MPID_RecvFree( runex ); */

    rhandle->is_complete = 1;
    if (rhandle->finish) 
	(rhandle->finish)( rhandle );
    
    DSECTLEAVE
	return 0;
}

/***************************************************************************/
#endif /* MPID_RNDV_SELF */

/* 
 * CancelSend 
 * This is fairly hard.  We need to send a "please_cancel_send", 
 * which, if the message is found in the unexpected queue, removes it.
 * However, if the message is being received at the "same" moment, the
 * ok_to_send and cancel_send messages could cross.  To handle this, the
 * receiver must ack the cancel_send message (making the success of the
 * cancel non-local).  There are even more complex protocols, but we won't
 * bother.
 * 
 * Don't forget to update MPID_n_pending as needed.
 */

void MPID_USOCK_Rndvb_delete( p )
MPID_Protocol *p;
{
    FREE( p );
}

/*
 * The only routing really visable outside this file; it defines the
 * Blocking Rendezvous protocol.
 */
MPID_Protocol *MPID_USOCK_Rndvb_setup()
{
    DSECTION("MPID_USOCK_Rndvb_setup");
    MPID_Protocol *p;

    DSECTENTRYPOINT;

    p = (MPID_Protocol *) MALLOC( sizeof(MPID_Protocol) );
    if (!p) { DSECTLEAVE return 0;}
    p->send	   = MPID_USOCK_Rndvb_send;
    p->recv	   = 0;
    p->isend	   = MPID_USOCK_Rndvb_isend;
    p->wait_send   = 0;
    p->push_send   = 0;
    p->cancel_send = 0;
    p->irecv	   = MPID_USOCK_Rndvb_irecv;
    p->wait_recv   = 0;
    p->push_recv   = 0;
    p->cancel_recv = 0;
    p->do_ack      = MPID_USOCK_Rndvb_ack;
    p->unex        = MPID_USOCK_Rndvb_save;
    p->delete      = MPID_USOCK_Rndvb_delete;

    DSECTLEAVE
	return p;
}
