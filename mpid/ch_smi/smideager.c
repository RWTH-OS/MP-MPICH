/* $Id$ */

#include "smieager.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* pointers to read and write offsets for eager buffers */
static MPID_SMI_lEageroffset_t *MPID_SMI_lEager_Soffsets, *MPID_SMI_lEager_Roffsets;

/* local copies of MPID_SMI_lEager_Roffsets.r */
static unsigned int *read_ptrs;

static char **MPID_SMI_eager_offsets_rmt, **MPID_SMI_eager_offsets_local;


/* Allocate local SCI memory buffers and SMP-local shared memory for eager communication. */
int MPID_SMI_dEager_init ( void )
{
    smi_region_info_t shreg_info;
    int proc, local_procs, sgmnt_ok, local_sgmt_id, global_bufsize, adpt_nbr;
    uint mem_per_proc, local_memperproc;

    /* allocate local memory */
    ZALLOCATE (MPID_SMI_lEager_Soffsets, MPID_SMI_lEageroffset_t *, 
			   MPID_SMI_numids * sizeof(MPID_SMI_lEageroffset_t));
    ZALLOCATE (MPID_SMI_lEager_Roffsets, MPID_SMI_lEageroffset_t *, 
			   MPID_SMI_numids * sizeof(MPID_SMI_lEageroffset_t));
    ZALLOCATE (read_ptrs, unsigned int *, MPID_SMI_numids * sizeof(unsigned int));
    ZALLOCATE (MPID_SMI_eager_offsets_rmt, char**, MPID_SMI_numids*sizeof(char *));
    ZALLOCATE (MPID_SMI_eager_offsets_local, char**, MPID_SMI_numids*sizeof(char *));
    ZALLOCATE (MPID_SMI_Eagern_connect_info, MPID_SMI_Eagern_connect_info_t *, 
			   MPID_SMI_numids*sizeof(MPID_SMI_Eagern_connect_info_t));

    /* if running multiple nodes with more than one procs each: don't allocate 
       SCI memory for local procs */
    local_procs = MPID_SMI_use_SMP ? MPID_SMI_numProcsOnNode[MPID_SMI_myNode]  : 0;

    global_bufsize = MPID_SMI_EAGERSIZE * MPID_SMI_EAGERBUFS;
    if (global_bufsize > 0) {
		/* allocate global memory with fallback-strategy if not enough resources are available */
		MPID_INFO("  global -");
		do {
			mem_per_proc = global_bufsize;;
			MPID_SMI_PAGESIZE_ALIGN(mem_per_proc);
			shreg_info.size    = (MPID_SMI_numids - local_procs) * mem_per_proc;
			shreg_info.offset  = 0;
			shreg_info.owner   = MPID_SMI_myid;
			shreg_info.adapter = SMI_ADPT_DEFAULT;

			MPID_STAT_CALL(create_sgmnt);
			sgmnt_ok = SMI_Create_shreg(SMI_SHM_LOCAL|SMI_SHM_NONFIXED, &shreg_info,
										&MPID_SMI_Shregid_eagerbufs[MPID_SMI_myid],
										(void **)&MPID_SMI_global_eagerbufs) == SMI_SUCCESS;
			MPID_STAT_RETURN(create_sgmnt);
			if (!sgmnt_ok) {
				MPID_INFO("X ");
				MPID_STAT_COUNT(create_sgmnt_fail);

				/* not enough memory for th eager buffers, reduce size of the buffers */
				if (global_bufsize > MPID_SYS_PAGESIZE) {
					/* reduce the size of the eager buffers */
					global_bufsize /= 2;
					if (global_bufsize < MPID_SYS_PAGESIZE)
						global_bufsize = MPID_SYS_PAGESIZE;
				} else {
					/* we can't reduce the size any more -> no eager for remote processes */
					global_bufsize = 0;
					break;
				}
			} else {
				EAGER_DEBUG(fprintf (stderr, "[%d] incoming global eager buffers located at 0x%p, size 0x%x\n", 
									 MPID_SMI_myid, MPID_SMI_global_eagerbufs, mem_per_proc););
			}
		} while (!sgmnt_ok);
	

		if (MPID_SMI_use_SMP) {
			/* allocate local memory for communication between processes 
			   running on the same node  - XXX implement fallback-strategy ? */
			local_memperproc = MPID_SMI_EAGERBUFS * MPID_SMI_EAGERSIZE;
			MPID_SMI_PAGESIZE_ALIGN(local_memperproc);
			mem_per_proc = MPID_SMI_numProcsOnNode[MPID_SMI_myNode] * local_memperproc;
			shreg_info.size   = MPID_SMI_numProcsOnNode[MPID_SMI_myNode] * mem_per_proc;
			shreg_info.offset = 0;
			shreg_info.adapter= SMI_ADPT_DEFAULT;
	    
			MPID_INFO(" local -");
			MPID_STAT_CALL(create_sgmnt);
			if (SMI_Create_shreg(SMI_SHM_SMP|SMI_SHM_NONFIXED|SMI_SHM_INTERN, &shreg_info, 
								 &MPID_SMI_Locregid_eagerbufs, (void **)&MPID_SMI_local_eagerbufs) != SMI_SUCCESS) {
				MPID_ABORT ("Not enough local shared memory for eager buffers");
			}
			MPID_STAT_RETURN(create_sgmnt);
			EAGER_DEBUG(fprintf (stderr, "[%d] local eager buffers located at 0x%p, size 0x%x\n", 
								 MPID_SMI_myid, MPID_SMI_local_eagerbufs, shreg_info.size););
		}
    }
    
    /* publish the setup of the global eager buffers */
    if (global_bufsize > 0) {
		SMIcall (SMI_Query (SMI_Q_SMI_REGION_SGMT_ID, MPID_SMI_Shregid_eagerbufs[MPID_SMI_myid], &local_sgmt_id));
		SMIcall (SMI_Query (SMI_Q_SMI_REGION_ADPTNBR, MPID_SMI_Shregid_eagerbufs[MPID_SMI_myid], &adpt_nbr));
    } else {
		local_sgmt_id = -1;
		adpt_nbr      = -1;
    }

    do {
		for (proc = 0; proc < MPID_SMI_numids; proc++) {
			MPID_SMI_Int_info_exp[proc]->Eager_setup.sgmt_id = local_sgmt_id;
			MPID_SMI_Int_info_exp[proc]->Eager_setup.adptr   = adpt_nbr;
			MPID_SMI_Int_info_exp[proc]->Eager_setup.nbrbufs = (global_bufsize > 0) ? 1 : 0;
			MPID_SMI_Int_info_exp[proc]->Eager_setup.bufsize = global_bufsize;
		}
    } while (SMI_Check_transfer(CHECK_MODE) != SMI_SUCCESS);
    
    /* Set up the ptr arrays towards the other processes (this is where ptrs to free'd
       buffers are stored). These arrays are stored behind the short buffers of each process. */
    for (proc = 0; proc < MPID_SMI_numids; proc++) {
		MPID_SMI_eager_offsets_rmt[proc] = MPID_SMI_shmem_short[proc] 
			+ MPID_SMI_SHORTSIZE*MPID_SMI_SHORTBUFS * MPID_SMI_numids
			+ MPID_SMI_numids*sizeof(MPID_SMI_Int_data) + MPID_SMI_myid*MPID_SMI_EAGERBUFS*sizeof(char **);
		MPID_SMI_eager_offsets_local[proc] = MPID_SMI_shmem_short[MPID_SMI_myid]
			+ MPID_SMI_SHORTSIZE*MPID_SMI_SHORTBUFS * MPID_SMI_numids
			+ MPID_SMI_numids*sizeof(MPID_SMI_Int_data) + proc*MPID_SMI_EAGERBUFS*sizeof(char **);
    }

    /* set connection status for all shared memory segments;
	   only local segments are connected right from the beginning */
    for (proc = 0; proc < MPID_SMI_numids; proc++) {
		if (MPID_SMI_use_localseg[proc]) {
			MPID_SMI_eagerseg_connected[proc] = true;
		} else {
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
    int proc, offset, i;

    /* SCI remote reads ! */
    for (proc = 0; proc < MPID_SMI_numids; proc++) {
		MPID_SMI_Eagern_connect_info[proc].sgmt_id   = MPID_SMI_Int_info_imp[proc]->Eager_setup.sgmt_id;
		MPID_SMI_Eagern_connect_info[proc].adptr_nbr = MPID_SMI_Int_info_imp[proc]->Eager_setup.adptr;
		MPID_SMI_Eagern_connect_info[proc].bufsize   = MPID_SMI_Int_info_imp[proc]->Eager_setup.bufsize;
		MPID_SMI_Eagern_connect_info[proc].mem_size  = MPID_SMI_Eagern_connect_info[proc].bufsize;
		MPID_SMI_Eagern_connect_info[proc].nbr_bufs  = MPID_SMI_Int_info_imp[proc]->Eager_setup.nbrbufs ;

		MPID_SMI_Eagern_connect_info[proc].nbr_cncts = 0;

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
	
    }

    return;
}


/* Initialize the ptrs for reading incoming / writing outgoing msgs. */
void MPID_SMI_dEager_init_complete(void)
{
	int proc;

	get_connect_info();
	for (proc = 0; proc < MPID_SMI_numids; proc++)
		MPID_SMI_dEager_meminit(proc);
  
	return;
}


/* on-demand connection to a remote eager segment */
int MPID_SMI_dEager_connect(int cnct_proc)
{
    int  err;
    
    if (MPID_SMI_use_localseg[cnct_proc])
		return MPI_SUCCESS;

    if (MPID_SMI_Eagern_connect_info[cnct_proc].nbr_bufs == 0) {
		/* no remote eager buffers available, will use rendez-vous instead */
		MPID_SMI_eagerseg_connected[cnct_proc] = true;
		return MPI_ERR_INTERN;
    }

    if( ( err = MPID_SMI_Rmt_mem_map (cnct_proc, MPID_SMI_Eagern_connect_info[cnct_proc].sgmt_id, MPID_SMI_Eagern_connect_info[cnct_proc].mem_size, 
									  MPID_SMI_Eagern_connect_info[cnct_proc].mem_offset, MPID_SMI_Eagern_connect_info[cnct_proc].adptr_nbr, 
									  (void **)&MPID_SMI_outgoing_eagerbufs[cnct_proc], 
									  &MPID_SMI_Shregid_eagerbufs[cnct_proc],
									  MPID_SMI_RSRC_PARTIAL_MAP) ) != MPI_SUCCESS) {
		MPID_SMI_DEBUG_PRINT_MSG2("Could not import EAGER buffers from (%d)", cnct_proc);
		return MPI_ERR_INTERN;
    }
    MPID_SMI_Eagern_connect_info[cnct_proc].nbr_cncts++;

    MPID_STAT_COUNT(eager_sgmnt_cnct);
    return MPI_SUCCESS;
}

/* Basically decrement usage counter for the remote eager memory. */
void MPID_SMI_dEager_disconnect(int proc)
{
    if (!MPID_SMI_use_localseg[proc]) {
		MPID_SMI_Rmt_mem_release (MPID_SMI_outgoing_eagerbufs[proc], 0, MPID_SMI_RSRC_CACHE);
		MPID_SMI_outgoing_eagerbufs[proc] = NULL;
    }
    return;
}


/* initialize the local data structures for send & receive */
void MPID_SMI_dEager_meminit (int proc)
{
    int i, local_procs = 0;
    uint local_bufmem, mem_per_proc;

    if (MPID_SMI_use_localseg[proc]) {
		/* buffers in local shmem have always the original size */
		local_bufmem = MPID_SMI_EAGERBUFS * MPID_SMI_EAGERSIZE;
		mem_per_proc = local_bufmem;
		MPID_SMI_PAGESIZE_ALIGN(mem_per_proc);
		
		/* for processes located on the same node, use local shared memory */
		/* calculate the beginning of the buffer to which we write if we send proc a message */
		MPID_SMI_outgoing_eagerbufs[proc] = MPID_SMI_local_eagerbufs 
			+ MPID_SMI_localRankForProc[proc] * MPID_SMI_numProcsOnNode[MPID_SMI_myNode] * mem_per_proc; 
		MPID_SMI_outgoing_eagerbufs[proc] += MPID_SMI_localRankForProc[MPID_SMI_myid] * mem_per_proc;
		
		MPID_SMI_incoming_eagerbufs[proc] = MPID_SMI_local_eagerbufs 
			+ MPID_SMI_localRankForProc[MPID_SMI_myid] * MPID_SMI_numProcsOnNode[MPID_SMI_myNode] * mem_per_proc;
		MPID_SMI_incoming_eagerbufs[proc] += MPID_SMI_localRankForProc[proc] * mem_per_proc;
    } else {
		/* processes on the same node do not have buffers in SCI memory allocated */
		for (i = 0; i < proc; i++)
			if (MPID_SMI_use_localseg[i])
				local_procs++;
		MPID_SMI_incoming_eagerbufs[proc] = MPID_SMI_global_eagerbufs 
			+ MPID_SMI_Eagern_connect_info[MPID_SMI_myid].mem_size*(proc - local_procs);

		/* to be determined on connection */
		MPID_SMI_outgoing_eagerbufs[proc] = 0;
    }

    /* The read offsets need to be communicated directly via shared memory between the writer and 
       reader of a buffer to ensure flow control. The write offsets are communicated via the
       control messages.

       First, read offset of local incoming buffer for msgs from 'proc', to be read by 'proc'
       and written by local process.*/
    MPID_SMI_lEager_Roffsets[proc].r = (volatile unsigned int *)MPID_SMI_eager_offsets_rmt[proc];
    /* Read offset of remote outgoing buffer for msgs to 'proc', to be written by 'proc' 
       and read by local process. */
    MPID_SMI_lEager_Soffsets[proc].r = (volatile unsigned int *)MPID_SMI_eager_offsets_local[proc];

    /* Write offset in remote outgoing buffer for msgs to 'proc' - only relevant for local process */
    MPID_SMI_lEager_Soffsets[proc].w = MPID_SMI_lEager_Soffsets[proc].r + 1;
    /* Write offset in local incoming buffer for msgs from 'proc' - only relevant for 'proc',
       local process can ignore it.*/
    MPID_SMI_lEager_Roffsets[proc].w = NULL;

    /* initialize read and write pointer with 0 (they are used as an offset 
       relative to the beginning of the buffer). */
    *(MPID_SMI_lEager_Soffsets[proc].r) = *(MPID_SMI_lEager_Soffsets[proc].w) = 0;

    return;
}

/* if there is enough free memory in the eager buffer of the receiver to send the message,
   then return the address (valid for the sender) of the beginning of this free memory */
void *MPID_SMI_dEager_get_buf( dest, size )
	 int dest, size;
{
    char *buffer = 0;
    int checked_device = 0;
    int available_bufsize;
    unsigned int read, write;
    
    MPID_TRACE_CODE("GetEagerAddress", dest);

    /* make sure the segment towards this proc is connected */
    if (!MPID_SMI_eagerseg_connected[dest]) {
		if (MPID_SMI_dEager_connect(dest) != MPI_SUCCESS)
			return 0;
    }

    /* Is the eager protocol available for communication with this process ? */
    if (MPID_SMI_Eagern_connect_info[dest].bufsize == 0)
		return 0;

    /* Integrity check for partial writes - the read-index is written by the remote
       receiver and may be in transit when reading it. 
       XXX: The method below is not necessarily bullet-proof. A more safe (but also 
       non-guaranteed) way to ensure the integrity of the value would be to store 
       the last value used and check for plausibility: the read-index may not have
       increased further than the amount of data written since the last change of it.
       To make it really bullet-proof, a detailed analysis of the possible non-consistent
       states of the value is required. */
    SMI_Flush_read (dest);
    do {
		read = *(MPID_SMI_lEager_Soffsets[dest].r);
    } while (read >= MPID_SMI_Eagern_connect_info[dest].bufsize);
    write = *(MPID_SMI_lEager_Soffsets[dest].w);

    /* align message size to a multiple of MPID_SMI_EAGER_ALIGN_SIZE */
    if (size & (MPID_SMI_EAGER_ALIGN_SIZE - 1))
		size = (size / MPID_SMI_EAGER_ALIGN_SIZE + 1) * MPID_SMI_EAGER_ALIGN_SIZE;

    /* calculate the free memory in the eager buffer */
    available_bufsize = (read > write) ?
		read - write - 1 :  MPID_SMI_Eagern_connect_info[dest].bufsize - (write - read) - 1;

    while (checked_device <= MPID_SMI_EAGER_MAXCHECKDEV) {
		/* is there enough room in the eager buffer to send this message ? */
		if (size <= available_bufsize) { 
			/* Yes, this is the location we can write to. */
			buffer = MPID_SMI_outgoing_eagerbufs[dest];
			buffer += write;
    
			/* Increment write-ptr with wrap around. */
			write += size;
			write = write & (MPID_SMI_Eagern_connect_info[dest].bufsize - 1);	    
			*(MPID_SMI_lEager_Soffsets[dest].w) = write;
	    
			break;
		} else {
			/* Not enough space left in remote buffer. */
			buffer = NULL;
		}

		checked_device++;
		if (MPID_SMI_EAGER_MAXCHECKDEV > 0)
			MPID_DeviceCheck( MPID_NOTBLOCKING );
    }

    /* If we got no buffer, we don't need the remote segment any longer. */
    if (buffer == NULL)
		MPID_SMI_sEager_disconnect (dest);

    EAGER_DEBUG (fprintf (stderr, "[%d] sending eager to (%d): size 0x%x,  offset 0x%x\n", MPID_SMI_myid, dest,
						  size, buffer -  (ulong)MPID_SMI_outgoing_eagerbufs[dest]););

    return (void *)buffer;
}

/* After having received the message, advance the read buffer to mark the memory as free
   bufptr is a valid adress for the receiver i.e. it points into the eager buffer in which
   we receive messages from "from" */
int MPID_SMI_dEager_free_buf ( void *bufptr, int size, int from )
{
    unsigned int read;

    MPID_TRACE_CODE ("FreeEagerAdress", from);

    if (MPID_SMI_Eagern_connect_info[from].nbr_bufs == 0)
		return MPI_ERR_INTERN;

    /* Get current read ptr from local cache. */
    read = read_ptrs[from];
    
    /* align message size to a multiple of MPID_SMI_EAGER_ALIGN_SIZE */
    if (size & (MPID_SMI_EAGER_ALIGN_SIZE - 1))
		size = (size / MPID_SMI_EAGER_ALIGN_SIZE + 1) * MPID_SMI_EAGER_ALIGN_SIZE;

    EAGER_DEBUG(fprintf (stderr, "[%d] freeing eager from (%d): size 0x%x,  offset 0x%x\n", MPID_SMI_myid, from,
						 size, read););

    /* increment read with wrap around */
    read += size;
    if (MPID_SMI_use_localseg[from]) {
		if (read >= MPID_SMI_EAGERBUFS * MPID_SMI_EAGERSIZE)
			read -= MPID_SMI_EAGERBUFS * MPID_SMI_EAGERSIZE;
    } else {
		if (read >= MPID_SMI_Eagern_connect_info[MPID_SMI_myid].bufsize)
			read -= MPID_SMI_Eagern_connect_info[MPID_SMI_myid].bufsize;
    }
    
    /* Store updated readptr in local cache and in remote memory (to be read by remote process). */
    read_ptrs[from] = read;

    do {
		*(MPID_SMI_lEager_Roffsets[from].r) = read;
    } while (MPID_SMI_cfg.DO_VERIFY 
			 && (SMI_Check_transfer_proc(from, CHECK_MODE) != SMI_SUCCESS));

    return MPI_SUCCESS;
}

/* copy data from user buffer (buffer) to eager buffer on destination process (eager_addr);
   eager_addr is valid for the sender process */
void MPID_SMI_dEager_sendcpy(int dest, void *eager_addr, void *buffer, int len, struct MPIR_DATATYPE *dtp)
{
    int copy_len, aligned_len, align_size, copied, retry = 0;
    char *rest_of_buffer;

    MPID_STAT_ENTRY (eager_scopy);
    EAGER_DEBUG(fprintf (stderr, "[%d] doing eager sendcpy to (%d): size 0x%x,  address 0x%p\n", 
						 MPID_SMI_myid, dest, len, eager_addr););

    MEMCPYSYNC_ENTER(dest, len);
    
    /* first test for non-contig send */
    if (dtp != NULL) {
		if (len <= (MPID_SMI_Eagern_connect_info[dest].bufsize - ((size_t)eager_addr - (size_t)MPID_SMI_outgoing_eagerbufs[dest]))) {
			/* there is enough room in the eager buffer after eager_addr -> copy in one piece */
			copied = 0;
			copied += MPID_SMI_Pack_ff ((char *)buffer, dtp, (char *)eager_addr, dest, len, copied);
		} else {
			/* there isn't enough room in the eager buffer after eager_addr -> we have to copy
			   the data in two pieces */
			/* first: copy until the end of the eager buffer */
			copied = 0;
			copy_len = MPID_SMI_Eagern_connect_info[dest].bufsize - ((size_t)eager_addr - (size_t)MPID_SMI_outgoing_eagerbufs[dest]);
			copied += MPID_SMI_Pack_ff ((char *)buffer, dtp, (char *)eager_addr, dest, copy_len, copied);
			/* second: copy the rest of the user buffer to the beginning of the eager buffer */
			copy_len = len - copy_len;
			copied += MPID_SMI_Pack_ff ((char *)buffer, dtp, (char *)MPID_SMI_outgoing_eagerbufs[dest], dest, copy_len, copied);
		}
    } else {
		/* contiguous datatype */
		if (len <= (MPID_SMI_Eagern_connect_info[dest].bufsize - ((size_t)eager_addr - (size_t)MPID_SMI_outgoing_eagerbufs[dest]))) {
			/* there is enough room in the eager buffer after eager_addr 
			   -> copy in one piece */
	    
			if (!MPID_SMI_is_remote[dest]) {
				/* local shmem copy */
				MEMCPY(eager_addr, buffer, len);
			} else {
				/* remote SCI write */
				aligned_len = len;
				if( ( align_size = ( len & (MPID_SMI_EAGER_ALIGN_SIZE-1) ) ) ) {
					aligned_len -= align_size;
					MEMCPY(MPID_SMI_eager_align_buf, ((char *)buffer) + aligned_len, align_size);
				}
				do {
					MEMCPY_W_FAST(eager_addr, buffer, aligned_len, dest);
					if (align_size) {
						MEMCPY_S(((char *)eager_addr) + aligned_len, MPID_SMI_eager_align_buf, 
								 MPID_SMI_EAGER_ALIGN_SIZE);
					}
					if (++retry > 1)
						MPID_STAT_COUNT(sci_transm_error);	    
				}  while (MPID_SMI_cfg.DO_VERIFY 
						  && SMI_Check_transfer_proc(dest, CHECK_MODE) != SMI_SUCCESS);
			}
		} else {
			/* there isn't enough room in the eager buffer after eager_addr 
			   -> we have to copy the data in two pieces */
	    
			/* first: copy until the end of the eager buffer */
			copy_len = MPID_SMI_Eagern_connect_info[dest].bufsize - ((size_t)eager_addr - (size_t)MPID_SMI_outgoing_eagerbufs[dest]);
			MEMCPY_W(eager_addr, buffer, copy_len, dest);
	    
			/* second: copy the rest of the user buffer to the beginning of the eager buffer */
			rest_of_buffer = (char *)buffer + copy_len;
			copy_len = len - copy_len;
	    
			if (!MPID_SMI_is_remote[dest]) {
				/* local shmem copy */
				MEMCPY(MPID_SMI_outgoing_eagerbufs[dest], rest_of_buffer, copy_len);
			} else {
				aligned_len = copy_len;
				if( ( align_size = (copy_len & (MPID_SMI_EAGER_ALIGN_SIZE-1) ) ) ) {
					aligned_len -= align_size;
					MEMCPY(MPID_SMI_eager_align_buf, rest_of_buffer + aligned_len, align_size);
				}
				do {
					MEMCPY_W_FAST(MPID_SMI_outgoing_eagerbufs[dest], rest_of_buffer, aligned_len, dest);
					if (align_size) {
						MEMCPY_S(((char *)(MPID_SMI_outgoing_eagerbufs[dest])) + aligned_len, 
								 MPID_SMI_eager_align_buf, MPID_SMI_EAGER_ALIGN_SIZE);		    
					}
					if (++retry > 1)
						MPID_STAT_COUNT(sci_transm_error);	    
				}  while (MPID_SMI_cfg.DO_VERIFY 
						  && SMI_Check_transfer_proc(dest, CHECK_MODE) != SMI_SUCCESS);
			}
		}
    }
	
    MEMCPYSYNC_LEAVE (dest, len);
    MPID_STAT_EXIT (eager_scopy);
    
    return;
}

/* copy data from eager_buffer (eager_addr) to user buffer (buffer);
   eager_addr is valid for the receiver process and marks the beginning of the sent data */
void MPID_SMI_dEager_recvcpy(int from, void *eager_addr, void *buffer, int len, struct MPIR_DATATYPE *dtp,
							 struct MPIR_OP *op_ptr)
{
    int copy_len, actual_eager_bufsize, copied;
    char *rest_of_buffer;

    
    MPID_STAT_ENTRY(eager_rcopy);
    EAGER_DEBUG(fprintf (stderr, "[%d] doing eager recvcpy from (%d): size 0x%x,  address 0x%p\n", 
						 MPID_SMI_myid, from, len, eager_addr););

    /* XXX For now, we do not use this here. It wouldn't be a big problem to implement it, though. */
    MPID_ASSERT (op_ptr == NULL, "Direct reduce operation not available in dynamic eager protocol!");

    /* first test for non-contig recv */
    if (dtp) {
		if( MPID_SMI_use_localseg[from] )
			actual_eager_bufsize = MPID_SMI_EAGERBUFS * MPID_SMI_EAGERSIZE;
		else
			actual_eager_bufsize = MPID_SMI_Eagern_connect_info[MPID_SMI_myid].bufsize;
    
		if (len <= (actual_eager_bufsize - ((size_t)eager_addr - (size_t)MPID_SMI_incoming_eagerbufs[from]))) {
			/* message has arrived in one piece -> just copy it to the user buffer */
			copied = 0;
			copied += MPID_SMI_UnPack_ff ((char *)eager_addr, dtp, (char *)buffer, from, len, copied);
		} else {
			/* message has arrived in two pieces */
			/* first: copy until the end of the eager buffer */
			copied = 0;
			copy_len = actual_eager_bufsize - ((size_t)eager_addr - (size_t)MPID_SMI_incoming_eagerbufs[from]);
			copied += MPID_SMI_UnPack_ff ((char *)eager_addr, dtp, (char *)buffer, from, copy_len, copied);
			/* second: copy the rest from the beginning of the eager buffer to the rest of the user buffer */
			copy_len = len - copy_len;
			copied += MPID_SMI_UnPack_ff ((char *)MPID_SMI_incoming_eagerbufs[from], dtp, (char *)buffer, from, 
										  copy_len, copied);
		}
		MPID_STAT_EXIT(eager_rcopy);
		return;
    }
    
    actual_eager_bufsize = (MPID_SMI_use_localseg[from]) ? 
		MPID_SMI_EAGERBUFS * MPID_SMI_EAGERSIZE : MPID_SMI_Eagern_connect_info[MPID_SMI_myid].bufsize;
    
    if (len <= (actual_eager_bufsize - ((size_t)eager_addr - (size_t)MPID_SMI_incoming_eagerbufs[from]))) {
		/* message has arrived in one piece -> just copy it to the user buffer */
		MEMCPY_R(buffer, eager_addr, len);
    } else {
		/* message has arrived in two pieces */
		/* first: copy until the end of the eager buffer */
		copy_len = actual_eager_bufsize - ((size_t)eager_addr - (size_t)MPID_SMI_incoming_eagerbufs[from]);
		MEMCPY_R(buffer, eager_addr, copy_len);
		/* second: copy the rest from the beginning of the eager buffer to the rest of the user buffer */
		rest_of_buffer = (char *)buffer + copy_len;
		copy_len = len - copy_len;
		MEMCPY_R(rest_of_buffer, MPID_SMI_incoming_eagerbufs[from], copy_len);
    }
    
    MPID_STAT_EXIT(eager_rcopy);
    return;
}

void MPID_SMI_dEager_delete(void)
{
    int proc;

    /* free global memory */
    for ( proc = 0; proc < MPID_SMI_numids; proc++) 
		if (MPID_SMI_eagerseg_connected[proc] && (MPID_SMI_Eagern_connect_info[proc].bufsize > 0) && !MPID_SMI_use_localseg[proc])
			SMIcall (SMI_Free_shreg(MPID_SMI_Shregid_eagerbufs[proc]));

    /* free localmemory */
    FREE (MPID_SMI_lEager_Soffsets);
    FREE (MPID_SMI_lEager_Roffsets);
    FREE (read_ptrs);
    FREE (MPID_SMI_eager_offsets_local);
    FREE (MPID_SMI_eager_offsets_rmt);

    if (MPID_SMI_Eagern_connect_info != NULL)
		FREE (MPID_SMI_Eagern_connect_info);

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
