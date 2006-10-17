/* $Id$ */
#include "dev.h"
#include "mpiddev.h"

MPID_Protocol *MPID_Gateway_Eagern_setup( void  );
void MPID_Gateway_Eagern_delete( MPID_Protocol * );

void MPID_Gateway_Eagern_delete( p )
MPID_Protocol *p;
{
    free( p );
}

MPID_Protocol *MPID_Gateway_Eagern_setup( void )
{
    MPID_Protocol *p;

    p = (MPID_Protocol *)malloc( sizeof(MPID_Protocol) );
    if (!p) return 0;
    p->send	   = MPID_Gateway_Unified_send;
    p->recv	   = 0;
    p->isend	   = MPID_Gateway_Unified_isend;
    p->wait_send   = 0;
    p->push_send   = 0;
    p->cancel_send = 0;
    p->irecv	   = 0;
    p->wait_recv   = 0;
    p->push_recv   = 0;
    p->cancel_recv = 0;
    p->do_ack      = 0;
    p->unex        = 0;
    p->delete      = MPID_Gateway_Eagern_delete;


    return p;
}
