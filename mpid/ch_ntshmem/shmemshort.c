
#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "mpipt2pt.h"
#include "reqalloc.h"
#include "ntshmemdebug.h"

/* Prototype definitions */
int MPID_SHMEM_Eagerb_send_short ANSI_ARGS(( void *, int, int, int, int, int, 
					     MPID_Msgrep_t,struct MPIR_DATATYPE* ));
int MPID_SHMEM_Eagerb_isend_short ANSI_ARGS(( void *, int, int, int, int, int, 
					      MPID_Msgrep_t, MPIR_SHANDLE *,struct MPIR_DATATYPE* ));
int MPID_SHMEM_Eagerb_recv_short ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
int MPID_SHMEM_Eagerb_save_short ANSI_ARGS(( MPIR_RHANDLE *, int, void *));
int MPID_SHMEM_Eagerb_unxrecv_start_short ANSI_ARGS(( MPIR_RHANDLE *, void * ));
void MPID_SHMEM_Eagerb_short_delete ANSI_ARGS(( MPID_Protocol * ));
/*
 * Definitions of the actual functions
 */



int MPID_SHMEM_Eagerb_isend_short( buf, len, src_lrank, tag, context_id, dest,
				  msgrep,shandle, dtypeptr )
void          *buf;
int           len, tag, context_id, src_lrank, dest;
MPIR_SHANDLE  *shandle;
struct MPIR_DATATYPE* dtypeptr;

MPID_Msgrep_t msgrep;
{
    //MPID_PKT_SHORT_TSH spkt;
    MPID_PKT_SHORT_TSH *pkt;// = &spkt;
    
    //MPID_STAT_ENTRY(mpid_send_short);
    
    if(dest==MPID_MyWorldRank)
	return MPID_SHMEM_Eagern_send_local(buf, len, src_lrank, tag, context_id, dest,
		    msgrep, shandle,MPID_NOTBLOCKING,dtypeptr );
        
    MPID_SHMEM_GetSendPacket((MPID_PKT_TSH**)&pkt,dest);
    
    pkt->mode	    = MPID_PKT_SHORTSH;
    pkt->context_id = context_id;
    pkt->lrank	    = src_lrank;
    pkt->tag	    = tag;
    pkt->len	    = len;
    MPID_AINT_SET(pkt->send_id,shandle);
    
    shandle->is_complete = 1;
    shandle->cancel = 0;
    shandle->partner = dest;
    
    DEBUG_PRINT_SEND_PKT("S Sending",pkt);
    
    if (len > 0) {
	if(dtypeptr) {
	    MPIR_Pack_flat_type(pkt->buffer,buf,len,dtypeptr);
	} else {
	    MEMCPY( pkt->buffer, buf, len );
	    DEBUG_PRINT_PKT_DATA("S Writing data to buf",pkt);
	}
    }
    /* Always use a blocking send for short messages.
    (May fail with systems that do not provide adequate
    buffering.  These systems should switch to non-blocking sends)
    */
    DEBUG_PRINT_SEND_PKT("S Sending message in a single packet",pkt);
    
    /* In case the message is marked as non-blocking, indicate that we don't
    need to wait on it.  We may also want to use nonblocking operations
    to send the envelopes.... */
#if defined(MPID_STATISTICS) && defined(LATENCY)
    SMI_Get_ticks(&pkt->time);
#endif
    MPID_SHMEM_SetPacketReady((MPID_PKT_TSH*)pkt,dest);
    
    DEBUG_PRINT_MSG("S Sent message in a single packet");
    
    //MPID_STAT_EXIT(mpid_send_short);
    return MPI_SUCCESS;
}

int MPID_SHMEM_Eagerb_send_short( buf, len, src_lrank, tag, context_id, dest,
			 msgrep,dtypeptr )
void          *buf;
int           len, tag, context_id, src_lrank, dest;
MPID_Msgrep_t msgrep;
struct MPIR_DATATYPE* dtypeptr;
{
    //MPID_PKT_SHORT_TSH spkt;
    MPID_PKT_SHORT_TSH *pkt;// = &spkt;
    MPIR_SHANDLE  shandle;
    MPID_STAT_ENTRY(mpid_send_short);

    if(dest==MPID_MyWorldRank) {
	DEBUG_INIT_STRUCT(&shandle,sizeof(shandle));
	shandle.finish=0;
	MPID_STAT_EXIT(mpid_send_short);
	return MPID_SHMEM_Eagern_send_local(buf, len, src_lrank, tag, context_id, dest,
	    msgrep, &shandle,MPID_BLOCKING,dtypeptr );
    } 
    
    MPID_SHMEM_GetSendPacket((MPID_PKT_TSH**)&pkt,dest);
    
    pkt->mode	    = MPID_PKT_SHORTSH;
    pkt->context_id = context_id;
    pkt->lrank	    = src_lrank;
    pkt->tag	    = tag;
    pkt->len	    = len;
    /*MPID_AINT_SET(pkt->send_id,0);*/
    
    
    //DEBUG_PRINT_SEND_PKT("S Sending",pkt);
    
    if (len > 0) {
	if(dtypeptr) {
	    pkt->len	    *= dtypeptr->size;
	    MPIR_Pack_flat_type(pkt->buffer,buf,len,dtypeptr);
	} else {
	    MEMCPY( pkt->buffer, buf, len );
	    	    DEBUG_PRINT_PKT_DATA("S Writing data to buf",pkt);
	}
    }
    //len +=  sizeof(MPID_PKT_HEAD_T)+sizeof(MPID_Aint);
    /* Always use a blocking send for short messages.
    (May fail with systems that do not provide adequate
    buffering.  These systems should switch to non-blocking sends)
    */
    //DEBUG_PRINT_SEND_PKT("S Sending message in a single packet",pkt);
    
    /* In case the message is marked as non-blocking, indicate that we don't
    need to wait on it.  We may also want to use nonblocking operations
    to send the envelopes.... */
    //MPID_SHMEM_SendControl( (MPID_PKT_TSH*)pkt, len , dest );
    
    
    
#if defined(MPID_STATISTICS) && defined(LATENCY)
    SMI_Get_ticks(&pkt->time);
#endif
    MPID_SHMEM_SetPacketReady((MPID_PKT_TSH*)pkt,dest);
    //DEBUG_PRINT_MSG("S Sent message in a single packet");
    
    MPID_STAT_EXIT(mpid_send_short);
    
    return MPI_SUCCESS;
}

int MPID_SHMEM_Eagerb_recv_short( rhandle, from_grank, in_pkt )
MPIR_RHANDLE *rhandle;
int          from_grank;
void         *in_pkt;
{
    MPID_PKT_SHORT_TSH *pkt = (MPID_PKT_SHORT_TSH *)in_pkt;
    int          msglen,dummy=0;
    int          err = MPI_SUCCESS;
    
    MPID_STAT_ENTRY(mpid_short_recv);
    
    msglen		  = pkt->len;
    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;
    MPID_CHK_MSGLEN(rhandle,msglen,err);
    if (msglen > 0) {
#if 0
    This does not work, since MPI_BOTTOM is zero,
    which causes rhandle->start to be zero too, even if the message 
    has been packed by MPI_SEND!!!
	if(rhandle->count && !rhandle->start) {
	    /* This is a noncontig message, that we have to handle here...*/
	    if(rhandle->len>msglen) {
		rhandle->start = rhandle->buf;
		MPIR_Unpack_flat_restricted(&rhandle->start,pkt->buffer,&dummy,msglen,rhandle->datatype);
	    } else {
		MPIR_Unpack_flat_type(rhandle->buf,pkt->buffer,rhandle->count,rhandle->datatype);
	    }
	} else 
#endif
	{
	    MEMCPY( rhandle->buf, pkt->buffer, msglen ); 
	}
    }
    rhandle->s.count	  = msglen;
    rhandle->s.MPI_ERROR  = err;
    if (rhandle->finish) {
	(rhandle->finish)( rhandle );
    }
    MPID_PKT_READY_CLR(&(pkt->ready));
    rhandle->is_complete = 1;
    
    MPID_STAT_EXIT(mpid_short_recv);
    return err;
}

int MPID_SHMEM_Eagerb_cancel_recv(MPIR_RHANDLE*);

int MPID_SHMEM_Eagerb_cancel_recv(runex)
MPIR_RHANDLE *runex;
{
	if (runex->s.count > 0)
		FREE( runex->start );
	MPID_Recv_free( runex );
	return 0;
}

/* 
 * This routine is called when it is time to receive an unexpected
 * message
 */
int MPID_SHMEM_Eagerb_unxrecv_start_short( rhandle, in_runex )
MPIR_RHANDLE *rhandle;
void         *in_runex;
{
    MPIR_RHANDLE *runex = (MPIR_RHANDLE *)in_runex;
    int          msglen, err = 0,dummy=0;
    
    msglen = runex->s.count;
    MPID_CHK_MSGLEN(rhandle,msglen,err);
    /* Copy the data from the local area and free that area */
    if (runex->s.count > 0) {
#if 0
    This does not work, since MPI_BOTTOM is zero,
    which causes rhandle->start to be zero too, even if the message 
    has been packed by MPI_SEND!!!
	if(rhandle->count && !rhandle->start) {
	    /* This is a noncontig message, that we have to handle here...*/
	    if(rhandle->len>msglen) {
		rhandle->start = rhandle->buf;
		MPIR_Unpack_flat_restricted(&rhandle->start,runex->start,&dummy,msglen,rhandle->datatype);
	    } else {
		MPIR_Unpack_flat_type(rhandle->buf,runex->start,rhandle->count,rhandle->datatype);
	    }
	} else 
#endif
	{
	    MEMCPY( rhandle->buf, runex->start, msglen );
	}
	FREE( runex->start );
    }
    rhandle->s		 = runex->s;
    rhandle->s.MPI_ERROR = err;
    rhandle->wait	 = 0;
    rhandle->test	 = 0;
    rhandle->push	 = 0;
    rhandle->is_complete = 1;
    if (rhandle->finish) 
	(rhandle->finish)( rhandle );
    MPID_Recv_free( runex );
    
    return err;
}

/* Save an unexpected message in rhandle */
int MPID_SHMEM_Eagerb_save_short( rhandle, from, in_pkt )
MPIR_RHANDLE *rhandle;
int          from;
void         *in_pkt;
{
    MPID_PKT_SHORT_TSH   *pkt = (MPID_PKT_SHORT_TSH *)in_pkt;

    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;
    rhandle->s.MPI_ERROR  = 0;
    rhandle->s.count      = pkt->len;
	MPID_AINT_SET(rhandle->send_id,pkt->send_id);
    rhandle->is_complete  = 1;
    if (pkt->len > 0) {
	rhandle->start	  = (void *)MALLOC( pkt->len );
	if (!rhandle->start) {
	    rhandle->s.MPI_ERROR = MPI_ERR_INTERN;
	    MPID_PKT_READY_CLR(&(pkt->ready));
	    return 1;
	}
	MEMCPY( rhandle->start, pkt->buffer, pkt->len );
    }
    rhandle->push = MPID_SHMEM_Eagerb_unxrecv_start_short;
	rhandle->cancel = MPID_SHMEM_Eagerb_cancel_recv;
    MPID_PKT_READY_CLR(&(pkt->ready));
    return 0;
}

void MPID_SHMEM_Eagerb_short_delete( p )
MPID_Protocol *p;
{
    FREE( p );
}

MPID_Protocol *MPID_SHMEM_Short_setup()
{
    MPID_Protocol *p;

    p = (MPID_Protocol *) MALLOC( sizeof(MPID_Protocol) );
    if (!p) return 0;
    p->send	   = MPID_SHMEM_Eagerb_send_short;
    p->recv	   = MPID_SHMEM_Eagerb_recv_short;
    p->isend	   = MPID_SHMEM_Eagerb_isend_short;
    p->wait_send   = 0;
    p->push_send   = 0;
    p->cancel_send = 0;
    p->irecv	   = 0;
    p->wait_recv   = 0;
    p->push_recv   = 0;
    p->cancel_recv = 0;
    p->do_ack      = 0;
    p->unex        = MPID_SHMEM_Eagerb_save_short;
    p->delete      = MPID_SHMEM_Eagerb_short_delete;

    return p;
}
