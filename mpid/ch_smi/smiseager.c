/* $Id$ */

/* setup and copy functions for the static variant of the eager protocol */

#include "smieager.h"

/* This is 2**31: a 4 byte unsigned integer with the most significant bit (the "toggle bit")
   set and all other bits cleared. This bitmask is used to test for or set the toggle bit.
   Because we use the most significant bit of an offset for the toggle bit, we only have 31 bits
   left for the "real" offset, which should be more than enough. */
#define MPID_SMI_SEAGER_TOGGLE_SET   2147483648U

/* MPID_SMI_lEagerRRingbuf[proc] manages the eager buffers into which we receive from proc*/
static MPID_SMI_lEagerRingbuf_t *MPID_SMI_lEagerRRingbuf;

/* MPID_SMI_lEagerRSingbuf[proc] manages the eager buffers into which we send to proc*/
static MPID_SMI_lEagerRingbuf_t *MPID_SMI_lEagerSRingbuf;

static MPID_SMI_Eagern_receive_info_t *receive_info = NULL;
static char **MPID_SMI_eager_bufptrs_rmt, **MPID_SMI_eager_bufptrs_loc;

/* MPID_SMI_sEager_RToggle[proc] contains the bitmask for the toggle bit to be used
   when freeing an eager buffer the next time regarding receiving from proc */
static unsigned int *MPID_SMI_sEager_RToggle; 

/* MPID_SMI_sEager_SToggle[proc] contains the bitmask for the toggle bit that is expected
   the next time an eager buffer is free'd by proc */
static unsigned int *MPID_SMI_sEager_SToggle; 

/* This function allocates the eager buffers into which this process receives. These are either global buffers,
   which are accessible via SCI to receive from remote processes, or local buffers, to receive from processes
   running on the same node.
   Information about the global buffers are published so that the remote processes can connect to them. */

int MPID_SMI_sEager_init ( void )
{
    smi_region_info_t shreg_info;
    int proc, sgmnt_ok, local_procs, i, global_bufsize, global_nbrbufs;
    int adpt_nbr = -1, local_sgmt_id = -1;
    unsigned int mem_per_proc, total_local_mem;
	
    ZALLOCATE (MPID_SMI_lEagerSRingbuf, MPID_SMI_lEagerRingbuf_t*, MPID_SMI_numids*sizeof(MPID_SMI_lEagerRingbuf_t));
    ZALLOCATE (MPID_SMI_lEagerRRingbuf, MPID_SMI_lEagerRingbuf_t*, MPID_SMI_numids*sizeof(MPID_SMI_lEagerRingbuf_t));
    ZALLOCATE (MPID_SMI_eager_bufptrs_rmt, char**, MPID_SMI_numids*sizeof(char *));
    ZALLOCATE (MPID_SMI_eager_bufptrs_loc, char**, MPID_SMI_numids*sizeof(char *));
    ZALLOCATE (MPID_SMI_Eagern_connect_info, MPID_SMI_Eagern_connect_info_t *, 
			   MPID_SMI_numids*sizeof(MPID_SMI_Eagern_connect_info_t));
    ZALLOCATE (receive_info, MPID_SMI_Eagern_receive_info_t *, 
			   MPID_SMI_numids*sizeof(MPID_SMI_Eagern_receive_info_t));
    ZALLOCATE (MPID_SMI_sEager_RToggle, unsigned int *, MPID_SMI_numids*sizeof(unsigned int));
    ZALLOCATE (MPID_SMI_sEager_SToggle, unsigned int *, MPID_SMI_numids*sizeof(unsigned int));
    
    /* If running multiple nodes with more than one procs each: don't allocate 
       SCI memory for local procs. This saves valuable SCI memory. Therefore, we need to know
       how many procs are running on our node. */
    local_procs = MPID_SMI_use_SMP ? MPID_SMI_numProcsOnNode[MPID_SMI_myNode] : 0;

    global_bufsize = MPID_SMI_EAGERSIZE; /* start value for size of global eager buffers */
    global_nbrbufs = MPID_SMI_EAGERBUFS; /* start value for number of global eager buffers */
    
    if (MPID_SMI_EAGERBUFS > 0) {
		/* allocate global memory with fallback-strategy if not enough resources are available */
		MPID_INFO("  global -");
		do {
			mem_per_proc = global_nbrbufs * global_bufsize;
			MPID_SMI_PAGESIZE_ALIGN(mem_per_proc);

			/* create SCI memory;
			   this memory is located on each node and for each process,
			   MPID_SMI_global_eagerbufs points to the SCI memory it has created */
			SMI_Init_reginfo (&shreg_info, (MPID_SMI_numids - local_procs) * mem_per_proc, 0, 
							  MPID_SMI_myid, SMI_ADPT_DEFAULT, 0, 0, NULL);
			sgmnt_ok = SMI_Create_shreg(SMI_SHM_LOCAL|SMI_SHM_NONFIXED, &shreg_info, 
										&MPID_SMI_Shregid_eagerbufs[MPID_SMI_myid], 
										(void **)&MPID_SMI_global_eagerbufs) == SMI_SUCCESS;
			if (!sgmnt_ok) {
				MPID_INFO("X ");
				MPID_STAT_COUNT(create_sgmnt_fail);
		
				/* not enough memory for the eager buffers, reduce number and size of the buffers */
				if (global_nbrbufs > 1) {
					/* reduce the number of the eager buffers */
					global_nbrbufs--; 
				} else {
					if (global_bufsize > MPID_SYS_PAGESIZE) {
						/* then reduce the size of the eager buffers */
						global_bufsize /= 2; 
						if (global_bufsize < MPID_SYS_PAGESIZE)
							global_bufsize = MPID_SYS_PAGESIZE;
					} else {
						/* we can't reduce anything any more -> no eager for remote processes */
						global_nbrbufs = 0;
						break;
					}
				}
			}
		} while (!sgmnt_ok);

		/* get information about global buffers that we need to publish */
		if (global_nbrbufs > 0) {
			EAGER_DEBUG(fprintf (stderr, "[%d] incoming global eager buffers located at 0x%p, size 0x%x\n", 
								 MPID_SMI_myid, MPID_SMI_global_eagerbufs, global_nbrbufs*global_bufsize););
			SMIcall (SMI_Query (SMI_Q_SMI_REGION_SGMT_ID, MPID_SMI_Shregid_eagerbufs[MPID_SMI_myid], 
								&local_sgmt_id));
			SMIcall (SMI_Query (SMI_Q_SMI_REGION_ADPTNBR, MPID_SMI_Shregid_eagerbufs[MPID_SMI_myid], 
								&adpt_nbr));
		} 

		if (MPID_SMI_use_SMP) {
			/* allocate local memory for communication between processes running on the same node;
			   in contrast to global SCI memory, there is no fallback-strategy, so if we cannot allocate
			   the whole eager memory, we abort */

			/* memory to receive from one process */
			mem_per_proc = MPID_SMI_EAGERBUFS * MPID_SMI_EAGERSIZE;
			MPID_SMI_PAGESIZE_ALIGN(mem_per_proc);

			/* memory for all local processes to receive from all local processes */
			total_local_mem = MPID_SMI_numProcsOnNode[MPID_SMI_myNode] * MPID_SMI_numProcsOnNode[MPID_SMI_myNode] * mem_per_proc; 

			/* create local shared region;
			   on all local processes, MPID_SMI_local_eagerbufs points to start of region i.e. to the same memory location;
			   this operation is collective among all processes running on the same node */
			SMI_Init_reginfo (&shreg_info, total_local_mem, 0, 0, SMI_ADPT_DEFAULT, 0, 0, NULL);
			MPID_INFO(" local -"); 
			if (SMI_Create_shreg(SMI_SHM_SMP|SMI_SHM_NONFIXED|SMI_SHM_INTERN, &shreg_info, 
								 &MPID_SMI_Locregid_eagerbufs, (void **)&MPID_SMI_local_eagerbufs) != SMI_SUCCESS) {
				MPID_ABORT ("Not enough local shared memory for eager buffers");
			}
			EAGER_DEBUG(fprintf (stderr, "[%d] local eager buffers located at 0x%p, size 0x%x\n", 
								 MPID_SMI_myid, MPID_SMI_local_eagerbufs, shreg_info.size););
		}
    } /* if (MPID_SMI_EAGERBUFS > 0) */

    /* XXXXXboris: check this for consistency problems (remote operations with transfer checks) */
    /* publish the setup of the global eager buffers */
    do {
		for (i = 0; i < MPID_SMI_numids; i++) {
			if( MPID_SMI_use_localseg[i] && i != MPID_SMI_myid ) {
				MPID_SMI_Int_info_exp[i]->Eager_setup.nbrbufs = MPID_SMI_EAGERBUFS;
				MPID_SMI_Int_info_exp[i]->Eager_setup.bufsize = MPID_SMI_EAGERSIZE;
			}
			else {
				MPID_SMI_Int_info_exp[i]->Eager_setup.sgmt_id = local_sgmt_id;
				MPID_SMI_Int_info_exp[i]->Eager_setup.adptr   = adpt_nbr;
				MPID_SMI_Int_info_exp[i]->Eager_setup.nbrbufs = global_nbrbufs;
				MPID_SMI_Int_info_exp[i]->Eager_setup.bufsize = global_bufsize;
			}
		}
    } while (SMI_Check_transfer(CHECK_MODE) != SMI_SUCCESS);
    
	/* For verbose output, we need this information before connect-time */
	MPID_SMI_Eagern_connect_info[MPID_SMI_myid].nbr_bufs = global_nbrbufs;
	MPID_SMI_Eagern_connect_info[MPID_SMI_myid].bufsize  = global_bufsize;

    /* Set up the ptr arrays towards the other processes (this is where ptrs to free'd/available
       buffers are stored). These arrays are stored behind the short buffers of each process. */
    for (i = 0; i < MPID_SMI_numids; i++) {
		MPID_SMI_eager_bufptrs_rmt[i] = MPID_SMI_shmem_short[i] 
			+ MPID_SMI_SHORTSIZE*MPID_SMI_SHORTBUFS * MPID_SMI_numids
			+ MPID_SMI_numids*sizeof(MPID_SMI_Int_data) + MPID_SMI_myid*MPID_SMI_EAGERBUFS*sizeof(char **);
		MPID_SMI_eager_bufptrs_loc[i] = MPID_SMI_shmem_short[MPID_SMI_myid]
			+ MPID_SMI_SHORTSIZE*MPID_SMI_SHORTBUFS * MPID_SMI_numids
			+ MPID_SMI_numids*sizeof(MPID_SMI_Int_data) + i*MPID_SMI_EAGERBUFS*sizeof(char **);
    }

    /* set connection status for all shared memory segments;
	   only local segments are connected right from the beginning */
    for (proc = 0; proc < MPID_SMI_numids; proc++) {
		if (MPID_SMI_use_localseg[proc])
			MPID_SMI_eagerseg_connected[proc] = true;
		else {
			if (proc != MPID_SMI_myid)
				MPID_SMI_eagerseg_connected[proc] = false;
			else
				MPID_SMI_eagerseg_connected[proc] = true;
		}
    }

    return MPI_SUCCESS;
}


/* Read the eager-buffer configuration from a remote process and store it in a local data structure. */
static void get_connect_info ()
{    
    int proc, i;
    unsigned int offset;

    MPID_SMI_min_EAGERSIZE = MPID_SMI_EAGERSIZE;

    for (proc = 0; proc < MPID_SMI_numids; proc++) {
		MPID_SMI_Eagern_connect_info[proc].sgmt_id   = MPID_SMI_Int_info_imp[proc]->Eager_setup.sgmt_id;
		MPID_SMI_Eagern_connect_info[proc].adptr_nbr = MPID_SMI_Int_info_imp[proc]->Eager_setup.adptr;
		MPID_SMI_Eagern_connect_info[proc].bufsize   = MPID_SMI_Int_info_imp[proc]->Eager_setup.bufsize;
		MPID_SMI_Eagern_connect_info[proc].nbr_bufs  = MPID_SMI_Int_info_imp[proc]->Eager_setup.nbrbufs ;
	
		MPID_SMI_Eagern_connect_info[proc].nbr_cncts = 0;

		MPID_SMI_Eagern_connect_info[proc].mem_size = MPID_SMI_Eagern_connect_info[proc].nbr_bufs*MPID_SMI_Eagern_connect_info[proc].bufsize;
		MPID_SMI_PAGESIZE_ALIGN(MPID_SMI_Eagern_connect_info[proc].mem_size);

		/* Calculate the offset for mapping: we want "our" buffers to start at the
		   beginnning of the segment we map to save address space. The offset is increased
		   by 'size' for each other process 'proc' which connects remotely to the
		   SCI-exported eager buffers of 'cnct_proc'. */
		for (offset = 0, i = 0; i < MPID_SMI_myid; i++)
			if (MPID_SMI_procNode[i] != MPID_SMI_procNode[proc]
				|| (i == proc && !MPID_SMI_use_SMP)
				|| MPID_SMI_numNodes == 1)
				offset += MPID_SMI_Eagern_connect_info[proc].mem_size;
		MPID_SMI_Eagern_connect_info[proc].mem_offset = offset;
		EAGER_DEBUG(fprintf (stderr, "[%d] segment mapping to (%d): size = 0x%x,  offset 0x%x\n", 
							 MPID_SMI_myid, proc, MPID_SMI_Eagern_connect_info[proc].mem_size, MPID_SMI_Eagern_connect_info[proc].mem_offset););
      
		/* determine minimal eager bufsize in the system (req. for assumptions on 
		   blocking unexpected sends, i.e. in reduce_scatter). */
		if (MPID_SMI_Eagern_connect_info[proc].bufsize < MPID_SMI_min_EAGERSIZE)
			MPID_SMI_min_EAGERSIZE = MPID_SMI_Eagern_connect_info[proc].bufsize;
    }

#if 0
    fprintf (stderr, "[%d] local_EAGERSIZE = %d, nbr_EAGERBUFS = %d, min_EAGERSIZE = %d\n", MPID_SMI_myid,
			 MPID_SMI_Int_info_exp[0]->Eager_setup.bufsize, MPID_SMI_Int_info_exp[0]->Eager_setup.nbrbufs,
			 MPID_SMI_min_EAGERSIZE);
#endif

    return;
}


void MPID_SMI_sEager_init_complete ()
{
	int proc;

	get_connect_info();
	for (proc = 0; proc < MPID_SMI_numids; proc++)
		MPID_SMI_sEager_meminit(proc);

	return;
}


/* Basically decrement usage counter for the remote eager memory. */
void MPID_SMI_sEager_disconnect(int proc)
{
    if (!MPID_SMI_use_localseg[proc]) {
		MPID_SMI_Rmt_mem_release (MPID_SMI_outgoing_eagerbufs[proc], 0, MPID_SMI_RSRC_CACHE);
		MPID_SMI_outgoing_eagerbufs[proc] = NULL;
    }
    return;
}


/* on-demand connection to a remote eager segment */
int MPID_SMI_sEager_connect(int cnct_proc)
{
    int err;

    if (MPID_SMI_use_localseg[cnct_proc])
		return MPI_SUCCESS;

    if (MPID_SMI_Eagern_connect_info[cnct_proc].nbr_bufs == 0) {
		/* no remote eager buffers available, will use rendez-vous instead */
		MPID_SMI_eagerseg_connected[cnct_proc] = true;
		MPID_SMI_ASYNC_UNLOCK(&MPID_SMI_connect_lck);
		return MPI_ERR_INTERN;
    }

    if( ( err = MPID_SMI_Rmt_mem_map (cnct_proc, MPID_SMI_Eagern_connect_info[cnct_proc].sgmt_id, 
									  MPID_SMI_Eagern_connect_info[cnct_proc].mem_size, MPID_SMI_Eagern_connect_info[cnct_proc].mem_offset,
									  MPID_SMI_Eagern_connect_info[cnct_proc].adptr_nbr, 
									  (void **)&MPID_SMI_outgoing_eagerbufs[cnct_proc], 
									  &MPID_SMI_Shregid_eagerbufs[cnct_proc], 
									  MPID_SMI_RSRC_PARTIAL_MAP) ) != MPI_SUCCESS ) {
		MPID_SMI_DEBUG_PRINT_MSG2("Could not import EAGER buffers from (%d)", cnct_proc);
		MPID_SMI_Eagern_connect_info[cnct_proc].nbr_bufs = 0;
		return MPI_ERR_INTERN;
    }
    EAGER_DEBUG(fprintf (stderr, "[%d] outgoing eager buffers to (%d) located at 0x%p, size 0x%x\n", 
						 MPID_SMI_myid, cnct_proc, MPID_SMI_outgoing_eagerbufs[cnct_proc], MPID_SMI_Eagern_connect_info[cnct_proc].mem_size););
    MPID_SMI_Eagern_connect_info[cnct_proc].nbr_cncts++;

    MPID_STAT_COUNT(eager_sgmnt_cnct);
    return MPI_SUCCESS;;
}

/* initialize the local data structures for send & receive */
void MPID_SMI_sEager_meminit (int proc)
{
    int buf, i, local_procs = 0;
    size_t mem_per_proc;
    unsigned int buf_offset = 0;
    
    if (MPID_SMI_use_localseg[proc]) {

		/* infos about how to receive from proc */
		receive_info[proc].bufsize  = MPID_SMI_EAGERSIZE;
		receive_info[proc].nbr_bufs = MPID_SMI_EAGERBUFS;

		mem_per_proc = MPID_SMI_EAGERBUFS * MPID_SMI_EAGERSIZE;

		/* For processes located on the same node, use local shared memory - this won't be dis-/re-connected and
		   can thus be 'statically' initialized here. With each eager-msg, the offset relative to the beginning 
		   of the full local memory segment wil be transmitted. */
		MPID_SMI_outgoing_eagerbufs[proc] = MPID_SMI_local_eagerbufs /* points to same memory on all paticipating procs */
			+ MPID_SMI_localRankForProc[proc] * MPID_SMI_numProcsOnNode[MPID_SMI_myNode] * mem_per_proc /* start of proc's incoming eager buffers */
			+ MPID_SMI_localRankForProc[MPID_SMI_myid]*mem_per_proc; /* start of my eager buffers for sending to proc */

		MPID_SMI_incoming_eagerbufs[proc] = MPID_SMI_local_eagerbufs /* points to same memory on all paticipating procs */
			+ MPID_SMI_localRankForProc[MPID_SMI_myid] * MPID_SMI_numProcsOnNode[MPID_SMI_myNode] * mem_per_proc /* start of my incoming eager buffers */
			+ MPID_SMI_localRankForProc[proc]*mem_per_proc; /* start of proc's eager buffers for sending to me */

		/* locate the buffer pointers in local memory, too (see below) */
		MPID_SMI_lEagerRRingbuf[proc].offsets = (volatile unsigned int *)MPID_SMI_eager_bufptrs_rmt[proc];
		MPID_SMI_lEagerSRingbuf[proc].offsets = (volatile unsigned int *)MPID_SMI_eager_bufptrs_loc[proc];
    } else {
		/* infos about how to receive from proc */
		receive_info[proc].bufsize  = MPID_SMI_Eagern_connect_info[MPID_SMI_myid].bufsize;
		receive_info[proc].nbr_bufs = MPID_SMI_Eagern_connect_info[MPID_SMI_myid].nbr_bufs;

		/* Eager-buffers of processes on remote nodes may be dis-/re-connected dynamically, thus
		   we can only define the offsets with respect to the related starting address here. */

		/* Receiving */
		mem_per_proc = MPID_SMI_Eagern_connect_info[MPID_SMI_myid].nbr_bufs*MPID_SMI_Eagern_connect_info[MPID_SMI_myid].bufsize;
		MPID_SMI_PAGESIZE_ALIGN(mem_per_proc);

		/* processes on the same node do not have buffers in SCI memory allocated */
		for (i = 0; i < proc; i++)
			if (MPID_SMI_use_localseg[i])
				local_procs++;
		/* the remote memory is already mapped with the offset, procs' own  memory 
		   needs to be used with offset */
		MPID_SMI_incoming_eagerbufs[proc] = MPID_SMI_global_eagerbufs + mem_per_proc*(proc-local_procs);

		/* ptr to available local receive buffers are returned to this 
		   ring buffer located on the sender process */
		MPID_SMI_lEagerRRingbuf[proc].offsets = (volatile unsigned int *)MPID_SMI_eager_bufptrs_rmt[proc];
	
		/* Sending */
		mem_per_proc = MPID_SMI_Eagern_connect_info[proc].nbr_bufs*MPID_SMI_Eagern_connect_info[proc].bufsize;
		MPID_SMI_PAGESIZE_ALIGN(mem_per_proc);
		/* local send information - this process gets the ptrs to available
		   buffers on the destination process from here */
		MPID_SMI_outgoing_eagerbufs[proc] = 0;
		MPID_SMI_lEagerSRingbuf[proc].offsets = (volatile unsigned int *)MPID_SMI_eager_bufptrs_loc[proc];
    }

    /* init the ptrs to the available bufffers with the start of the ringbuffer */
    MPID_SMI_lEagerSRingbuf[proc].ptr = MPID_SMI_lEagerSRingbuf[proc].offsets;
    MPID_SMI_lEagerRRingbuf[proc].ptr = MPID_SMI_lEagerRRingbuf[proc].offsets;

    /* Store the offsets to all send buffers in the ringbuffer and initialize the bitmasks for the toggle bits for communication with proc */
	for (buf = 0; buf < MPID_SMI_Eagern_connect_info[proc].nbr_bufs; buf++) {
		(MPID_SMI_lEagerSRingbuf[proc].offsets)[buf] = buf_offset;
		buf_offset += MPID_SMI_Eagern_connect_info[proc].bufsize;
		MPID_SMI_sEager_SToggle[proc] = 0; /* first, we expect all toggle bits to be clear */
	}
	for( buf = 0; buf < receive_info[proc].nbr_bufs; buf++ )
		MPID_SMI_sEager_RToggle[proc] = MPID_SMI_SEAGER_TOGGLE_SET; /* when the buffers are free'd for the first time, all toggle bits are set */
}


/* get the address of a free eager buffer located on the receiver to send him a message */
void *MPID_SMI_sEager_get_buf( int dest, int size )
{
    unsigned int offset;
    int checked_device = 0;
    unsigned int togglemask;
    char *buffer = 0;
    
    MPID_TRACE_CODE ("GetEagerAddress", dest);
	
    /* make sure the segment towards this proc is connected */
    if (MPID_SMI_sEager_connect(dest) != MPI_SUCCESS)
		return 0;
    
    /* Is the eager protocol available for communication with this process? And are its
       buffers big enough (may be sized differently for every process!) ? */
    if ((MPID_SMI_Eagern_connect_info[dest].nbr_bufs == 0) || (size > MPID_SMI_Eagern_connect_info[dest].bufsize))
		return 0;
    
    while (checked_device <= MPID_SMI_EAGER_MAXCHECKDEV) {
		
		offset = *MPID_SMI_lEagerSRingbuf[dest].ptr;
		
		/* extract bitmask for toggle bit from offset */
		togglemask = offset & MPID_SMI_SEAGER_TOGGLE_SET;
		
		/* clear toggle bit for calculation of buffer index */
		offset -= togglemask;
		
		/* If the toggle bit has the expected value, the buffer starting at offset has been free'd again
		   since it was last used. Otherwise, no free buffer is currently available. */
		if( togglemask == MPID_SMI_sEager_SToggle[dest]) {

			/* transform offset into ptr */
			buffer = MPID_SMI_outgoing_eagerbufs[dest] + offset;
			
			if (MPID_SMI_lEagerSRingbuf[dest].slotindex < MPID_SMI_Eagern_connect_info[dest].nbr_bufs - 1) {
				MPID_SMI_lEagerSRingbuf[dest].ptr++;
				MPID_SMI_lEagerSRingbuf[dest].slotindex++;
			} else {
				MPID_SMI_lEagerSRingbuf[dest].ptr = MPID_SMI_lEagerSRingbuf[dest].offsets;
				MPID_SMI_lEagerSRingbuf[dest].slotindex = 0;

				/* At wrap-around, the expected value for the toggle bit changes, i.e. this addition operation
				   should toggle the toggle bit in the bitmask. If the toggle bit is clear in MPID_SMI_sEager_SToggle[dest],
				   the value of this variable is zero and the result of the addition is the bitmask with the toggle bit set.
				   If the toggle bit is set, we get an overflow by the addition, which is ignored, and MPID_SMI_sEager_SToggle[dest]
				   should be zero afterwards. */
				MPID_SMI_sEager_SToggle[dest] += MPID_SMI_SEAGER_TOGGLE_SET;
			}
			break;
		} else {
			buffer = NULL;
		}
		checked_device++;
		
		if (MPID_SMI_EAGER_MAXCHECKDEV > 0)
			MPID_DeviceCheck( MPID_NOTBLOCKING );
    }
    
    /* If we got no buffer, we don't need the remote segment any longer. */
    if (buffer == NULL)
		MPID_SMI_sEager_disconnect (dest);
    else {
		EAGER_DEBUG(fprintf (stderr, "[%d] sending eager to (%d): size 0x%x,  offset 0x%x\n", 
							 MPID_SMI_myid, dest, size, buffer - (ulong)(MPID_SMI_outgoing_eagerbufs[dest])););
    }
    
    return ((void *)buffer);
}


/* after having received the message, return the buffer to the 
   ring buffer of available buffer-ptrs */
int MPID_SMI_sEager_free_buf (void *bufptr, int size, int from)
{
    char *free_addr = (char *)bufptr;
    unsigned int free_offset;
	
    MPID_TRACE_CODE ("FreeEagerAddress", from);
	
    if (MPID_SMI_Eagern_connect_info[from].nbr_bufs == 0)
		return MPI_ERR_INTERN;
	
    /* transform ptr back to offset */
    free_offset = (unsigned int)(free_addr - MPID_SMI_incoming_eagerbufs[from]);
	
    EAGER_DEBUG(fprintf (stderr, "[%d] freeing eager from (%d): size 0x%x,  offset 0x%x\n", 
						 MPID_SMI_myid, from, size, free_addr););
	
	/* set the toggle bit in offset to its currently used value */
	free_offset |= MPID_SMI_sEager_RToggle[from];
	
    /* write offset into ring buffer */
    WRITE_RMT_PTR(MPID_SMI_lEagerRRingbuf[from].ptr, free_offset, from);
	
	if (MPID_SMI_lEagerRRingbuf[from].slotindex < receive_info[from].nbr_bufs - 1) {
		MPID_SMI_lEagerRRingbuf[from].ptr++;
		MPID_SMI_lEagerRRingbuf[from].slotindex++;
	} else {
		MPID_SMI_lEagerRRingbuf[from].ptr = MPID_SMI_lEagerRRingbuf[from].offsets;
		MPID_SMI_lEagerRRingbuf[from].slotindex = 0;
		
		/* At wrap-around, the used value for the toggle bit changes, i.e. this addition operation
		   should toggle the toggle bit in the bitmask. If the toggle bit is clear in MPID_SMI_sEager_RToggle[dest],
		   the value of this variable is zero and the result of the addition is the bitmask with the toggle bit set.
		   If the toggle bit is set, we get an overflow by the addition, which is ignored, and MPID_SMI_sEager_RToggle[dest]
		   should be zero afterwards. */
		MPID_SMI_sEager_RToggle[from] += MPID_SMI_SEAGER_TOGGLE_SET;
	}

    return MPI_SUCCESS;
}


void MPID_SMI_sEager_sendcpy(int dest, void *eager_addr, void *buffer, int len, struct MPIR_DATATYPE *dtp)
{
    int align_size, copy_len, retry = 0;
	
    MPID_STAT_ENTRY (eager_scopy);
    EAGER_DEBUG(fprintf (stderr, "[%d] doing eager sendcpy to (%d): size 0x%x,  address 0x%p\n", 
						 MPID_SMI_myid, dest, len, eager_addr););
	
    MEMCPYSYNC_ENTER(dest, len);
	
    /* first, test for non-contig send */
    if (dtp != NULL) {
		MPID_SMI_Pack_ff ((char *)buffer, dtp, (char *)eager_addr, dest, len, 0);
    } else { 
		/* copy the data with alignment to stream buffer size */
		if (!MPID_SMI_is_remote[dest]) {
			MEMCPY(eager_addr, buffer, len);
		} else {
			copy_len = len;
			if( ( align_size = ( len & (MPID_SMI_EAGER_ALIGN_SIZE-1) ) ) )  {
				copy_len -= align_size;
				MEMCPY(MPID_SMI_eager_align_buf, (char *)buffer + copy_len, align_size);
			}
			do {
				MEMCPY_W_FAST(eager_addr, buffer, copy_len, dest);
				if (align_size) {
					MEMCPY_S((char *)eager_addr + copy_len, MPID_SMI_eager_align_buf, 
							 MPID_SMI_EAGER_ALIGN_SIZE);
				}
				WC_FLUSH;
				if (++retry > 1)
					MPID_STAT_COUNT(sci_transm_error);	    
			}  while (MPID_SMI_cfg.DO_VERIFY 
					  && SMI_Check_transfer_proc(dest, CHECK_MODE) != SMI_SUCCESS);
		}
    }
	
    MEMCPYSYNC_LEAVE (dest, len); 
    MPID_STAT_EXIT (eager_scopy);
    return;
}

void MPID_SMI_sEager_recvcpy (int from, void *eager_addr, void *buffer, int len, struct MPIR_DATATYPE *dtp,
							  struct MPIR_OP *op_ptr)
{
    MPID_STAT_ENTRY(eager_rcopy);
    EAGER_DEBUG(fprintf (stderr, "[%d] doing eager recvcpy from (%d): size 0x%x,  address 0x%p\n", 
						 MPID_SMI_myid, from, len, eager_addr););
    
    if (op_ptr != NULL) {
		/* direct reduction operation which also requires a valid datatype_ptr! */
		MPID_ASSERT(dtp != NULL, "Missing datatype ptr for direct reduce operation!");
		CALL_OP_FUNC (buffer, eager_addr, len, op_ptr, dtp);
    } else {
		if (dtp != NULL) {
			/* direct non-contig datatype operation! */
			int copy_len = 0;
			MPID_SMI_UnPack_ff ((char *) eager_addr, dtp, (char *) buffer, from, len, copy_len);
		} else {
			/* finally, the usual contiguous data copy. */
			MEMCPY_R(buffer, eager_addr, len);
		}
    }
    
    MPID_STAT_EXIT(eager_rcopy);
    return;
}


void MPID_SMI_sEager_delete (void)
{
    int proc;

    /* free global memory */
    for ( proc = 0; proc < MPID_SMI_numids; proc++ ) {
		if (MPID_SMI_eagerseg_connected[proc] && (MPID_SMI_Eagern_connect_info[proc].nbr_bufs > 0) 
			&& !MPID_SMI_use_localseg[proc])
			SMIcall (SMI_Free_shreg(MPID_SMI_Shregid_eagerbufs[proc]));
    }

    /* free local memory */
    FREE (MPID_SMI_sEager_SToggle);
    FREE (MPID_SMI_sEager_RToggle);
    FREE (MPID_SMI_lEagerSRingbuf);
    FREE (MPID_SMI_lEagerRRingbuf);
    FREE (MPID_SMI_eager_bufptrs_rmt);
    FREE (MPID_SMI_eager_bufptrs_loc);

    if (MPID_SMI_Eagern_connect_info != NULL)
		FREE (MPID_SMI_Eagern_connect_info);
	if( receive_info != NULL )
		FREE( receive_info );
    
    return;
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
