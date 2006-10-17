/* $Id$ */

/* sending messages of abitrary size, using a handshake-protocol */   

#include "smirndv.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif




/* exports */
int MPID_SMI_RNDVSIZE;
int MPID_SMI_RNDVBLOCK;
int MPID_SMI_RNDVRECEIPT;
int MPID_SMI_RNDVDMASIZE;
int MPID_SMI_RNDVBLOCKING_MODE;

int MPID_SMI_Rndvrecvs_in_progress = 0;    /* nbr of ongoing recvs (data has been received) */
int MPID_SMI_Rndvsends_in_progress = 0;    /* nbr of ongoing sends (transmitting data) */
int MPID_SMI_Rndvrecvs_scheduled = 0;      /* nbr of ongoing recvs (only ok_to_send has been replied) */
int MPID_SMI_Rndvsends_scheduled = 0;      /* nbr of scheduled sends (only request_to_send has been sent) */

int *MPID_SMI_Rndvsend_to_proc;        /* nbr of ongoing sends towards each other processes 
										  (sum is = sens_in_progress) - XXX no longer needed */
int MPID_SMI_register_flag, MPID_SMI_connect_flag;

/* globals for this protocol */
char *MPID_SMI_rndv_scipool;           /* address of the local SCI-exported memory pool for incoming rndv msgs */
int   MPID_SMI_rndv_scipool_regid;     /* SMI region id of this region */
int   MPID_SMI_rndv_scipool_mapsize;
int   MPID_SMI_Rndv_sgmt_id, MPID_SMI_Rndv_adpt_nbr; /* SCI sgmt id and adpt nbr of this region */
char *MPID_SMI_rndv_shmempool;         /* address of the local shared-memory pool for incoming rndv msgs */
int   MPID_SMI_rndv_shmempool_regid;   /* SMI region id of this buffer */

MPID_SMI_Rndv_int_t MPID_SMI_Rndv_int;   /* function pointers for different versions of rndv */
MPID_dataqueue_t MPID_SMI_brndv_queue;  /* queue for storing pending recvs in blocking rndv version */
MPID_SBHeader rndvinfo_allocator;

static int *preconnect_rndv_ids;

/*
 * Definitions of the actual functions
 */


/*
 * The only routing really visable outside this file; it defines the
 * rendez-vous protocol and choses the variant (synchronous blocking/non-blocking or
 * asynchronous) as defined in the device configuration file.
 */
MPID_Protocol *MPID_SMI_Rndv_setup(void)
{
    MPID_Protocol *p;

    ZALLOCATE (p, MPID_Protocol *, sizeof(MPID_Protocol));

    /* set up function pointers for version of rndv protocol that is actually used */
    if (MPID_SMI_RNDVBLOCKING_MODE) {
		/* blocking version (ringbuffers with flow-control by ptrs) */
		MPID_SMI_Brndv_memsetup();
		MPID_INFO(" ok.\n bRNDV : ");
		MPID_SMI_Rndv_int.Recv_ack_sync = MPID_SMI_Brndv_recv_ack;
		MPID_SMI_Rndv_int.Send_ack_sync = MPID_SMI_Brndv_send_ack;
		MPID_SMI_Rndv_int.Setup_rndv_addr = MPID_SMI_Brndv_get_recvbuf;;
    } else {
		/* non-blocking version (flow-control by control messages) */
		MPID_INFO(" ok.\n  RNDV : ");
		MPID_SMI_Rndv_int.Recv_ack_sync = MPID_SMI_Nbrndv_recv_ack;
		MPID_SMI_Rndv_int.Send_ack_sync = MPID_SMI_Nbrndv_send_ack;
		MPID_SMI_Rndv_int.Setup_rndv_addr = MPID_SMI_Nbrndv_get_recvbuf;
    }

    if (MPID_SMI_Rndv_mem_setup() != MPI_SUCCESS) 
		MPID_ABORT("Not enough shared memory for rendez-vous protocol.");
    
    /* initialize the async. rendez-vous protocol */
    if (MPID_SMI_cfg.ASYNC_PROGRESS) {
		if (MPID_SMI_Arndv_setup () != MPI_SUCCESS)
			MPID_SMI_cfg.ASYNC_PROGRESS = 0;
		MPID_SMI_Rndv_int.Recv_ack_async = MPID_SMI_Arndv_recv_ack;
		MPID_SMI_Rndv_int.Send_ack_async = MPID_SMI_Arndv_send_ack;
    }

    p->do_ack = MPID_SMI_Rndv_ack;
    p->delete = MPID_SMI_Rndv_delete;

    p->send   = MPID_SMI_Rndv_send;
    /* the asynchronous rendez-vous is invoked for isend only */
    p->isend  = MPID_SMI_cfg.ASYNC_PROGRESS ? MPID_SMI_Arndv_isend : MPID_SMI_Rndv_isend;
    p->cancel_send = MPID_SMI_Rndv_cancel_send;

    p->irecv  = MPID_SMI_Rndv_irecv;
    p->unex   = MPID_SMI_Rndv_save;
    p->cancel_recv = MPID_SMI_Rndv_cancel_recv;

    ZALLOCATE (MPID_SMI_Rndvsend_to_proc, int *, sizeof(int)*MPID_SMI_numids);

	/* fixed-size-block memory manager */
	rndvinfo_allocator = MPID_SBinit (sizeof(MPID_SMI_RNDV_info), INIT_RNDV_INFOS, INCR_RNDV_INFOS);
    /* create queue for storing pending receives in blocking rndv version */
    MPID_SMI_brndv_queue = MPID_dataqueue_init( MPID_UTIL_THREADSAFE );
    	
	MPID_SMI_register_flag = MPID_SMI_cfg.CACHE_REGISTERED ?
		MPID_SMI_RSRC_CACHE : MPID_SMI_RSRC_DESTROY;
	MPID_SMI_connect_flag = MPID_SMI_cfg.CACHE_CONNECTED ?
		MPID_SMI_RSRC_CACHE : MPID_SMI_RSRC_DESTROY;

    return p;
}

/* allocate SCI memory - this process will only manage the local SCI segment */
int MPID_SMI_Rndv_mem_setup( void )
{
    smi_region_info_t shreg_info;
    int proc, sgmnt_ok, segmode, i; 
	size_t rndv_size, local_size;
    
    /* align rndv memory pool size to page size */
    MPID_SMI_PAGESIZE_ALIGN(MPID_SMI_RNDVSIZE);
    local_size = (size_t)MPID_SMI_RNDVSIZE;
	rndv_size  = (size_t)MPID_SMI_RNDVSIZE;

    /* allocate SCI shared memory for inter-node communication */
    MPID_INFO(" global -");
	/* get as much memory as possible, starting with MPID_SMI_RNDVSIZE */
	sgmnt_ok = MPID_SMI_Local_mem_create (&rndv_size, MPID_SMI_RNDV_MINPOOL, 
										  (void **)&MPID_SMI_rndv_scipool, &MPID_SMI_rndv_scipool_regid, 
										  &MPID_SMI_Rndv_sgmt_id);
	MPID_ASSERT(sgmnt_ok == MPI_SUCCESS, "Out of global shared memory for rendez-vous protocol.");
    
    SMIcall (SMI_Init_shregMMU (MPID_SMI_rndv_scipool_regid));
    SMIcall (SMI_Query (SMI_Q_SMI_REGION_ADPTNBR, MPID_SMI_rndv_scipool_regid, &MPID_SMI_Rndv_adpt_nbr));

    /* allocation of local shared memory if necessary */
    if (MPID_SMI_use_SMP) {
#if 0
		int smp_region_size = (MPID_SMI_numProcsOnNode[MPID_SMI_myNode] > 1) ?  
			MPID_SMI_numProcsOnNode[MPID_SMI_myNode] * local_size : 2*MPID_SMI_PAGESIZE;
#else
		/* XXX This is a workaround for a problem that was reported for 
		   LINUX_X86_64: the MMU_init of a small region (size 2*MPID_SMI_PAGESIZE)
		   caused a SEGV in SMI. I could not test this myself, thus simply 
		   had this workaround tested. Using a large region in this case does not really
		   do any harm. */
		int smp_region_size = MPID_SMI_numProcsOnNode[MPID_SMI_myNode] * local_size;
#endif

		do {
			MPID_INFO(" local -"); 

			SMI_Init_reginfo (&shreg_info, smp_region_size, 0, 0, SMI_ADPT_DEFAULT, 0, 0, NULL);
			/* this region needs to be mapped in FIXED mode (because SMI does not
			   support non-fixed local regions for memory management) */
			sgmnt_ok = SMI_Create_shreg(SMI_SHM_SMP|SMI_SHM_INTERN, &shreg_info, &MPID_SMI_rndv_shmempool_regid, 
										(void **)&MPID_SMI_rndv_shmempool ) == SMI_SUCCESS;

			if (!sgmnt_ok) {
				MPID_INFO(" X");
				MPID_STAT_COUNT(create_sgmnt_fail);
				/* memory allocation did not succeed, we have to use less memory */
				smp_region_size /= 2;
				if (smp_region_size < MPID_SMI_PAGESIZE) {
					MPID_ABORT ("Out of local shared memory for rendez-vous protocol");
				}
				MPID_SMI_PAGESIZE_ALIGN(shreg_info.size);
			}
		} while (!sgmnt_ok);

		SMIcall (SMI_Init_shregMMU(MPID_SMI_rndv_shmempool_regid));
    }

    return MPI_SUCCESS;
}


void MPID_SMI_Rndv_GetConfig (int *number, int *size) {
    *number = MPID_SMI_RNDVBLOCK;
    *size   = MPID_SMI_RNDVSIZE;

    return;
}


/* pre-connect to all remote segments (required for CONNECT_ON_INIT = 1) 
   We do this by sending a short message to each process which contains
   the SCI segment id etc. of the rendez-vous segment. Each receiver uses
   this to map the remote memory. */
void MPID_SMI_Rndv_preconnect( void )
{
	struct _rndv_info {
		int    sgmt_id;
		size_t sgmt_len;
		size_t offset;
		int    adptr;
	};
    int i, proc;
	struct _rndv_info *rmt_rndv_info, my_rndv_info;
	void *rndv_buf;

	ALLOCATE (rmt_rndv_info, struct _rndv_info *, sizeof(struct _rndv_info)*MPID_SMI_numids);	
	ALLOCATE (preconnect_rndv_ids, int *, sizeof(int)*MPID_SMI_numids);	
	my_rndv_info.sgmt_id = MPID_SMI_Rndv_sgmt_id;
	my_rndv_info.sgmt_len = 0; /* map complete region */
	my_rndv_info.offset = 0;
	my_rndv_info.adptr = MPID_SMI_Rndv_adpt_nbr;

	/* Post the recvs for the incoming remote messages, then send.  
	   We will not receive messages from processes on the local node. */
    for (i = 0; i < MPID_SMI_numids; i++) {
		proc = (MPID_SMI_myid + i) % MPID_SMI_numids;
		if (MPID_SMI_is_remote[proc]) {
			SMIcall(SMI_Isend(&my_rndv_info, sizeof(struct _rndv_info), proc));
		}
    }
    for (i = 0; i < MPID_SMI_numids; i++) {
		proc = (MPID_SMI_myid + i) % MPID_SMI_numids;
		if (MPID_SMI_is_remote[proc]) {
			SMIcall(SMI_Recv(&rmt_rndv_info[proc], sizeof(struct _rndv_info), proc));
		}
    }
    for (i = 0; i < MPID_SMI_numids; i++) {
		proc = (MPID_SMI_myid + i) % MPID_SMI_numids;
		if (MPID_SMI_is_remote[proc]) {
			SMIcall(SMI_Send_wait(proc));
		}
    }

    for (i = 0; i < MPID_SMI_numids; i++) {
		proc = (MPID_SMI_myid + i) % MPID_SMI_numids;
		if (MPID_SMI_is_remote[proc]) {
			MPID_SMI_Rmt_mem_map (proc, rmt_rndv_info[proc].sgmt_id, rmt_rndv_info[proc].sgmt_len, 
								  rmt_rndv_info[proc].offset, rmt_rndv_info[proc].adptr, 
								  &rndv_buf, &preconnect_rndv_ids[proc], 0);
			/* two variants of pre-connect: allow the disconnection from remote
			   segments if resources run short (mode '1'), or enforce the establishment 
			   of *all* mappings (mode '2') */
			if (MPID_SMI_cfg.CONNECT_ON_INIT == 1)
				MPID_SMI_Rmt_mem_release (0, preconnect_rndv_ids[proc], 0);
		}
	}
	free (rmt_rndv_info);
    return;
}


void MPID_SMI_Rndv_delete( p )
	 MPID_Protocol *p;
{
    int i, proc;

    if (MPID_SMI_cfg.ASYNC_PROGRESS) {
		MPID_SMI_Arndv_delete();
    }	

	if( MPID_SMI_RNDVBLOCKING_MODE )
		MPID_SMI_Brndv_memdelete();

    /* free shared memory */
	if (MPID_SMI_cfg.CONNECT_ON_INIT > 0) {
		if (MPID_SMI_cfg.CONNECT_ON_INIT > 1) {
			for (i = 0; i < MPID_SMI_numids; i++) {
				proc = (MPID_SMI_myid + i) % MPID_SMI_numids;
				if (MPID_SMI_is_remote[proc]) {
					MPID_SMI_Rmt_mem_release (0, preconnect_rndv_ids[proc], 0);
				}
			}
			free (preconnect_rndv_ids);
		}
	}
	MPID_SMI_Local_mem_release (NULL, MPID_SMI_rndv_scipool_regid, MPID_SMI_RSRC_DESTROY);
   
    if (MPID_SMI_use_SMP)
		SMIcall (SMI_Free_shreg(MPID_SMI_rndv_shmempool_regid));
    
    FREE (MPID_SMI_Rndvsend_to_proc);

    FREE( p );

    MPID_dataqueue_destroy( MPID_SMI_brndv_queue );
	MPID_SBdestroy (rndvinfo_allocator);
}

/*#define MPI_LINUX*/

int MPID_SMI_Rndv_map_remote_mem (int rmt_grank, MPID_PKT_RNDV_T *pkt, MPIR_SHANDLE *shandle,
								  boolean dma_fallback_ok)
{
    int map_size, map_offset, map_flags, map_tries, sci_sgmtid, err;
	
	if (!MPID_SMI_use_localseg[rmt_grank]) {
		map_tries = 0;
		do {
#ifdef MPI_LINUX
		    /* XXX: workaround for Linux SYS-V shmem bug (?) in 2.4 kernel:
		       mapping a shmem segment without specifying the size of the segment
		       (which means mapping it completely) fails randomly or returns nonsense
		       sizes. Therefore, we specify the size which *should* be used by all
		       processes (but is not really guaranteed to be correct!). Usually, this
		       works because SYS-V shmem should not really be a limited resource.*/
		    if (MPID_SMI_numNodes == 1) {
				map_size = MPID_SMI_RNDVSIZE;
				map_offset = 0;
				map_flags = MPID_SMI_RSRC_PARTIAL_MAP;
		    } else {
#endif
				map_size = pkt->len_avail;
				map_offset = pkt->sgmt_offset;
				map_flags = MPID_SMI_RSRC_PARTIAL_MAP;
#ifdef MPI_LINUX
		    }			
#endif
		    err = MPID_SMI_Rmt_mem_map (rmt_grank, pkt->sgmt_id, map_size, 
										map_offset, pkt->adpt_nbr, 
										&shandle->recv_handle->dest_addr, 
										&shandle->recv_handle->smi_regid_dest, map_flags);
#ifdef MPI_LINUX
		    if (err != MPIR_ERR_EXHAUSTED && MPID_SMI_numNodes == 1) {
				shandle->recv_handle->dest_addr = (char *)shandle->recv_handle->dest_addr
					+ pkt->sgmt_offset;
		    }
#endif
		    map_tries++;
		    while (err == MPIR_ERR_EXHAUSTED && MPID_SMI_Rndvsends_in_progress > 0)
				MPID_DeviceCheck (MPID_NOTBLOCKING);
		} while (err == MPIR_ERR_EXHAUSTED && map_tries <= MPID_SMI_RNDV_CNCT_MAX_RETRIES);
		
		/* Use DMA as a fall-back as it needs less resources. */
		if (err == MPIR_ERR_EXHAUSTED && dma_fallback_ok &&
			OK_FOR_DMA(shandle->start,shandle->bytes_as_contig)) {
		    err = MPID_SMI_Local_mem_register (shandle->start, shandle->bytes_as_contig, 
											   &shandle->recv_handle->smi_regid_src, 
											   &sci_sgmtid);
		    MPID_ASSERT (err != MPIR_ERR_EXHAUSTED, "Could not register send buffer.");
		    err = MPID_SMI_Rmt_region_connect (rmt_grank, pkt->sgmt_id, pkt->adpt_nbr, 
											   &shandle->recv_handle->smi_regid_dest);
		    MPID_ASSERT (err != MPIR_ERR_EXHAUSTED, "Could not connect to recv buffer.");
			
		    shandle->recv_handle->mode = RNDV_SYNC_SEND_DMA;
		}
		/* No access to recv buffer possible -> we have to give up! */
		MPID_ASSERT (err != MPIR_ERR_EXHAUSTED, "Could not access recv buffer.");
	} else {
		/* use the local SMP region */
		shandle->recv_handle->dest_addr      = MPID_SMI_rndv_shmempool + pkt->sgmt_offset;
		shandle->recv_handle->smi_regid_dest = MPID_SMI_rndv_shmempool_regid;
	}
	
	return MPI_SUCCESS;
}


/* dynamically connect to/map a remote segment for zero-copy operations */
/* XXX could be replaced by a macro */
int MPID_SMI_Rndv_connect_zerocopy (char **zc_dst, int cnct_rank, int adptr_nbr, int sgmt_id, 
			       ulong sgmt_size, ulong sgmt_offset, int map_sgmt, int *region_id)
{
  smi_region_info_t shreg_info;
  int retval = MPI_SUCCESS, proc = 0, shreg_mode, cnct_tries = 0, err;
  
  MPID_STAT_ENTRY (arndv_cnct_dstbuf);
  MPID_DEBUG_IFCODE(fprintf (MPID_DEBUG_FILE, "[%d] connecting to zerocpy sgmt %d of (%d)\n", 
			     MPID_SMI_myid, sgmt_id, cnct_rank););
  *zc_dst = NULL;
  do {
      err = map_sgmt ? 
		  MPID_SMI_Rmt_mem_map (cnct_rank, sgmt_id, sgmt_size, sgmt_offset, 
								adptr_nbr, (void **)zc_dst, region_id, 0) :
		  MPID_SMI_Rmt_region_connect (cnct_rank, sgmt_id, adptr_nbr, region_id);
      
      cnct_tries++;
      while (err == MPIR_ERR_EXHAUSTED && MPID_SMI_Rndvsends_in_progress > 0)
		  MPID_DeviceCheck (MPID_NOTBLOCKING);
  } while (err == MPIR_ERR_EXHAUSTED && cnct_tries <= MPID_SMI_RNDV_CNCT_MAX_RETRIES);
	  
  if (err == MPIR_ERR_EXHAUSTED)
      retval = -1;
  
  MPID_STAT_EXIT (arndv_cnct_dstbuf);
  if (retval == MPI_SUCCESS) {
	  MPID_STAT_COUNT(destbuf_imported);
  }
  
  return retval;
}


/* Free the memory for the inbuffer of a pipelined rendez-vous recv. */
void MPID_SMI_Rndv_free_recvbuf( ulong sgmt_offset, int from_drank )
{
    char *free_addr;

    MPID_TRACE_CODE("Rndv_free_recvbuf", MPID_SMI_myid );
    MPID_SMI_DEBUG_PRINT_MSG("Entering Rndv_free_recvbuf");

    /* transform offset into a ptr */
    free_addr = MPID_SMI_use_localseg[from_drank] ?
		MPID_SMI_rndv_shmempool : MPID_SMI_rndv_scipool;
	free_addr += sgmt_offset;

	MPID_SMI_shfree( free_addr );
}


int MPID_SMI_Rndv_resume_send ( MPIR_SHANDLE *sh ) 
{
	return MPID_SMI_Rndv_isend (sh->start, sh->bytes_as_contig, sh->src_lrank, sh->tag,
								sh->context_id, sh->partner, sh->msgrep, sh, sh->datatype);
}

/*
 * Send a message anouncing the availablility of data.  An "ack" must be
 * sent by the receiver to initiate data transfers (the ack type is
 * MPID_PKT_OK_TO_SEND).
 */
int MPID_SMI_Rndv_isend( buf, len, src_lrank, tag, context_id, dest, msgrep, shandle, dtype )
	 void  *buf;
	 int    len, tag, context_id, src_lrank, dest;
	 MPID_Msgrep_t msgrep;
	 MPIR_SHANDLE *shandle;
	 struct MPIR_DATATYPE *dtype;
{
    MPID_SMI_CTRLPKT_T pkt_desc;
    MPID_PKT_RNDV_T prepkt;
	int err, do_register, zc_regid, zc_sciid, flags = 0;

    MPID_STAT_ENTRY(rndv_isend);   
    MPID_TRACE_CODE ("Rndvn_isend", dest);
    MPID_SMI_DEBUG_PRINT_MSG("Entering Rndvn_isend");

    if (MPID_SMI_cfg.SENDSELF && dest == MPID_SMI_myid) {
		/* we can transfer the message directly into the receive buffer */
		return MPID_SMI_Isend_self (buf, len, src_lrank, tag, context_id, dest, msgrep, shandle );
    }

	/* Only a maximum number of concurrent send operations is allowed to 
	   control resource usage. */
	if (CURRENT_SENDS > MPID_SMI_cfg.MAX_SENDS) {
		shandle->push = MPID_SMI_Rndv_resume_send;
		MPID_RNDV_POSTPONE_SEND( buf, len, src_lrank, tag, context_id, dest, msgrep, shandle, dtype );
		return MPI_SUCCESS;
	}

	ALLOC_RNDVINFO_SEND(shandle);

	/* This registering could also be done *after* sending the request packet (for overlapping),
	   but it's better to know in advance if DMA is possible at all. The receiver does not
	   need to register its buffer if we don't want to do DMA.
	   Generally, zero-copy does only make sense for contiguous data. The user can also
	   conrol the usage of DMA via the device configuraion. */
	do_register = MPID_SMI_cfg.USE_DMA_PT2PT && MPID_SMI_cfg.REGISTER && dtype == NULL
		&& len >= MPID_SMI_cfg.SYNC_DMA_MINSIZE && MPID_SMI_is_remote[dest] && OK_FOR_DMA(buf,len)
		&& !MPID_SMI_RNDVBLOCKING_MODE;
	if (do_register) {
		err = MPID_SMI_Local_mem_register (buf, len, &zc_regid, &zc_sciid);
		if (err == MPIR_ERR_EXHAUSTED) {
			do_register = 0;
		} else {
			MPID_STAT_COUNT(sendbuf_registered);
			shandle->recv_handle->mode = (err == MPI_SUCCESS) ? 
				RNDV_SYNC_SEND_DMAZC : RNDV_SYNC_SEND_DMAZC_PERS;
			shandle->recv_handle->smi_regid_src = zc_regid;
			flags = MPID_SMI_RNDV_DMAOK;
		}
	}
	if (!do_register) {
		shandle->recv_handle->mode = RNDV_SYNC;
		shandle->recv_handle->smi_regid_src = MPID_SMI_INVALID_REGID; 
		flags = MPID_SMI_RNDV_PIOONLY;
	}
	shandle->recv_handle->smi_regid_dest = MPID_SMI_INVALID_REGID; 
	
    MPID_SMI_DEBUG_PRINT_MSG("S About to get pkt for request to send");
    MPID_GETSENDPKT (pkt_desc, sizeof(MPID_PKT_RNDV_T), &prepkt, 0, NULL, dest, 0);

    MPID_INIT_RNDVREQ_PREPKT(prepkt, context_id, src_lrank, tag, len, shandle,
							 0, 0, 0, 0, 0, flags);

    MPID_SMI_DEBUG_PRINT_SEND_PKT("S Sending rndv-get message", &prepkt, dest);
    while (MPID_SMI_SendControl(&pkt_desc) != MPI_SUCCESS)
		;
	MPID_STAT_PERIOD_START(shandle->recv_handle->sync_delay);
    
    /* Store info in the request for completing the message */
    shandle->is_complete     = 0;
    shandle->start	         = buf;
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
    MPID_STAT_EXIT(rndv_isend);

    return MPI_SUCCESS;
}

/*
 * This is just isend/wait
 */
int MPID_SMI_Rndv_send( buf, len, src_lrank, tag, context_id, dest, msgrep, dtype )
	 void          *buf;
	 int           len, tag, context_id, src_lrank, dest;
	 MPID_Msgrep_t msgrep;
	 struct MPIR_DATATYPE *dtype;
{
    MPIR_SHANDLE shandle;

    MPID_STAT_ENTRY(rndv_send);
    MPID_TRACE_CODE("Rndvn_send", dest);
    MPID_SMI_DEBUG_PRINT_MSG("Entering Rndvn_send");

    MPID_SMI_DEBUG_INIT_STRUCT(&shandle,sizeof(shandle));

    MPID_Send_init( &shandle );
    shandle.finish = 0;
    MPID_SMI_Rndv_isend( buf, len, src_lrank, tag, context_id, dest, msgrep, &shandle, dtype);

    /* if sending to myself, everything may already be finished */
    if (!shandle.is_complete) {
		MPID_SMI_DEBUG_TEST_FCN(shandle.wait,"req->wait");
		shandle.wait( &shandle );
    }

    MPID_STAT_EXIT(rndv_send);
    return MPI_SUCCESS;
}


/* This is the routine called when a packet of type MPID_PKT_REQUEST_SEND is
   seen and the receive has been posted. */
int MPID_SMI_Rndv_irecv( rhandle, from_grank, in_pkt )
	 MPIR_RHANDLE *rhandle;
	 int          from_grank;
	 void         *in_pkt;
{
    MPID_PKT_RNDV_T recv_pkt;
    int len_local, msglen, err = MPI_SUCCESS;

    MPID_STAT_ENTRY (rndv_irecv)
    MPID_TRACE_CODE ("Rndv_irecv", from_grank);
    MPID_SMI_DEBUG_PRINT_MSG ("Entering Rndv_irecv");
    MPID_CHECK_COOKIE (rhandle);

	/* check for asynchronous transfer */
	if (((MPID_PKT_RNDV_T *)in_pkt)->flags & MPID_SMI_RNDV_ASYNC)
		return MPID_SMI_Arndv_irecv (rhandle, from_grank, in_pkt);

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
	
	if (CURRENT_RECVS < MPID_SMI_cfg.MAX_RECVS) {
		rhandle->push = 0;    
		err = MPID_SMI_Rndv_start_recv (rhandle, NULL);
	} else {
		rhandle->push = MPID_SMI_Rndv_start_recv;
		MPID_SMI_Recv_postpone (rhandle);
	}
		
    MPID_STAT_EXIT(rndv_irecv);
    return err;
}


/*
 * This is the routine called when a packet of type MPID_PKT_REQUEST_SEND_NOZC is
 * seen which implies that the receive has been posted: the receiver can not import
 * the remote (this process') receive buffer and needs to fallback to standard 
 * rendez-vous protocol. The rhandle is already initialized from the first try.
 */
int MPID_SMI_Rndv_irecv_nozc (void *in_pkt, int from_grank )
{
    MPIR_RHANDLE *rhandle;
    MPID_PKT_RNDV_T recv_pkt;
    MPID_SMI_CTRLPKT_T pkt_desc;
    MPID_PKT_RNDV_T prepkt;
    MPID_Aint send_id;
    int   sgmt_id, adpt_nbr, len_local, msglen, err = MPI_SUCCESS;
    ulong sgmt_offset = 0;

    MPID_TRACE_CODE ("Rndvn_irecv_nozc", from_grank);
    MPID_SMI_DEBUG_PRINT_MSG("Entering Rndvn_irecv_nozc");

    MEMCPY (&recv_pkt, in_pkt, sizeof(MPID_PKT_RNDV_T));
    MPID_SMI_FreeRecvPkt ((MPID_PKT_T *)in_pkt, from_grank, IS_CTRL_MSG);    
 
    MPID_AINT_GET(rhandle, recv_pkt.recv_id);
    MPID_CHECK_COOKIE (rhandle);

    /* for this transfer, we do not need the receiver buffer to be registered */    
    if (rhandle->recv_handle->mode == RNDV_ASYNC_RECV_ZC) {
        MPID_SMI_Local_mem_release(NULL, rhandle->recv_handle->smi_regid_dest, MPID_SMI_RSRC_DESTROY);
    }

    /* get local shared memory for the message - we always get a non-zero amount of memory back */
    len_local = msglen = rhandle->len;
    MPID_SMI_Rndv_int.Setup_rndv_addr (&sgmt_id, &sgmt_offset, &adpt_nbr, &len_local, from_grank);

    /* Check for truncation */
    MPID_CHK_MSGLEN(rhandle,msglen,err);
	/* Note that if we truncate, We really must receive the message in two 
	   parts; the part that we can store, and the part that we discard.
	   This case is not yet handled. (XXX check!) */
	rhandle->recv_handle->mode = RNDV_ASYNC_RECV;
    rhandle->wait	  = MPID_SMI_Arndv_unxrecv_end;
    rhandle->test	  = MPID_SMI_Arndv_unxrecv_test_end;
    rhandle->cancel   = MPID_SMI_Arndv_cancel_recv;
    /* remember the locally available space for partial transfers */
    rhandle->recv_handle->len_local  = len_local;
	rhandle->is_valid = 1;

    MPID_SMI_DEBUG_PRINT_MSG("R about to get packet for ok to send");
    MPID_GETSENDPKT (pkt_desc, sizeof(MPID_PKT_RNDV_T), &prepkt, 0, NULL, from_grank, 0);

    /* Send back an "ok to proceed" packet: fill up the new send packet with 
       information, partly taken from the received packet */
    MPID_INIT_RNDVOK_PREPKT(prepkt, msglen, recv_pkt.send_id, sgmt_offset, rhandle, 
							sgmt_id, adpt_nbr, len_local, sgmt_offset, MPID_SMI_RNDV_ASYNC);

    MPID_SMI_DEBUG_PRINT_BASIC_SEND_PKT("R Sending ok-to-send message", &prepkt, from_grank);    
    while (MPID_SMI_SendControl( &pkt_desc ) != MPI_SUCCESS)
		;

    /* signals the worker thread of the sender, that a packet was sent */
    SMI_Signal_send(from_grank|SMI_SIGNAL_ANY);

    return err;
}


/*  Send ack routines.  When a receive is ready for data, it sends
 *  a message of type MPID_PKT_OK_TO_SEND.  The sending side
 *  responds to this by calling the "do_ack" function; the
 *  shandle is looked up (from pkt->send_id), a shared area is created,
 *  data is placed there, and the packet is returned.  If the data will
 *  not fit in a single packet, then the receiver sends additional
 *  MPID_PKT_CONT packets.  Once all of the data is available in
 *  shared memory, then the send side is complete.   
 *  Note that the it is possible to send zero data; this is how 
 *  Ssend(count=0) is implemented 
 */

/* 
 * This is the routine that is called when an "ok to send" packet is
 * received OR when an "cont get" packet is received.  (one ack entry
 * in the check-device routine)
 */
int MPID_SMI_Rndv_ack( void *in_pkt, int from_grank )
{
    MPID_PKT_RNDV_T *recv_pkt = (MPID_PKT_RNDV_T *)in_pkt;
    
    MPID_TRACE_CODE ("Rndvn_ack", from_grank);
    MPID_SMI_DEBUG_PRINT_MSG("Entering Rndvn_ack");

    switch (recv_pkt->mode) {
    case MPID_PKT_OK_TO_SEND:
		if (recv_pkt->flags & MPID_SMI_RNDV_ASYNC) {
			if (recv_pkt->flags & MPID_SMI_RNDV_ZEROCOPY)
				return MPID_SMI_Arndv_send_ack_zc(in_pkt, from_grank);
			else 
				return MPID_SMI_Rndv_int.Send_ack_async(in_pkt, from_grank);
		} else 
			return MPID_SMI_Rndv_int.Send_ack_sync(in_pkt, from_grank);
		break;
    case MPID_PKT_CONT:
		if (recv_pkt->flags & MPID_SMI_RNDV_ASYNC)
			return MPID_SMI_Rndv_int.Recv_ack_async(in_pkt, from_grank);
		else
			return MPID_SMI_Rndv_int.Recv_ack_sync(in_pkt, from_grank);
    default:
		/* illegal type! */
		MPID_ABORT ("Illegal packet mode for rndv ack");
		break;
    }

    return MPI_SUCCESS;
}
	

/* Save an unexpected message in rhandle. */
int MPID_SMI_Rndv_save( rhandle, from_grank, in_pkt )
	 MPIR_RHANDLE *rhandle;
	 int          from_grank;
	 void         *in_pkt;
{
    MPID_PKT_RNDV_T *pkt = (MPID_PKT_RNDV_T *)in_pkt;

    MPID_TRACE_CODE ("Rndv_save", from_grank);
    MPID_SMI_DEBUG_PRINT_MSG("Entering Rndv_save");
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
    rhandle->push         = MPID_SMI_Rndv_unxrecv_start;
    rhandle->cancel       = MPID_SMI_Rndv_cancel_recv;
	/* The sender tells us via the flags how he would like to transmit the data, and
	   how he *can not* transmit it. We need to preserve this information! */
	rhandle->recv_handle->len_local      = 0;
    rhandle->recv_handle->flags          = pkt->flags;
    rhandle->recv_handle->dma_outbuf_len = pkt->len_avail;

    rhandle->is_complete  = 0;	
    rhandle->is_valid     = 1;  /* not yet valid for receiving data */

    MPID_SMI_FreeRecvPkt( (MPID_PKT_T *)pkt, from_grank, IS_CTRL_MSG);

    return MPI_SUCCESS;
}


/*  This routine is called when it is time to perform a receive that 
	has been postponed due to the limitation of the nuber of concurrent
	recvs. In contrast to "unxrecv_start()", it has to the handle which
	was stored when postponing the recv (since this *is* the handle that
	the user supplied - it was *not* an unexpected recv!).
	This routine is also called by "unxrecv_start()", because after setting
	up the rhandles, the necessary actions are identical. 
	It allocates local shared memory and returns its adddress to the sender. 

	The dummy parameter is for prototype compatibility. */
int MPID_SMI_Rndv_start_recv (rhandle, dummy)
	 MPIR_RHANDLE *rhandle;
	 void *dummy;
{
	MPID_SMI_CTRLPKT_T pkt_desc;
    MPID_PKT_RNDV_T prepkt;
    ulong sgmt_offset = 0;
	int sgmt_id = 0,my_sgmt_id = 0;
	int reg_id= 0, do_register= 0,  adpt_nbr= 0, err= 0, len_local= 0, msglen= 0, flags= 0;

    MPID_TRACE_CODE ("Rndv_start_recv", rhandle->from);
	
	flags     = rhandle->recv_handle->flags; 
	len_local = msglen = rhandle->s.count;
    if (len_local > 0) {
		/* Check if registering of the recv buffer ("zerocopy") or use of the incoming buffer, 
		   and DMA or PIO is desired/possible. For now, a registered recv buffer can only be
		   accessed via DMA, but PIO-access will be possible, too, when the driver supports it. */
		/* XXX put this check into a nice macro */
		do_register = MPID_SMI_cfg.ZEROCOPY && MPID_SMI_cfg.REGISTER && MPID_SMI_cfg.USE_DMA_PT2PT 
			&& msglen >= MPID_SMI_cfg.SYNC_DMA_MINSIZE && OK_FOR_DMA(rhandle->buf,msglen)
			&& MPID_SMI_is_remote[rhandle->from] && !MPID_SMI_RNDVBLOCKING_MODE;
		if (do_register) {
			err = MPID_SMI_Local_mem_register(rhandle->buf, rhandle->len,  &reg_id, &sgmt_id);

			if (err == MPIR_ERR_EXHAUSTED) {
				do_register = 0;
			} else {
				MPID_STAT_COUNT(recvbuf_registered);
				len_local = msglen;
				flags |= MPID_SMI_RNDV_ZEROCOPY|MPID_SMI_RNDV_DMAONLY;
				rhandle->recv_handle->smi_regid_dest = reg_id;

				SMI_Query (SMI_Q_SMI_REGION_ADPTNBR, reg_id, &adpt_nbr);

				/* If the buffer is located within a regular SCI segment, we need to determine the
				   offset of the buffer within this segment. For registered user memory, the offset 
				   is always 0 because the SCI segment has been created exclusively for this buffer. */
				if (err == MPID_SMI_ISSCI) {
					ulong sgmt_addr;

					rhandle->recv_handle->mode = RNDV_SYNC_RECV_ZC_PERS;
					SMI_Query (SMI_Q_SMI_REGION_ADDRESS, reg_id, &sgmt_addr);
					MPID_ASSERT (sgmt_addr != 0, "Invalid segment address for persistent communication buffer");
					sgmt_offset = (size_t)rhandle->buf - sgmt_addr;
#if 0					
					printf ("[%d] registered SCI recv buffer, offset 0x%x\n", MPID_SMI_myid, sgmt_addr);
#endif

				} else {
					rhandle->recv_handle->mode = RNDV_SYNC_RECV_ZC;
					sgmt_offset = 0;
#if 0					
					printf ("[%d] registered user recv buffer\n", MPID_SMI_myid);
#endif
				}
			}
		} 
		if (!do_register) {
			MPID_SMI_Rndv_int.Setup_rndv_addr (&sgmt_id, &sgmt_offset, &adpt_nbr, &len_local, rhandle->from); 
			rhandle->recv_handle->mode = RNDV_SYNC;
		}
	}
    /* remember the locally available space for partial transfers */
    rhandle->recv_handle->len_local = len_local;
	rhandle->is_valid = 1;

    MPID_SMI_DEBUG_PRINT_MSG("R about to get packet for ok to send");
    MPID_GETSENDPKT (pkt_desc, sizeof(MPID_PKT_RNDV_T), &prepkt, 0, NULL, rhandle->from, 0);
    
    /* Send back an "ok to proceed" packet */
	/* Runtime ERROR #3 occurs in the following line:
		sgmt_id is used without being defined
	*/
	my_sgmt_id = sgmt_id;
    MPID_INIT_RNDVOK_PREPKT(prepkt, msglen, rhandle->send_id, sgmt_offset, rhandle, 
							my_sgmt_id, adpt_nbr, len_local, 0, flags);
    MPID_SMI_DEBUG_PRINT_BASIC_SEND_PKT("R Sending ok-to-send message", &prepkt, pkt_desc.dest);	
    while (MPID_SMI_SendControl (&pkt_desc) != MPI_SUCCESS)
		;

	RNDV_RECV_SCHEDULED(rhandle->from);
	return MPI_SUCCESS;
}


/*  This routine is called when it is time to receive an unexpected message. 
	Basically, it transfer the information from the rhandle that was created on 
	the unexpected recv into the user-supplied rhandle (aka MPI_Request). */
int MPID_SMI_Rndv_unxrecv_start( rhandle, in_runex )
	 MPIR_RHANDLE *rhandle;
	 void         *in_runex;
{
    MPIR_RHANDLE   *runex = (MPIR_RHANDLE *)in_runex;

    MPID_TRACE_CODE ("Rndv_unxrecv_start", runex->from);

    if (runex->recv_handle->flags & MPID_SMI_RNDV_ASYNC) {
		/* receive this packet via asynchronous rndv */
		return (MPID_SMI_Arndv_unxrecv_start (rhandle, in_runex));
    }

    rhandle->s		  = runex->s;
    rhandle->send_id  = runex->send_id;
    rhandle->wait	  = MPID_SMI_Rndv_unxrecv_end;
    rhandle->test	  = MPID_SMI_Rndv_unxrecv_test_end;

	/* save the original finish function */
	runex->recv_handle->finish = (int (*)(void *))rhandle->finish;

	rhandle->finish   = runex->finish;
    rhandle->push	  = 0;
	rhandle->from     = runex->from;
    /* the message can not be canceled once we get here */
    rhandle->cancel	     = 0;
    rhandle->is_complete = 0;
	/* continue to use the rndv_info! */
	rhandle->recv_handle = runex->recv_handle;

    MPID_Recv_free(runex);

    return MPID_SMI_Rndv_start_recv (rhandle, NULL);
}


/* User called MPI_Wait for a recv request related to an unexpected message.
   This is the wait routine for a rendezvous message that was unexpected.
   A request for the message has already been sent and the receive 
   transfer has been started.  We wait for the "continue get" packets
   to set the completed bit. */
int MPID_SMI_Rndv_unxrecv_end( rhandle )
	 MPIR_RHANDLE *rhandle;
{
    while (!rhandle->is_complete) {
		MPID_DeviceCheck( MPID_NOTBLOCKING );
    }
    if (rhandle->finish) 
		(rhandle->finish)( rhandle );

    return MPI_SUCCESS;
}


/* User called MPI_Test for a recv request related to an unexpected message.
   A request for the message has already been sent, and the receive has been started. */
int MPID_SMI_Rndv_unxrecv_test_end( rhandle )
	 MPIR_RHANDLE *rhandle;
{
    if (!rhandle->is_complete) {
		MPID_DeviceCheck( MPID_NOTBLOCKING );
    } else {
		if (rhandle->finish) 
			(rhandle->finish)( rhandle );
	}
	
    return MPI_SUCCESS;
}


/* User called MPI_Wait or the lib is waiting for a blocking send to complete.
 * These wait for the "ack" and then change the wait routine on the handle 
 */
int MPID_SMI_Rndv_send_wait_ack( shandle )
	 MPIR_SHANDLE *shandle;
{
    MPID_STAT_ENTRY (rndv_send_w_ack);

    MPID_SMI_DEBUG_PRINT_MSG("Entering Rndv_send_wait_ack");

    while (!shandle->is_complete && shandle->wait == MPID_SMI_Rndv_send_wait_ack) {
		MPID_DeviceCheck( MPID_NOTBLOCKING );
    }
    if (!shandle->is_complete) {
		MPID_SMI_DEBUG_TEST_FCN(shandle->wait,"shandle->wait");
		MPID_STAT_EXIT (rndv_send_w_ack);
		return (shandle->wait)( shandle );
    }

    MPID_STAT_EXIT (rndv_send_w_ack);
    return MPI_SUCCESS;
}


/* User called MPI_Test for a send request. */
int MPID_SMI_Rndv_send_test_ack( shandle )
	 MPIR_SHANDLE *shandle;
{
    MPID_SMI_DEBUG_PRINT_MSG("Testing for Rndv_ack" );
    if (!shandle->is_complete && shandle->test == MPID_SMI_Rndv_send_test_ack) {
		MPID_DeviceCheck( MPID_NOTBLOCKING );
    }

    return MPI_SUCCESS;
}

/* Free resources if canceling a rndv-send */
int MPID_SMI_Rndv_cancel_send(MPIR_SHANDLE *shandle)
{
	switch (shandle->recv_handle->mode) {
	case RNDV_SYNC_SEND_DMAZC:
	case RNDV_ASYNC_SEND_DMAZC:
		MPID_SMI_Local_mem_release (NULL, shandle->recv_handle->smi_regid_src, MPID_SMI_RSRC_DESTROY);
		break;
	case RNDV_ASYNC_SEND_DMA:
		MPID_SMI_shfree (shandle->recv_handle->dma_outbuf_addr);
		break;
	default:
		/* no resources to free */
		break;
	}
	
	FREE_RNDVINFO(shandle);
	RNDV_SEND_FINISHED(shandle->partner);

    return MPI_SUCCESS;
}


/* when a rndv-recv is canceled, no data transfer has yet taken place
   (canceling only succeeds if the request was still located in the 
   unexpected queue), thus no recv buffers to be free'd. */
int MPID_SMI_Rndv_cancel_recv (MPIR_RHANDLE *runex)
{
	FREE_RNDVINFO (runex);
    MPID_Recv_free (runex);

    return MPI_SUCCESS;
}

int MPID_SMI_Rndv_free_rndvinfo(void *h)
{
	union MPIR_HANDLE *handle = (union MPIR_HANDLE *)h;
	MPID_RNDV_T *rh_ptr;
	int rc = MPI_SUCCESS;

	switch (handle->chandle.handle_type) {
    case MPIR_PERSISTENT_SEND:
		rh_ptr = &handle->persistent_shandle.shandle.recv_handle;
		handle->persistent_shandle.shandle.finish = 0;
		break;
	case MPIR_PERSISTENT_RECV:
		rh_ptr = &handle->persistent_rhandle.rhandle.recv_handle;
		handle->persistent_rhandle.rhandle.finish = 0;
		break;
	case MPIR_SEND:
		rh_ptr = &handle->shandle.recv_handle;
		handle->shandle.finish = 0;
		break;
	case MPIR_RECV:
		rh_ptr = &handle->rhandle.recv_handle;
		handle->rhandle.finish = 0;
		break;
	}

	/* First, call the original finish function that may have been set. Then,
	   free the memory for the rndv_info. */
	if (*rh_ptr != NULL) {
		if ((*rh_ptr)->finish)
			rc = (*rh_ptr)->finish(h);

		MPID_SBfree (rndvinfo_allocator, *rh_ptr); 
		*rh_ptr = NULL;
	}

	return rc;
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
