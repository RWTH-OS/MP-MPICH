/* $Id$ */

#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "reqalloc.h"

/* Shared memory by rendezvous.  Messages are sent in one of two ways 
   (not counting the short in packet way):

   All of the data is copied into shared memory, the address is sent,
   and the receiver returns the shared memory.

   Only some of the data is copied into shared memory, and the address
   for THAT is sent.  Getting the rest of the message is done by sending
   the original sender a message (or possibly many messages) to 
   provide the rest of the data.  Not yet implemented.

 */
   
/*
 * Definitions of the actual functions
 */



void MPID_Gateway_Rndvn_delete( p )
MPID_Protocol *p;
{
    DEBUG_PRINT_MSG("Gateway: Entering Rndvn_delete");

    FREE( p );

    DEBUG_PRINT_MSG("Gateway: Leaving Rndvn_delete");
}

/*
 * The only routing really visable outside this file; it defines the
 * Blocking Rendezvous protocol.
 */
MPID_Protocol *MPID_Gateway_Rndvn_setup( void )
{
    MPID_Protocol *p;

    DEBUG_PRINT_MSG("Gateway: Entering Rndvn_setup()");

    /* this is the external protocol */
    p = (MPID_Protocol *) MALLOC( sizeof(MPID_Protocol) );
    if (!p) return 0;
    /* for big rndv messages it may be better to use pipelining */
#ifdef META_PIPELINE
    p->send        = MPID_Gateway_Pipelined_send;
#else
    p->send        = MPID_Gateway_Unified_send;
#endif
    p->recv	   = 0;
    p->isend       = MPID_Gateway_Unified_isend;
    p->wait_send   = 0;
    p->push_send   = 0;
    p->cancel_send = 0;
    p->irecv	   = 0;
    p->wait_recv   = 0;
    p->push_recv   = 0;
    p->cancel_recv = 0;
    p->do_ack      = 0;
    p->unex        = 0;
    p->delete      = MPID_Gateway_Rndvn_delete;

    DEBUG_PRINT_MSG("Gateway: Leaving Rndvn_setup()");

    return p;
}
