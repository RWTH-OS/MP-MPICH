/* $Id$
   Shared memory by rendezvous.  Messages are sent in one of two ways 
   (not counting the short in packet way):

   All of the data is copied into shared memory, the address is sent,
   and the receiver returns the shared memory.

   Only some of the data is copied into shared memory, and the address
   for THAT is sent.  Getting the rest of the message is done by sending
   the original sender a message (or possibly many messages) to 
   provide the rest of the data.  Not yet implemented.

 */
   
#include "mpiimpl.h"
#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "reqalloc.h"

/* Prototype definitions */
MPID_Protocol *MPID_Tunnel_Rndvn_setup ANSI_ARGS(( void ));
int MPID_Tunnel_Rndvn_send ANSI_ARGS(( void *, int, int, int, int, int, MPID_Msgrep_t, struct MPIR_DATATYPE * ));
int MPID_Tunnel_Rndvn_isend ANSI_ARGS(( void *, int, int, int, int, int, MPID_Msgrep_t, MPIR_SHANDLE *, struct MPIR_DATATYPE * ));
void MPID_Tunnel_Rndvn_delete ANSI_ARGS(( MPID_Protocol * ));


int MPID_Tunnel_Rndvn_isend( buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank,
			    msgrep, shandle, dty )
void          *buf;
int           len, tag, context_id, src_comm_lrank, dest_dev_lrank;
MPID_Msgrep_t msgrep;
MPIR_SHANDLE *shandle;
struct MPIR_DATATYPE * dty;
{
    int mpi_errno;
    MPID_PKT_SHORT_T *pkt;
    Meta_Header *tnl_msg = (Meta_Header *)buf;
    MPID_Device *dev;
    int dest_grank;
    
    DEBUG_PRINT_MSG("Tunnel: Entering Rndvn_isend()");
    
    /* device relative ranks for ch_tunnel are ranks in MPI_COMM_HOST; get global
       rank of destination process */
    dest_grank = MPIR_COMM_HOST->lrank_to_grank[dest_dev_lrank];

    /* get device via which to send to dest_grank */
    dev = MPID_Tunnel_native_dev[dest_grank];

    /* messages to be tunneled should all be in MPI_COMM_ALL */
    if( (tag == MPIR_MPIMSG_TAG) && (context_id == MPIR_ALL_PT2PT_CONTEXT) ) {
	/* this msg is a meta msg to be tunneled */
	pkt = (MPID_PKT_SHORT_T *)(tnl_msg + 1);
#if 1
	/*
	 |   XXX
	 |   Since the (multi-) device instanciability, always the adi2-wrapper functions
	 |   must be called instead of the direct device function pointers!
	 |
	 |   (remove this comment and the obsolete part in future!)
	 */
	{
	  MPID_Device *me = MPID_devset->active_dev;

	  mpi_errno = MPID_Protocol_call_isend(dev->rndv->isend, pkt, tnl_msg->msg.MPI.count, tnl_msg->msg.MPI.src_comm_lrank, 
				      tnl_msg->msg.MPI.tag, tnl_msg->msg.MPI.context_id, 
					       dest_dev_lrank, msgrep, shandle, dty, dev);
	  MPID_devset->active_dev = me;
	}
#else
	mpi_errno = dev->rndv->isend( pkt, tnl_msg->msg.MPI.count, tnl_msg->msg.MPI.src_comm_lrank, 
				      tnl_msg->msg.MPI.tag, tnl_msg->msg.MPI.context_id, 
				      dest_dev_lrank, msgrep, shandle, dty);
#endif
    }
    else
#if 1
	/*
	 |   XXX
	 |   Since the (multi-) device instanciability, always the adi2-wrapper functions
	 |   must be called instead of the direct device function pointers!
	 |
	 |   (remove this comment and the obsolete part in future!)
	 */
	{
	  MPID_Device *me = MPID_devset->active_dev;

	  mpi_errno = MPID_Protocol_call_isend(dev->rndv->isend, buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank, msgrep, shandle, dty, dev );

	  MPID_devset->active_dev = me;
	}
#else
 	mpi_errno = dev->rndv->isend( buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank, msgrep, shandle, dty );
#endif
    
    return mpi_errno;
}


int MPID_Tunnel_Rndvn_send( buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank,
			   msgrep , dty )
void          *buf;
int           len, tag, context_id, src_comm_lrank, dest_dev_lrank;
MPID_Msgrep_t msgrep;
struct MPIR_DATATYPE * dty;
{
    int mpi_errno;
    MPID_PKT_SHORT_T *pkt;
    Meta_Header *tnl_msg = (Meta_Header *)buf;
    MPID_Device *dev;
    int dest_grank;
    
    DEBUG_PRINT_MSG("Tunnel: Entering Rndvn_send()");
    
    /* device relative ranks for ch_tunnel are ranks in MPI_COMM_HOST; get global
       rank of destination process */
    dest_grank = MPIR_COMM_HOST->lrank_to_grank[dest_dev_lrank];

    /* get device via which to send to dest_grank */
    dev = MPID_Tunnel_native_dev[dest_grank];

    /* messages to be tunneled should all be in MPI_COMM_ALL */
    if( (tag == MPIR_MPIMSG_TAG) && (context_id == MPIR_ALL_PT2PT_CONTEXT) ) {
	/* this msg is a meta msg to be tunneled */
	pkt = (MPID_PKT_SHORT_T *)(tnl_msg + 1);
#if 1
	/*
	 |   XXX
	 |   Since the (multi-) device instanciability, always the adi2-wrapper functions
	 |   must be called instead of the direct device function pointers!
	 |
	 |   (remove this comment and the obsolete part in future!)
	 */
	{
	  MPID_Device *me = MPID_devset->active_dev;

	  mpi_errno = MPID_Protocol_call_send(dev->rndv->send, pkt, tnl_msg->msg.MPI.count, tnl_msg->msg.MPI.src_comm_lrank, 
					       tnl_msg->msg.MPI.tag, tnl_msg->msg.MPI.context_id, 
					       dest_dev_lrank, msgrep, dty, dev);
	  MPID_devset->active_dev = me;
	}
#else
	mpi_errno = dev->rndv->send( pkt, tnl_msg->msg.MPI.count, tnl_msg->msg.MPI.src_comm_lrank, 
				      tnl_msg->msg.MPI.tag, tnl_msg->msg.MPI.context_id, 
				      dest_dev_lrank, msgrep, dty);
#endif
    }
    else
#if 1
	/*
	 |   XXX
	 |   Since the (multi-) device instanciability, always the adi2-wrapper functions
	 |   must be called instead of the direct device function pointers!
	 |
	 |   (remove this comment and the obsolete part in future!)
	 */
	{
	  MPID_Device *me = MPID_devset->active_dev;

	  mpi_errno = MPID_Protocol_call_send(dev->rndv->send, buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank, msgrep, dty, dev );
	}
#else
 	mpi_errno = dev->rndv->send( buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank, msgrep, dty );
#endif
    
    DEBUG_PRINT_MSG("Tunnel: Leaving Rndvn_send()");

    return mpi_errno;
}

void MPID_Tunnel_Rndvn_delete( p )
MPID_Protocol *p;
{
    FREE( p );
}

/*
 * The only routing really visable outside this file; it defines the
 * Blocking Rendezvous protocol.
 */
MPID_Protocol *MPID_Tunnel_Rndvn_setup( void )
{
    MPID_Protocol *p;

    p = (MPID_Protocol *) MALLOC( sizeof(MPID_Protocol) );
    if (!p) return 0;
    p->send	   = MPID_Tunnel_Rndvn_send;
    p->recv	   = 0;
    p->isend	   = MPID_Tunnel_Rndvn_isend;
    p->wait_send   = 0;
    p->push_send   = 0;
    p->cancel_send = 0;
    p->irecv	   = 0;
    p->wait_recv   = 0;
    p->push_recv   = 0;
    p->cancel_recv = 0;
    p->do_ack      = 0;
    p->unex        = 0;
    p->delete      = MPID_Tunnel_Rndvn_delete;

    return p;
}
