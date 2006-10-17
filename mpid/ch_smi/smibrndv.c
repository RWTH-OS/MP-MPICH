/* $Id$ */

/* Rendez-vous protocol with synchronous, blocking data transfer via a ringbuffer */


#include "smirndv.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

typedef struct {
    MPID_PKT_RNDV_T pkt;
    void *target_addr;
    int from_grank;
    int len_left;
} MPID_SMI_BRNDV_PKT_T;

/* imports */
extern MPID_SMI_Rndv_int_t MPID_SMI_Rndv_int;
extern int MPID_SMI_Locregid_rndv;
extern MPID_dataqueue_t MPID_SMI_brndv_queue;

#define INIT_BRNDV_PKTS 10
#define INCR_BRNDV_PKTS 10
#define INIT_ALIGNBUFS  10
#define INCR_ALIGNBUFS  10

static MPID_SBHeader brndvpkt_allocator;
static MPID_SBHeader alignbuf_allocator;

void MPID_SMI_Brndv_memsetup( void )
{
    /* fixed-size-block memory manager */
    brndvpkt_allocator  = MPID_SBinit (sizeof(MPID_SMI_BRNDV_PKT_T), INIT_BRNDV_PKTS, INCR_BRNDV_PKTS);
    alignbuf_allocator = MPID_SBinit (MPID_SMI_STREAMSIZE, INIT_ALIGNBUFS, INCR_ALIGNBUFS);

    return;
}

void MPID_SMI_Brndv_memdelete( void )
{
    MPID_SBdestroy (brndvpkt_allocator);
    MPID_SBdestroy (alignbuf_allocator);

    return;
}


int MPID_SMI_Brndv_ack( void *in_pkt, int from_grank )
{
    MPID_PKT_RNDV_T *recv_pkt = (MPID_PKT_RNDV_T *)in_pkt;
    
    MPID_TRACE_CODE ("Brndv_ack", from_grank);
    MPID_SMI_DEBUG_PRINT_MSG("Entering Brndv_ack");

    switch (recv_pkt->mode) {
    case MPID_PKT_OK_TO_SEND_RDY:
	return MPID_SMI_Brndv_send_ack(in_pkt, from_grank);
	break;
    case MPID_PKT_CONT_RDY:
	return MPID_SMI_Brndv_recv_ack(in_pkt, from_grank);
	break;
    default:
	/* illegal type! */
	MPID_ABORT ("Illegal packet mode for rndv ack");
	break;
    }

    return MPI_SUCCESS;
}

/*
 * This is just isend/wait
 */
int MPID_SMI_Brndv_send( buf, len, src_lrank, tag, context_id, dest, msgrep, dtype )
    void          *buf;
    int           len, tag, context_id, src_lrank, dest;
    MPID_Msgrep_t msgrep;
    struct MPIR_DATATYPE *dtype;
{
    MPIR_SHANDLE shandle;

    MPID_STAT_ENTRY(brndv_send);
    MPID_TRACE_CODE("Brndv_send", dest);
    MPID_SMI_DEBUG_PRINT_MSG("Entering Brndv_send");

    MPID_SMI_DEBUG_INIT_STRUCT(&shandle,sizeof(shandle));

    MPID_Send_init( &shandle );
    shandle.finish = 0;
    MPID_SMI_Brndv_isend( buf, len, src_lrank, tag, context_id, dest, msgrep, &shandle, dtype);
    
    /* if sending to myself, everything may already be finished */
    if (!shandle.is_complete) {
	MPID_SMI_DEBUG_TEST_FCN(shandle.wait,"req->wait");
	shandle.wait( &shandle );
    }

    MPID_STAT_EXIT(brndv_send);
    return MPI_SUCCESS;
}


int MPID_SMI_Brndv_resume_send ( MPIR_SHANDLE *sh ) 
{
    return MPID_SMI_Brndv_isend (sh->start, sh->bytes_as_contig, sh->src_lrank, sh->tag,
				 sh->context_id, sh->partner, sh->msgrep, sh, sh->datatype);
}


/* Send a message anouncing the availablility of data.  An "ack" must be
 * sent by the receiver to initiate data transfers (the ack type is
 * MPID_PKT_OK_TO_SEND_RDY).
 */
int MPID_SMI_Brndv_isend( buf, len, src_lrank, tag, context_id, dest, msgrep, shandle, dtype )
	 void  *buf;
	 int    len, tag, context_id, src_lrank, dest;
	 MPID_Msgrep_t msgrep;
	 MPIR_SHANDLE *shandle;
	 struct MPIR_DATATYPE *dtype;
{
    MPID_SMI_CTRLPKT_T pkt_desc;
    MPID_PKT_RNDV_T prepkt;
    int err, flags = 0;
	
    MPID_STAT_ENTRY(brndv_isend);   
    MPID_TRACE_CODE ("Brndvn_isend", dest);
    MPID_SMI_DEBUG_PRINT_MSG("Entering Brndvn_isend");
    
    if (MPID_SMI_cfg.SENDSELF && dest == MPID_SMI_myid) {
	/* we can transfer the message directly into the receive buffer */
	return MPID_SMI_Isend_self (buf, len, src_lrank, tag, context_id, dest, msgrep, shandle );
    }
    
    /* Only a maximum number of concurrent send operations is allowed to 
       control resource usage. */
    if (MPID_SMI_Rndvsends_in_progress >= MPID_SMI_cfg.MAX_SENDS) {
	shandle->push = MPID_SMI_Brndv_resume_send;
	MPID_RNDV_POSTPONE_SEND( buf, len, src_lrank, tag, context_id, dest, msgrep, shandle, dtype );
	return MPI_SUCCESS;
    }
    
    ALLOC_RNDVINFO_SEND(shandle);
    
    shandle->recv_handle->mode = RNDV_SYNC;
    shandle->recv_handle->smi_regid_src = MPID_SMI_INVALID_REGID; 
    shandle->recv_handle->smi_regid_dest = MPID_SMI_INVALID_REGID; 
    flags = MPID_SMI_RNDV_PIOONLY;
	
    MPID_SMI_DEBUG_PRINT_MSG("S About to get pkt for request to send");
    MPID_GETSENDPKT (pkt_desc, sizeof(MPID_PKT_RNDV_T), &prepkt, 0, NULL, dest, 0);

    MPID_INIT_RNDVREQ_RDY_PREPKT(prepkt, context_id, src_lrank, tag, len, shandle,
				 0, 0, 0, 0, 0, flags);

    MPID_SMI_DEBUG_PRINT_SEND_PKT("S Sending rndv-get message", &prepkt, dest);
    while (MPID_SMI_SendControl(&pkt_desc) != MPI_SUCCESS)
		;
    
    /* Store info in the request for completing the message */
    shandle->is_complete     = 0;
    shandle->start	     = buf;
    shandle->bytes_as_contig = len;
    shandle->is_cancelled    = 0;
    shandle->cancel_complete = 0;
    shandle->partner         = dest;
    /* shandle->finish must NOT be set here; it must be cleared/set when the request is created */
    shandle->wait   = MPID_SMI_Rndv_send_wait_ack;
    shandle->test   = MPID_SMI_Rndv_send_test_ack;
    shandle->cancel = MPID_SMI_Rndv_cancel_send;
    shandle->datatype = dtype;
    shandle->is_valid = 1;
	
    RNDV_SEND_SCHEDULED(dest);
    MPID_STAT_EXIT(brndv_isend);

    return MPI_SUCCESS;
}


/* This is the routine called when a packet of type MPID_PKT_REQUEST_SEND_RDY is
   seen and the receive has been posted (it better should be!). */
int MPID_SMI_Brndv_irecv( rhandle, from_grank, in_pkt )
    MPIR_RHANDLE *rhandle;
    int          from_grank;
    void         *in_pkt;
{
    MPID_PKT_RNDV_T recv_pkt;
    MPID_SMI_CTRLPKT_T pkt_desc;
    MPID_PKT_RNDV_T prepkt;
    int len_local, msglen, sgmt_id, adpt_nbr, err, flags;
    ulong sgmt_offset;

    MPID_STAT_ENTRY (brndv_irecv);
    MPID_TRACE_CODE ("Bndv_irecv", from_grank);
    MPID_SMI_DEBUG_PRINT_MSG ("Entering Brndv_irecv");

    MPID_CHECK_COOKIE (rhandle);
    
    MEMCPY (&recv_pkt, in_pkt, sizeof(MPID_PKT_RNDV_T));
    MPID_SMI_FreeRecvPkt ((MPID_PKT_T *)in_pkt, from_grank, IS_CTRL_MSG);    
 
    msglen = recv_pkt.len;
    MPID_CHK_MSGLEN(rhandle,msglen,err);

    ALLOC_RNDVINFO_RECV(rhandle);

    rhandle->s.count	  = msglen;
    rhandle->s.MPI_TAG	  = recv_pkt.tag;
    rhandle->s.MPI_SOURCE = recv_pkt.lrank;
    rhandle->s.MPI_ERROR  = MPI_SUCCESS;
    rhandle->send_id	  = recv_pkt.send_id;
    rhandle->is_complete  = 0;
    rhandle->wait	  = MPID_SMI_Rndv_unxrecv_end;
    rhandle->test	  = MPID_SMI_Rndv_unxrecv_test_end;
    rhandle->cancel   = MPID_SMI_Rndv_cancel_recv;
    rhandle->partner  = from_grank;
    rhandle->from     = from_grank;
    rhandle->recv_handle->flags = recv_pkt.flags;
    rhandle->recv_handle->mode = RNDV_SYNC;
    rhandle->is_valid    = 1;
	
    if (MPID_SMI_Rndvrecvs_in_progress >= MPID_SMI_cfg.MAX_RECVS) {
	rhandle->push = MPID_SMI_Brndv_resume_recv;
	MPID_SMI_Recv_postpone (rhandle);
	return MPI_SUCCESS;
    }
		
    MPID_SMI_DEBUG_PRINT_MSG("R about to get packet for ok to send");
    MPID_GETSENDPKT (pkt_desc, sizeof(MPID_PKT_RNDV_T), &prepkt, 0, NULL, rhandle->from, 0);

    flags = 0;
    len_local = msglen = rhandle->s.count;
    MPID_SMI_Brndv_get_recvbuf (&sgmt_id, &sgmt_offset, &adpt_nbr, &len_local, rhandle->from); 
    rhandle->recv_handle->len_local = len_local;

    MPID_INIT_RNDVOK_RDY_PREPKT(prepkt, msglen, rhandle->send_id, sgmt_offset, rhandle, 
				sgmt_id, adpt_nbr, len_local, 0, flags);
    MPID_SMI_DEBUG_PRINT_SEND_PKT("R Sending ok-to-send message", &prepkt, pkt_desc.dest);	
    while (MPID_SMI_SendControl (&pkt_desc) != MPI_SUCCESS)
	;

    RNDV_RECV_SCHEDULED(rhandle->from);

    MPID_STAT_EXIT(brndv_irecv);
    return MPI_SUCCESS;
}


/* Save an unexpected message in rhandle. */
int MPID_SMI_Brndv_save( rhandle, from_grank, in_pkt )
	 MPIR_RHANDLE *rhandle;
	 int          from_grank;
	 void         *in_pkt;
{
    MPID_PKT_RNDV_T *pkt = (MPID_PKT_RNDV_T *)in_pkt;

    MPID_TRACE_CODE ("Brndv_save", from_grank);
    MPID_SMI_DEBUG_PRINT_MSG("Entering Brndv_save");
    MPID_SMI_DEBUG_PRINT_MSG("Saving info on unexpected message");

    ALLOC_RNDVINFO_RECV(rhandle);

    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;
    rhandle->s.MPI_ERROR  = 0;
    rhandle->s.count      = pkt->len;
    rhandle->partner      = from_grank;
    rhandle->from         = from_grank;
    rhandle->send_id      = pkt->send_id;
    /* Need to set the push etc routine to complete this transfer */
    rhandle->push         = MPID_SMI_Brndv_unxrecv_start;
    rhandle->cancel       = MPID_SMI_Rndv_cancel_recv;
    /* The sender tells us via the flags how he would like to transmit the data, and
       how he *can not* transmit it. We need to preserve this information! */
    rhandle->recv_handle->len_local = 0;
    rhandle->recv_handle->flags     = pkt->flags;

    rhandle->is_complete  = 0;	
    rhandle->is_valid     = 0; /* not yet valid for receiving data */

    MPID_SMI_FreeRecvPkt( (MPID_PKT_T *)pkt, from_grank, IS_CTRL_MSG);

    return MPI_SUCCESS;
}

int MPID_SMI_Brndv_resume_recv (rhandle, dummy)
    MPIR_RHANDLE *rhandle;
    void *dummy;
{
    MPID_SMI_CTRLPKT_T pkt_desc;
    MPID_PKT_RNDV_T prepkt;
    int len_local, msglen, sgmt_id, adpt_nbr, flags;
    ulong sgmt_offset;

    MPID_SMI_DEBUG_PRINT_MSG("R about to get packet for ok to send");
    MPID_GETSENDPKT (pkt_desc, sizeof(MPID_PKT_RNDV_T), &prepkt, 0, NULL, rhandle->from, 0);

    flags = 0;
    len_local = msglen = rhandle->s.count;
    MPID_SMI_Brndv_get_recvbuf (&sgmt_id, &sgmt_offset, &adpt_nbr, &len_local, rhandle->from); 
    rhandle->recv_handle->len_local = len_local;

    MPID_INIT_RNDVOK_RDY_PREPKT(prepkt, msglen, rhandle->send_id, sgmt_offset, rhandle, 
				sgmt_id, adpt_nbr, len_local, 0, flags);
    MPID_SMI_DEBUG_PRINT_SEND_PKT("R Sending ok-to-send message", &prepkt, pkt_desc.dest);	
    while (MPID_SMI_SendControl (&pkt_desc) != MPI_SUCCESS)
	;

    RNDV_RECV_SCHEDULED(rhandle->from);

    return MPI_SUCCESS;   
}


/*  This routine is called when it is time to receive an unexpected message. 
	Basically, it transfer the information from the rhandle that was created on 
	the unexpected recv into the user-supplied rhandle (aka MPI_Request). */
int MPID_SMI_Brndv_unxrecv_start( rhandle, in_runex )
    MPIR_RHANDLE *rhandle;
    void         *in_runex;
{
    MPIR_RHANDLE   *runex = (MPIR_RHANDLE *)in_runex;
    MPID_SMI_CTRLPKT_T pkt_desc;
    MPID_PKT_RNDV_T prepkt;
    int len_local, sgmt_id, adpt_nbr, msglen, flags;
    ulong sgmt_offset;

    MPID_TRACE_CODE ("Brndv_unxrecv_start", runex->from);

    rhandle->s		 = runex->s;
    rhandle->send_id     = runex->send_id;
    rhandle->wait	 = MPID_SMI_Rndv_unxrecv_end;
    rhandle->test	 = MPID_SMI_Rndv_unxrecv_test_end;
    rhandle->push	 = 0;
    rhandle->from        = runex->from;
    rhandle->partner     = runex->from;
    /* the message can not be canceled once we get here */
    rhandle->cancel	 = 0;
    rhandle->is_complete = 0;
    rhandle->recv_handle = runex->recv_handle;
    rhandle->is_valid    = 1;

    MPID_Recv_free(runex);

    if (MPID_SMI_Rndvrecvs_in_progress >= MPID_SMI_cfg.MAX_RECVS) {
	rhandle->push = MPID_SMI_Brndv_resume_recv;
	MPID_SMI_Recv_postpone (rhandle);
	return MPI_SUCCESS;
    }
		
    MPID_SMI_DEBUG_PRINT_MSG("R about to get packet for ok to send");
    MPID_GETSENDPKT (pkt_desc, sizeof(MPID_PKT_RNDV_T), &prepkt, 0, NULL, rhandle->from, 0);

    flags = 0;
    len_local = msglen = rhandle->s.count;
    MPID_SMI_Brndv_get_recvbuf (&sgmt_id, &sgmt_offset, &adpt_nbr, &len_local, rhandle->from); 
    rhandle->recv_handle->len_local = len_local;

    MPID_INIT_RNDVOK_RDY_PREPKT(prepkt, msglen, rhandle->send_id, sgmt_offset, rhandle, 
				sgmt_id, adpt_nbr, len_local, 0, flags);
    MPID_SMI_DEBUG_PRINT_SEND_PKT("R Sending ok-to-send message", &prepkt, pkt_desc.dest);	
    while (MPID_SMI_SendControl (&pkt_desc) != MPI_SUCCESS)
	;

    RNDV_RECV_SCHEDULED(rhandle->from);

    return MPI_SUCCESS;
}


/* To avoid deadlocks, a receiver which is blocked on reading because the
   sender does not arrive at writing data puts his current state into a BRNDV_PKT
   and this PKT into a queue before calling Check_device(). This gives recursively
   called recv-functions the chance to complete upper-call-stack located recvs
   if they themselves should be blocked without any new control-packets arriving
   (which could also resolve the deadlock). */
static void brndv_recv_part()
{
    MPID_SMI_BRNDV_PKT_T *recv_pkt;
    ulong cpy_len = 0, misalign_size;
    ulong *roffset_ptr, roffset, *woffset_ptr, woffset;
    int ptrmem_size, databuf_size;
    char *target_addr, *rndv_base_addr, *rndv_data_addr;

    if ((recv_pkt = (MPID_SMI_BRNDV_PKT_T *)MPID_dataqueue_dequeue(MPID_SMI_brndv_queue)) != NULL ) {
	/* transform offset into pointer */
	rndv_base_addr = (MPID_SMI_use_localseg[recv_pkt->from_grank]) ? 
	    MPID_SMI_rndv_shmempool + recv_pkt->pkt.sgmt_offset :
	    MPID_SMI_rndv_scipool + recv_pkt->pkt.sgmt_offset;

	/* Rebuild buffer and pointer configuration. */
	/* XXX: Couldn't this be stored in the related BRNDV packet? */
	misalign_size = (size_t)rndv_base_addr % MPID_SMI_STREAMSIZE;
	rndv_base_addr += (misalign_size == 0) ? 0 : (MPID_SMI_STREAMSIZE - misalign_size);

	woffset_ptr = (ulong *)(rndv_base_addr + MPID_SMI_STREAMSIZE - sizeof(ulong *));
	roffset_ptr = woffset_ptr + MPID_SMI_STREAMSIZE/sizeof(ulong);
	woffset = *woffset_ptr;
	roffset = *roffset_ptr;

	ptrmem_size = 2*MPID_SMI_STREAMSIZE;
	rndv_data_addr = rndv_base_addr + ptrmem_size;
	databuf_size = (recv_pkt->pkt.len_avail - ptrmem_size)/ MPID_SMI_STREAMSIZE 
	  * MPID_SMI_STREAMSIZE;
	target_addr = (char *)(recv_pkt->target_addr);

	/* Check if any new data has arrived for this transfer; if yes, copy it */
	if (recv_pkt->len_left > MPID_SMI_STREAMSIZE 
	    && (woffset - roffset > MPID_SMI_STREAMSIZE || roffset - woffset > MPID_SMI_STREAMSIZE)) {
	  if (recv_pkt->len_left == recv_pkt->pkt.len)
	    RNDV_RECV_STARTED(recv_pkt->from_grank);
	  
	  /* Now, data is available. Read it with wrap-around of ptrs. */
	  if (woffset > roffset) {
	    /* no wrap-around - read a single block. */
	    cpy_len = woffset - roffset - MPID_SMI_STREAMSIZE;
	    MEMCPY_R(target_addr, rndv_data_addr + roffset, cpy_len);
	    roffset      += cpy_len; 
	    *roffset_ptr  = roffset;
	    target_addr  += cpy_len;
	  } else 
	    if (woffset < roffset) {
	      /* We need to wrap around the end of the buffer. We read the remainder
		 of the buffer now, the rest wil be read in the next iteration. */
	      cpy_len = databuf_size - roffset;
	      if (woffset == 0)
		cpy_len -= MPID_SMI_STREAMSIZE;
	      MEMCPY_R(target_addr, rndv_data_addr + roffset, cpy_len);
	      roffset      = (roffset + cpy_len) % databuf_size;
	      *roffset_ptr = roffset;
	      target_addr += cpy_len;
	    }
	}
	
	recv_pkt->target_addr = (char *)recv_pkt->target_addr + cpy_len;
	recv_pkt->len_left -= cpy_len;
	MPID_dataqueue_enqueue( MPID_SMI_brndv_queue, recv_pkt );
    }
    
    return;
}


int MPID_SMI_Brndv_recv_ack(in_pkt, from_grank)
void *in_pkt;
int from_grank;
{
    MPID_SMI_BRNDV_PKT_T *recv_pkt;
    MPIR_RHANDLE *rhandle=0;
    int len_left, ptrmem_size, databuf_size, misalign_size;
    ulong cpy_len = 0;
    char *source_addr, *rndv_base_addr, *rndv_data_addr, *target_addr;
    ulong *roffset_ptr;
    volatile ulong *woffset_ptr;
    ulong woffset, roffset;

    MPID_STAT_ENTRY(brndv_ack_recv);
    MPID_TRACE_CODE ("Brndv_recv_ack", from_grank);
    MPID_SMI_DEBUG_PRINT_MSG("Entering Brndv_recv_ack");

    recv_pkt = (MPID_SMI_BRNDV_PKT_T *)MPID_SBalloc(brndvpkt_allocator);
    MEMCPY (&(recv_pkt->pkt), in_pkt, sizeof(MPID_PKT_RNDV_T));
    MPID_SMI_FreeRecvPkt( (MPID_PKT_T *)in_pkt, from_grank, IS_CTRL_MSG);

    MPID_AINT_GET(rhandle, (recv_pkt->pkt).recv_id);
    MPID_CHECK_COOKIE (rhandle);
    VALIDATE_HANDLE (rhandle);

    /* transform offset into pointer */
    rndv_base_addr = MPID_SMI_use_localseg[from_grank] ?
	MPID_SMI_rndv_shmempool + recv_pkt->pkt.sgmt_offset :
	MPID_SMI_rndv_scipool + recv_pkt->pkt.sgmt_offset;
    misalign_size = (size_t)rndv_base_addr % MPID_SMI_STREAMSIZE;
    rndv_base_addr += (misalign_size == 0) ? 0 : (MPID_SMI_STREAMSIZE - misalign_size);

    /* initalize addresses & sizes */
    ptrmem_size = 2 * MPID_SMI_STREAMSIZE;
    woffset_ptr = (volatile ulong *)(rndv_base_addr + MPID_SMI_STREAMSIZE - sizeof(ulong *));
    roffset_ptr = (ulong *)(woffset_ptr + MPID_SMI_STREAMSIZE/sizeof(ulong));
    roffset = *roffset_ptr;

    rndv_data_addr = rndv_base_addr + ptrmem_size;
    databuf_size   = (rhandle->recv_handle->len_local - ptrmem_size)/ MPID_SMI_STREAMSIZE 
	* MPID_SMI_STREAMSIZE;
    source_addr    = rndv_data_addr;
    len_left       = recv_pkt->pkt.len;    

    /* XXX hack: if non-contigous transfer is enabled, we expect direct_pack_ff for
       rendez-vous messages - but the blocking rndv does (not yet) support this. 
       Therefore, we need to allocate the unpack-buffer at this point (normally, this
       would have been done when setting up the receive! */
    if (rhandle->buf == NULL) {
	ALLOCATE(rhandle->buf, void *, rhandle->datatype->extent);
    } 
    target_addr = (char *)rhandle->buf;

    while (1) {
	/* This is the ptr to the write-offset controlled/advanced by the sender. 
	   XXX: Minimal integrity checking applied: check if the write offset
	   is legal which means it does not exceed the available ring buffer. */
	do {
	    woffset = *woffset_ptr;
	} while (woffset >= databuf_size);

	while (woffset == roffset) {
	    /* This means no data is available! */
	    if (MPID_SMI_myid < from_grank) {
		/* Store the current state of the transfer in the pending-recv-queue. */
		recv_pkt->target_addr = target_addr;
		recv_pkt->from_grank  = from_grank;
		recv_pkt->len_left    = len_left;
		MPID_dataqueue_enqueue( MPID_SMI_brndv_queue, recv_pkt );

		/* Check for new control messages to avoid deadlock; if no new messages
		   did arrive, process the queue of pending recvs. The combination 
		   of the two techniques will avoid deadlocks. */
		if (MPID_DeviceCheck( MPID_NOTBLOCKING ) == -1)
		    brndv_recv_part();

		/* Get back our own recv status (it might have changed!) and continue */
		recv_pkt = (MPID_SMI_BRNDV_PKT_T *)MPID_dataqueue_remove( MPID_SMI_brndv_queue, recv_pkt );
		target_addr = recv_pkt->target_addr;
		len_left    = recv_pkt->len_left;
		roffset     = *roffset_ptr;
		woffset     = *woffset_ptr;
		source_addr = rndv_data_addr + roffset;

	    if (len_left == 0)
		break;
	    }
	    /* Another read of the remote-controlled ptr. */
	    do {
		woffset = *woffset_ptr;
	    } while (woffset >= databuf_size);
	}
	
	if (len_left > MPID_SMI_STREAMSIZE 
	    && (woffset - roffset > MPID_SMI_STREAMSIZE || roffset - woffset > MPID_SMI_STREAMSIZE)) {
	    if (len_left == (recv_pkt->pkt).len)
		RNDV_RECV_STARTED(from_grank);

	    /* Now, data is available. Read it with wrap-around of ptrs. */
	    if (woffset > roffset) {
		/* no wrap-around - read a single block. */
		cpy_len = woffset - roffset - MPID_SMI_STREAMSIZE;
		MEMCPY_R( target_addr, source_addr, cpy_len);
		roffset      += cpy_len; 
		*roffset_ptr  = roffset;
		target_addr  += cpy_len;
		source_addr  += cpy_len;
	    } else {
		/* We need to wrap around the end of the buffer. We read the remainder
		   of the buffer now, the rest wil be read in the next iteration. */
		cpy_len = databuf_size - roffset;
		if (woffset == 0)
		    cpy_len -= MPID_SMI_STREAMSIZE;
		MEMCPY_R( target_addr, source_addr, cpy_len);
		roffset      = (roffset + cpy_len) % databuf_size;
		*roffset_ptr = roffset;
		target_addr += cpy_len;
		source_addr += cpy_len;
		if (woffset != 0)
		    source_addr  = rndv_data_addr;
	    }
	    len_left -= cpy_len;
	} else {
	    break;
	}
    }
    
    /* now copy the remainder. */
    if (len_left > 0)
	MEMCPY_R( target_addr, source_addr, len_left);
    
    MPID_SMI_Rndv_free_recvbuf((recv_pkt->pkt).sgmt_offset, from_grank);
    MPID_SBfree(brndvpkt_allocator, recv_pkt);

    COMPLETE_RHANDLE( rhandle );
    RNDV_RECV_FINISHED( from_grank );
    MPID_STAT_EXIT( brndv_ack_recv );

    return MPI_SUCCESS;
    
}

/* new (blocking) version of MPID_SMI_Brndv_send_ack with use of ringbuffer */
int MPID_SMI_Brndv_send_ack(in_pkt, from_grank)
void *in_pkt;
int from_grank;
{
    MPID_PKT_RNDV_T recv_pkt, prepkt;
    MPID_SMI_CTRLPKT_T pkt_desc; /* for PART_READY msg we send to receiver */
    MPIR_SHANDLE *shandle = 0;
    ulong *woffset_ptr, *roffset_ptr;    /* pointers for ringbuffer administration */
    char *rndv_base_addr;    /* base address of whole rndv memory */
    char *rndv_data_addr;    /* base address of part in which we write the message */
    unsigned int ptrmem_size, misalign_size;
    int msglen, len_sent = 0, len_to_send;    /* len_sent + len_to_send = msglen */
    int avail_bufsize, databuf_size, err;
    /* variables for sending message */
    ulong roffset, woffset, cpy_len = 0;
    char *target_addr, *source_addr, *align_buf;

    MPID_STAT_ENTRY(brndv_ack_send);
    MPID_TRACE_CODE ("Brndv_send_ack", from_grank);
    MPID_SMI_DEBUG_PRINT_MSG("Entering Brndv_send_ack");

    MEMCPY (&recv_pkt, in_pkt, sizeof(MPID_PKT_RNDV_T));
    MPID_SMI_FreeRecvPkt( (MPID_PKT_T *)in_pkt, from_grank, IS_CTRL_MSG);

    MPID_AINT_GET(shandle, recv_pkt.send_id);
    MPID_CHECK_COOKIE (shandle);
    VALIDATE_HANDLE (shandle);
    RNDV_CHECK_ZEROMSG (&recv_pkt, shandle);
    MPID_STAT_PERIOD_END(rndv_sync_delay, shandle->recv_handle->sync_delay);

    /* make sure the segment is connected: If region id is invalid, we still need to
       connect/import the remote memory. */
    if (recv_pkt.len_avail > 0 && shandle->recv_handle->smi_regid_dest == MPID_SMI_INVALID_REGID) {
	switch (shandle->recv_handle->mode) {
	case RNDV_SYNC:
	    MPID_SMI_Rndv_map_remote_mem (from_grank, &recv_pkt, shandle, false);
	    break;
	default:
	    /* no other modes supported by this protocol */
	    MPID_ABORT ("Illegal send mode in Brndv_send_ack.");
	    break;
	}
    }

    /* Initialize all ptrs and buffers sizes. */
    rndv_base_addr = (char *)shandle->recv_handle->dest_addr;
    misalign_size = (size_t)rndv_base_addr % MPID_SMI_STREAMSIZE;
    rndv_base_addr += (misalign_size == 0) ? 0 : (MPID_SMI_STREAMSIZE - misalign_size);

    ptrmem_size = 2*MPID_SMI_STREAMSIZE;
    woffset_ptr = (ulong *)(rndv_base_addr + MPID_SMI_STREAMSIZE - sizeof(ulong *));
    roffset_ptr = (ulong *)(woffset_ptr + MPID_SMI_STREAMSIZE/sizeof(ulong));
    woffset  = 0;
    roffset  = 0;
    /* remote SCI writes */
    *woffset_ptr = 0; 
    *roffset_ptr = 0; 
    SCI_SYNC_WRITE(from_grank);

    rndv_data_addr = rndv_base_addr + ptrmem_size;
    source_addr    = (char *) (shandle->start);
    target_addr    = rndv_data_addr;
    databuf_size   = (recv_pkt.len_avail - ptrmem_size)/MPID_SMI_STREAMSIZE * MPID_SMI_STREAMSIZE;
    /* We need to avoid the the write offset "catches" the read offset from behind: 
       if these two are equal, we'll have a deadlock. */
    avail_bufsize  = databuf_size - MPID_SMI_STREAMSIZE;
    len_to_send = msglen = shandle->bytes_as_contig;

    /* from now on, the receiver can poll on the read and write pointers, so we send him a 
       control packet of type MPID_PKT_CONT_RDY, so that he reaches MPID_SMI_Brndv_recv_ack */
    MPID_GETSENDPKT(pkt_desc, sizeof(MPID_PKT_RNDV_T), &prepkt, 0, 0, from_grank, 0);
    MPID_INIT_RNDVCONT_RDY_PREPKT(prepkt, shandle->bytes_as_contig, 0, recv_pkt.sgmt_offset, 
				  recv_pkt.recv_id, recv_pkt.len_avail, recv_pkt.data_offset, 0);

    while (MPID_SMI_SendControl( &pkt_desc ) != MPI_SUCCESS)
	;

    /* now the actual copying of the data begins; first, we copy until we have less than
       MPID_SMI_STREAMSIZE bytes left, then we copy the rest with alignment */
    RNDV_SEND_STARTED(from_grank);

    MEMCPYSYNC_ENTER_NODEVCHECK(from_grank, msglen);
    while (len_to_send > MPID_SMI_STREAMSIZE) {
	/* calculate length of data which we actually send now, this is the minimum 
	   of len(aligned downwards), MPID_SMI_RNDVBLOCK, avail_bufsize */
	cpy_len = len_to_send < avail_bufsize ? len_to_send : avail_bufsize;
	if (cpy_len > MPID_SMI_RNDVBLOCK)
	    cpy_len = MPID_SMI_RNDVBLOCK;

	len_to_send -= cpy_len;
	len_sent    += cpy_len;
	if (cpy_len <= databuf_size - (target_addr - rndv_data_addr)) {
	    /* we can copy in one piece */
	    MEMCPY_W( target_addr, source_addr, cpy_len, from_grank );

	    target_addr += cpy_len;
	    source_addr += cpy_len;
	    woffset     += cpy_len;

	    /* check for wrap-around in the target ringbuffer */
	    if (woffset >= databuf_size) {
		woffset = 0;
		target_addr = rndv_data_addr;
	    }

	    /* Update remote ptr - SCI remote write. */
	    do {
		*woffset_ptr = woffset;
	    } while (SMI_Check_transfer(CHECK_MODE) != SMI_SUCCESS);
	} else {
	    /* we have to copy in two pieces: first, we copy until we reach the end of the buffer,
	       the we copy the rest to the beginning of the buffer */
	    /* first: copy into the rest of the buffer */
	    int part_len = databuf_size - (target_addr - rndv_data_addr);

	    MEMCPY_W( target_addr, source_addr, part_len, from_grank );

	    target_addr = rndv_data_addr;
	    source_addr += part_len;
	    len_sent    += part_len;
	    woffset = 0;

	    /* second: copy the rest to the beginning of the buffer */
	    cpy_len = cpy_len - part_len;
	    MEMCPY_W( target_addr, source_addr, cpy_len, from_grank );

	    target_addr += cpy_len;
	    source_addr += cpy_len;
	    woffset      = cpy_len;

	    /* Update remote ptr - SCI remote write. */
	    do {
		*woffset_ptr = woffset;
	    } while (SMI_Check_transfer(CHECK_MODE) != SMI_SUCCESS);
	}

	if (len_to_send > MPID_SMI_STREAMSIZE) {
	    /* Calculate amount of free memory in buffer, if there is nothing available,
	       wait until receiver has read some data. Again, substract MPID_SMI_STREAMSIZE
	       from the available length to avoid "pointer catching". */
	    do {
		avail_bufsize = (roffset > woffset) ?
		    roffset - woffset : databuf_size - (woffset - roffset);
		avail_bufsize -= MPID_SMI_STREAMSIZE;

		if (avail_bufsize < MPID_SMI_STREAMSIZE) {
		    /* Check if remote process has read data by reading the remote ptr. */
		    SMI_Flush_read(from_grank);
		    /* Update via remote ptr - SCI remote read. */
		    do {
			roffset = *roffset_ptr;
		    } while (SMI_Check_transfer(CHECK_MODE) != SMI_SUCCESS);
		    
		    avail_bufsize = (roffset > woffset) ?
			roffset - woffset : databuf_size - (woffset - roffset);
		    avail_bufsize -= MPID_SMI_STREAMSIZE;
		}
		if (avail_bufsize < MPID_SMI_STREAMSIZE && MPID_SMI_myid > from_grank) {
		    MEMCPYSYNC_LEAVE(from_grank, msglen);
		    MPID_DeviceCheck (MPID_NOTBLOCKING);
		    MEMCPYSYNC_ENTER_NODEVCHECK (from_grank, msglen);
		}
	    } while (avail_bufsize < MPID_SMI_STREAMSIZE);
	}
    }
    
    /* Now, we have less than MPID_SMI_STREAMSIZE bytes left in our message 
       -> copy the rest with alignment. */
    if (len_to_send > 0) {
	/* Since we have copied in pieces of MPID_SMI_STREAMSIZE minimum, we should always 
	   have enough memory left in our buffer to copy this last bytes. */
	if (!MPID_SMI_is_remote[from_grank] ) {
	    MEMCPY (target_addr, source_addr, len_to_send);
	} else {
	    /* copy date into align buffer , then copy full streambuffer-size */
	    align_buf = (char *)MPID_SBalloc(alignbuf_allocator);
	    MEMCPY (align_buf, source_addr, len_to_send);
	    
	    MEMCPY_W( target_addr, align_buf, MPID_SMI_STREAMSIZE, from_grank );
	    MPID_SBfree (alignbuf_allocator, align_buf);
	}

	woffset += len_to_send;
	/* SCI remote write */
	do {
	    *woffset_ptr = woffset;
	} while (SMI_Check_transfer(CHECK_MODE) != SMI_SUCCESS);
    }
    MEMCPYSYNC_LEAVE(from_grank, msglen);

    switch (shandle->recv_handle->mode) {
    case RNDV_SYNC:
	/* Only release remote SCI memory. */
	if (!MPID_SMI_use_localseg[from_grank] && shandle->recv_handle->dest_addr != 0) 
	    MPID_SMI_Rmt_mem_release (NULL, shandle->recv_handle->smi_regid_dest, MPID_SMI_RSRC_CACHE);
	break;
    default:
	/* no other modes supported by this protocol */
	break;
    }

    COMPLETE_SHANDLE( shandle );
    RNDV_SEND_FINISHED( from_grank );
	
    MPID_STAT_EXIT(brndv_ack_send);
    return MPI_SUCCESS;
}

/* 
   Deliver an address (or offset for NONFIXED) in *addr to where the sender can write data.
   The desired length is given in len, in which the length which could be allocated is
   returned. dest is usually the local process, but could be another in the future.
   Here, we have to allocate more memory than in MPID_SMI_Nb_SetupRndvAddress, because we need space for our
   pointers to administer the ringbuffer. These pointers are NOT initialized afterwards.
 */
int MPID_SMI_Brndv_get_recvbuf (sgmt_id, sgmt_offset, adpt_nbr, len, from_grank)
ulong *sgmt_offset;
int   *len, from_grank, *sgmt_id, *adpt_nbr;
{
    char *addr;
    int tlen, shreg_id, ptr_len, data_len, stat_value;
    int ret = MPI_SUCCESS;

    MPID_STAT_ENTRY(setup_rndvadr);
    MPID_TRACE_CODE("Brndv_get_recvbuf", from_grank);
    MPID_SMI_DEBUG_PRINT_MSG("Entering Brndv_get_recvbuf");

    if (*len == 0)
	return ret;
    
    /* The buffer we allocate is for the data, the two pointers for  flow-control 
       (both placed at the end of a streambuffer) and an additional streambuffer-sized
       piece of memory for alignment of the complete buffer. */
    ptr_len = 2*MPID_SMI_STREAMSIZE;
    data_len = (MPID_SMI_RNDVRECEIPT == 0 || *len < MPID_SMI_RNDVBLOCK*MPID_SMI_RNDVRECEIPT) ? 
	*len : MPID_SMI_RNDVBLOCK*MPID_SMI_RNDVRECEIPT;
    MPID_SMI_STREAMBUF_ALIGN (data_len);
    tlen = data_len + MPID_SMI_BRNDV_PTRMEM;

    /* determine the shared memory region to use */
    *sgmt_id  = MPID_SMI_use_localseg[from_grank] ? -1 : MPID_SMI_Rndv_sgmt_id;
    *adpt_nbr = MPID_SMI_use_localseg[from_grank] ? -1 : MPID_SMI_Rndv_adpt_nbr;

    shreg_id = MPID_SMI_use_localseg[from_grank] ? 
	MPID_SMI_rndv_shmempool_regid : MPID_SMI_rndv_scipool_regid;
    addr = (char *)MPID_SMI_shmalloc( tlen, shreg_id );
    while (!addr) {
	MPID_STAT_COUNT(rndvmem_split);
	MPID_SMI_DEBUG_PRINT_MSG("Allocating partial space for long message");
	tlen = tlen / 2; 
	while ((tlen > 8 * MPID_SMI_STREAMSIZE) 
	       && !(addr = (char *)MPID_SMI_shmalloc( tlen, shreg_id )) )
	    tlen = tlen / 2;
	if (tlen <= (8 * MPID_SMI_STREAMSIZE)) {
	    MPID_ASSERT (MPID_SMI_Rndvrecvs_in_progress > 0, 
			 "Could not allocate rendez-vous buffer although no recvs are active.");
	    
	    MPID_DeviceCheck( MPID_NOTBLOCKING );
	    tlen = *len;
	}
    }

    /* transform ptr into an offset */
    *sgmt_offset = MPID_SMI_use_localseg[from_grank] ? 
	(size_t)(addr - (size_t)MPID_SMI_rndv_shmempool) : (size_t)(addr - (size_t)MPID_SMI_rndv_scipool);
    *len = tlen;

    /* Initialize the read- and write-ptrs which are located in the first 2 streambuffers. */
    if (tlen > MPID_SMI_BRNDV_PTRMEM) {
	memset (addr, 0, MPID_SMI_BRNDV_PTRMEM);
    }

    stat_value = *len >> 10;
    MPID_STAT_PROBE(rndv_inbuf_size, stat_value);
    MPID_STAT_EXIT(setup_rndvadr);

    return ret;
}

