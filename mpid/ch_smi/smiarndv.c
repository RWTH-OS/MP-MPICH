/* $Id$ */

/* Rendez-vous protocol for asynchronous, non-blocking transfers (MPI_Isend() etc.), 
   possibly using DMA or a second thread with PIO. */

#include "smidef.h"
#include "smirndv.h"

#include "mpid.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* some protocol parameters */
#define INITIAL_LOCKS    10
/* Signal the worker thread of the receiver that a packet was sent. */
#define SIGNAL_REMOTE(dest) if (MPID_SMI_cfg.MSGCHK_TYPE == MSGCHK_TYPE_POLLING) \
                                           SMI_Signal_send((dest)|SMI_SIGNAL_ANY);


/* Prototype definitions */
int MPID_SMI_Arndv_setup (void);
void MPID_SMI_Arndv_delete (void);

int MPID_SMI_Arndv_isend (void *, int, int, int, int, int, 
									MPID_Msgrep_t, MPIR_SHANDLE *, struct MPIR_DATATYPE * );
int MPID_SMI_Arndv_irecv (MPIR_RHANDLE *, int, void *);

int MPID_SMI_Arndv_send_ack (void *, int);
int MPID_SMI_Arndv_recv_ack (void *, int);

int MPID_SMI_Arndv_unxrecv_start (MPIR_RHANDLE *, void *);
int MPID_SMI_Arndv_unxrecv_end (MPIR_RHANDLE *);
int MPID_SMI_Arndv_unxrecv_test_end (MPIR_RHANDLE *);
int MPID_SMI_Arndv_send_wait (MPIR_SHANDLE *);
int MPID_SMI_Arndv_send_test_ack (MPIR_SHANDLE *);
int MPID_SMI_Arndv_send_wait_ack (MPIR_SHANDLE *);

int MPID_SMI_Arndv_cancel_send (MPIR_SHANDLE *shandle);
int MPID_SMI_Arndv_cancel_recv (MPIR_RHANDLE *runex);


/* imports */
extern MPID_SMI_Rndv_int_t MPID_SMI_Rndv_int;

/*
 * Globals for this protocol 
 */
/* local SCI segment for non-registered outgoing DMA transfers */
static char *dma_outbuf_addr = NULL;
static int dma_outbuf_size;
static int dma_outbuf_regid = -1;
static int dma_offset_align, dma_size_align;

static MPID_stack_t completion_locks;

int MPID_SMI_Arndv_setup (void)
{
	int l;
	MPID_SMI_LOCK_T *lock;

	/* create some mutex' used wait for completion of a transfer */
	completion_locks = MPID_stack_init (MPID_UTIL_THREADSAFE);

	for (l = 0; l < INITIAL_LOCKS; l++) {
		ALLOCATE (lock, MPID_SMI_LOCK_T *, sizeof (MPID_SMI_LOCK_T));
		MPID_SMI_INIT_LOCK (lock);
		MPID_stack_push (completion_locks, lock);
	}

	SMI_Query (SMI_Q_SCI_DMA_OFFSET_ALIGN, 0, &dma_offset_align);
	SMI_Query (SMI_Q_SCI_DMA_SIZE_ALIGN, 0, &dma_size_align);

	return MPI_SUCCESS;
}

void MPID_SMI_Arndv_delete ()
{
	MPID_SMI_LOCK_T *lock;

	while ((lock = MPID_stack_pop (completion_locks)) != NULL) {
		MPID_SMI_DESTROY_LOCK (lock);
		FREE (lock);
	}
	MPID_stack_destroy (completion_locks);
		
	return;
}

void MPID_SMI_Arndv_get_config (int *size)
{
	if (dma_outbuf_addr == NULL) 
		*size = MPID_SMI_RNDVSIZE;
	else
		*size = dma_outbuf_size;

	return;
}


/* If a send-buffer should be transferred via DMA, but can not be registered, 
	we need to allocate some local shared memory for intermediate copies. This
	is done from an SMI region which we create here. */
/* XXX perform this via region mngmt, too */
static int arndv_create_dma_outbuf( void )
{
	int sci_id;
	smi_region_info_t shreg_info;
    
	if (MPID_SMI_cfg.USE_DMA_PT2PT) {
		/* allocate SCI memory - this process will only manage the local SCI segment 
			which servers as DMA source */
		MPID_SMI_PAGESIZE_ALIGN(MPID_SMI_RNDVSIZE);
		dma_outbuf_size = MPID_SMI_RNDVSIZE;
		if (!MPID_SMI_Local_mem_create((size_t *)&dma_outbuf_size, MPID_SMI_EAGERSIZE, 
												 (void **)&dma_outbuf_addr, &dma_outbuf_regid, &sci_id))
			return MPI_ERR_INTERN;
		SMIcall (SMI_Init_shregMMU (dma_outbuf_regid));
	}
    
	return MPI_SUCCESS;
}


/* Acquired and release pthread locks for the application thread to wait
	for message transfer completion. */
static MPID_SMI_LOCK_T *arndv_get_lock()
{
	MPID_SMI_LOCK_T *lock = MPID_stack_pop (completion_locks);
	
	if (lock == NULL) {
		ALLOCATE (lock, MPID_SMI_LOCK_T *, sizeof (MPID_SMI_LOCK_T));
		MPID_SMI_INIT_LOCK (lock);
	}
	
	return lock;
}

static void arndv_free_lock(MPID_SMI_LOCK_T *lock)
{
	if (lock != NULL) {
		MPID_SMI_LOCK (lock);
		MPID_SMI_UNLOCK (lock);
		MPID_stack_push (completion_locks, lock);
		lock = NULL;
	}
	
	return;
}


/* Get local shared memory for DMA (required if user buffer can not be registered) */
static int arndv_get_dma_outbuf (long len, MPIR_SHANDLE *shandle)
{
	char *buf = NULL;
	long  tlen;
	int  devchecks = 0;

	MPID_TRACE_CODE("arndv_get_dma_outbuf", MPID_SMI_myid);

	shandle->recv_handle->smi_regid_src   = MPID_SMI_INVALID_REGID;
	shandle->recv_handle->dma_outbuf_addr = NULL;

	if (len == 0)
		return MPI_SUCCESS;

	if (dma_outbuf_addr == NULL && arndv_create_dma_outbuf() != MPI_SUCCESS) 
		return -1;
		
	tlen = (MPID_SMI_RNDVDMASIZE != 0 && len > MPID_SMI_RNDVDMASIZE) ? 
		MPID_SMI_RNDVDMASIZE : len;

	MPID_SMI_STREAMBUF_ALIGN (tlen);
	while (!buf) {
		buf = MPID_SMI_shmalloc (tlen, dma_outbuf_regid);
		if (!buf) {
			tlen /= 2;
			MPID_SMI_STREAMBUF_ALIGN (tlen);
			if (tlen < MPID_SMI_cfg.ASYNC_DMA_MINSIZE) {
				MPID_ASSERT (MPID_SMI_Rndvsends_in_progress > 0, "No DMA-out buffer available although no sends are active.");
				while (MPID_SMI_Rndvsends_in_progress > 0)
					MPID_DeviceCheck (MPID_NOTBLOCKING);
				tlen = len;
			}
		}
	}

	shandle->recv_handle->dma_outbuf_addr  = buf;
	shandle->recv_handle->dma_outbuf_len   = tlen;
	shandle->recv_handle->mode             = RNDV_ASYNC_SEND_DMA;
	shandle->recv_handle->dma_precopy_busy = true;
	shandle->recv_handle->smi_regid_src    = dma_outbuf_regid;

	return MPI_SUCCESS;
}


static int arndv_register_sendbuf (void *buf, int len, MPIR_SHANDLE *shandle)
{
	smi_region_info_t region_info;
	int region_id, sci_id, err = MPI_SUCCESS, registered;
	
	if (!MPID_SMI_cfg.REGISTER || !OK_FOR_DMA(buf,len))
		return -1;
	
	registered = MPID_SMI_Local_mem_register (buf, len, &region_id, &sci_id);
	if (registered != MPIR_ERR_EXHAUSTED) {
		MPID_STAT_COUNT(sendbuf_registered);

		shandle->recv_handle->smi_regid_src   = region_id;
		shandle->recv_handle->dma_outbuf_addr = buf;
		shandle->recv_handle->dma_outbuf_len  = len;
		shandle->recv_handle->mode = (registered == MPID_SMI_ISSCI) ? 
			RNDV_ASYNC_SEND_DMAZC_PERS : RNDV_ASYNC_SEND_DMAZC;
	} else {
		shandle->recv_handle->smi_regid_src = MPID_SMI_INVALID_REGID; 
		err = -1;
	}
	
	return err;
}

/* Prepares a buffer to recv a message. This buffer can be the user-supplied 
	buffer which is registered for zero-copy, or the usual in-between recv buffer.
	The transfer-mode is set accordignly in the recv handle, and all other info
	that is required for a specific transfer mode is also stored there. 

	If the user-supplied buffer could be registerd, '1' is returned; '0' in all
	other cases. */
static int arndv_setup_recvbuf (MPIR_RHANDLE *rhandle, int from_grank, int len, int *flags,
										  int *sgmt_id, ulong *sgmt_offset, int *adpt_nbr)
{
	ulong *sgmt_addr, sgmt_size;
	int region_id, retval, do_zc = 0;
	int msglen = rhandle->s.count;

	MPID_STAT_ENTRY (arndv_setup_recvbuff);
	
	if (len == 0) {
		/* Remember the locally available space for partial transfers */
		rhandle->recv_handle->mode = RNDV_ASYNC_RECV;
		rhandle->recv_handle->len_local = 0;
		return do_zc;
	}

	/* Try to register recv buffer if the sender says he can do DMA. */
	if ((*flags & MPID_SMI_RNDV_DMAOK) && MPID_SMI_cfg.ZEROCOPY && OK_FOR_DMA(rhandle->buf, len)
		 && MPID_SMI_is_remote[from_grank]) {
		if ((retval = MPID_SMI_Local_mem_register (rhandle->buf, len, &region_id, sgmt_id)) 
			 != MPIR_ERR_EXHAUSTED) {
			SMI_Query (SMI_Q_SMI_REGION_ADPTNBR, region_id, adpt_nbr);
			SMI_Query (SMI_Q_SMI_REGION_SGMT_ID, region_id, sgmt_id);

			switch (retval) {
			case MPI_SUCCESS:
				/* The memory has really been registered. The offset that the remote 
					process will need to consider is is the offset to the next lower
				   page boundary (remote memory can only be mapped with page-granularity). */
				/* XXX this will become unnecessary once SMI does this offset adaption
					upon attaching to the remote segment */
				/* XXX or is this functionality alredy in the IRM ? -> validate! */
#if 0
				*sgmt_offset = ((ulong)rhandle->buf) % MPID_SMI_PAGESIZE;
#else
				*sgmt_offset = 0;
#endif
				rhandle->recv_handle->mode = RNDV_ASYNC_RECV_ZC;
				*flags |= MPID_SMI_RNDV_ZEROCOPY|MPID_SMI_RNDV_DMAONLY;
				break;
			case MPID_SMI_ISSCI:
				/* The memory is an SCI region which was allocated elsewhere,
					i.e. via MPI_Alloc_mem() */
				SMI_Query (SMI_Q_SMI_REGION_ADDRESS, region_id, &sgmt_addr);
				*sgmt_offset = (size_t)rhandle->buf - (size_t)sgmt_addr;
				*flags |= MPID_SMI_RNDV_ZEROCOPY;
				rhandle->recv_handle->mode = RNDV_ASYNC_RECV_ZC_PERS;
				break;
			}
			
			rhandle->recv_handle->smi_regid_dest = region_id;
			rhandle->recv_handle->len_local = len;
			do_zc = 1;
			MPID_STAT_COUNT(recvbuf_registered);
		}
	}

	if (!do_zc) {
		/* No zero-copy possible or wanted -> get local shared memory for the message. */
		int local_len;
		
		rhandle->recv_handle->mode = RNDV_ASYNC_RECV;
		rhandle->recv_handle->smi_regid_dest = MPID_SMI_INVALID_REGID; 
		
		/* Match incoming buffer size to size of outgoing buffer on sender side. 
			We don't do pipelining for asynchronous rendez-vouz, therefore: 
			- if the sender offers DMA, the incoming buffer does not need to be bigger 
			than the senders outgoing buffer (for non-zerocopy mode)
			- for PIO, we instead try to allocate a buffer as big as possible to allow
			transmission in as few as possble operations. 
			- If we will use zero-copy, this value is meaningless because the sender will
			have the full recv buffer as destination. */
		local_len = (*flags & MPID_SMI_RNDV_DMAOK) ? rhandle->recv_handle->dma_outbuf_len : len;
		MPID_SMI_Rndv_int.Setup_rndv_addr (sgmt_id, sgmt_offset, adpt_nbr, &local_len, from_grank);
		
		/* local_len might have become changed by Setup_rndv_addr() */
		rhandle->recv_handle->len_local = local_len;
	}

	MPID_STAT_EXIT (arndv_setup_recvbuff);
	return do_zc;
}

int MPID_SMI_Arndv_resume_send ( MPIR_SHANDLE *sh ) 
{
	return MPID_SMI_Arndv_isend (sh->start, sh->bytes_as_contig, sh->src_lrank, sh->tag,
								sh->context_id, sh->partner, sh->msgrep, sh, sh->datatype);
}

/*
 * Initiate a non-blocking send process (Isend) utilizing the
 * rendez-vous protocol. The sender announces his wish to send data by
 * sending a MPID_PKT_REQUEST_SEND control packet with the async flag set.  
 * Since the Isend starts a non-blocking send process, some handler for
 * receiving (SMI_wait or SMI_test) the acknowledge control packet is
 * installed and control returns to the caller.
 *
 * The last action is the sending of a MPID_PKT_REQUEST_SEND
 * control packet. The receiver will then get some RS memory, and
 * answer with a MPID_PKT_OK_TO_SEND control packet.
 */
int MPID_SMI_Arndv_isend (buf, len, src_lrank, tag, context_id, dest, msgrep, shandle, dtype)
	  void          *buf;
	  int           len, tag, context_id, src_lrank, dest;
	  MPID_Msgrep_t msgrep;
	  MPIR_SHANDLE *shandle;
	  struct MPIR_DATATYPE *dtype;
{
	MPID_SMI_CTRLPKT_T pkt_desc;
	MPID_PKT_RNDV_T prepkt;
	MPID_Aint send_id;
	int adpt = 0, rndv_flag = 0, do_dma;
	
	MPID_STAT_ENTRY (arndv_isend);
	MPID_TRACE_CODE ("NRndvn_isend", dest);
	
	/* check if this is a message to myself */
	if (MPID_SMI_cfg.SENDSELF && dest == MPID_SMI_myid) {
		/* we can transfer the message directly into the receive buffer */
		return MPID_SMI_Isend_self (buf, len, src_lrank, tag, context_id, dest, msgrep, shandle);
	}
	
	/* DMA is not possible for intra-node msgs */
	if( MPID_SMI_procNode[dest] == MPID_SMI_myNode )
		return MPID_SMI_Rndv_isend( buf, len, src_lrank, tag, context_id, dest, msgrep, shandle, dtype );

	/* Only a maximum number of concurrent send operations is allowed to 
	   control resource usage. */
	if (MPID_SMI_Rndvsends_in_progress >= MPID_SMI_cfg.MAX_SENDS) {
		shandle->push = MPID_SMI_Arndv_resume_send;
		MPID_RNDV_POSTPONE_SEND( buf, len, src_lrank, tag, context_id, dest, msgrep, shandle, dtype );
		return MPI_SUCCESS;
	}

	ALLOC_RNDVINFO_SEND(shandle);	

	do_dma = MPID_SMI_cfg.USE_DMA_PT2PT && len >= MPID_SMI_cfg.ASYNC_DMA_MINSIZE && OK_FOR_DMA(buf,len)
		&& MPID_SMI_is_remote[dest];
	if (do_dma) {
		MPID_STAT_CALL(arndv_setup_sendbuf);
		/* If registering succeeds, we can do DMA from the user buffer. 
			If not, allocate intermediate buffer in local shared memory. */
		if (arndv_register_sendbuf (buf, len, shandle) != MPI_SUCCESS
			 && arndv_get_dma_outbuf (len, shandle) != MPI_SUCCESS) {
			do_dma = false;
		} else {
			rndv_flag = MPID_SMI_RNDV_ASYNC|MPID_SMI_RNDV_DMAOK;
		}
		MPID_STAT_RETURN(arndv_setup_sendbuf);
	} 

	/* Fallback is PIO - DMA may not be possible due to ressource shortage. */
	if (!do_dma) {
		shandle->recv_handle->mode = RNDV_ASYNC_SEND_PIO;
		rndv_flag = MPID_SMI_RNDV_ASYNC|MPID_SMI_RNDV_PIOONLY;
	}

	MPID_SMI_DEBUG_PRINT_MSG("S About to get pkt for request to send async");
	MPID_GETSENDPKT (pkt_desc, sizeof(MPID_PKT_RNDV_T), &prepkt, 0, NULL, dest, 0);

	/* We save the address of the send handle in the packet; the receiver will return this to us */
	MPID_AINT_SET (send_id, shandle);
	MPID_INIT_RNDVREQ_PREPKT(prepkt, context_id, src_lrank, tag, len, send_id, 0, 0, 
									 shandle->recv_handle->smi_regid_src, adpt, 
									 shandle->recv_handle->dma_outbuf_len, rndv_flag);
	 
	MPID_SMI_DEBUG_PRINT_BASIC_SEND_PKT ("S Sending rndv-get message", pkt_desc.pkt, dest);
	while (MPID_SMI_SendControl (&pkt_desc) != MPI_SUCCESS)
		;

	/* Signal the worker thread of the receiver that a packet was sent. */
	SIGNAL_REMOTE(dest);

	/* Store info in the request for completing the message */
	shandle->is_complete = 0;
	shandle->start	      = buf;
	shandle->bytes_as_contig = len;
	/* Set the test/wait functions */
	shandle->wait   = MPID_SMI_Arndv_send_wait_ack;
	shandle->test   = MPID_SMI_Arndv_send_test_ack;
	shandle->cancel = MPID_SMI_Arndv_cancel_send;
	shandle->is_cancelled    = 0;
	shandle->cancel_complete = 0;
	shandle->partner         = dest;
#if WAIT_ON_LOCK
	shandle->recv_handle->complete_lock = arndv_get_lock();
	MPID_SMI_LOCK (shandle->recv_handle->complete_lock);
#else
	shandle->recv_handle->complete_lock = NULL;
#endif
	shandle->recv_handle->smi_regid_dest = MPID_SMI_INVALID_REGID;
	shandle->is_valid            = 1;

	/* shandle->finish must NOT be set here; it must be cleared/set when the request is created */
    
	/* Start filling the DMA buffer if requried.*/
	if (shandle->recv_handle->mode == RNDV_ASYNC_SEND_DMA) {
		MPID_STAT_CALL(arndv_buf2ls);
		MEMCPY (shandle->recv_handle->dma_outbuf_addr, buf, shandle->recv_handle->dma_outbuf_len);
		MPID_STAT_RETURN(arndv_buf2ls);
		shandle->recv_handle->dma_precopy_busy = false;
	}
    
	RNDV_SEND_SCHEDULED(dest);
	MPID_STAT_EXIT (arndv_isend);

	return MPI_SUCCESS;
}


/* Sender: MPID_PKT_OK_TO_SEND was received and we are going to send the actual data. This is for 
	non-zero-copy operations, which means that the data is copied into an intermediate buffer at 
	the receiver, possibly in multiple parts. */
int MPID_SMI_Arndv_send_ack (in_pkt, from_grank)
	  void *in_pkt;
	  int from_grank;
{
	MPID_PKT_RNDV_T 	recv_pkt, prepkt;
	MPID_SMI_CTRLPKT_T 	pkt_desc;
	MPIR_SHANDLE        *shandle=0;
	MPID_Aint           send_id;
	smi_memcpy_handle   mcpy_handle = NULL;
	int len, cpy_len, err, is_done, parts, connect_tries;

	MPID_STAT_ENTRY (arndv_ack_send);
	MPID_TRACE_CODE ("NRndvn_Send_ack", from_grank);

	MEMCPY (&recv_pkt, in_pkt, sizeof(MPID_PKT_RNDV_T));
	MPID_SMI_FreeRecvPkt ((MPID_PKT_T*)in_pkt, from_grank, IS_CTRL_MSG);

	MPID_AINT_GET (shandle, recv_pkt.send_id);
	MPID_CHECK_COOKIE (shandle);
	VALIDATE_HANDLE (shandle);
	RNDV_CHECK_ZEROMSG (&recv_pkt, shandle);

	/* Ensure proper connection to remote segment. */
    /* make sure the segment is connected: If region id is invalid, we still need to
       connect/import the remote memory. */
	if (recv_pkt.len_avail > 0 && shandle->recv_handle->smi_regid_dest == MPID_SMI_INVALID_REGID) {
		MPID_STAT_PERIOD_END(rndv_sync_delay, shandle->recv_handle->sync_delay);

		switch (shandle->recv_handle->mode) {
		case RNDV_ASYNC_SEND_PIO:
			MPID_SMI_Rndv_map_remote_mem (from_grank, &recv_pkt, shandle, false);
			break;

		case RNDV_ASYNC_SEND_DMAZC:
		case RNDV_ASYNC_SEND_DMAZC_PERS:			
			MPID_STAT_COUNT( destbuf_imported );
			/* fall through */
		case RNDV_ASYNC_SEND_DMA:
			connect_tries = 0;
			do {
				err = MPID_SMI_Rmt_region_connect (from_grank, recv_pkt.sgmt_id, recv_pkt.adpt_nbr, 
															  &shandle->recv_handle->smi_regid_dest);
				connect_tries++;
				while (err == MPIR_ERR_EXHAUSTED && MPID_SMI_Rndvsends_in_progress > 0)
					MPID_DeviceCheck (MPID_NOTBLOCKING);
			} while (err == MPIR_ERR_EXHAUSTED && connect_tries <= MPID_SMI_RNDV_CNCT_MAX_RETRIES);

			if (err == MPIR_ERR_EXHAUSTED)
				MPID_ABORT ("Could not connect to remote memory.");
			break;

		default:
			MPID_ABORT ("Illegal send mode in Arndv_send_ack().");
			break;
		}
		RNDV_SEND_STARTED(from_grank);
	}

	/* Compute length available to send. If this is it, remember so
		that we can mark the operation as complete	 */
	len = shandle->bytes_as_contig - recv_pkt.data_offset;
	if (len > recv_pkt.len_avail)	{
			len = recv_pkt.len_avail;
			is_done = 0;
	} else {
		is_done = 1;
	}

	cpy_len = len;
	if (cpy_len > 0) {
		switch (shandle->recv_handle->mode) {
		case RNDV_ASYNC_SEND_DMA: 
			/* wait until the local DMA send-buffer is full */
			while (shandle->recv_handle->dma_precopy_busy) {
				MPID_SMI_YIELD;
			}

			MPID_STAT_CALL(arndv_dma);
			SMIcall (SMI_Put (shandle->recv_handle->smi_regid_dest, recv_pkt.sgmt_offset, 
									shandle->recv_handle->dma_outbuf_addr, cpy_len));
			MPID_STAT_RETURN(arndv_dma);
		 
			shandle->recv_handle->dma_precopy_busy = true;
			break;
		case RNDV_ASYNC_SEND_DMAZC_PERS:
		case RNDV_ASYNC_SEND_DMAZC:
			/* We can do DMA from the user buffer, but the receiver has an intermediate buffer. */
			MPID_STAT_CALL(arndv_dma);
			SMIcall (SMI_Put (shandle->recv_handle->smi_regid_dest, recv_pkt.sgmt_offset, 
									(char *)shandle->recv_handle->dma_outbuf_addr + recv_pkt.data_offset, cpy_len));
			MPID_STAT_RETURN(arndv_dma);
			break;
		case RNDV_ASYNC_SEND_PIO:
			/* transfer data via CPU */
			MPID_STAT_CALL(arndv_pio);
			if (MPID_SMI_is_remote[from_grank]) {
				MEMCPYSYNC_ENTER(from_grank, cpy_len);
				MEMCPY_W(shandle->recv_handle->dest_addr, 
							(char *)shandle->start + recv_pkt.data_offset, cpy_len, from_grank);
				SCI_SYNC_WRITE(from_grank);
				MEMCPYSYNC_LEAVE(from_grank, cpy_len);
			} else {
				MEMCPY(shandle->recv_handle->dest_addr, 
						 (char *)shandle->start + recv_pkt.data_offset, cpy_len);
			}
			MPID_STAT_RETURN(arndv_pio);
			break;
		}
	}

	MPID_SMI_DEBUG_PRINT_MSG("R about to get packet for cont_nb");
	MPID_GETSENDPKT (pkt_desc, sizeof(MPID_PKT_RNDV_T), &prepkt, 0, NULL, from_grank, 0);

	/* this many partial sends have already been done before this one */
	parts = recv_pkt.len_avail ? recv_pkt.data_offset / recv_pkt.len_avail : 0;
	MPID_INIT_RNDVCONT_PREPKT(prepkt, parts, recv_pkt.send_id, recv_pkt.sgmt_offset, recv_pkt.recv_id, 
									  len, recv_pkt.data_offset, MPID_SMI_RNDV_ASYNC);

	MPID_SMI_DEBUG_PRINT_BASIC_SEND_PKT ("S Sending cont-get message", pkt_desc.pkt, from_grank);
	while (MPID_SMI_SendControl( &pkt_desc ) != MPI_SUCCESS)
		;

	SIGNAL_REMOTE(from_grank);

	if (is_done) {
		if (recv_pkt.len_avail > 0) {
			switch (shandle->recv_handle->mode) {
			case RNDV_ASYNC_SEND_DMA:
				/* Free outgoing DMA buffer */
				MPID_SMI_shfree(shandle->recv_handle->dma_outbuf_addr);
				/* Disconnect from remote region (DESTROY because the receiver may withdraw it, too) */
				/* XXX If we know that the remote buffer is *not* the user recv buffer, 
					we can cache the connection! */
				MPID_SMI_Rmt_region_release (shandle->recv_handle->smi_regid_dest, MPID_SMI_connect_flag);
				break;
			case RNDV_ASYNC_SEND_DMAZC:
				/* unregister user send-buffer */
				MPID_SMI_Local_mem_release (NULL, shandle->recv_handle->smi_regid_src, MPID_SMI_register_flag);
				MPID_SMI_Rmt_region_release (shandle->recv_handle->smi_regid_dest, MPID_SMI_connect_flag);
				break;
			case RNDV_ASYNC_SEND_DMAZC_PERS:			
				MPID_SMI_Local_mem_release (NULL, shandle->recv_handle->smi_regid_src, MPID_SMI_RSRC_CACHE);
				MPID_SMI_Rmt_region_release (shandle->recv_handle->smi_regid_dest, MPID_SMI_connect_flag);
				break;
			case RNDV_ASYNC_SEND_PIO:
				MPID_SMI_Rmt_mem_release (NULL, shandle->recv_handle->smi_regid_dest, MPID_SMI_RSRC_CACHE);
				break;
			default: 
				MPID_ABORT ("Illegal rendez-vous send mode in Arndv_send_ack()." );
			}
		}
		
		COMPLETE_SHANDLE_ASYNC( shandle );
		RNDV_SEND_FINISHED(from_grank);
	} else {
		if (shandle->recv_handle->mode == RNDV_ASYNC_SEND_DMA) {
			/* fill the DMA buffer again for the next part to be sent */
			int fillup_len = MPID_MIN (shandle->recv_handle->dma_outbuf_len,
												shandle->bytes_as_contig - (recv_pkt.data_offset + cpy_len));

			MPID_STAT_CALL(arndv_buf2ls);
			MEMCPY (shandle->recv_handle->dma_outbuf_addr, 
					  (char *)shandle->start + recv_pkt.data_offset + cpy_len, fillup_len);
			MPID_STAT_RETURN(arndv_buf2ls);
			shandle->recv_handle->dma_precopy_busy = false;
		}
	}

	MPID_STAT_EXIT (arndv_ack_send);
	return MPI_SUCCESS;
}

/* Sender: MPID_PKT_OK_TO_SEND_ZC was received und we are going to send the actual 
	data directly into te user receive buffer (which means can do the full message transfer
	using a single transfer operation).*/
int MPID_SMI_Arndv_send_ack_zc (void *in_pkt, int from_grank)
{
	MPID_PKT_RNDV_T      recv_pkt, prepkt;
	MPID_SMI_CTRLPKT_T 	pkt_desc;
	MPIR_SHANDLE        *shandle=0;
	MPID_Aint            send_id;
	smi_memcpy_handle    mcpy_handle = NULL;
	int len, cpy_len, dmaout_size, loop, map_sgmt = 0, release_flag;
	char *zc_dest;

	MPID_STAT_ENTRY (arndv_ack_sendzc);
	MPID_TRACE_CODE ("Arndv_Send_ack_zc", from_grank);

	MEMCPY (&recv_pkt, in_pkt, sizeof(MPID_PKT_RNDV_T));
	MPID_SMI_FreeRecvPkt ((MPID_PKT_T*)in_pkt, from_grank, IS_CTRL_MSG);

	MPID_AINT_GET (shandle,recv_pkt.send_id);
	VALIDATE_HANDLE (shandle);
	MPID_CHECK_COOKIE (shandle);
	RNDV_CHECK_ZEROMSG (&recv_pkt, shandle);
	MPID_STAT_PERIOD_END(rndv_sync_delay, shandle->recv_handle->sync_delay);

	len = shandle->bytes_as_contig;
	/* Connect to the remote segment (the user receive buffer). Need to take care 
		of DMA alignment requirements regarding size and offsets. In case 
	   that they are invalid for pure DMA, we need to map the remote segment. If we do
		asynchronous PIO transfers, we need to map, too.*/
	map_sgmt = (len % dma_size_align) 
		|| ((size_t)shandle->recv_handle->dma_outbuf_addr % dma_offset_align)
		|| (recv_pkt.sgmt_offset % dma_offset_align) 
		|| (shandle->recv_handle->mode == RNDV_ASYNC_SEND_PIO);

	if (MPID_SMI_Rndv_connect_zerocopy (&zc_dest, from_grank, recv_pkt.adpt_nbr, recv_pkt.sgmt_id, 
													len + recv_pkt.sgmt_offset, 0, map_sgmt, 
													&shandle->recv_handle->smi_regid_dest) != MPI_SUCCESS) {
		/* Import of the zero-copy segment did not succesd - fallback is standard-rndv, 
			but the receiver must allocate a buffer for us - to do so, we need to inform 
			him with a special message	of type MPID_PKT_REQUEST_SEND_NOZC. */		
		MPID_SMI_NOTICE ("Could not import receive buffer for zero-copy.");
		MPID_STAT_COUNT(zerocopy_canceled);

		if (shandle->recv_handle->mode == RNDV_ASYNC_SEND_DMAZC) {
			MPID_SMI_Local_mem_release (NULL, shandle->recv_handle->smi_regid_src, MPID_SMI_register_flag);
		}
		shandle->recv_handle->mode = RNDV_ASYNC_SEND_PIO;

		MPID_GETSENDPKT (pkt_desc, sizeof(MPID_PKT_RNDV_T), &prepkt, 0, NULL, from_grank, 0);
		MPID_INIT_RNDVREQ_PREPKT(prepkt, recv_pkt.context_id, 0, recv_pkt.tag, 0, 
										 recv_pkt.send_id, 0, recv_pkt.recv_id, 0, 0, len, MPID_SMI_RNDV_PIOONLY);
		
		MPID_SMI_DEBUG_PRINT_BASIC_SEND_PKT ("S Sending req-send-nozc message", pkt_desc.pkt, from_grank);
		while (MPID_SMI_SendControl( &pkt_desc ) != MPI_SUCCESS)
			;
		SIGNAL_REMOTE(from_grank);

		MPID_STAT_EXIT (arndv_ack_sendzc);
		return 0;
	}
	MPID_STAT_COUNT( destbuf_imported );
	RNDV_SEND_STARTED(from_grank);

	MPID_SMI_DEBUG_PRINT_MSG ("Sending data to zero-copy receive buffer");
	switch (shandle->recv_handle->mode) {
	case RNDV_ASYNC_SEND_DMAZC:
	case RNDV_ASYNC_SEND_DMAZC_PERS:
		/* direct DMA from source to destination - true zero copy */
		MPID_STAT_CALL(arndv_dma);
		SMIcall (SMI_Put (shandle->recv_handle->smi_regid_dest, recv_pkt.sgmt_offset, 
								shandle->recv_handle->dma_outbuf_addr, len));
		MPID_STAT_RETURN(arndv_dma);
		break;
	case RNDV_ASYNC_SEND_DMA:
		/* Indirect DMA - one additional local copy into the DMA-outgoing buffer. 
			In the first pass, we copy the complete outgoing DMA buffer, while in 
			subsequent passes, we cut it in half and do a pipelining of local memcpy and DMA. */
		while (shandle->recv_handle->dma_precopy_busy) {
			MPID_SMI_YIELD;
		}
	  
		dmaout_size = shandle->recv_handle->dma_outbuf_len;
		cpy_len = dmaout_size;
		MPID_STAT_CALL(arndv_dma);
		SMIcall (SMI_Put (shandle->recv_handle->smi_regid_dest, 0, shandle->recv_handle->dma_outbuf_addr, cpy_len));
		len -= cpy_len;

		dmaout_size /= 2; 
		cpy_len = dmaout_size;
		loop = 2;
		while (len > 0) {
			SMIcall (SMI_Iput (shandle->recv_handle->smi_regid_dest, loop*cpy_len, 
									 shandle->recv_handle->dma_outbuf_addr, cpy_len, &mcpy_handle));

			loop++; len -= cpy_len;
			if (len < dmaout_size)
				cpy_len = len;
			if (len > 0) 
				MEMCPY ((char *)shandle->recv_handle->dma_outbuf_addr + (loop%2)*dmaout_size, 
						  (char *)shandle->start + loop*dmaout_size, cpy_len);

			SMIcall (SMI_Memwait (mcpy_handle));
		}
		MPID_STAT_RETURN(arndv_dma);
		break;
	case RNDV_ASYNC_SEND_PIO:
		/* direct PIO from source to destination - not really zero copy, but close */
		MPID_STAT_CALL(arndv_pio);
		MEMCPYSYNC_ENTER(from_grank, len);
		MEMCPY_W (zc_dest, shandle->start, len, from_grank);
		MEMCPYSYNC_LEAVE(from_grank, len);
		MPID_STAT_RETURN(arndv_pio);
		break;
	default:
		MPID_ABORT ("Illegal rendez-vous send mode in Arndv_send_ack_zc()." );
	}


	MPID_SMI_DEBUG_PRINT_MSG("R about to get packet for cont_nb");
	MPID_GETSENDPKT (pkt_desc, sizeof(MPID_PKT_RNDV_T), &prepkt, 0, NULL, from_grank, 0);

	MPID_INIT_RNDVCONT_PREPKT(prepkt, 0, recv_pkt.send_id, 0, recv_pkt.recv_id, len, 0, recv_pkt.flags);

	MPID_SMI_DEBUG_PRINT_BASIC_SEND_PKT ("S Sending cont-get message", pkt_desc.pkt, from_grank);
	while (MPID_SMI_SendControl( &pkt_desc ) != MPI_SUCCESS)
		;
	SIGNAL_REMOTE(from_grank);

	/* The de-registering of the source buffer and the un-import of the 
		remote buffer could be performed *after* setting is_complete to 1 - 
		but could this cause	race conditions when the same buffer is used 
		again immeadeletly by the application thread? */
	switch (shandle->recv_handle->mode) {
	case RNDV_ASYNC_SEND_DMA:
		/* Free LS memory */
		MPID_SMI_shfree (shandle->recv_handle->dma_outbuf_addr);
		break;
	case RNDV_ASYNC_SEND_DMAZC:
		/* Unregister user send-buffer - CACHE may be dangerous if the memory was not
			allocated through MPI: if it is free'd and new memory is allocated which has the
			same virtual address, SMI will think it is registered (but it is not!). 
			XXX: If we can ensure that all memory allocation/de-allocation is done through MPI,
			we can cache the registering of any buffer. */
		MPID_SMI_Local_mem_release (NULL, shandle->recv_handle->smi_regid_src, MPID_SMI_register_flag);
		MPID_SMI_Rmt_region_release (shandle->recv_handle->smi_regid_dest, MPID_SMI_connect_flag);
		break;
	case RNDV_ASYNC_SEND_DMAZC_PERS:
		/* disconnect from remote region (DESTROY because the receiver will withdraw it, too */
		MPID_SMI_Local_mem_release (NULL, shandle->recv_handle->smi_regid_src, MPID_SMI_RSRC_CACHE);
		MPID_SMI_Rmt_region_release (shandle->recv_handle->smi_regid_dest, MPID_SMI_connect_flag);
		break;
	case RNDV_ASYNC_SEND_PIO:
		MPID_SMI_Rmt_mem_release (NULL, shandle->recv_handle->smi_regid_dest, MPID_SMI_RSRC_CACHE);
		break;
	}

	COMPLETE_SHANDLE_ASYNC( shandle );
	RNDV_SEND_FINISHED( from_grank );

	MPID_STAT_EXIT( arndv_ack_sendzc );
	return MPI_SUCCESS;
}


/*
 * Receiver: MPID_PKT_CONT was received and we are going to read the
 * data, which should be present now.
 */
int MPID_SMI_Arndv_recv_ack  (in_pkt, from_grank)
	  void *in_pkt;
	  int from_grank;
{
	MPID_PKT_RNDV_T      prepkt, recv_pkt;
	MPID_SMI_CTRLPKT_T 	pkt_desc;
	MPIR_RHANDLE *rhandle = 0;
	int 		len, cpy_len, offset, is_done;
	char 		*rndv_addr;

	MPID_STAT_ENTRY (arndv_ack_recv);
	MPID_TRACE_CODE ("NRndvn_Recv_ack", from_grank);

	/* Data is available */
	MEMCPY (&recv_pkt, in_pkt, sizeof(MPID_PKT_RNDV_T));
	MPID_SMI_FreeRecvPkt ((MPID_PKT_T*)in_pkt, from_grank, IS_CTRL_MSG);

	MPID_AINT_GET(rhandle, recv_pkt.recv_id);
	VALIDATE_HANDLE (rhandle);
	MPID_CHECK_COOKIE (rhandle);
	/* Check if this is a new incoming data transfer: len is (in this case) the number
		of partial transfer already performed. */
	if (recv_pkt.len == 0)
		RNDV_RECV_STARTED(from_grank);

	switch (rhandle->recv_handle->mode) {
	case RNDV_ASYNC_RECV:
		len    = recv_pkt.len_avail;
		offset = recv_pkt.data_offset;
		/* recv_pkt.len contains the number of partial transfers that have been done */
		rndv_addr = MPID_SMI_rndv_scipool + recv_pkt.sgmt_offset 
			+ offset - recv_pkt.len*rhandle->recv_handle->len_local;

		if (len > 0) {
			/* If we have some data here, we copy it to the applications receive buffer */
			MPID_STAT_CALL(arndv_ls2buf);
			MEMCPY((char *)rhandle->buf + offset, rndv_addr, len);
			MPID_STAT_RETURN(arndv_ls2buf);
		}

		/* Have we received all the data to come? */
		if (len + offset >= rhandle->s.count) {
			COMPLETE_RHANDLE_ASYNC( rhandle );

			if (rhandle->s.count > 0)
				MPID_SMI_Rndv_free_recvbuf (recv_pkt.sgmt_offset, from_grank);
			RNDV_RECV_FINISHED(from_grank);
		} else {
			ulong data_offset = (recv_pkt.len + 1) * rhandle->recv_handle->len_local;
			/* There is still data to receive. We wait for the next packet */
			MPID_SMI_DEBUG_PRINT_MSG("R about to get packet for ok to send");
			MPID_GETSENDPKT (pkt_desc, sizeof(MPID_PKT_RNDV_T), &prepkt, 0, NULL, from_grank, 0);
			
			MPID_INIT_RNDVOK_PREPKT(prepkt, recv_pkt.len, recv_pkt.send_id, recv_pkt.sgmt_offset, 
											recv_pkt.recv_id, recv_pkt.sgmt_id, recv_pkt.adpt_nbr,
											rhandle->recv_handle->len_local, data_offset, recv_pkt.flags);
			
			MPID_SMI_DEBUG_PRINT_BASIC_SEND_PKT("R Sending ok-to-send message", pkt_desc.pkt, from_grank);
			while (MPID_SMI_SendControl( &pkt_desc) != MPI_SUCCESS)
				;
			SIGNAL_REMOTE(from_grank);
		}
		break;
	case RNDV_ASYNC_RECV_ZC:
		/* XXX We need to de-register every time because the user may free() this buffer, 
			than call malloc() again which may deliver the same virtual addresses, but other 
			physical pages behind them. Possible work-around would be to redirect all 
			malloc()/free() calls through MPI_Alloc_mem/Free_mem(). */
		MPID_SMI_Local_mem_release (NULL, rhandle->recv_handle->smi_regid_dest, MPID_SMI_register_flag);
		COMPLETE_RHANDLE_ASYNC( rhandle );
		RNDV_RECV_FINISHED(from_grank);
		break;
	case RNDV_ASYNC_RECV_ZC_PERS:
		MPID_SMI_Local_mem_release (NULL, rhandle->recv_handle->smi_regid_dest, MPID_SMI_RSRC_CACHE);
		COMPLETE_RHANDLE_ASYNC( rhandle );
		RNDV_RECV_FINISHED(from_grank);
		break;
	}

	MPID_STAT_EXIT (arndv_ack_recv);
	return MPI_SUCCESS;
}


/* This is the routine called when a packet of type
 * MPID_PKT_REQUEST_SEND is seen and the receive has been posted
 * (MPI_recv was called). If the receive was not posted,
 * MPID_SMI_arndv_save_start is called instead). Note the use of a
 * non-blocking receiver BEFORE sending the ack. */
int MPID_SMI_Arndv_irecv( rhandle, from_grank, in_pkt )
	  MPIR_RHANDLE *rhandle;
	  int          from_grank;
	  void         *in_pkt;
{
	MPID_PKT_RNDV_T    *recv_pkt = (MPID_PKT_RNDV_T *)in_pkt;
	MPID_PKT_RNDV_T     prepkt;
	MPID_SMI_CTRLPKT_T  pkt_desc;
	MPID_Aint recv_id;
	ulong sgmt_offset = 0;
	int sgmt_id = 0, adpt_nbr = 0, flags, msglen, err = MPI_SUCCESS;

	MPID_STAT_ENTRY(arndv_irecv);
	MPID_TRACE_CODE ("Arndv_irecv", from_grank);

	msglen = recv_pkt->len;		  /* complete length of data to receive */
	flags  = recv_pkt->flags;
	MPID_CHK_MSGLEN(rhandle, msglen, err); 	/* Check for truncation */
	
	/* fill up recv handle */
	ALLOC_RNDVINFO_RECV(rhandle);

	rhandle->s.count	      = msglen;
	rhandle->s.MPI_TAG	   = recv_pkt->tag;
	rhandle->s.MPI_SOURCE   = recv_pkt->lrank;
	rhandle->s.MPI_ERROR    = err;
	rhandle->send_id	      = recv_pkt->send_id;
	rhandle->wait	         = MPID_SMI_Arndv_unxrecv_end;
	rhandle->test	         = MPID_SMI_Arndv_unxrecv_test_end;
	rhandle->cancel         = MPID_SMI_Arndv_cancel_recv;
	rhandle->push	         = MPID_SMI_Arndv_unxrecv_start;
	rhandle->is_complete    = 0;
#if WAIT_ON_LOCK
	rhandle->recv_handle->complete_lock = arndv_get_lock();
	MPID_SMI_LOCK (rhandle->recv_handle->complete_lock);
#else 
	rhandle->recv_handle->complete_lock = NULL;
#endif
	rhandle->recv_handle->dma_outbuf_len = recv_pkt->len_avail;

	arndv_setup_recvbuf(rhandle, from_grank, msglen, &flags, &sgmt_id, &sgmt_offset, &adpt_nbr);

	rhandle->recv_handle->flags = flags;
	rhandle->is_valid = 1;

	/* Send back an "ok to  proceed" packet: fill up the new send packet with 
		information, partly taken from the received packet */
	MPID_AINT_SET(recv_id, rhandle);
	MPID_INIT_RNDVOK_PREPKT(prepkt, msglen, recv_pkt->send_id, sgmt_offset, recv_id, sgmt_id, 
									adpt_nbr, rhandle->recv_handle->len_local, 0, flags);

	/* now we can free the received packet */
	MPID_SMI_FreeRecvPkt ((MPID_PKT_T *)recv_pkt, from_grank, IS_CTRL_MSG);    

	MPID_SMI_DEBUG_PRINT_MSG("R about to get packet for ok to send");
	MPID_GETSENDPKT (pkt_desc, sizeof(MPID_PKT_RNDV_T), &prepkt, 0, NULL, from_grank, 0);

	MPID_SMI_DEBUG_PRINT_BASIC_SEND_PKT("R Sending ok-to-send message",pkt_desc.pkt, from_grank);
	while (MPID_SMI_SendControl( &pkt_desc ) != MPI_SUCCESS)
		;
	SIGNAL_REMOTE(from_grank);

	RNDV_RECV_SCHEDULED(from_grank);
	MPID_STAT_EXIT(arndv_irecv);

	return err;
}

/* 
 * It is called when the receiver process posts a receive and the
 * MPID_PKT_REQUEST_SEND was received unxepected.
 * This routine is called when it is time to receive an unexpected
 * message. It allocates local shared memory and returns its adddress to
 * the sender.
 */
int MPID_SMI_Arndv_unxrecv_start (rhandle, in_runex)
	  MPIR_RHANDLE *rhandle;
	  void         *in_runex;
{
	MPID_SMI_CTRLPKT_T pkt_desc;
	MPID_PKT_RNDV_T prepkt;
	MPIR_RHANDLE  *runex = (MPIR_RHANDLE *)in_runex;
	MPID_Aint recv_id;
	ulong sgmt_offset = 0;
	int msglen, i, sgmt_id = 0, adpt_nbr = 0, do_zc, flags;

	MPID_TRACE_CODE ("Arndv_unexrecv_start", runex->from);

	rhandle->recv_handle = runex->recv_handle;
	rhandle->s		  = runex->s;
	rhandle->send_id = runex->send_id;
	rhandle->wait	  = MPID_SMI_Arndv_unxrecv_end;
	rhandle->test	  = MPID_SMI_Arndv_unxrecv_test_end;
	rhandle->cancel  = MPID_SMI_Arndv_cancel_recv;
	rhandle->push	  = 0;
	rhandle->from    = runex->from;
	rhandle->send_id = runex->send_id;
	rhandle->is_complete = 0;	/* reset to false */
#if WAIT_ON_LOCK
	rhandle->recv_handle->complete_lock = arndv_get_lock();
	MPID_SMI_LOCK (rhandle->recv_handle->complete_lock);
#else 
	rhandle->recv_handle->complete_lock = NULL;
#endif

	flags = rhandle->recv_handle->flags;
	msglen = runex->s.count;

	arndv_setup_recvbuf(rhandle, rhandle->from, msglen, &flags, &sgmt_id, &sgmt_offset, &adpt_nbr);

	rhandle->is_valid      = 1;
	MPID_AINT_SET(recv_id, rhandle);
	MPID_INIT_RNDVOK_PREPKT(prepkt, msglen, rhandle->send_id, sgmt_offset, recv_id, sgmt_id, 
									adpt_nbr, rhandle->recv_handle->len_local, 0, flags);

	MPID_SMI_DEBUG_PRINT_MSG("R about to get packet for ok to send");
	MPID_GETSENDPKT (pkt_desc, sizeof(MPID_PKT_RNDV_T), &prepkt, 0, NULL, rhandle->from, 0);

	MPID_SMI_DEBUG_PRINT_BASIC_SEND_PKT("R Sending ok-to-send message", pkt_desc.pkt, pkt_desc.dest);
	while (MPID_SMI_SendControl( &pkt_desc ) != MPI_SUCCESS)
		;
	SIGNAL_REMOTE(rhandle->from);

	MPID_Recv_free( runex );
	RNDV_RECV_SCHEDULED(from_grank);

	return 0;
}

/* User called MPI_Wait for a recv request.
 *
 * This is the wait routine for a rendezvous message that was unexpected.
 * A request for the message has already been sent and the receive 
 * transfer has been started.  We wait for the "continue get" packets
 * to set the completed bit.
 */
int MPID_SMI_Arndv_unxrecv_end (MPIR_RHANDLE *rhandle)
{
	MPID_DeviceCheck (MPID_NOTBLOCKING);

	while (!rhandle->is_complete) {
		if (rhandle->recv_handle->complete_lock != NULL) {
			MPID_SMI_LOCK (rhandle->recv_handle->complete_lock);
			MPID_SMI_UNLOCK (rhandle->recv_handle->complete_lock);
		} else {
			MPID_DeviceCheck (MPID_NOTBLOCKING);
		}
	}
	arndv_free_lock(rhandle->recv_handle->complete_lock);
	
	if (rhandle->finish)
		(rhandle->finish) (rhandle);

	return MPI_SUCCESS;
}

/* User called MPI_Test for a recv request.
 *
 * This is the test routine for a rendezvous message that was unexpected.
 * A request for the message has already been sent, and the receive has been
 * started.
 */
int MPID_SMI_Arndv_unxrecv_test_end (MPIR_RHANDLE *rhandle)
{
	MPID_DeviceCheck (MPID_NOTBLOCKING);

	if (rhandle->is_complete) {
		arndv_free_lock(rhandle->recv_handle->complete_lock);

		if (rhandle->finish) 
			(rhandle->finish) (rhandle);
	} else {
		MPID_DeviceCheck (MPID_NOTBLOCKING);
	}

	return MPI_SUCCESS;
}

/* User called MPI_Wait for a send request.
 *
 * To avoid polling for asynchronous communication (which is performed by a
 * the device thread), we instead let the application thread block at this
 * place, and have the device thread signal it once the transfer is complete.
 */
int MPID_SMI_Arndv_send_wait_ack (shandle)
	  MPIR_SHANDLE *shandle;
{
	MPID_STAT_ENTRY (arndv_send_w_ack);
	MPID_SMI_DEBUG_PRINT_MSG("Waiting for Arndv ack");

	MPID_DeviceCheck (MPID_NOTBLOCKING);
	while (!shandle->is_complete) {
		if (shandle->recv_handle->complete_lock != NULL) {
			MPID_SMI_LOCK (shandle->recv_handle->complete_lock);
			MPID_SMI_UNLOCK (shandle->recv_handle->complete_lock);
		} else {
			/* Check queue and react to message if necessary */
			MPID_DeviceCheck (MPID_NOTBLOCKING);
		}
	}
	arndv_free_lock(shandle->recv_handle->complete_lock);

	MPID_STAT_EXIT (arndv_send_w_ack);
	return MPI_SUCCESS;
}

/* User called MPI_Test for a send request.
 *
 * Check, if there is a message in the queue. If, it will be forwarded 
 * to the appropiate handler automagically (in MPID_DeviceCheck).
 */
int MPID_SMI_Arndv_send_test_ack (MPIR_SHANDLE * shandle)
{
	MPID_SMI_DEBUG_PRINT_MSG("Testing for NRndvn ack" );

	MPID_DeviceCheck (MPID_NOTBLOCKING);
	if (!shandle->is_complete && shandle->test == MPID_SMI_Arndv_send_test_ack) {
		/* Check queue and react to message if necessary */
		MPID_DeviceCheck (MPID_NOTBLOCKING);
	}

	if (shandle->is_complete && shandle->recv_handle->complete_lock != NULL) {
		arndv_free_lock(shandle->recv_handle->complete_lock);
	}

	return MPI_SUCCESS;
}

/* 
 * Free ressources that the posted send has already allocated.
 */
int MPID_SMI_Arndv_cancel_send (MPIR_SHANDLE *shandle)
{
	/* free locally allocated buffers (or registered memory areas) */
	switch (shandle->recv_handle->mode) {
	case RNDV_ASYNC_SEND_DMA:
		MPID_SMI_shfree(shandle->recv_handle->dma_outbuf_addr);
		break;
	case RNDV_ASYNC_SEND_DMAZC:
		MPID_SMI_Local_mem_release (NULL, shandle->recv_handle->smi_regid_src, MPID_SMI_register_flag);
		break;
	}    
    
	FREE_RNDVINFO( shandle );
	RNDV_SEND_FINISHED(shandle->partner);

	return MPI_SUCCESS;
}

/* when a rndv-recv is canceled, no data transfer has yet taken place
   (canceling only succeeds if the request was still located in the 
   unexpected queue), thus no recv buffers to be free'd, only the handle */
int MPID_SMI_Arndv_cancel_recv (MPIR_RHANDLE *runex)
{
	FREE_RNDVINFO( runex );
	MPID_Recv_free(runex);

	RNDV_RECV_FINISHED(runex->partner);

	return MPI_SUCCESS;
}


/*  Overrides for XEmacs and vim so that we get a uniform tabbing style.
 *  XEmacs/vim will notice this stuff at the end of the file and automatically 
 *  adjust the settings for this buffer only.  This must remain at the end 
 *  of the file. 
 *  ---------------------------------------------------------------------------
 *  Local variables: 
 *  c-indent-level: 3 
 *  c-basic-offset: 3 
 *  tab-width: 3 
 *  End: 
 *  vim:tw=0:ts=3:wm=0: 
 */
