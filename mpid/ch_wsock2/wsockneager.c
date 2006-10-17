/*
 *  $Id$
 *
 */

#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "reqalloc.h"
#include "wsock2debug.h"

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
int MPID_WSOCK_Eagern_isend ANSI_ARGS(( void *, int, int, int, int, int, 
				     MPID_Msgrep_t,
				     MPIR_SHANDLE * ,struct MPIR_DATATYPE*) );
int MPID_WSOCK_Eagern_send ANSI_ARGS(( void *, int, int, int, int, int, 
				     MPID_Msgrep_t,struct MPIR_DATATYPE*));
int MPID_WSOCK_Eagern_cancel_send ANSI_ARGS(( MPIR_SHANDLE * ));
int MPID_WSOCK_Eagern_wait_send ANSI_ARGS(( MPIR_SHANDLE * ));
int MPID_WSOCK_Eagern_test_send ANSI_ARGS(( MPIR_SHANDLE * ));
void MPID_WSOCK_Eagern_delete ANSI_ARGS(( MPID_Protocol * ));


extern int MPID_WSOCK_Eagerb_recv ANSI_ARGS(( MPIR_RHANDLE *, int, void *));
extern int MPID_WSOCK_Eagerb_irecv ANSI_ARGS(( MPIR_RHANDLE *, int, void *));
extern int MPID_WSOCK_Eagerb_save ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
extern int MPID_WSOCK_Eagerb_unxrecv_start ANSI_ARGS(( MPIR_RHANDLE *, void * ));

/*
 * Definitions of the actual functions
 */

int MPID_WSOCK_Eagern_isend( buf, len, src_lrank, tag, context_id, dest,
			    msgrep, shandle, dtypeptr)
			    void          *buf;
int           len, tag, context_id, src_lrank, dest;
MPID_Msgrep_t msgrep;
MPIR_SHANDLE  *shandle;
struct MPIR_DATATYPE* dtypeptr;
{
    int              pkt_len;
    int res;
    MPID_PKT_LONG_T  *pkt;
    WSABUF SBuf[2];
#ifdef WSOCK2
    if(dest==MPID_MyWorldRank)
	return MPID_SHMEM_Eagern_send_local(buf, len, src_lrank, tag, context_id, dest,
	msgrep, shandle,MPID_NOTBLOCKING,dtypeptr );
#endif
    pkt = MALLOC(sizeof(MPID_PKT_LONG_T));
    shandle->start=pkt;
    
    pkt->mode	   = MPID_PKT_LONG;
    pkt_len	   = sizeof(MPID_PKT_LONG_T); 
    pkt->context_id = context_id;
    pkt->lrank	   = src_lrank;
    pkt->tag	   = tag;
    pkt->len	   = len;
    MPID_AINT_SET(pkt->send_id,shandle);
    MPID_DO_HETERO(pkt->msgrep = msgrep);
    
    DEBUG_PRINT_SEND_PKT("S Sending extra-long message",pkt);
    pkt->TCP_TYPE=CTRL;
    pkt->TCP_SIZE=sizeof(MPID_PKT_LONG_T);
    MPID_PKT_PACK( pkt, sizeof(MPID_PKT_LONG_T), dest );
    
    /* Send as packet only */
    MPID_DRAIN_INCOMING_FOR_TINY(1);
    
    SBuf[0].len=pkt_len;
    SBuf[0].buf=(char*)pkt;
    SBuf[1].len=len;
    SBuf[1].buf=(char*)buf;
    res=MPID_WSOCK_Send(SBuf,pkt_len+len,2,dest,shandle->sid);
    if(!res) { /* The send operation completed already*/
	shandle->is_complete = 1;
	FREE(shandle->start);
	if (shandle->finish) 
	    (shandle->finish)( shandle );
    } else {	
	shandle->wait	 = MPID_WSOCK_Eagern_wait_send;
	shandle->test	 = MPID_WSOCK_Eagern_test_send;
	shandle->finish	 = 0;
	shandle->is_complete = 0;
    }
    shandle->cancel = 0;
    shandle->partner = dest;
    return MPI_SUCCESS;
}


int MPID_WSOCK_Eagern_send( buf, len, src_lrank, tag, context_id, dest,
			   msgrep,dtypeptr)
			   void          *buf;
int           len, tag, context_id, src_lrank, dest;
MPID_Msgrep_t msgrep;
struct MPIR_DATATYPE* dtypeptr;

{
    int              pkt_len;
    MPID_PKT_LONG_T  pkt;
    MPIR_SHANDLE  shandle;
    WSABUF SBuf[2];
#ifdef WSOCK2
    if(dest==MPID_MyWorldRank) {
	shandle.finish =0;
	return MPID_SHMEM_Eagern_send_local(buf, len, src_lrank, tag, context_id, dest,
	msgrep, &shandle,MPID_BLOCKING,dtypeptr );
    }
#endif
   
    pkt.mode	   = MPID_PKT_LONG;
    pkt_len	   = sizeof(MPID_PKT_LONG_T); 
    pkt.context_id = context_id;
    pkt.lrank	   = src_lrank;
    pkt.tag	   = tag;
    pkt.len	   = len;
    MPID_DO_HETERO(pkt.msgrep = msgrep);
    
    DEBUG_PRINT_SEND_PKT("S Sending extra-long message",&pkt);
    pkt.TCP_TYPE=CTRL;
    pkt.TCP_SIZE=sizeof(MPID_PKT_LONG_T);
    MPID_PKT_PACK( &pkt, sizeof(MPID_PKT_LONG_T), dest );
    
    /* Send as packet only */
    MPID_DRAIN_INCOMING_FOR_TINY(1);
    
    SBuf[0].len=pkt_len;
    SBuf[0].buf=(char*)&pkt;
    SBuf[1].len=len;
    SBuf[1].buf=(char*)buf;
    MPID_WSOCK_SendBlock(SBuf,pkt_len+len,2,dest);

    return MPI_SUCCESS;
}

int MPID_WSOCK_Eagern_cancel_send( shandle )
MPIR_SHANDLE *shandle;
{
    return 0;
}

int MPID_WSOCK_Eagern_test_send( shandle )
MPIR_SHANDLE *shandle;
{
    /* Test for completion */
    
    if (!shandle->is_complete) {
	if (MPID_WSOCK_TestSend( shandle->sid )) {
	    shandle->is_complete = 1;
	    FREE(shandle->start);
	    if (shandle->finish) 
		(shandle->finish)( shandle );
	}
    }
    return MPI_SUCCESS;
}

int MPID_WSOCK_Eagern_wait_send( shandle )
MPIR_SHANDLE *shandle;
{
    if (!shandle->is_complete) {
#ifdef MPID_LIMITED_BUFFERS
    /* We do this to keep us from blocking in a wait in the event that
	   we must handle some incoming messages before we can execute the
	wait. */
	while (!MPID_WSOCK_TestSend(shandle->sid))
	    (void) MPID_DeviceCheck( MPID_NOTBLOCKING );
	/* Once we have it, the message is completed */
#else
	MPID_WSOCK_WaitSend( shandle->sid );
#endif
	shandle->is_complete = 1;
	FREE(shandle->start);
	if (shandle->finish) 
	    (shandle->finish)( shandle );
    }
    return MPI_SUCCESS;
}

void MPID_WSOCK_Eagern_delete( p )
MPID_Protocol *p;
{
    FREE( p );
}

MPID_Protocol *MPID_WSOCK_Eagern_setup()
{
    MPID_Protocol *p;

    p = (MPID_Protocol *) MALLOC( sizeof(MPID_Protocol) );
    if (!p) return 0;
    p->send	   = MPID_WSOCK_Eagern_send;
    p->recv	   = MPID_WSOCK_Eagerb_recv;
    p->isend	   = MPID_WSOCK_Eagern_isend;
    p->wait_send   = 0;
    p->push_send   = 0;
    p->cancel_send = 0;
    p->irecv	   = MPID_WSOCK_Eagerb_irecv;
    p->wait_recv   = 0;
    p->push_recv   = 0;
    p->cancel_recv = 0;
    p->do_ack      = 0;
    p->unex        = MPID_WSOCK_Eagerb_save;
    p->delete      = MPID_WSOCK_Eagern_delete;

    return p;
}
