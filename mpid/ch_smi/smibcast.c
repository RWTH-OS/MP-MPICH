/* $Id$ */

/* Pipelined Broadcast ("Pcast") for large messages. Supports single-
   and multi-dimensional pipelines (for multi-dimensional torus topologies)
   and overlaps DMA- with PIO-operations for large vectors. Probably one
   of the best broadcasts in the world... */

#include "smicoll.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

static int pcast_send_pio (int *to_granks, int ndims);
static int pcast_recv_pio (int from_grank);
static int pcast_sendrecv_pio (int from_grank, int to_grank);
static int pcast_send_dma (int *to_granks, int ndims);
static int pcast_recv_dma (int from_grank);
static int pcast_sendrecv_dma (int from_grank, int to_grank);
static int tree_bcast (void *buffer, int count, struct MPIR_DATATYPE *datatype, 
					   int root, struct MPIR_COMMUNICATOR *comm );

/* Determine the send/receive partners for multi-dimensional pipelined broadcast.
   Background: in an n-dimensional torus, perform n concurrent pipelined broadcasts
   along the n dimensions. This promises better performance because the root should be 
   able to feed all pipelines (at least two) fast enough as he does not need to do
   a local copy of the data as all the process in the inner pipeline need to do. 

   The routing for each pipeline goes into its "primary direction" until there is 
   no process left on this ring; then changes tot the next ring along the secondary
   direction and so on. As all mapping of processes to nodes in a grid are to be
   supported, we can not define a "general formula" to calculate the ranks of the
   two communication partners. This is mostly due to "conflicts" when processes
   would be used in more than one pipelines. Therefore, we simulate the selection
   of nodes once and store the derived ranks of partners for this process.

   Any process can be root of the broadcast, resulting in n different routing 
   schemes for n processes in a communicator. We calculate these routing schemes
   on demand and store the information for all other bcasts with the same root. */

/* Find the nearest partner in the ring specified by 'lproc' and 'rdim'. */
static int find_nearest_proc_in_ring (MPID_SMI_comm_info_t *ci, 
									  int lproc, int rdim, boolean *already_chosen)
{
	int from[MAX_SCI_DIMS];
	int lnode, to_lproc, p, n, try_n, d, try_dim;

	LNODE_TO_GRID(ci, ci->lproc_to_lnode[lproc], from[X_DIM], from[Y_DIM], from[Z_DIM]);

	for (n = 0; n < ci->lnodes_in_dim[rdim]; n++) {
		/* Get the next node in this dimension, starting with the local node. */
		try_n = (from[rdim] + n) % ci->lnodes_in_dim[rdim];
		switch (rdim) {
		case X_DIM:
			lnode = GRID_TO_LNODE(ci, try_n, from[Y_DIM], from[Z_DIM]);
			break;
		case Y_DIM:
			lnode = GRID_TO_LNODE(ci, from[X_DIM], try_n, from[Z_DIM]);
			break;
		case Z_DIM:
			lnode = GRID_TO_LNODE(ci, from[X_DIM], from[Y_DIM], try_n);
			break;
		}
		if (lnode < 0)
			continue;

		/* Are there available processes on this node? */
		for (p = 0; p < ci->procs_on_gnode[ci->lnode_to_gnode[lnode]]; p++) {
			to_lproc = ci->lranks_on_gnode[ci->lnode_to_gnode[lnode]][p];
			if (!already_chosen[to_lproc]) {
				/* Found the destination process! */
				already_chosen[to_lproc] = true;
				return to_lproc;
			}
		}
	}

	return -1;
}

/* Return the next process that 'lproc' should send data to in the primary
   dimension 'pdim'. */
static int find_nearest_proc (MPID_SMI_comm_info_t *ci, 
							  int lproc, int pdim, boolean *already_chosen)
{	
	int from[MAX_SCI_DIMS];
	int to_lproc, try_dim, try_proc, dim, d, n, try_n, lnode;

	LNODE_TO_GRID(ci, ci->lproc_to_lnode[lproc], from[X_DIM], from[Y_DIM], from[Z_DIM]);
	
	/* Find partner in directly connected rings, starting with the primary
	   dimension. */
	for (d = 0; d < ci->active_ndims; d++) {
		try_dim = (pdim + d) % ci->active_ndims;
		to_lproc = find_nearest_proc_in_ring (ci, lproc, try_dim, already_chosen);
		if (to_lproc >= 0)
			return (to_lproc);
	}

	/* No available partner found -> we need to to a multi-dimension hop! Walk
	   along a directly connected ring into one dimension and find a partner 
	   from one of the nodes of this ring into another dimension. */
	for (d = 0; d + 1 < ci->active_ndims; d++) {
		dim = (pdim + d) % ci->active_ndims;
		try_dim = (pdim + d + 1) % ci->active_ndims;

		for (n = 0; n < ci->lnodes_in_dim[dim]; n++) {
			/* Get the next node in this dimension, starting with the local node. */
			try_n = (from[dim] + n) % ci->lnodes_in_dim[dim];
			switch (dim) {
			case X_DIM:
				lnode = GRID_TO_LNODE(ci, try_n, from[Y_DIM], from[Z_DIM]);
				break;
			case Y_DIM:
				lnode = GRID_TO_LNODE(ci, from[X_DIM], try_n, from[Z_DIM]);
				break;
			case Z_DIM:
				lnode = GRID_TO_LNODE(ci, from[X_DIM], from[Y_DIM], try_n);
				break;
			}
			if (lnode < 0)
				continue;

			/* Are there available processes on this node? */
			if (ci->procs_on_gnode[ci->lnode_to_gnode[lnode]] > 0) {
				try_proc = ci->lranks_on_gnode[ci->lnode_to_gnode[lnode]][0];
				to_lproc = find_nearest_proc_in_ring(ci, try_proc, try_dim, already_chosen);
				if (to_lproc >= 0) 
					return (to_lproc);
			}
		}
	}
	/* We should never get here because we always need to find a partner! */
	MPID_ASSERT(true, "Could not find partner for pipelined broadcast");
	return -1;
}


static void pcast_calculate_routing(MPID_SMI_comm_info_t *ci, int root)
{
	boolean *already_chosen;
	int dest[MAX_SCI_DIMS], src[MAX_SCI_DIMS];
	int dim, procs_left;

	/* The one-dimensinal case is trivial. */
	if (MPID_SMI_cfg.COLL_BCAST_TYPE == BCAST_PIPELINE || ci->active_ndims == 1) {
		if (root == ci->lrank) {
			ci->bcast.root_send_to[X_DIM] = (ci->lrank + 1) % ci->lsize;
		}
		ci->bcast.send_to[root]   = (ci->lrank == (root + ci->lsize - 1) % ci->lsize) ?
			-1 : (ci->lrank + 1) % ci->lsize;
		ci->bcast.recv_from[root] = (ci->lrank == root) ?
			-1 : (ci->lrank + ci->lsize - 1) % ci->lsize;
	
		return;
	}

	ZALLOCATE(already_chosen, boolean *, sizeof(int) * ci->lsize);
	already_chosen[root] = true;
	procs_left = ci->lsize - 1;

	/* Find destinations for root process. */
	for (dim = 0; dim < ci->active_ndims; dim++) {
		dest[dim] = find_nearest_proc(ci, root, dim, already_chosen);
		if (root == ci->lrank) 
			ci->bcast.root_send_to[dim] = dest[dim];
		if (dest[dim] == ci->lrank) {
			src[dim] = root;
			ci->bcast.recv_from[root] = src[dim];
		}
	}
	procs_left -= ci->active_ndims;

	/* Now proceed from these process through all other processes. */
	while (procs_left > 0) {
		for (dim = 0; dim < ci->active_ndims; dim++) {
			src[dim] = dest[dim];
			dest[dim] = find_nearest_proc(ci, src[dim], dim, already_chosen);
			if (src[dim] == ci->lrank)
				ci->bcast.send_to[root] = dest[dim];
			if (dest[dim] == ci->lrank)
				ci->bcast.recv_from[root] = src[dim];

			if (--procs_left == 0)
				break;
		}
	}

#if 0
	/* XXX debug ouput */
	if (ci->lrank != root)
		fprintf (stderr, "[%d] pcast_route: lrank = %d, root = %d, from = %d, to = %d\n",
				 MPID_SMI_myid, ci->lrank, root, ci->bcast.recv_from[root], ci->bcast.send_to[root]);
	else
		fprintf (stderr, "[%d] pcast_route: lrank = %d (ROOT) to_1 = %d, to_2 = %d\n",
				 MPID_SMI_myid, ci->lrank, root, ci->bcast.root_send_to[1], ci->bcast.root_send_to[2]);		
	fflush (stderr);
#endif

	FREE (already_chosen);
	return;
}

/* These two functions are called upon creation of each new communicator. */
void MPID_SMI_Pcast_coll_init (MPID_SMI_comm_info_t *ci)
{
	int p;

	ALLOCATE(ci->bcast.send_to, int *, sizeof(int)*ci->lsize);
	ALLOCATE(ci->bcast.recv_from, int *, sizeof(int)*ci->lsize);

	for (p = 0; p < ci->lsize; p++) {
		ci->bcast.send_to[p] = -1;
		ci->bcast.recv_from[p] = -1;
	}

	for (p = 0; p < MAX_SCI_DIMS; p++)
		ci->bcast.root_send_to[p] = -1;

	return;
}

void MPID_SMI_Pcast_coll_destroy (MPID_SMI_comm_info_t *ci)
{
	FREE(ci->bcast.send_to);
	FREE(ci->bcast.recv_from);
}


/* This is the actual replacement for MPI_Bcast(). */
int MPID_SMI_Pcast (void *buffer, int count, struct MPIR_DATATYPE *dtype_ptr, int root, 
					struct MPIR_COMMUNICATOR *comm )
{
	MPID_SMI_comm_info_t *ci = (MPID_SMI_comm_info_t *)comm->adiCollCtx;
    int        mylrank, size, pcast_ndims;
    int        relative_rank, to_grank, from_grank;
    int        mpi_errno = MPI_SUCCESS;
    int        contig_size, dtypesize;
    ulong      sgmt_offset;
    static char myname[] = "MPID_SMI_PCAST";
	int        do_dma;

    /* See the overview in Collection Operations for why this is ok */
    if (count == 0) 
		return MPI_SUCCESS;

    dtypesize = MPIR_GET_DTYPE_SIZE(datatype,dtype_ptr);
    contig_size = count * dtypesize;
	
	size = comm->local_group->np;
        
#ifndef MPIR_NO_ERROR_CHECKING
    if (root >= size) 
		mpi_errno = MPIR_Err_setmsg( MPI_ERR_ROOT, MPIR_ERR_ROOT_TOOBIG, 
									 myname, (char *)0, (char *)0, root, size );
    if (mpi_errno)
		return MPIR_ERROR(comm, mpi_errno, myname );
#endif
    
    /* If there is only one process */
    if (size == 1) 
		return (mpi_errno);
	
	/* XXX For now, we only do pipeline-bcast for contiguous data. */
	/* Small data (short and eager size messages) is transmitted
	   faster via the tree-based bcast. */
	if (contig_size <= MPID_SMI_cfg.COLL_PIPE_MIN) 
		return tree_bcast (buffer, count, dtype_ptr, root, comm);

	/* Determine optimal blocksize when dynamicblocksize is set. */
	if ( MPID_SMI_cfg.COLL_PIPE_DYNAMIC ) {
		/* Really big packets (greater BLOCK_INC_LIMIT) need a blocksize of at least BIG_BLOCK for
		   good DMA performance. This is more important than many blocks. */
		if (contig_size >= PIPE_BLOCK_INC_LIMIT 
			&& MPID_SMI_cfg.COLL_PIPE_BLOCKSIZE < PIPE_BIG_BLOCK ) {
			mpid_smi_pipe_nbrblocks = MPID_SMI_RNDVSIZE / 4 / PIPE_BIG_BLOCK;
			mpid_smi_pipe_bsize = PIPE_BIG_BLOCK;
			if ( mpid_smi_pipe_nbrblocks < 3 ) {
				/* Number of blocks must be at least 3.
				   If it isn't possible to get enough blocks (with size BIG_BLOCK) forget it,
				   take original values and continue with bad performance */
				mpid_smi_pipe_bsize = MPID_SMI_cfg.COLL_PIPE_BLOCKSIZE;
				mpid_smi_pipe_nbrblocks = MPID_SMI_cfg.COLL_PIPE_NBRBLOCKS;
			}
		} else {
			mpid_smi_pipe_nbrblocks = MPID_SMI_cfg.COLL_PIPE_NBRBLOCKS;
			/* For small packets reduce blocksize for constant number of blocks.
			   Fewer blocks would reduce pipeline performance */
			mpid_smi_pipe_bsize = MPID_MIN(MPID_SMI_cfg.COLL_PIPE_BLOCKSIZE * mpid_smi_pipe_nbrblocks,
								   contig_size)  / mpid_smi_pipe_nbrblocks;
		}
		MPID_SMI_STREAMBUF_ALIGN(mpid_smi_pipe_bsize);
	} else {
		mpid_smi_pipe_bsize = MPID_SMI_cfg.COLL_PIPE_BLOCKSIZE;
		mpid_smi_pipe_nbrblocks = MPID_SMI_cfg.COLL_PIPE_NBRBLOCKS;
	}

    /* Get my rank and switch communicators to the hidden collective */
    mylrank = comm->local_group->local_rank;
    comm = comm->comm_coll;

    /* Set up the pcast routing, if not yet done, and set the ranks this
	   process needs to communicate with. */
	if (ci->bcast.send_to[root] < 0 && ci->bcast.recv_from[root] < 0)
		pcast_calculate_routing (ci, root);
	from_grank = ci->bcast.recv_from[root] >= 0 ? 
		comm->lrank_to_grank[ci->bcast.recv_from[root]] : -1;
	to_grank   = ci->bcast.send_to[root] >= 0 ? 
		comm->lrank_to_grank[ci->bcast.send_to[root]] : -1;
    relative_rank = (mylrank - root + size) % size;

    /* initialize rhandle */
    MPID_Recv_init( &mpid_smi_pipe_rhandle );
	mpid_smi_pipe_rhandle.recv_handle = &mpid_smi_pipe_rhandle_recv_handle;
	memset(mpid_smi_pipe_rhandle.recv_handle, 0, sizeof(mpid_smi_pipe_rhandle_recv_handle));
	
    mpid_smi_pipe_rhandle.buf = buffer;
    
    /* Send PIPE_READY to rank - 1 (except root) */
    if (relative_rank > 0) {
		mpid_smi_send_pipeready_pkt (from_grank, comm, contig_size, &sgmt_offset);
		pcast_ndims = 1;
	} else {
		pcast_ndims = (MPID_SMI_cfg.COLL_BCAST_TYPE == BCAST_MULTIDIM) ? ci->active_ndims : 1;
	}
	
    /* Wait for PIPE_READY packets to arrive.
	   The root may receive more than one of these packets, all other receive 
	   exactly one packet. The last processes in the pipelines recv no packet. */
    if (ci->bcast.send_to[root] >= 0 || relative_rank == 0) {
		while (mpid_smi_pipe_ready_arrived < pcast_ndims)
			MPID_DeviceCheck( MPID_NOTBLOCKING );
		mpid_smi_pipe_ready_arrived = 0;
		mpid_smi_pipe_shandle[0].start = buffer;
    }

	do_dma = mpid_smi_pipe_do_dma(contig_size, comm, ci);
	
    /* Now we can start receive and/or send. The root does only send data,
	   the last process does only receive, all other processes need to send
	   and receive. */
	if (do_dma) {
		if (relative_rank == 0) {
			mpi_errno = pcast_send_dma(ci->bcast.root_send_to, pcast_ndims);
		} else if (ci->bcast.send_to[root] < 0) {
			mpi_errno = pcast_recv_dma (from_grank);
		} else {
			mpi_errno = pcast_sendrecv_dma (from_grank, to_grank);
		}
	} else {
		if (relative_rank == 0) {
			mpi_errno = pcast_send_pio(ci->bcast.root_send_to, pcast_ndims);
		} else if (ci->bcast.send_to[root] < 0) {
			mpi_errno = pcast_recv_pio (from_grank);
		} else {
			mpi_errno = pcast_sendrecv_pio (from_grank, to_grank);
		}
	}
	if (relative_rank != 0)
		mpid_smi_pipe_free_recvbuf (sgmt_offset, from_grank);

	return mpi_errno;
}


/* main part from int MPID_SMI_Brndv_send_ack(in_pkt, from_grank) */
static int pcast_send_pio (int *to_granks, int ndims)
{
    long * volatile woffset_ptr[MAX_SCI_DIMS], * volatile roffset_ptr[MAX_SCI_DIMS];  
    char *pcast_base_addr[MAX_SCI_DIMS];   /* addresses of mapped recv buffers */
    char *pcast_data_addr[MAX_SCI_DIMS];   /* adresses of data section of recv buffers */
    unsigned int ptrmem_size, misalign_size;
    int dim, msglen, len_sent = 0, len_to_send;    /* len_sent + len_to_send = msglen */
    int avail_bufsize[MAX_SCI_DIMS], databuf_size[MAX_SCI_DIMS];
    /* variables for sending message */
    volatile long roffset[MAX_SCI_DIMS], woffset[MAX_SCI_DIMS], cpy_len;
    char *target_addr[MAX_SCI_DIMS], *source_addr, *align_buf;
	
	MPID_STAT_ENTRY(bcast);
	
    /* Initialize all ptrs and buffer sizes. */
	for (dim = 0; dim < ndims; dim++) {
		pcast_base_addr[dim]  = (char *)mpid_smi_pipe_shandle[dim].recv_handle->dest_addr;
		misalign_size        = (size_t)pcast_base_addr[dim] % MPID_SMI_STREAMSIZE;
		pcast_base_addr[dim] += (misalign_size == 0) ? 0 : (MPID_SMI_STREAMSIZE - misalign_size);

		ptrmem_size = 2*MPID_SMI_STREAMSIZE;
		woffset_ptr[dim] = (long * volatile)(pcast_base_addr[dim] + MPID_SMI_STREAMSIZE - sizeof(long *));
		roffset_ptr[dim] = (long * volatile)(woffset_ptr[dim] + MPID_SMI_STREAMSIZE/sizeof(long));
		woffset[dim] = 0;
		roffset[dim] = 0;

		pcast_data_addr[dim] = pcast_base_addr[dim] + ptrmem_size;
		target_addr[dim]     = pcast_data_addr[dim];
		databuf_size[dim]    = (mpid_smi_pipe_shandle[dim].recv_handle->len_local - ptrmem_size)
			/ MPID_SMI_STREAMSIZE * MPID_SMI_STREAMSIZE;
		/* We need to avoid the the write offset "catches" the read offset from behind: 
		   if these two are equal, we'll have a deadlock. */
		avail_bufsize[dim]  = databuf_size[dim] - MPID_SMI_STREAMSIZE;
	}
	/* We copy synchronously towards all processes; this means equal sized blocks
	   from the same source address. */
	source_addr = (char *)mpid_smi_pipe_shandle[0].start;
	len_to_send = msglen = mpid_smi_pipe_shandle[0].bytes_as_contig;

    /* now the actual copying of the data begins; first, we copy until we have less than
       MPID_SMI_STREAMSIZE bytes left, then we copy the rest with alignment */
    while (len_to_send >= MPID_SMI_STREAMSIZE) {
		for (dim = 0; dim < ndims; dim++) {
			/* calculate length of data which we actually send now, this is the minimum 
			   of len(aligned downwards), pipe_bsize, avail_bufsize */
 			cpy_len = MPID_MIN(len_to_send, mpid_smi_pipe_bsize);

			if (cpy_len < mpid_smi_pipe_bsize || avail_bufsize[dim] >= mpid_smi_pipe_bsize) {
				cpy_len = cpy_len / MPID_SMI_STREAMSIZE * MPID_SMI_STREAMSIZE;
				
				MPID_STAT_CALL(bcast_scopy);
				MEMCPYSYNC_ENTER(to_granks[dim], cpy_len);
				if (cpy_len <= databuf_size[dim] - (target_addr[dim] - pcast_data_addr[dim])) {
					/* we can copy in one piece */
					MEMCPY_W( target_addr[dim], source_addr, cpy_len, to_granks[dim] );
				
					target_addr[dim] += cpy_len;
					woffset[dim]     += cpy_len;
					/* check for wrap-around in the target ringbuffer */
					/* woffset > databuf_size is an error! */
					if (woffset[dim] >= databuf_size[dim]) {
						woffset[dim] = 0;
						target_addr[dim] = pcast_data_addr[dim];
					}
				} else {
					/* we have to copy in two pieces: first, we copy until we reach the end of the buffer,
					   the we copy the rest to the beginning of the buffer */
					/* first: copy into the rest of the buffer */
					int part_len = databuf_size[dim] - (target_addr[dim] - pcast_data_addr[dim]);
					char *part_src_addr = source_addr;

					MEMCPY_W( target_addr[dim], part_src_addr, part_len, to_granks[dim] );
				
					target_addr[dim] = pcast_data_addr[dim];
					part_src_addr += part_len;
					woffset[dim] = 0;
				
					/* second: copy the rest to the beginning of the buffer */
					MEMCPY_W( target_addr[dim], source_addr, cpy_len - part_len, to_granks[dim]);
				
					target_addr[dim] += (cpy_len - part_len);
					woffset[dim]      = cpy_len - part_len;			
				}
				WRITE_RMT_PTR(woffset_ptr[dim], woffset[dim], to_granks[dim]);
				MEMCPYSYNC_LEAVE(to_granks[dim], cpy_len);
				avail_bufsize[dim] -= cpy_len;
			}
		}
		MPID_STAT_RETURN(bcast_scopy);
		source_addr += cpy_len;
		len_to_send -= cpy_len;
		len_sent    += cpy_len;

		/* Calculate amount of free memory in the recv buffers;
		   wait until receiver has read some data. Again, substract MPID_SMI_STREAMSIZE
		   from the available length to avoid "pointer catching". */
		if (len_to_send > 0) {
			for (dim = 0; dim < ndims; dim++) {
				while (avail_bufsize[dim] < MPID_SMI_STREAMSIZE 
					   || (len_to_send >= mpid_smi_pipe_bsize && avail_bufsize[dim] < mpid_smi_pipe_bsize)) {
					/* Check if remote process has read data by reading the remote ptr. */
					READ_RMT_PTR(roffset_ptr[dim], roffset[dim], to_granks[dim]);
					
					avail_bufsize[dim] = (roffset[dim] > woffset[dim]) ?
						roffset[dim]- woffset[dim] : databuf_size[dim] - (woffset[dim] - roffset[dim]);
					avail_bufsize[dim] -= MPID_SMI_STREAMSIZE;
				} 
			}
		}
    }
    
    /* Now, we have less than MPID_SMI_STREAMSIZE bytes left in our message 
       -> copy the rest with alignment. */
    if (len_to_send > 0) {
		for (dim = 0; dim < ndims; dim++) {
			/* Since we have copied in pieces of MPID_SMI_STREAMSIZE minimum, we should always 
			   have enough memory left in our buffer to copy this last bytes. */
			if (!MPID_SMI_is_remote[to_granks[dim]]) {
				MEMCPY (target_addr[dim], source_addr, len_to_send);
			} else {
				/* copy date into align buffer , then copy full streambuffer-size */
				align_buf = (char *)MPID_SBalloc(mpid_smi_pipe_alignbuf_allocator);
				MEMCPY (align_buf, source_addr, len_to_send);
				
				MEMCPY_W( target_addr[dim], align_buf, MPID_SMI_STREAMSIZE, to_granks[dim] );
				MPID_SBfree (mpid_smi_pipe_alignbuf_allocator, align_buf);
			}
			woffset[dim] += len_to_send;
			WRITE_RMT_PTR(woffset_ptr[dim], woffset[dim], to_granks[dim]);
		}
    }
	
	for (dim = 0; dim < ndims; dim++) {
		/* Only release remote SCI memory. */
		if (!MPID_SMI_use_localseg[to_granks[dim]]) 
			MPID_SMI_Rmt_mem_release (NULL, mpid_smi_pipe_shandle[dim].recv_handle->smi_regid_dest, 
									  MPID_SMI_RSRC_CACHE);
	    COMPLETE_SHANDLE( &mpid_smi_pipe_shandle[dim] );
	}
	
	MPID_STAT_EXIT(bcast);

    return MPI_SUCCESS;
}



/* main part from int MPID_SMI_Brndv_recv_ack(in_pkt, from_grank) */
static int pcast_recv_pio (int from_grank)
{
    int len_left, ptrmem_size, databuf_size, misalign_size;
    ulong cpy_len = 0;
    char *source_addr, *rndv_base_addr, *rndv_data_addr, *target_addr;
    long * volatile roffset_ptr, * volatile woffset_ptr;
    volatile long woffset, roffset;

	MPID_STAT_ENTRY(bcast);

    rndv_base_addr = (char *)mpid_smi_pipe_rhandle.start;
    misalign_size = (size_t)rndv_base_addr % MPID_SMI_STREAMSIZE;
    rndv_base_addr += (misalign_size == 0) ? 0 : (MPID_SMI_STREAMSIZE - misalign_size);

    /* initalize addresses & sizes */
    ptrmem_size = 2 * MPID_SMI_STREAMSIZE;
    woffset_ptr = (long * volatile)(rndv_base_addr + MPID_SMI_STREAMSIZE - sizeof(long *));
    roffset_ptr = (long * volatile)(woffset_ptr + MPID_SMI_STREAMSIZE/sizeof(long));
    woffset = 0;
    roffset = 0;
	
    rndv_data_addr = rndv_base_addr + ptrmem_size;
    databuf_size   = (mpid_smi_pipe_rhandle.recv_handle->len_local - ptrmem_size)
		/ MPID_SMI_STREAMSIZE * MPID_SMI_STREAMSIZE;
    source_addr    = rndv_data_addr;
    target_addr    = (char *)mpid_smi_pipe_rhandle.buf;
    len_left       = mpid_smi_pipe_rhandle.len;

    while (len_left >= MPID_SMI_STREAMSIZE) {
		/* This is the ptr to the write-offset controlled/advanced by the sender.  */
		do {
			READ_LOCAL_SHARED_PTR(woffset_ptr, woffset, databuf_size);
		} while (woffset == roffset);
		
		/* Now, data is available. */
		MPID_STAT_CALL(bcast_rcopy);
		if (woffset > roffset && woffset - roffset >= MPID_SMI_STREAMSIZE) {
			/* no wrap-around - read a single block. */
			/* try to recv with size mpid_smi_pipe_bsize. If it will be possible, skip
			   copy this time. */
			if ( len_left < mpid_smi_pipe_bsize || woffset - roffset >= mpid_smi_pipe_bsize ) {
				cpy_len = woffset - roffset;
				/* cpy_len must be <= mpid_smi_pipe_bsize for a faster pipeline */
				cpy_len = MPID_MIN(cpy_len, mpid_smi_pipe_bsize);
				MEMCPY_R( target_addr, source_addr, cpy_len);
				roffset      += cpy_len; 
				WRITE_RMT_PTR(roffset_ptr, roffset, from_grank);
				target_addr  += cpy_len;
				source_addr  += cpy_len;
				len_left -= cpy_len;
			}
		} else if ( woffset < roffset ){
			/* We need to wrap around the end of the buffer. We read the remainder
			   of the buffer now, the rest wil be read in the next iteration. */
			cpy_len = databuf_size - roffset;
			/* cpy_len must be <= mpid_smi_pipe_bsize for a faster pipeline */
			cpy_len = MPID_MIN(cpy_len, mpid_smi_pipe_bsize);
			MEMCPY_R( target_addr, source_addr, cpy_len);
			roffset      = (roffset + cpy_len) % databuf_size;
			WRITE_RMT_PTR(roffset_ptr, roffset, from_grank);
			target_addr += cpy_len;
			if ( roffset == 0 ) {
				source_addr  = rndv_data_addr;
			} else {
				source_addr  += cpy_len;
			}
			len_left -= cpy_len;
		}
		MPID_STAT_RETURN(bcast_rcopy);
    }
	
    /* now copy the remainder. */
    if (len_left > 0) {
		/* wait until sender finished */
		do {
			READ_LOCAL_SHARED_PTR(woffset_ptr, woffset, databuf_size);
		} while (woffset - roffset != len_left && databuf_size - (roffset - woffset) != len_left );
		MEMCPY_R( target_addr, source_addr, len_left);
	}
	
    COMPLETE_RHANDLE( &mpid_smi_pipe_rhandle );
	MPID_STAT_EXIT (bcast);

    return MPI_SUCCESS;
}


/* Recv data and pass it on to the next process. Both data transfers are done via PIO 
   in this function.  */
static int pcast_sendrecv_pio (int from_grank, int to_grank)
{
    unsigned int ptrmem_size, misalign_size;

	char *recv_rndv_base_addr, *recv_rndv_data_addr;
    int recv_len_left, recv_databuf_size;
    ulong recv_cpy_len = 0;
    char *recv_source_addr, *recv_target_addr;
    long * volatile recv_roffset_ptr, * volatile recv_woffset_ptr;
    volatile long recv_woffset, recv_roffset;

	char *send_rndv_base_addr, *send_rndv_data_addr;
	/* pointers for ringbuffer administration */
    long * volatile send_woffset_ptr, * volatile send_roffset_ptr;      
	/* len_sent + len_to_send = msglen */
    int send_msglen, send_len_sent = 0, send_len_to_send;
    int send_avail_bufsize, send_databuf_size;
    /* variables for sending message */
    volatile long send_roffset, send_woffset, send_cpy_len = 0;
    char *send_target_addr, *send_source_addr, *send_align_buf;

	MPID_STAT_ENTRY(bcast);

	/* receive: initalize addresses & sizes */
    recv_rndv_base_addr = (char *)mpid_smi_pipe_rhandle.start;
    misalign_size = (size_t)recv_rndv_base_addr % MPID_SMI_STREAMSIZE;
    recv_rndv_base_addr += (misalign_size == 0) ? 0 : (MPID_SMI_STREAMSIZE - misalign_size);

    ptrmem_size = 2 * MPID_SMI_STREAMSIZE;
    recv_woffset_ptr = (long * volatile)(recv_rndv_base_addr + MPID_SMI_STREAMSIZE - sizeof(long *));
    recv_roffset_ptr = (long * volatile)(recv_woffset_ptr + MPID_SMI_STREAMSIZE/sizeof(long));
    recv_roffset = *recv_roffset_ptr;

    recv_rndv_data_addr = recv_rndv_base_addr + ptrmem_size;
    recv_databuf_size   = (mpid_smi_pipe_rhandle.recv_handle->len_local - ptrmem_size)
		/ MPID_SMI_STREAMSIZE * MPID_SMI_STREAMSIZE;
    recv_source_addr    = recv_rndv_data_addr;
    recv_target_addr    = (char *)mpid_smi_pipe_rhandle.buf;
    recv_len_left       = mpid_smi_pipe_rhandle.len;

	/* send: initalize addresses & sizes */
    send_rndv_base_addr = (char *)mpid_smi_pipe_shandle[0].recv_handle->dest_addr;
    misalign_size = (size_t)send_rndv_base_addr % MPID_SMI_STREAMSIZE;
    send_rndv_base_addr += (misalign_size == 0) ? 0 : (MPID_SMI_STREAMSIZE - misalign_size);

    ptrmem_size = 2*MPID_SMI_STREAMSIZE;
    send_woffset_ptr = (long * volatile)(send_rndv_base_addr + MPID_SMI_STREAMSIZE - sizeof(long *));
    send_roffset_ptr = (long * volatile)(send_woffset_ptr + MPID_SMI_STREAMSIZE/sizeof(long));
    send_woffset  = 0;
    send_roffset  = 0;
    /* remote SCI writes */
	WRITE_RMT_PTR(send_woffset_ptr, 0, to_grank);
	WRITE_RMT_PTR(send_roffset_ptr, 0, to_grank);

    send_rndv_data_addr = send_rndv_base_addr + ptrmem_size;
    send_source_addr    = (char *)mpid_smi_pipe_shandle[0].start;
    send_target_addr    = send_rndv_data_addr;
    send_databuf_size   = (mpid_smi_pipe_shandle[0].recv_handle->len_local - ptrmem_size)
		/ MPID_SMI_STREAMSIZE * MPID_SMI_STREAMSIZE;
    /* We need to avoid the the write offset "catches" the read offset from behind: 
       if these two are equal, we'll have a deadlock. */
    send_avail_bufsize  = send_databuf_size - MPID_SMI_STREAMSIZE;
    send_len_to_send    = send_msglen = mpid_smi_pipe_shandle[0].bytes_as_contig;
	
    MEMCPYSYNC_ENTER_NODEVCHECK(to_grank, send_msglen);
	
    while ( send_len_to_send >= MPID_SMI_STREAMSIZE || recv_len_left != 0 ) {
		/* Recv: cut and paste from pcast_recv_pio */
		if ( recv_len_left != 0 ) {
			/* more to receive? check pointer and copy when data available */
			READ_LOCAL_SHARED_PTR(recv_woffset_ptr, recv_woffset, recv_databuf_size);

			if ( recv_woffset != recv_roffset ) {
				/* Now, data is available. */
				MPID_STAT_CALL(bcast_pcopy);
				if (recv_woffset > recv_roffset && recv_woffset - recv_roffset >= MPID_SMI_STREAMSIZE) {
					/* no wrap-around - read a single block. */
					/* try to recv with size mpid_smi_pipe_bsize. If it will be possible, skip
					   copy this time. */
					if (recv_len_left < mpid_smi_pipe_bsize 
						|| recv_woffset - recv_roffset >= mpid_smi_pipe_bsize ) {
						recv_cpy_len = recv_woffset - recv_roffset;
						/* cpy_len must be <= mpid_smi_pipe_bsize for a faster pipeline */
						recv_cpy_len = MPID_MIN(recv_cpy_len, mpid_smi_pipe_bsize);
						MEMCPY_R( recv_target_addr, recv_source_addr, recv_cpy_len);
						recv_roffset      += recv_cpy_len; 
						WRITE_RMT_PTR(recv_roffset_ptr, recv_roffset, from_grank);
						recv_target_addr  += recv_cpy_len;
						recv_source_addr  += recv_cpy_len;
						recv_len_left     -= recv_cpy_len;
					}
				} else {
					if (recv_woffset < recv_roffset) {
						/* We need to wrap around the end of the buffer. But only if the end
						   is less than PCAST_SIZE near. We read the remainder of the buffer now,
						   the rest will be read in the next iteration. */
						recv_cpy_len = recv_databuf_size - recv_roffset;
						/* cpy_len must be <= mpid_smi_pipe_bsize for a faster pipeline */
						recv_cpy_len = MPID_MIN(recv_cpy_len, mpid_smi_pipe_bsize);
						MEMCPY_R( recv_target_addr, recv_source_addr, recv_cpy_len);
						recv_roffset      = (recv_roffset + recv_cpy_len) % recv_databuf_size;
						WRITE_RMT_PTR(recv_roffset_ptr, recv_roffset, from_grank);
						recv_target_addr += recv_cpy_len;
						recv_source_addr = (recv_roffset == 0) ? recv_rndv_data_addr :
							recv_source_addr + recv_cpy_len;
						recv_len_left    -= recv_cpy_len;
					} else {
						if (recv_len_left < MPID_SMI_STREAMSIZE 
							&& recv_woffset - recv_roffset == recv_len_left) {
							MEMCPY_R( recv_target_addr, recv_source_addr, recv_len_left);
							recv_len_left = 0;
						}
					}
				}
				MPID_STAT_RETURN(bcast_pcopy);
			}
		}

		/* Send: calculate length of data which we actually send now, this is the minimum 
		   of len(aligned downwards), mpid_smi_pipe_bsize, avail_bufsize and new received*/
		send_cpy_len = MPID_MIN(send_len_to_send, send_avail_bufsize);
		send_cpy_len = MPID_MIN(send_cpy_len, send_len_to_send - recv_len_left);
 		send_cpy_len = MPID_MIN(send_cpy_len, mpid_smi_pipe_bsize);

		if ( send_len_to_send < mpid_smi_pipe_bsize || send_avail_bufsize >= mpid_smi_pipe_bsize) {
			send_cpy_len = send_cpy_len / MPID_SMI_STREAMSIZE * MPID_SMI_STREAMSIZE;
			send_len_to_send -= send_cpy_len;
			send_len_sent    += send_cpy_len;
			
			MPID_STAT_CALL(bcast_pcopy);
			if (send_cpy_len <= send_databuf_size - (send_target_addr - send_rndv_data_addr)) {
				/* we can copy in one piece */
				MEMCPY_W( send_target_addr, send_source_addr, send_cpy_len, to_grank );
				
				send_target_addr += send_cpy_len;
				send_source_addr += send_cpy_len;
				send_woffset     += send_cpy_len;
				
				/* check for wrap-around in the target ringbuffer */
				/* woffset > databuf_size is an error! */
				if (send_woffset >= send_databuf_size) {
					send_woffset = 0;
					send_target_addr = send_rndv_data_addr;
				}
			} else {
				/* we have to copy in two pieces: first, we copy until we reach the end of the buffer,
				   the we copy the rest to the beginning of the buffer */
				/* first: copy into the rest of the buffer */
				int part_len = send_databuf_size - (send_target_addr - send_rndv_data_addr);
				
				MEMCPY_W( send_target_addr, send_source_addr, part_len, to_grank );
				
				send_target_addr = send_rndv_data_addr;
				send_source_addr += part_len;
				send_woffset = 0;
				
				/* second: copy the rest to the beginning of the buffer */
				send_cpy_len = send_cpy_len - part_len;
				MEMCPY_W( send_target_addr, send_source_addr, send_cpy_len, to_grank );
				
				send_target_addr += send_cpy_len;
				send_source_addr += send_cpy_len;
				send_woffset      = send_cpy_len;
			}
			WRITE_RMT_PTR(send_woffset_ptr, send_woffset, to_grank);
			MPID_STAT_RETURN(bcast_pcopy);
			send_avail_bufsize -= send_cpy_len;
		}
			
		/* Calculate amount of free memory in buffer, if there is nothing available,
		   wait until receiver has read some data. Again, substract MPID_SMI_STREAMSIZE
		   from the available length to avoid "pointer catching". */
		if (send_len_to_send > 0) {
			do {
				/* Check if remote process has read data by reading the remote ptr. */
				READ_RMT_PTR(send_roffset_ptr, send_roffset, to_grank);
				
				send_avail_bufsize = (send_roffset > send_woffset) ?
					send_roffset - send_woffset : send_databuf_size - (send_woffset - send_roffset);
				send_avail_bufsize -= MPID_SMI_STREAMSIZE;
			} while (send_avail_bufsize < MPID_SMI_STREAMSIZE);
		}
	}
	
	MPID_STAT_CALL(bcast_pcopy);
    /* send: now copy the remainder. */
    /* Now, we have less than MPID_SMI_STREAMSIZE bytes left in our message 
       -> copy the rest with alignment. */
    if (send_len_to_send > 0) {
		/* Since we have copied in pieces of MPID_SMI_STREAMSIZE minimum, we should always 
		   have enough memory left in our buffer to copy this last bytes. */
		if (!MPID_SMI_is_remote[to_grank]) {
			MEMCPY (send_target_addr, send_source_addr, send_len_to_send);
		} else {
			/* copy date into align buffer , then copy full streambuffer-size */
			send_align_buf = (char *)MPID_SBalloc(mpid_smi_pipe_alignbuf_allocator);
			MEMCPY (send_align_buf, send_source_addr, send_len_to_send);
			
			MEMCPY_W( send_target_addr, send_align_buf, MPID_SMI_STREAMSIZE, to_grank );
			MPID_SBfree (mpid_smi_pipe_alignbuf_allocator, send_align_buf);
		}
		send_woffset += send_len_to_send;
		WRITE_RMT_PTR(send_woffset_ptr, send_woffset, to_grank);
    }
    MEMCPYSYNC_LEAVE(to_grank, send_msglen);
	MPID_STAT_RETURN(bcast_pcopy);
	
    switch (mpid_smi_pipe_shandle[0].recv_handle->mode) {
    case RNDV_SYNC:
		/* Only release remote SCI memory. */
		if (!MPID_SMI_use_localseg[to_grank]) 
			MPID_SMI_Rmt_mem_release (NULL, mpid_smi_pipe_shandle[0].recv_handle->smi_regid_dest, 
									  MPID_SMI_RSRC_CACHE);
	break;
    default:
		/* no other modes supported by this protocol */
		break;
    }
	
    COMPLETE_RHANDLE( &mpid_smi_pipe_rhandle );
	MPID_STAT_EXIT(bcast);

    return MPI_SUCCESS;   
}



/*
  DMA-based pipelined bcast
  
  This is a first (simple) version of the protocol which has strict buffer size constraints
  and does not perform with the maximum degree of concurrency. Also, it has quite al long 
  pipeline startup and will thus perform best for really long messages. 
*/


/* Recv data and pass it on to the next process. The incoming copy operations are done via PIO,
   while the data transfer to pass the data on to the next process is performed concurrently via DMA. */
static int pcast_recv_dma(int from_grank)
{
	char *recv_base_addr, *recv_databuf_addr, *recv_source_addr, *recv_target_addr;
    long * volatile recv_woffset_ptr, * volatile recv_roffset_ptr;
    volatile long recv_woffset, recv_roffset;
    int recv_databuf_size, recv_avail_bufsize;
	int bufblocks_avail, total_blocks, blocks_in, blocks_out;
	int remaining, cpy_len;
    uint ptrmem_size, misalign_size;
	
	MPID_STAT_ENTRY(bcast_dma);
    ptrmem_size = 2 * MPID_SMI_STREAMSIZE;

	/* receive: initalize addresses & sizes */
    recv_base_addr = (char *)mpid_smi_pipe_rhandle.start;
    misalign_size = (size_t)recv_base_addr % MPID_SMI_STREAMSIZE;
    recv_base_addr += (misalign_size == 0) ? 0 : (MPID_SMI_STREAMSIZE - misalign_size);

    recv_woffset_ptr = (long * volatile )(recv_base_addr + MPID_SMI_STREAMSIZE - sizeof(long *));
    recv_roffset_ptr = recv_woffset_ptr + MPID_SMI_STREAMSIZE/sizeof(long);
	recv_woffset = 0;
    recv_roffset = 0;

    recv_databuf_addr = recv_base_addr + ptrmem_size;
    recv_databuf_size = (mpid_smi_pipe_rhandle.recv_handle->len_local - ptrmem_size)
		/ mpid_smi_pipe_bsize * mpid_smi_pipe_bsize;
	recv_avail_bufsize  = 0;
	recv_target_addr    = (char *)mpid_smi_pipe_rhandle.buf;
		
	bufblocks_avail = recv_databuf_size / mpid_smi_pipe_bsize;
	total_blocks    = mpid_smi_pipe_rhandle.len / mpid_smi_pipe_bsize;
	blocks_in = 0; blocks_out = 0;
    while (blocks_out + 1 < total_blocks) {
		if (recv_avail_bufsize < 2*mpid_smi_pipe_bsize) {
			/* Wait for incoming data. */
			int still_free; 
			still_free = recv_avail_bufsize/mpid_smi_pipe_bsize;

			READ_LOCAL_SHARED_PTR(recv_woffset_ptr, recv_woffset, recv_databuf_size);
			recv_avail_bufsize = recv_woffset >= recv_roffset ? 
				recv_woffset - recv_roffset : recv_woffset + (recv_databuf_size - recv_roffset);
			
			blocks_in += (recv_avail_bufsize / mpid_smi_pipe_bsize) - still_free;
		}
		
		/* Loop over DMA-operations for complete mpid_smi_pipe_bsize blocks. 
		   While reading, always stay one block behind the writer except when
		   reaching the end of the transmission when the danger of ptr-catching 
		   does lo longer exist because we do not update the read ptr below. */
		MPID_STAT_CALL(bcast_rcopy);
		while (blocks_out + 1 < blocks_in|| blocks_out + 2 == total_blocks) {
			recv_source_addr = recv_databuf_addr + (blocks_out % bufblocks_avail)*mpid_smi_pipe_bsize;
			MEMCPY_R( recv_target_addr, recv_source_addr, mpid_smi_pipe_bsize);
			recv_target_addr   += mpid_smi_pipe_bsize;
			
			recv_avail_bufsize -= mpid_smi_pipe_bsize;
			recv_roffset = (recv_roffset + mpid_smi_pipe_bsize) % recv_databuf_size;
			blocks_out++;
			
			/* Update read ptr. */
			if (blocks_out + 1 < total_blocks)
				*recv_roffset_ptr = recv_roffset;
		}
		MPID_STAT_RETURN(bcast_rcopy);
	}

	/* Copy the remaining part of the data: wait until all data has arrived. */
	remaining = mpid_smi_pipe_rhandle.len - (total_blocks-1)*mpid_smi_pipe_bsize;
	do {
		READ_LOCAL_SHARED_PTR(recv_woffset_ptr, recv_woffset, recv_databuf_size);
		recv_avail_bufsize = (recv_woffset >= recv_roffset) ? 
			recv_woffset - recv_roffset : recv_woffset + (recv_databuf_size - recv_roffset);
	} while (recv_avail_bufsize < remaining);
	
	/* Copy the remainder, but take care for wrap-around! */
	MPID_STAT_CALL(bcast_rcopy);
	recv_source_addr = recv_databuf_addr + (blocks_out*mpid_smi_pipe_bsize) % recv_databuf_size;
	cpy_len = MPID_MIN( recv_databuf_size - (blocks_out % bufblocks_avail * mpid_smi_pipe_bsize),
						remaining );
	MEMCPY_R (recv_target_addr, recv_source_addr, cpy_len);

	remaining -= cpy_len;
	recv_target_addr += cpy_len;	
	if (remaining > 0) {
		MEMCPY_R (recv_target_addr, recv_databuf_addr, remaining);
	}	
	MPID_STAT_RETURN(bcast_rcopy);

    COMPLETE_RHANDLE( &mpid_smi_pipe_rhandle );
	MPID_STAT_EXIT(bcast_dma);

    return MPI_SUCCESS;   
}


static int pcast_send_dma(int *to_granks, int ndims)
{
    unsigned int ptrmem_size, misalign_size;

    /* variables for sending data */
	char *send_base_addr[MAX_SCI_DIMS], *send_databuf_addr[MAX_SCI_DIMS];
	char *send_target_addr[MAX_SCI_DIMS], *send_source_addr;
    volatile long send_roffset[MAX_SCI_DIMS], send_woffset[MAX_SCI_DIMS];
	long * volatile send_woffset_ptr[MAX_SCI_DIMS], * volatile send_roffset_ptr[MAX_SCI_DIMS];
    int send_avail_bufsize[MAX_SCI_DIMS], min_avail_bufsize, send_databuf_size;
	int bufblocks_avail, total_blocks;
	int blocks_in, blocks_out;
	int remaining, cpy_len, dim;

	MPID_STAT_ENTRY(bcast_dma);

	/* send: initalize addresses & sizes */
    ptrmem_size = 2 * MPID_SMI_STREAMSIZE;
    send_databuf_size = (mpid_smi_pipe_shandle[0].recv_handle->len_local - ptrmem_size)
		/ mpid_smi_pipe_bsize * mpid_smi_pipe_bsize;
	min_avail_bufsize = send_databuf_size;
	send_source_addr  =  (char *)mpid_smi_pipe_shandle[0].start;
	bufblocks_avail   = send_databuf_size / mpid_smi_pipe_bsize;
	total_blocks      = mpid_smi_pipe_shandle[0].bytes_as_contig / mpid_smi_pipe_bsize;
	blocks_out = 0;

	for (dim = 0; dim < ndims; dim++) {
		send_base_addr[dim] = (char *)mpid_smi_pipe_shandle[dim].recv_handle->dest_addr;
		misalign_size = (size_t)send_base_addr[dim] % MPID_SMI_STREAMSIZE;
		send_base_addr[dim] += (misalign_size == 0) ? 0 : (MPID_SMI_STREAMSIZE - misalign_size);

		send_woffset_ptr[dim] = (long * volatile)(send_base_addr[dim] 
												  + MPID_SMI_STREAMSIZE - sizeof(long *));
		send_roffset_ptr[dim] = send_woffset_ptr[dim] + MPID_SMI_STREAMSIZE/sizeof(long);
		send_woffset[dim] = 0;
		send_roffset[dim] = 0;

		send_databuf_addr[dim]  = send_base_addr[dim] + ptrmem_size;
		send_avail_bufsize[dim] = send_databuf_size;
	}

    while (blocks_out + 1 < total_blocks) {
		/* Loop over DMA-operations for complete mpid_smi_pipe_bsize blocks. */
		MPID_STAT_CALL(bcast_scopy);
		while (min_avail_bufsize / mpid_smi_pipe_bsize > 1
			   && blocks_out + 1 < total_blocks) {
			for (dim = 0; dim < ndims; dim++) {
				send_target_addr[dim] = send_databuf_addr[dim] 
					+ (blocks_out % bufblocks_avail)*mpid_smi_pipe_bsize;
				MEMCPY_W (send_target_addr[dim], send_source_addr, mpid_smi_pipe_bsize, to_granks[dim]);

				send_woffset[dim] = (send_woffset[dim] + mpid_smi_pipe_bsize) % send_databuf_size;
				
				WRITE_RMT_PTR(send_woffset_ptr[dim], send_woffset[dim], to_granks[dim]);
			}
			blocks_out++;
			send_source_addr  += mpid_smi_pipe_bsize; 
			min_avail_bufsize -= mpid_smi_pipe_bsize;
		}
		MPID_STAT_RETURN(bcast_scopy);

		/* Check if remote process has read data by reading the remote ptr. */
		min_avail_bufsize = send_databuf_size + 1;
		for (dim = 0; dim < ndims; dim++) {
			READ_RMT_PTR(send_roffset_ptr[dim], send_roffset[dim], to_granks[dim]);
			send_avail_bufsize[dim] = (send_roffset[dim] >= send_woffset[dim]) ? 
				send_roffset[dim] - send_woffset[dim] : 
				send_roffset[dim] + (send_databuf_size - send_woffset[dim]);
			min_avail_bufsize = MPID_MIN(send_avail_bufsize[dim], min_avail_bufsize);
		}
	}

	/* Copy the remaining part of the data: wait until the outbuffer has enough room. */
	remaining = mpid_smi_pipe_shandle[0].bytes_as_contig - (total_blocks-1)*mpid_smi_pipe_bsize;
	for (dim = 0; dim < ndims; dim++) {
		while (send_avail_bufsize[dim] <= 2*mpid_smi_pipe_bsize) {
			READ_RMT_PTR(send_roffset_ptr[dim], send_roffset[dim], to_granks[dim]);
			send_avail_bufsize[dim] = (send_roffset[dim] > send_woffset[dim]) ? 
				send_roffset[dim]- send_woffset[dim] : 
				send_roffset[dim] + (send_databuf_size - send_woffset[dim]);
		}
	}
	
	/* Copy the remainder, but take care for wrap-around! */
	MPID_STAT_CALL(bcast_scopy);
	cpy_len = MPID_MIN( send_databuf_size - (blocks_out % bufblocks_avail * mpid_smi_pipe_bsize),
						remaining );
	for (dim = 0; dim < ndims; dim++) {
		send_target_addr[dim] = send_databuf_addr[dim] + 
			(blocks_out*mpid_smi_pipe_bsize) % send_databuf_size;
		MEMCPY_W (send_target_addr[dim], send_source_addr, cpy_len, to_granks[dim]);
		send_woffset[dim] = (send_woffset[dim] + cpy_len) % send_databuf_size;			
		WRITE_RMT_PTR(send_woffset_ptr[dim], send_woffset[dim], to_granks[dim]);
	}

	send_source_addr += cpy_len;
	remaining -= cpy_len;
	if (remaining > 0) {
		for (dim = 0; dim < ndims; dim++) {
			MEMCPY_W (send_databuf_addr[dim], send_source_addr, remaining, to_granks[dim]);
			send_woffset[dim] = (send_woffset[dim] + cpy_len) % send_databuf_size;			
			WRITE_RMT_PTR(send_woffset_ptr[dim], send_woffset[dim], to_granks[dim]);
		}
	}
	MPID_STAT_RETURN(bcast_scopy);

	/* Release remote SCI memory. */
	for (dim = 0; dim < ndims; dim++) {
		MPID_SMI_Rmt_mem_release (NULL, mpid_smi_pipe_shandle[dim].recv_handle->smi_regid_dest, 
								  MPID_SMI_RSRC_CACHE);	
		COMPLETE_SHANDLE( &mpid_smi_pipe_shandle[dim] );
	}
	MPID_STAT_EXIT(bcast_dma);

    return MPI_SUCCESS;   
}


static int pcast_sendrecv_dma(int from_grank, int to_grank)
{
	smi_memcpy_handle dma_handle;
    unsigned int ptrmem_size, misalign_size;
	int bufblocks_avail, total_blocks, blocks_in, blocks_out;
	int remaining, cpy_len;

    /* variables for receiving data */
	char *recv_base_addr, *recv_databuf_addr, *recv_source_addr, *recv_target_addr;
    volatile long *recv_woffset_ptr, *recv_roffset_ptr;
    volatile long recv_woffset, recv_roffset, final_recv_woffset;
    int recv_databuf_size, recv_avail_bufsize;
	
    /* variables for sending data */
	char *send_base_addr, *send_databuf_addr, *send_target_addr, *send_source_addr;
	volatile long *send_woffset_ptr, *send_roffset_ptr;
    volatile long send_roffset, send_woffset, addr_offset;
    int send_avail_bufsize, send_databuf_size;

	MPID_STAT_ENTRY(bcast_dma);

    ptrmem_size = 2 * MPID_SMI_STREAMSIZE;

	/* receive: initalize addresses & sizes */
    recv_base_addr = (char *)mpid_smi_pipe_rhandle.start;
    misalign_size = (size_t)recv_base_addr % MPID_SMI_STREAMSIZE;
    recv_base_addr += (misalign_size == 0) ? 0 : (MPID_SMI_STREAMSIZE - misalign_size);

    recv_woffset_ptr = (volatile long *)(recv_base_addr + MPID_SMI_STREAMSIZE - sizeof(long *));
    recv_roffset_ptr = (long *)(recv_woffset_ptr + MPID_SMI_STREAMSIZE/sizeof(long));
    recv_roffset = 0;
    recv_woffset = 0;

    recv_databuf_addr = recv_base_addr + ptrmem_size;
    recv_databuf_size = (mpid_smi_pipe_rhandle.recv_handle->len_local - ptrmem_size)
		/ mpid_smi_pipe_bsize * mpid_smi_pipe_bsize;
	recv_avail_bufsize  = 0;
	recv_target_addr = mpid_smi_pipe_rhandle.buf;

	/* send: initalize addresses & sizes */
    send_base_addr = (char *)mpid_smi_pipe_shandle[0].recv_handle->dest_addr;
    misalign_size = (size_t)send_base_addr % MPID_SMI_STREAMSIZE;
    send_base_addr += (misalign_size == 0) ? 0 : (MPID_SMI_STREAMSIZE - misalign_size);

    send_woffset_ptr = (long *)(send_base_addr + MPID_SMI_STREAMSIZE - sizeof(long *));
    send_roffset_ptr = (long *)(send_woffset_ptr + MPID_SMI_STREAMSIZE/sizeof(long));
    send_woffset  = 0;
    send_roffset  = 0;

    send_databuf_addr = send_base_addr + ptrmem_size;
    send_databuf_size = (mpid_smi_pipe_shandle[0].recv_handle->len_local - ptrmem_size)
		/ mpid_smi_pipe_bsize * mpid_smi_pipe_bsize;
	send_avail_bufsize  = send_databuf_size;
	
	bufblocks_avail = send_databuf_size / mpid_smi_pipe_bsize;
	total_blocks    = mpid_smi_pipe_rhandle.len / mpid_smi_pipe_bsize;
	blocks_in = 0; blocks_out = 0;
    while (blocks_out + 1 < total_blocks) {		
		if (recv_avail_bufsize < 2*mpid_smi_pipe_bsize) {
			/* Wait for incoming data. */
			int still_free; 
			still_free = recv_avail_bufsize/mpid_smi_pipe_bsize;

			READ_LOCAL_SHARED_PTR(recv_woffset_ptr, recv_woffset, recv_databuf_size);
			recv_avail_bufsize = recv_woffset >= recv_roffset ? 
				recv_woffset - recv_roffset : recv_woffset + (recv_databuf_size - recv_roffset);
			
			blocks_in += (recv_avail_bufsize / mpid_smi_pipe_bsize) - still_free;
		}
		
		/* Loop over DMA-operations for complete mpid_smi_pipe_bsize blocks. 
		   While reading, always stay one block behind the writer except when
		   reaching the end of the transmission when the danger of ptr-catching 
		   does lo longer exist. */
		MPID_STAT_CALL(bcast_pcopy);
		while ((blocks_out + 1 < blocks_in || blocks_out + 2 == total_blocks) 
			   && send_avail_bufsize / mpid_smi_pipe_bsize > 1) {
			/* First, launch the DMA transfer for a block. */
			dma_handle = NULL;
			addr_offset = (blocks_out % bufblocks_avail) * mpid_smi_pipe_bsize;
			send_target_addr = send_databuf_addr + addr_offset;
			send_source_addr = recv_databuf_addr + addr_offset;

			if (MPID_SMI_cfg.USE_DMA_COLL && MPID_SMI_is_remote[to_grank]) {
				SMIcall(SMI_Imemcpy(send_target_addr, send_source_addr, mpid_smi_pipe_bsize, 
									SMI_MEMCPY_LS_RS, &dma_handle));
			} else {
				MEMCPY_W (send_target_addr, send_source_addr, mpid_smi_pipe_bsize, to_grank);
			}

			send_avail_bufsize -= mpid_smi_pipe_bsize;
			send_woffset = (send_woffset + mpid_smi_pipe_bsize) % send_databuf_size;
			
			/* While DMA is busy, do the PIO incoming copy operation. We can not increment the 
			   recv_roffset_ptr-value before the DMA has completed. */
			MEMCPY_R( recv_target_addr, send_source_addr, mpid_smi_pipe_bsize);
			recv_target_addr += mpid_smi_pipe_bsize;
			
			recv_avail_bufsize -= mpid_smi_pipe_bsize;
			recv_roffset = (recv_roffset + mpid_smi_pipe_bsize) % recv_databuf_size;
			if (MPID_SMI_cfg.USE_DMA_COLL && MPID_SMI_is_remote[to_grank]) {
				/* Wait for DMA to complete; then update remote ptr to indicate new
				   data at the receiver. (SCI remote write) */
				MPID_ASSERT (SMI_Memwait(dma_handle) == SMI_SUCCESS, "DMA transfer in bcast failed.");
			}
			blocks_out++;
			
			/* Update read and write ptrs. */
			WRITE_RMT_PTR(send_woffset_ptr, send_woffset, to_grank);
			if (blocks_out + 1 < total_blocks)
				*recv_roffset_ptr = recv_roffset;
		}
		MPID_STAT_RETURN(bcast_pcopy);
		
		/* Check if remote process has read data by reading the remote ptr. */
		READ_RMT_PTR( send_roffset_ptr, send_roffset, to_grank);
		send_avail_bufsize = (send_roffset > send_woffset) ? 
			send_roffset - send_woffset : send_roffset + (send_databuf_size - send_woffset);
	}

	/* Copy the remaining part of the data: wait until the data has completely arrived in the
	   inbuffer, and the outbuffer has enough room. */
	final_recv_woffset = mpid_smi_pipe_rhandle.len % (bufblocks_avail*mpid_smi_pipe_bsize);
	remaining = mpid_smi_pipe_rhandle.len - (total_blocks-1)*mpid_smi_pipe_bsize;
	do {
		recv_woffset = *recv_woffset_ptr;
	} while (recv_woffset != final_recv_woffset);	
	do {
		READ_RMT_PTR( send_roffset_ptr, send_roffset, to_grank);
		send_avail_bufsize = (send_roffset >= send_woffset) ? 
			send_roffset - send_woffset : send_roffset + (send_databuf_size - send_woffset);
	} while (send_avail_bufsize <= 2*mpid_smi_pipe_bsize);

	/* Copy the remainder, but take care for wrap-around! */
	MPID_STAT_CALL(bcast_pcopy);
	cpy_len = MPID_MIN( recv_databuf_size - (blocks_out % bufblocks_avail * mpid_smi_pipe_bsize),
						remaining );
	addr_offset = (blocks_out % bufblocks_avail) * mpid_smi_pipe_bsize;
	send_target_addr = send_databuf_addr + addr_offset;
	send_source_addr = recv_databuf_addr + addr_offset;
	dma_handle = NULL;
	/* XXX What about intra-node DMA? New SISCI segment mapping type should allow it. */
	if (MPID_SMI_cfg.USE_DMA_COLL && MPID_SMI_is_remote[to_grank]) {
		SMIcall(SMI_Imemcpy(send_target_addr, send_source_addr, cpy_len, SMI_MEMCPY_LS_RS, &dma_handle));
	} else {
		MEMCPY_W (send_target_addr, send_source_addr, cpy_len, to_grank);
	}
	MEMCPY_R( recv_target_addr, send_source_addr, cpy_len);
	recv_target_addr += cpy_len;
	if (MPID_SMI_cfg.USE_DMA_COLL && MPID_SMI_is_remote[to_grank]) {
		SMIcall (SMI_Memwait(dma_handle));
	}
	/* Update write ptr. */
	send_woffset = (send_woffset + cpy_len) % send_databuf_size;			
	WRITE_RMT_PTR(send_woffset_ptr, send_woffset, to_grank);
	
	/* If data remains at the start of the incoming buffers, copy it w/o DMA. */
	remaining -= cpy_len;
	if (remaining > 0) {
		MEMCPY_W (send_databuf_addr, recv_databuf_addr, remaining, to_grank);
		MEMCPY_R (recv_target_addr, recv_databuf_addr, remaining);
		/* Update write ptr. */
		send_woffset = (send_woffset + cpy_len) % send_databuf_size;			
		WRITE_RMT_PTR(send_woffset_ptr, send_woffset, to_grank);
	}
	MPID_STAT_RETURN(bcast_pcopy);

	/* Release remote SCI memory. */
	MPID_SMI_Rmt_mem_release (NULL, mpid_smi_pipe_shandle[0].recv_handle->smi_regid_dest, 
							  MPID_SMI_RSRC_CACHE);
	
    COMPLETE_RHANDLE(&mpid_smi_pipe_rhandle);
	MPID_STAT_EXIT(bcast_dma);

    return MPI_SUCCESS;   
}



/* 
   Tree-based broadcast is faster for for short- and eager messages - we 
   use the original MPICH broadcast (from src/coll/intra_fns.c) in this case. 
*/
static int tree_bcast (void *buffer, int count, struct MPIR_DATATYPE *datatype, 
					   int root, struct MPIR_COMMUNICATOR *comm )
{
  MPI_Status status;
  int        rank, size, src, dst;
  int        relative_rank, mask;
  int        mpi_errno = MPI_SUCCESS;
  static char myname[] = "TREE_BCAST";

  MPIR_Comm_size ( comm, &size );
  MPIR_Comm_rank ( comm, &rank );
  comm = comm->comm_coll;
  
  relative_rank = (rank >= root) ? rank - root : rank - root + size;
  
  mask = 0x1;
  while (mask < size) {
	  if (relative_rank & mask) {
		  src = rank - mask; 
		  if (src < 0) 
			  src += size;
		  mpi_errno = MPICH_Recv(buffer, count, datatype->self, src, MPIR_BCAST_TAG, comm->self, &status);
		  if (mpi_errno) 
			  return mpi_errno;
		  break;
	  }
	  mask <<= 1;
  }
  
  mask >>= 1;
  while (mask > 0) {
	  if (relative_rank + mask < size) {
		  dst = rank + mask;
		  if (dst >= size) 
			  dst -= size;
		  mpi_errno = MPICH_Send (buffer, count, datatype->self, dst, MPIR_BCAST_TAG, comm->self);
		  if (mpi_errno) 
			  return mpi_errno;
	  }
	  mask >>= 1;
  }
  
  return (mpi_errno);
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
