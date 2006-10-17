/*
 *  $Id: usockneager.c 3227 2005-03-15 13:51:10Z carsten $
 *
 */

#include "mpid.h"
#include "usockdev.h"
#include "mpimem.h"
#include "reqalloc.h"
#include "usockpriv.h"
#include "usockdebug.h"

#include "mydebug.h" 

#define WSA_OVERLAPPED     1
#define WSA_NOT_OVERLAPPED 0

/*
   Nonblocking, eager send/recv.
   These are ALWAYS for long messages.  Short messages are always
   handled in blocking eager mode.  

   We COULD write the eager code to use nonblocking receives as well
   as sends, but that is too much like the rendezvous code.  Instead,
   this code RECEIVES using the eager blocking approach but SENDS with
   nonblocking sends.
 */

/* Prototype definitions */
int MPID_USOCK_Eagern_isend ANSI_ARGS(( void *, int, int, int, int, int, 
				     MPID_Msgrep_t,
				     MPIR_SHANDLE *, struct MPIR_DATATYPE * ));
int MPID_USOCK_Eagern_send ANSI_ARGS(( void *, int, int, int, int, int, 
                                     MPID_Msgrep_t, struct MPIR_DATATYPE *));
int MPID_USOCK_Eagern_cancel_send ANSI_ARGS(( MPIR_SHANDLE * ));
int MPID_USOCK_Eagern_wait_send ANSI_ARGS(( MPIR_SHANDLE * ));
int MPID_USOCK_Eagern_test_send ANSI_ARGS(( MPIR_SHANDLE * ));
void MPID_USOCK_Eagern_delete ANSI_ARGS(( MPID_Protocol * ));


extern int MPID_USOCK_Eagerb_recv ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
extern int MPID_USOCK_Eagerb_irecv ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
extern int MPID_USOCK_Eagerb_save ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
extern int MPID_USOCK_Eagerb_unxrecv_start ANSI_ARGS(( MPIR_RHANDLE *, void * ));

/*
 * Definitions of the actual functions
 */

int MPID_USOCK_Eagern_isend( buf, len, src_lrank, tag, context_id, dest,
			    msgrep, shandle, dtype )
			    void          *buf;
int           len, tag, context_id, src_lrank, dest;
MPID_Msgrep_t msgrep;
MPIR_SHANDLE  *shandle;
struct MPIR_DATATYPE *dtype;
{
    DSECTION("MPID_USOCK_Eagern_isend");
    int              pkt_len;
    int res;
    MPID_PKT_LONG_T  *pkt;
    MPID_PKT_LONG_T  pkt_cont;
    WSABUF SBuf[1];   

    DSECTENTRYPOINT;
    
    /*   WORKAROUND:
     |   To avoid validity problems with the pkt-buffer and since its shipping
     |   is always a blocking WSASend() (also a workaround), we placed it on
     |   stack (see above) instead of calling this omitted malloc():
     |
     |   pkt = MALLOC(sizeof(MPID_PKT_LONG_T));
     */

    pkt=&pkt_cont;

    shandle->start=pkt;  
    pkt->mode	   = MPID_PKT_LONG;
    pkt_len	   = sizeof(MPID_PKT_LONG_T); 
    pkt->context_id = context_id;
    pkt->lrank	   = src_lrank;
    pkt->tag	   = tag;
    pkt->len	   = len;
    MPID_AINT_SET(pkt->send_id,shandle);
    MPID_DO_HETERO(pkt->msgrep = msgrep);

    MPID_USOCK_DEBUG_PRINT_SEND_PKT("S Sending extra-long message",pkt);
    pkt->TCP_TYPE=CTRL;
    pkt->TCP_SIZE=sizeof(MPID_PKT_LONG_T);
    MPID_PKT_PACK( pkt, sizeof(MPID_PKT_LONG_T), dest );

    /* Send as packet only */
    MPID_DRAIN_INCOMING_FOR_TINY(1);

    /*   WORKAROUND:
     |   To avoid synchronisation faults when using the overlapped mode of WASSend()
     |   we send all control messages via the blocking mode, while the user parts of
     |   the messages mut be sent in an overlapped manner to avoid deadlocks:
     */

    /* firstly send the control message NOT_OVERLAPPD: */
    SBuf[0].len=pkt_len;
    SBuf[0].buf=(char*)pkt;

    res=MPID_USOCK_SendChannel(SBuf, pkt_len, 1, dest, shandle->sid, WSA_NOT_OVERLAPPED);
    
    /* after this send the user message OVERLAPPED in a separated WSASend: */
    SBuf[0].len=len;
    SBuf[0].buf=(char*)buf;

    res=MPID_USOCK_SendChannel(SBuf, len, 1, dest, shandle->sid, WSA_OVERLAPPED);

    /*   This is old call with two buffers to send:
     |   res=MPID_USOCK_Send(SBuf, pkt_len+len, 1, dest, shandle->sid, WSA_OVERLAPPED);
     |
     |   (One day we'll may return to this ...)
     */

    if(!res)  /* The send operation completed already */ 
    {
      	shandle->is_complete = 1;

	/*   Omitted, since the pkt-buffer is placed on the stack:
	 |
         |   FREE(shandle->start);
	 */

	if (shandle->finish) (shandle->finish)( shandle );
    }
    else
    {
	shandle->wait	 = MPID_USOCK_Eagern_wait_send;
	shandle->test	 = MPID_USOCK_Eagern_test_send;
	shandle->finish	 = 0;
	shandle->is_complete = 0;
    }
    shandle->cancel = 0;
    shandle->partner = dest;
    
    DSECTLEAVE
      return MPI_SUCCESS;
}


int MPID_USOCK_Eagern_send( buf, len, src_lrank, tag, context_id, dest,
			   msgrep, dtype)
    void          *buf;
int           len, tag, context_id, src_lrank, dest;
MPID_Msgrep_t msgrep;
struct MPIR_DATATYPE *dtype;
{
    DSECTION("MPID_USOCK_Eagern_send");
    MPIR_SHANDLE  shandle;
    int err;

    DSECTENTRYPOINT;

    MPID_USOCK_DEBUG_INIT_STRUCT(&shandle,sizeof(shandle));
    MPIR_SET_COOKIE((&shandle),MPIR_REQUEST_COOKIE);
    MPID_SendInit( &shandle );
    shandle.finish=0;
    err = MPID_USOCK_Eagern_isend(buf,len,src_lrank,tag,context_id,dest,msgrep,&shandle, dtype );
    if(!shandle.is_complete) {
	MPID_USOCK_DEBUG_PRINT_MSG("Waiting for finish")
	    shandle.wait( &shandle );
	MPID_USOCK_DEBUG_PRINT_MSG("Finished")
    }
    
    DSECTLEAVE
	return err;
}

int MPID_USOCK_Eagern_cancel_send( shandle )
MPIR_SHANDLE *shandle;
{
    return 0;
}

int MPID_USOCK_Eagern_test_send( shandle )
MPIR_SHANDLE *shandle;
{
    DSECTION("MPID_USOCK_Eagern_test_send");
    /* Test for completion */
    
    DSECTENTRYPOINT;

    if (!shandle->is_complete) {
	if (MPID_USOCK_TestSend( shandle->sid ))
        {
	    shandle->is_complete = 1;
	
            /*   Omitted, since the pkt-buffer is placed on the stack:
             |
             |   FREE(shandle->start);
	     */
	
             if (shandle->finish) (shandle->finish)( shandle );
	}
    }

    DSECTLEAVE
	return MPI_SUCCESS;
}

int MPID_USOCK_Eagern_wait_send( shandle )
MPIR_SHANDLE *shandle;
{
    DSECTION("MPID_USOCK_Eagern_wait_send");
    
    DSECTENTRYPOINT;
   
    if (!shandle->is_complete) {
#ifdef MPID_LIMITED_BUFFERS
    /* We do this to keep us from blocking in a wait in the event that
	   we must handle some incoming messages before we can execute the
	wait. */
	while (!MPID_USOCK_TestSend(shandle->sid))
	    (void) MPID_DeviceCheck( MPID_NOTBLOCKING );
	/* Once we have it, the message is completed */
#else
	MPID_USOCK_WaitSend( shandle->sid );
#endif
	shandle->is_complete = 1; 

        /*   Omitted, since the pkt-buffer is placed on the stack:
         |
         |   FREE(shandle->start);
	 */

	if (shandle->finish) 
	    (shandle->finish)( shandle );
    }

    DSECTLEAVE
	return MPI_SUCCESS;
}

void MPID_USOCK_Eagern_delete( p )
MPID_Protocol *p;
{
    FREE( p );
}

MPID_Protocol *MPID_USOCK_Eagern_setup()
{
    DSECTION("MPID_USOCK_Eagern_setup");
    MPID_Protocol *p;

    DSECTENTRYPOINT;

    p = (MPID_Protocol *) MALLOC( sizeof(MPID_Protocol) );
    if (!p) {DSECTLEAVE return 0;}
    p->send	   = MPID_USOCK_Eagern_send;
    p->recv	   = MPID_USOCK_Eagerb_recv;
    p->isend	   = MPID_USOCK_Eagern_isend;
    p->wait_send   = 0;
    p->push_send   = 0;
    p->cancel_send = 0;
    p->irecv	   = MPID_USOCK_Eagerb_irecv;
    p->wait_recv   = 0;
    p->push_recv   = 0;
    p->cancel_recv = 0;
    p->do_ack      = 0;
    p->unex        = MPID_USOCK_Eagerb_save;
    p->delete      = MPID_USOCK_Eagern_delete;

    DSECTLEAVE
	return p;
}
