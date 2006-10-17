/* $Id$ */

/* Synchronous, non-blocking rendez-vous protocol: partial sends of the 
   user data are performed separately, synchronized via control messages.
   Other (control) messages can be processed in-between. */

#include "smirndv.h"
#include "smicoll.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* imports */
extern MPID_SMI_Rndv_int_t MPID_SMI_Rndv_int;
extern int MPID_SMI_Locregid_rndv;


/* Deliver an offset relative ot one of the incoming pools to where the sender can write data.
   The desired length is given in len, in which the length which could be allocated is
   returned. dest is usually the local process, but could be another in the future. */
int MPID_SMI_Nbrndv_get_recvbuf( sgmt_id, sgmt_offset, adpt_nbr, len, sender )
ulong *sgmt_offset;
int   *len, sender, *sgmt_id, *adpt_nbr;
{
    char *addr;
    int tlen, shreg_id, ret = MPI_SUCCESS, stat_value;

    MPID_STAT_ENTRY(setup_rndvadr);
    MPID_TRACE_CODE("nbrndv_get_recvbuf", sender);
    MPID_SMI_DEBUG_PRINT_MSG("Entering nbrndv_get_recvbuf");

    if (*len == 0)
	return ret;

    tlen = (MPID_SMI_RNDVRECEIPT == 0 || *len < MPID_SMI_RNDVBLOCK*MPID_SMI_RNDVRECEIPT) ? 
	*len : MPID_SMI_RNDVBLOCK*MPID_SMI_RNDVRECEIPT;
    MPID_SMI_STREAMBUF_ALIGN (tlen);

    /* determine the shared memory region to use */
    *sgmt_id  = (MPID_SMI_use_localseg[sender]) ? -1 : MPID_SMI_Rndv_sgmt_id;
    *adpt_nbr = (MPID_SMI_use_localseg[sender]) ? -1 : MPID_SMI_Rndv_adpt_nbr;
    
    shreg_id = (MPID_SMI_use_localseg[sender]) ? MPID_SMI_rndv_shmempool_regid : MPID_SMI_rndv_scipool_regid;
    addr = (char *)MPID_SMI_shmalloc(tlen, shreg_id);
    while (addr == NULL) {
	MPID_STAT_COUNT(rndvmem_split);
	MPID_SMI_DEBUG_PRINT_MSG("Allocating partial space for long message");

	do {
	  tlen /= 2; 
	  addr = (char *)MPID_SMI_shmalloc(tlen, shreg_id);
	} while (addr == NULL && tlen >= 2*MPID_SMI_RNDV_MINMEM);

	if (addr == NULL) {
#if 0
	    MPID_ASSERT (MPID_SMI_Rndvrecvs_in_progress > 0, 
			 "Could not allocate rndv incoming buffer although no recvs are active.");
#endif
	    MPID_DeviceCheck (MPID_NOTBLOCKING);
	    tlen = *len * 2;
	}
    }
    MPID_SMI_DEBUG_PRINT_RNDV_ADDRESS_ALLOC();

    /* transform ptr into an offset */
    *sgmt_offset = (MPID_SMI_use_localseg[sender]) ? 
	(size_t)(addr - (size_t)MPID_SMI_rndv_shmempool) : 
	(size_t)(addr - (size_t)MPID_SMI_rndv_scipool);

    *len = tlen;
    if (*len == 0)
	ret = MPI_ERR_INTERN;

    stat_value = tlen >> 10;
    MPID_STAT_PROBE(rndv_inbuf_size, stat_value);
    MPID_STAT_EXIT(setup_rndvadr);

    return ret;
}


/* push_recv() is called when a packet of type MPID_PKT_PART_READY 
   has arrived - it is to speed up the transmission of long messages
   by interleaving the remote- and local copy operations */
int MPID_SMI_Nbrndv_push_recv (in_pkt, from_grank)
void  *in_pkt;
int   from_grank;
{
    MPID_PKT_RNDV_T recv_pkt;
    MPIR_RHANDLE *rhandle = 0;
    char *rndv_addr;

    MPID_STAT_ENTRY(rndv_pushrecv);
    MPID_TRACE_CODE ("Rndvn_push_recv", from_grank);
    MPID_SMI_DEBUG_PRINT_MSG("Entering Rndvn_push_recv");

    MEMCPY ((void *)&recv_pkt, in_pkt, sizeof(MPID_PKT_RNDV_T));
    MPID_SMI_FreeRecvPkt (in_pkt, from_grank, IS_CTRL_MSG);

    MPID_AINT_GET(rhandle, recv_pkt.recv_id);

    /* transform offset into a ptr */
    rndv_addr = MPID_SMI_use_localseg[from_grank] ? MPID_SMI_rndv_shmempool : MPID_SMI_rndv_scipool;
    rndv_addr += recv_pkt.sgmt_offset + (recv_pkt.data_offset - rhandle->recv_handle->len_local*recv_pkt.len);

    /* First, check for "direct reduce" */
    if (rhandle->op_ptr != NULL) {
	long count = (long)recv_pkt.len_avail/rhandle->datatype->size;

	/* direct reduction operation which also requires a valid datatype_ptr! */
	MPID_ASSERT(rhandle->datatype != NULL, "Missing datatype ptr for direct reduce operation!");
	CALL_OP_FUNC ((char *)rhandle->buf + recv_pkt.data_offset, rndv_addr, count, 
		      rhandle->op_ptr, rhandle->datatype);
    } else 
	if (rhandle->buf == 0) {
	    /* Non-contigous data */
	    MPID_SMI_UnPack_ff (rndv_addr, rhandle->datatype, (char *)rhandle->start, from_grank, 
				recv_pkt.len_avail, recv_pkt.data_offset);
	} else {
	    /* "normal" data */
	    MPID_STAT_CALL(rndv_pushrcopy);
	    MEMCPY_R ((char *)rhandle->buf + recv_pkt.data_offset, rndv_addr, recv_pkt.len_avail);
	    MPID_STAT_RETURN(rndv_pushrcopy);
	}

    MPID_STAT_EXIT(rndv_pushrecv);
    return MPI_SUCCESS;
}	


int MPID_SMI_Nbrndv_recv_ack(in_pkt, from_grank)
void *in_pkt;
int from_grank;
{
    MPID_PKT_RNDV_T recv_pkt, prepkt;
    MPID_SMI_CTRLPKT_T pkt_desc;
    MPIR_RHANDLE *rhandle=0;
    char *rndv_addr;

    MPID_STAT_ENTRY(rndv_ack_recv);
    MPID_TRACE_CODE ("Rndvn_Recv_ack", from_grank);
    MPID_SMI_DEBUG_PRINT_MSG("Entering Rndvn_Recv_ack");

    MEMCPY (&recv_pkt, in_pkt, sizeof(MPID_PKT_RNDV_T));
    MPID_SMI_FreeRecvPkt((MPID_PKT_T*)in_pkt, from_grank, IS_CTRL_MSG);

    MPID_AINT_GET(rhandle, recv_pkt.recv_id);
    VALIDATE_HANDLE (rhandle);
    MPID_CHECK_COOKIE (rhandle);
    /* Check if this is a new incoming data transfer: len is (in this case) 
       the number of partial transfer already performed. */
    if (recv_pkt.len == 0)
	RNDV_RECV_STARTED(from_grank);

    /* copy data that was not copied via the push_send/recv mechanism */
    if (recv_pkt.len_avail > 0) {
	switch (rhandle->recv_handle->mode) {
	case RNDV_SYNC:
	    /* transform offset into a ptr */
	    rndv_addr = MPID_SMI_use_localseg[from_grank] ? MPID_SMI_rndv_shmempool : MPID_SMI_rndv_scipool;
	    rndv_addr += recv_pkt.sgmt_offset + recv_pkt.data_offset - recv_pkt.len*rhandle->recv_handle->len_local;
	    
	    /* First, check for "direct reduce" */
	    if (rhandle->op_ptr != NULL) {
		long count = (long)recv_pkt.len_avail/rhandle->datatype->size;

		/* direct reduction operation which also requires a valid datatype_ptr! */
		MPID_ASSERT(rhandle->datatype != NULL, "Missing datatype ptr for direct reduce operation!");
		CALL_OP_FUNC ((char *)rhandle->buf + recv_pkt.data_offset, rndv_addr, count, 
			      rhandle->op_ptr, rhandle->datatype);
	    } else 
		if (rhandle->buf == NULL) {
		    MPID_SMI_UnPack_ff (rndv_addr, rhandle->datatype, (char *)rhandle->start, 
					from_grank, recv_pkt.len_avail, recv_pkt.data_offset);
		} else {
		    MPID_STAT_CALL(rndv_rcopy);
		    MEMCPY_R((char *)rhandle->buf + recv_pkt.data_offset, rndv_addr, recv_pkt.len_avail);
		    MPID_STAT_RETURN(rndv_rcopy);
		}
	    break;
	case RNDV_SYNC_RECV_ZC:
	case RNDV_SYNC_RECV_ZC_PERS:
	    /* Nothing to do - this is zero copy! ;-) */
	    break;
	}
    }

    /* Check if we are finished here - request remaining data or clean up. */
    if (recv_pkt.len_avail + recv_pkt.data_offset < rhandle->s.count) {
	MPID_INIT_RNDVOK_PREPKT(prepkt, recv_pkt.len, recv_pkt.send_id, recv_pkt.sgmt_offset, 
				recv_pkt.recv_id, 0, 0, rhandle->recv_handle->len_local, 
				(recv_pkt.len + 1)*rhandle->recv_handle->len_local, MPID_SMI_RNDV_PIOONLY);
	MPID_GETSENDPKT(pkt_desc, sizeof(MPID_PKT_RNDV_T), &prepkt, 0, 0, from_grank, 0);

	MPID_SMI_DEBUG_PRINT_BASIC_SEND_PKT("R Sending ok-to-send message", &prepkt, from_grank);
	while (MPID_SMI_SendControl( &pkt_desc) != MPI_SUCCESS)
	    ;
    } else {
	/* We have all the data; the transfer is complete - release ressources */
	switch (rhandle->recv_handle->mode) {
	case RNDV_SYNC:
	    /* free incoming buffer */
		MPID_SMI_DEBUG_PRINT_RNDV_ADDRESS_FREE();
	    MPID_SMI_Rndv_free_recvbuf(recv_pkt.sgmt_offset, from_grank);
	    break;
	case RNDV_SYNC_RECV_ZC:
	    /* Un-register user-supplied receive buffer */
	    MPID_SMI_Local_mem_release (NULL, rhandle->recv_handle->smi_regid_dest, MPID_SMI_register_flag);
	    break;
	case RNDV_SYNC_RECV_ZC_PERS:
	    /* Registrationn of persistent recv buffers will always be cached. */
	    MPID_SMI_Local_mem_release (NULL, rhandle->recv_handle->smi_regid_dest, MPID_SMI_RSRC_CACHE);
	    break;
	}

	COMPLETE_RHANDLE(rhandle);
	RNDV_RECV_FINISHED(from_grank);
    }

    MPID_STAT_EXIT (rndv_ack_recv);

    return MPI_SUCCESS;
}


/* push_send() is called for sending long rndv-messages in parts
   to allow simoultaneous write of the sender and reads of the receiver:
   after each block of MPID_SMI_RNDVBLOCK bytes, a control msg of type
   MPID_PKT_PART_READY is send to indicate the receiver that he can
   transfer this part of the message to his private memory while the
   copy process goes on. */
int MPID_SMI_Nbrndv_push_send( rndv_pkt, shandle, to_grank, len )
MPID_PKT_RNDV_T *rndv_pkt;
MPIR_SHANDLE    *shandle;
int to_grank;
int len;
{
    MPID_SMI_CTRLPKT_T pkt_desc;
    MPID_PKT_RNDV_T prepkt;
    char *rndv_addr = (char *)shandle->recv_handle->dest_addr;
    char *from_addr = ((char *)shandle->start) + rndv_pkt->data_offset;
    int blocks, nbr_blocks, block_mode, unancd_blocks = 0;  /* blocks that are copied but not announced */
    int push_offset, nbr_parts;

    MPID_STAT_ENTRY(rndv_pushsend);
    MPID_TRACE_CODE ("Rndvn_push_send", to_grank);
    MPID_SMI_DEBUG_PRINT_MSG("Entering Rndvn_push_send");

    pkt_desc.dest   = to_grank;
    pkt_desc.hsize  = sizeof(MPID_PKT_RNDV_T);
    pkt_desc.header = (MPID_PKT_T *)&prepkt;
    pkt_desc.dsize  = 0;
    pkt_desc.data   = NULL;

    push_offset = rndv_pkt->data_offset;
    nbr_parts = rndv_pkt->data_offset/rndv_pkt->len_avail;
    /* copy a number of blocks, trying to inform the receiver on the advance */
    for (nbr_blocks = len/MPID_SMI_RNDVBLOCK, blocks = 1; blocks <= nbr_blocks; blocks++) {
	if (MPID_SMI_cfg.NC_ENABLE && shandle->datatype && !shandle->datatype->is_contig) {
	    push_offset += MPID_SMI_Pack_ff ((char *)shandle->start, shandle->datatype, rndv_addr, 
					     to_grank, MPID_SMI_RNDVBLOCK, push_offset);
	} else {
	    MPID_STAT_CALL(rndv_pushscopy);
	    MEMCPY_W(rndv_addr, from_addr, MPID_SMI_RNDVBLOCK, to_grank);
	    MPID_STAT_RETURN(rndv_pushscopy);
	}
	unancd_blocks++;

	/* chose blocking or non-blocking request for a pkt:
	   if we don't get a pkt, we forget it and contiue copying unless
	   the current block is the last one (which needs to be announced) */
	block_mode = (blocks < nbr_blocks);

	if (MPID_SMI_GetSendPkt (block_mode, &pkt_desc) == MPI_SUCCESS) {
	    /* 'len' gets "abused" here: it indicates how many partial sends
	       (not block transfers!) have already been done */
	    MPID_INIT_RNDVPART_PREPKT(prepkt, nbr_parts, rndv_pkt->send_id, rndv_pkt->sgmt_offset, rndv_pkt->recv_id, 
				      unancd_blocks*MPID_SMI_RNDVBLOCK, 
				      rndv_pkt->data_offset + (blocks - unancd_blocks)*MPID_SMI_RNDVBLOCK);
	    
	    MPID_SMI_DEBUG_PRINT_BASIC_SEND_PKT("S Sending part-ready message", &prepkt, to_grank);
	    while (MPID_SMI_SendControl( &pkt_desc ) != MPI_SUCCESS)
		;

	    unancd_blocks = 0;
	} 

	rndv_addr += MPID_SMI_RNDVBLOCK;
	from_addr += MPID_SMI_RNDVBLOCK;
    }

    MPID_STAT_EXIT(rndv_pushsend);
    return (blocks-1)*MPID_SMI_RNDVBLOCK;
}


int MPID_SMI_Nbrndv_send_ack(in_pkt, from_grank)
void *in_pkt;
int from_grank;
{
    MPID_PKT_RNDV_T recv_pkt, prepkt;
    MPID_SMI_CTRLPKT_T pkt_desc;
    MPIR_SHANDLE *shandle = 0;
    int len, cpy_len, is_done, partial_sends, err;
    int push_len = 0, connect_tries;
    char *rndv_addr;

    MPID_STAT_ENTRY(rndv_ack_send);
    MPID_TRACE_CODE ("Rndvn_Send_ack", from_grank);
    MPID_SMI_DEBUG_PRINT_MSG("Entering Rndvn_Send_Ack");

    MEMCPY (&recv_pkt, in_pkt, sizeof(MPID_PKT_RNDV_T));
    MPID_SMI_FreeRecvPkt( (MPID_PKT_T*)in_pkt, from_grank, IS_CTRL_MSG);

    MPID_AINT_GET(shandle,recv_pkt.send_id);
    VALIDATE_HANDLE (shandle);
    MPID_CHECK_COOKIE (shandle);
    RNDV_CHECK_ZEROMSG (&recv_pkt, shandle);
    
    /* First: Make destination buffer accessible.
       If region id is invalid, we still need to connect/import the remote memory. */
    if (recv_pkt.len_avail > 0 && shandle->recv_handle->smi_regid_dest == MPID_SMI_INVALID_REGID) {
		MPID_STAT_PERIOD_END(rndv_sync_delay, shandle->recv_handle->sync_delay);

		/* If send buffer was registered, we assumed we would do zerocopy. But depending on 
		the kind of buffer the recv process has allocated, we need to go back to 2-way DMA. */
		if ((recv_pkt.flags & MPID_SMI_RNDV_DMAOK)
			&& !(recv_pkt.flags & MPID_SMI_RNDV_ZEROCOPY))
			shandle->recv_handle->mode = RNDV_SYNC_SEND_DMA;

		switch (shandle->recv_handle->mode) {
			case RNDV_SYNC:
				MPID_SMI_Rndv_map_remote_mem (from_grank, &recv_pkt, shandle, true);
				break;

			case RNDV_SYNC_SEND_DMAZC:
			case RNDV_SYNC_SEND_DMAZC_PERS:
				MPID_STAT_COUNT( destbuf_imported );
				/* fall through */
			case RNDV_SYNC_SEND_DMA:
				connect_tries = 0;
				do {
				err = MPID_SMI_Rmt_region_connect (from_grank, recv_pkt.sgmt_id, recv_pkt.adpt_nbr, 
								&shandle->recv_handle->smi_regid_dest);
				connect_tries++;
				while (err == MPIR_ERR_EXHAUSTED && MPID_SMI_Rndvsends_in_progress > 0)
					MPID_DeviceCheck (MPID_NOTBLOCKING);
				} while (err == MPIR_ERR_EXHAUSTED && connect_tries <= MPID_SMI_RNDV_CNCT_MAX_RETRIES);

				MPID_ASSERT (err != MPIR_ERR_EXHAUSTED, "Could not connect to remote memory.");
				break;

			default:
				MPID_ABORT ("Illegal send mode in nbrndv_send_ack().");
				break;
		}
    }
    
    /* Check if this is a new outgoing data transfer. */
    if (recv_pkt.data_offset == 0)
	RNDV_SEND_STARTED(from_grank);

    /* Compute length available to send.  If this is it,
       remember so that we can mark the operation as complete */
    len = shandle->bytes_as_contig - recv_pkt.data_offset;
    if (len > recv_pkt.len_avail) {
	len = recv_pkt.len_avail;
	is_done = 0;
    } else {
	is_done = 1;
    }
	    
    /* Second: Transfer the data. */
    cpy_len = len;
    if (len > 0) {
	switch (shandle->recv_handle->mode) {
	case RNDV_SYNC:
	    MEMCPYSYNC_ENTER(from_grank, len);
	    if (MPID_SMI_RNDVBLOCK > 0) {
		/* initiate pipelining of remote write and local read for long messages */
		push_len = MPID_SMI_Nbrndv_push_send (&recv_pkt, shandle, from_grank, len);
		cpy_len -= push_len;
	    }
	    /* now copy the rest of the data, if there is any */
	    if (cpy_len > 0) {
		/* transform offset into a ptr */
		rndv_addr = MPID_SMI_use_localseg[from_grank] ?
		    MPID_SMI_rndv_shmempool + (ulong)recv_pkt.sgmt_offset :
		    (char *)shandle->recv_handle->dest_addr;
		rndv_addr += push_len;
		
		if (MPID_SMI_cfg.NC_ENABLE && shandle->datatype && !shandle->datatype->is_contig) {
		    MPID_SMI_Pack_ff ((char *)shandle->start, shandle->datatype, rndv_addr, 
				      from_grank, cpy_len, recv_pkt.data_offset + push_len);
		} else {
		    MPID_STAT_CALL(rndv_scopy);
		    MEMCPY_W(rndv_addr, (char *)shandle->start + recv_pkt.data_offset + push_len, 
			     cpy_len, from_grank);
		    MPID_STAT_RETURN(rndv_scopy);
		}
	    }
	    SCI_SYNC_WRITE (from_grank);
	    MEMCPYSYNC_LEAVE (from_grank, len);
	    break;
	case RNDV_SYNC_SEND_DMA:
	    MPID_STAT_CALL( rndv_dma );
	    SMIcall (SMI_Put (shandle->recv_handle->smi_regid_dest, recv_pkt.sgmt_offset, 
			      (char *)shandle->start + recv_pkt.data_offset, cpy_len));
	    MPID_STAT_RETURN( rndv_dma );
	    break;
	case RNDV_SYNC_SEND_DMAZC:
	case RNDV_SYNC_SEND_DMAZC_PERS:
	    MPID_STAT_CALL( rndv_dma );
	    SMIcall (SMI_Put (shandle->recv_handle->smi_regid_dest, 
			      recv_pkt.sgmt_offset + recv_pkt.data_offset,
			      (char*)shandle->start + recv_pkt.data_offset, cpy_len));
	    MPID_STAT_RETURN( rndv_dma );
	    break;
	}
    }

    /* send 'partial send' ackknowledge message */
    MPID_SMI_DEBUG_PRINT_MSG("S About to get pkt for cont-get msg");
    MPID_GETSENDPKT(pkt_desc, sizeof(MPID_PKT_RNDV_T), &prepkt, 0, 0, from_grank, 0);

    /* this many partial sends have already been done before this one */
    partial_sends = recv_pkt.len_avail ? recv_pkt.data_offset / recv_pkt.len_avail : 0;
    MPID_INIT_RNDVCONT_PREPKT(prepkt, partial_sends, recv_pkt.send_id, recv_pkt.sgmt_offset, 
			      recv_pkt.recv_id, len - push_len, recv_pkt.data_offset + push_len, 0);

    MPID_SMI_DEBUG_PRINT_BASIC_SEND_PKT("S Sending cont-get message", &prepkt, from_grank);
    while (MPID_SMI_SendControl( &pkt_desc ) != MPI_SUCCESS)
	;

    /* Third: Release all resources once we are done. */
    if (is_done) {
	switch (shandle->recv_handle->mode) {
	case RNDV_SYNC:
	    if (!MPID_SMI_use_localseg[from_grank] && shandle->recv_handle->dest_addr != NULL) 
		MPID_SMI_Rmt_mem_release (shandle->recv_handle->dest_addr, 0, MPID_SMI_RSRC_CACHE);
	    break;
	case RNDV_SYNC_SEND_DMA:
	case RNDV_SYNC_SEND_DMAZC:
	    MPID_SMI_Local_mem_release (NULL, shandle->recv_handle->smi_regid_src, MPID_SMI_register_flag);
	    MPID_SMI_Rmt_region_release (shandle->recv_handle->smi_regid_dest, MPID_SMI_connect_flag);
	    break;
	case RNDV_SYNC_SEND_DMAZC_PERS:
	    MPID_SMI_Local_mem_release (NULL, shandle->recv_handle->smi_regid_src, MPID_SMI_RSRC_CACHE);
	    MPID_SMI_Rmt_region_release (shandle->recv_handle->smi_regid_dest, MPID_SMI_connect_flag);
	    break;
	default:
	    MPID_ABORT ("Illegal send mode in Nbrndv_send_ack().");
	    break;
	}

	COMPLETE_SHANDLE( shandle );
	RNDV_SEND_FINISHED(from_grank);
    }

    MPID_STAT_EXIT (rndv_ack_send);
    return MPI_SUCCESS;
}


/*
 * Overrides for XEmacs and vim so that we get a uniform tabbing style.
 * XEmacs/vim will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:tw=0:ts=4:wm=0:
 */
