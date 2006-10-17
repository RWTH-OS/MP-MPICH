/* $Id$ */
#include "mpiimpl.h"
#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "reqalloc.h"


/* Prototype definitions */
MPID_Protocol *MPID_Tunnel_Eagern_setup ANSI_ARGS(( void ));
int MPID_Tunnel_Eagern_send ANSI_ARGS(( void *, int, int, int, int, int, MPID_Msgrep_t, struct MPIR_DATATYPE * ));
int MPID_Tunnel_Eagern_isend ANSI_ARGS(( void *, int, int, int, int, int, MPID_Msgrep_t, MPIR_SHANDLE *, struct MPIR_DATATYPE * ));
void MPID_Tunnel_Eagern_delete ANSI_ARGS(( MPID_Protocol * ));


/*
 * Definitions of the actual functions
 */

int MPID_Tunnel_Eagern_isend( buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank,
			 msgrep, shandle, dty )
void          *buf;
int           len, tag, context_id, src_comm_lrank, dest_dev_lrank;
MPID_Msgrep_t msgrep;
MPIR_SHANDLE *shandle;
struct MPIR_DATATYPE * dty;
{
    int mpi_errno;
    Meta_Header *tnl_msg = (Meta_Header *)buf;
    MPID_PKT_SHORT_T *pkt;
    MPID_Device *dev;
    int dest_grank;

    DEBUG_PRINT_MSG("Tunnel: Entering Eagern_isend");
	
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

	  mpi_errno = MPID_Protocol_call_isend(dev->eager->isend, pkt, tnl_msg->msg.MPI.count, tnl_msg->msg.MPI.src_comm_lrank, 
					       tnl_msg->msg.MPI.tag, tnl_msg->msg.MPI.context_id, 
					       dest_dev_lrank, msgrep, shandle, dty, dev);
	  MPID_devset->active_dev = me;
	}
#else
	mpi_errno = dev->eager->isend( pkt, tnl_msg->msg.MPI.count, tnl_msg->msg.MPI.src_comm_lrank, 
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

	  mpi_errno = MPID_Protocol_call_isend(dev->eager->isend, buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank, msgrep, shandle, dty, dev );

	  MPID_devset->active_dev = me;
	}
#else
 	mpi_errno = dev->eager->isend( buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank, msgrep, shandle, dty );
#endif

    DEBUG_PRINT_MSG("Tunnel: Leaving Eagern_isend");
    
    return mpi_errno;
}

int MPID_Tunnel_Eagern_send( buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank,
			    msgrep, dty )
void          *buf;
int           len, tag, context_id, src_comm_lrank, dest_dev_lrank;
MPID_Msgrep_t msgrep;
struct MPIR_DATATYPE * dty;
{
    int mpi_errno;
    Meta_Header *tnl_msg = (Meta_Header *)buf;
    MPID_PKT_SHORT_T *pkt;
    MPID_Device *dev;
    int dest_grank;

    DEBUG_PRINT_MSG("Tunnel: Entering Eagern_send");

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

	  mpi_errno = MPID_Protocol_call_send(dev->eager->send, pkt, tnl_msg->msg.MPI.count, tnl_msg->msg.MPI.src_comm_lrank, 
					      tnl_msg->msg.MPI.tag, tnl_msg->msg.MPI.context_id, 
					      dest_dev_lrank, msgrep, dty, dev);

	  MPID_devset->active_dev = me;
	}
#else
	mpi_errno = dev->eager->send( pkt, tnl_msg->msg.MPI.count, tnl_msg->msg.MPI.src_comm_lrank, 
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

	  mpi_errno = MPID_Protocol_call_send(dev->eager->send, buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank, msgrep, dty, dev );

	  MPID_devset->active_dev = me;
	}
#else
 	mpi_errno = dev->eager->send( buf, len, src_comm_lrank, tag, context_id, dest_dev_lrank, msgrep, dty );
#endif
    
    DEBUG_PRINT_MSG("Tunnel: Leaving Eagern_send");
    
    return mpi_errno;
}


void MPID_Tunnel_Eagern_delete( p )
MPID_Protocol *p;
{
    FREE( p );
}


MPID_Protocol *MPID_Tunnel_Eagern_setup( void )
{
    MPID_Protocol *p;

    p = (MPID_Protocol *) MALLOC( sizeof(MPID_Protocol) );
    if (!p) return 0;

    p->send	   = MPID_Tunnel_Eagern_send;
    p->recv	   = 0;
    p->isend	   = MPID_Tunnel_Eagern_isend;
    p->wait_send   = 0;
    p->push_send   = 0;
    p->cancel_send = 0;
    p->irecv	   = 0;
    p->wait_recv   = 0;
    p->push_recv   = 0;
    p->cancel_recv = 0;
    p->do_ack      = 0;
    p->unex        = 0;
    p->delete      = MPID_Tunnel_Eagern_delete;

    return p;
}
