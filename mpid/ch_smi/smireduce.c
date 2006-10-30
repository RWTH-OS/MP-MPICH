/* $Id$ */

/* Different optimized implementations of reduction operations 
   - tree-based with variable fan-out for small vectors (short and eager); and a
   variant of this using 'direct-reduce', which is 'zero-copy' for reduction ops.
   - Rabenseiffner variant for large vectors using MPI_Send/Recv primitives; and
   a (cleaner written) SCI/SMP-optimized version w/ direct-reduce.
   - pipelined implementations (custom protocol) with single-and multi-dimensional 
   pipelines and overlapping DMA- and PIO-operations for large vectors. 
   This pipeline algorithm handles MPI_Reduce, MPI_Allreduce, and MPI_Scan.
   - pseudo-pipelined MPI_Reduce_scatter (pipeline based on distinct parts of the
   vector being sent as individiual messages using MPI_Send/Recv). 
*/


#include "smicoll.h"
#include "mpiops.h"
#include "mpipt2pt.h"
#include "sendrecvstubs.h"

#define RMODE_REDUCE         0
#define RMODE_ALLREDUCE      1
#define RMODE_SCAN           2

#ifdef DMALLOC
#include <dmalloc.h>
#endif

static int MPID_SMI_Rpipe (void *sendbuf, void *recvbuf, int count, struct MPIR_DATATYPE *dtype_ptr,
						   MPI_Op op, int root, struct MPIR_COMMUNICATOR *comm, int rmode);
static int rpipe_recv (int from_grank, char *sendbuf, struct MPIR_OP *op_ptr, 
					   struct MPIR_DATATYPE *dtype_ptr);
static int rpipe_send (int to_grank, char *sendbuf, struct MPIR_OP *op_ptr, 
					   struct MPIR_DATATYPE *dtype_ptr);
static int rpipe_sendrecv (int from_grank, int to_grank, char *sendbuf, struct MPIR_OP *op_ptr,
						   struct MPIR_DATATYPE *dtype_ptr, int rmode, int do_dma);

static int MPID_SMI_VariableFanin_reduce ( void *sendbuf, void *recvbuf, int count, 
								  struct MPIR_DATATYPE *datatype, MPI_Op op, 
								  int root, struct MPIR_COMMUNICATOR *comm );

static int MPID_SMI_Tree_reduce ( void *sendbuf, void *recvbuf, int count, 
								  struct MPIR_DATATYPE *datatype, MPI_Op op, 
								  int root, struct MPIR_COMMUNICATOR *comm );
static int MPID_SMI_Direct_reduce ( void *sendbuf, void *recvbuf, int count, 
									struct MPIR_DATATYPE *datatype, MPI_Op op, 
									int root, struct MPIR_COMMUNICATOR *comm );

static int MPID_SMI_Rab_reduce (void *sendbuf, void *recvbuf, int count, struct MPIR_DATATYPE *datatype,
								MPI_Op op, int root, struct MPIR_COMMUNICATOR *comm, int rmode);
static int use_rabenseifner (int count, struct MPIR_DATATYPE *dtype_ptr, 
							 MPI_Op op, struct MPIR_COMMUNICATOR *comm, int rmode);
static int MPID_SMI_RabSCI_allreduce (void *sendbuf, void *recvbuf, int count, struct MPIR_DATATYPE *datatype,
									  MPI_Op op, struct MPIR_COMMUNICATOR *comm);

static int MPID_SMI_Traeff_scan (void *sendbuf, void *recvbuf, int count, struct MPIR_DATATYPE *datatype, 
								 MPI_Op op, struct MPIR_COMMUNICATOR *comm );

static int MPID_SMI_Allgather_allreduce (void *sendbuf, void *recvbuf, int count,
				struct MPIR_DATATYPE *datatype, MPI_Op op, struct MPIR_COMMUNICATOR *comm);


/* Thresholds for Rabenseifner-Allreduce():  (see also implemetation below): 
   Minimal size to use Rabenseifner, diferentiated by datatype 
   (sh = short, in = int, lg = long, fp = float, db = double, by = byte)
   1st index: routine =    reduce                allreduce          
   2nd index: size    =   2,  3, 2**n, other    2,  3, 2**n, other  */
/* XXX: Determine the optimal threshold values! For now, we choose 16kB-messages to be
   transmitted via the usual tree-orientied topology, as they are delivered eagerly. */
#define RBSFsh (16384/sizeof(short))
#define RBSFin (16384/sizeof(int))
#define RBSFlg (16384/sizeof(long))
#define RBSFfp (16384/sizeof(float))
#define RBSFdb (16384/sizeof(double))
#define RBSFby (16384/sizeof(char))
static int Lsh[2][4]={{ 4*RBSFsh, 2*RBSFsh, RBSFsh, 4*RBSFsh},{ 4*RBSFsh, 2*RBSFsh, RBSFsh, 4*RBSFsh}};
static int Lin[2][4]={{ 4*RBSFin, 2*RBSFin, RBSFin, 4*RBSFin},{ 4*RBSFin, 2*RBSFin, RBSFin, 4*RBSFin}};
static int Llg[2][4]={{ 4*RBSFlg, 2*RBSFlg, RBSFlg, 4*RBSFlg},{ 4*RBSFlg, 2*RBSFlg, RBSFlg, 4*RBSFlg}};
static int Lfp[2][4]={{ 4*RBSFfp, 2*RBSFfp, RBSFfp, 4*RBSFfp},{ 4*RBSFfp, 2*RBSFfp, RBSFfp, 4*RBSFfp}};
static int Ldb[2][4]={{ 4*RBSFdb, 2*RBSFdb, RBSFdb, 4*RBSFdb},{ 4*RBSFdb, 2*RBSFdb, RBSFdb, 4*RBSFdb}};
static int Lby[2][4]={{ 4*RBSFby, 2*RBSFby, RBSFby, 4*RBSFby},{ 4*RBSFby, 2*RBSFby, RBSFby, 4*RBSFby}};




/* Choose an allreduce-variant based on protocol selection an message size. If the allgather-variant
   is chosen, use it. If Rabenseifner or Pipeline have been chosen explicitely, use one of them
   if the seondary criterions are met. In all other cases, sequentially execute MPI_Reduce - MPI_Bcast. */
int MPID_SMI_Allreduce (void *sendbuf, void *recvbuf, int count, struct MPIR_DATATYPE *datatype,
						MPI_Op op, struct MPIR_COMMUNICATOR *comm)
{
	int mpi_errno;
	int done = 0;

	MPID_STAT_ENTRY(allreduce);

	switch (MPID_SMI_cfg.COLL_ALLREDUCE_TYPE) {
	case ALLREDUCE_PIPELINE:
		if (count*datatype->size >= MPID_SMI_cfg.COLL_PIPE_MIN) {
			mpi_errno = MPID_SMI_Rpipe (sendbuf, recvbuf, count, datatype, op, 0, comm, RMODE_REDUCE);
			if (mpi_errno == MPI_SUCCESS)
				mpi_errno = MPID_SMI_Pcast (recvbuf, count, datatype, 0, comm);
			done = 1;
		}
		break;
	case ALLREDUCE_RABENSEIFNER:
		if (use_rabenseifner (count, datatype, op, comm, RMODE_ALLREDUCE)) {
			mpi_errno = MPID_SMI_Rab_reduce (sendbuf, recvbuf, count, datatype, op, 0, comm, 
											 RMODE_ALLREDUCE);
			done = 1;
		}
		break;
	case ALLREDUCE_ALLGATHER:
		mpi_errno = MPID_SMI_Allgather_allreduce (sendbuf, recvbuf, count, datatype, op, comm);
		done = 1;
		break;
	case ALLREDUCE_RABSCI:
		/* XXX This algorithm is only effective for long vectors! */
		if (use_rabenseifner (count, datatype, op, comm, RMODE_ALLREDUCE)) {
			mpi_errno = MPID_SMI_RabSCI_allreduce (sendbuf, recvbuf, count, datatype, op, comm);
			done = 1;
		}
		break;
	}

	if (!done) {
		mpi_errno = comm->collops->Reduce (sendbuf, recvbuf, count, datatype, op, 0, comm);
		if (mpi_errno == MPI_SUCCESS)
			mpi_errno = comm->collops->Bcast (recvbuf, count, datatype, 0, comm);
	}
	
	MPID_STAT_EXIT(allreduce);
    return mpi_errno;
}


/* MPI_Scan can be done the "usual way" (as implemented by Jesper Traeff, see src/intra_scan.c), or
   it can be performed in the context of a pipelined reduction. */
int MPID_SMI_Scan (void *sendbuf, void *recvbuf, int count, struct MPIR_DATATYPE *datatype,
				   MPI_Op op, struct MPIR_COMMUNICATOR *comm)
{
	int mpi_errno;

	MPID_STAT_ENTRY(scan);
	
	mpi_errno = (count*datatype->size >= MPID_SMI_cfg.COLL_REDUCE_LONG
				 && MPID_SMI_cfg.COLL_REDUCE_LONG_TYPE == REDUCE_PIPELINE
				 && count*datatype->size >= MPID_SMI_cfg.COLL_PIPE_DMA_MIN) ? 
		MPID_SMI_Rpipe (sendbuf, recvbuf, count, datatype, op,  comm->np - 1, comm, RMODE_SCAN) :
		MPID_SMI_Traeff_scan (sendbuf, recvbuf, count, datatype, op,  comm);
	
	MPID_STAT_EXIT(scan);

	return mpi_errno;
}


/* This is the actual replacement for MPI_Reduce(). Based on the message size,
   it decides which protocol variant is actually used. */
int MPID_SMI_Reduce (void *sendbuf, void *recvbuf, int count, struct MPIR_DATATYPE *dtype_ptr,
					 MPI_Op op, int root, struct MPIR_COMMUNICATOR *comm)
{
	int mpi_errno = MPI_SUCCESS, rc;
	int prot, size;

	/* First, decide if this is a "short" or a "long" vector. */
	if (MPID_SMI_cfg.COLL_REDUCE_LONG <= count*dtype_ptr->size) {
		/* Fallback if none of the long custom protocols was choosen due to their own 
		   limits and settings.  */
		prot = REDUCE_TREE;
		
		/* Is pipelined-reduce enabled, and is the vector long enough for pipelined DMA? */
		if (MPID_SMI_cfg.COLL_REDUCE_LONG_TYPE == REDUCE_PIPELINE 
			&& count*dtype_ptr->size >= MPID_SMI_cfg.COLL_PIPE_DMA_MIN) {
			prot = REDUCE_PIPELINE;
		} 
		if (MPID_SMI_cfg.COLL_REDUCE_LONG_TYPE == REDUCE_RABENSEIFNER) {
			prot = use_rabenseifner(count, dtype_ptr, op, comm, RMODE_REDUCE);	
		}
	} else {
		/* For short vectors, just decide between the two tree variants only if 
		   the message is *not* a short message! */
		prot = (count*dtype_ptr->size > MPID_SMI_cfg.MAX_SHORT_PAYLOAD) ? 
			MPID_SMI_cfg.COLL_REDUCE_SHORT_TYPE : REDUCE_TREE; 
	}
	
	switch (prot) {
	case REDUCE_TREE: 
		mpi_errno = MPID_SMI_cfg.COLL_DIRECT_REDUCE_OK ?
			MPID_SMI_Direct_reduce(sendbuf, recvbuf, count, dtype_ptr, op, root, comm) :
			MPID_SMI_Tree_reduce(sendbuf, recvbuf, count, dtype_ptr, op, root, comm); 
			break;
	case REDUCE_RABENSEIFNER: 
		mpi_errno = MPID_SMI_Rab_reduce(sendbuf, recvbuf, count, dtype_ptr, op, root, comm, RMODE_REDUCE);
		break;
	case REDUCE_PIPELINE: 
		mpi_errno = MPID_SMI_Rpipe(sendbuf, recvbuf, count, dtype_ptr, op,  root, comm, RMODE_REDUCE);
		break;
	}

	return mpi_errno;
}


/* This is basically the original MPICH reduce, but with a variable FANIN. */
static int MPID_SMI_VariableFanin_reduce ( void *sendbuf, void *recvbuf, int count, 
								  struct MPIR_DATATYPE *datatype, MPI_Op op, 
								  int root, struct MPIR_COMMUNICATOR *comm )
{
	int mpi_errno = MPI_SUCCESS;
	MPI_Request *req; 
	MPI_Status status;
	void **inbuf;
	int size, rank;
	MPI_User_function *uop;
	struct MPIR_OP *op_ptr;
	MPI_Aint lb, ub, m_extent;
	int lroot, relrank;
	int dest, i, lastdist, newdist, firstproc, done, source, source_proc_nbr;
	void *buf;
	static char myname[] = "MPID_SMI_VariableFanin_reduce";

	MPIR_ERROR_DECL;
	mpi_comm_err_ret = 0;
	
#define FANIN MPID_SMI_cfg.COLL_REDUCE_FANIN
#define LRANK(r) ((r + lroot + size) % size )

	/* See the overview in Collection Operations for why this is ok */
	if (count == 0) 
		return MPI_SUCCESS;
	
	ALLOCATE(req, MPI_Request *, FANIN*sizeof(MPI_Request));
	ALLOCATE(inbuf, void **, FANIN*sizeof(void *));
	for (i = 0; i < FANIN; i++) 
		inbuf[i] = NULL;

	/* Get my rank and switch communicators to the hidden collective */
	size = comm->local_group->np;
	rank = comm->local_group->local_rank;
	comm = comm->comm_coll;

	op_ptr = MPIR_GET_OP_PTR(op);
	MPIR_TEST_MPI_OP(op,op_ptr,comm,myname);
	uop  = op_ptr->op;

	/* Get size of data for temporary buffers. */
	MPIR_Type_get_limits( datatype, &lb, &ub );
	m_extent = ub - lb;

	lroot   = op_ptr->commute ? root : 0;
    relrank = (rank - lroot + size) % size;

	/* number of processes whose data has already been combined before the next iteration of the loop, measured per process
	   which was a receiver in the last iteration */
	lastdist = 1;

	/* number of processes whose data will be combined after the next iteration of the loop, measured per process
	   which will be a receiver in the next iteration */
	newdist = FANIN;

	/* Buffer from which the next send operation is done: Originally, this is sendbuf, it then becomes
	   inbuf[0], which is the buffer into which data is reduced at an intermediate node. */
	buf = sendbuf;

	/* If a process has ever sent its data, it is done. */
	done = 0;

	/* loop to get the result into inbuf[0] at lroot */
	while( (!done) && (lastdist < size) ) {
		dest = (relrank / newdist) * newdist;

		if( dest == relrank ) {

			/* allocate intermediate buffers if not yet done */
			if (inbuf[0] == NULL) {

				firstproc = 0;

				/* the root process can use recvbuf as inbuf[0] */
				if( rank == root ) {
					firstproc = 1;
					inbuf[0] = recvbuf;
				}

				for (i = firstproc; i < FANIN; i++) {
					ALIGNEDALLOCATE(inbuf[i], void *, m_extent*count, 16);
					/* XXX: Is this strange ptr op correct? Got it from the orig. algorithm. */
					inbuf[i] = (void *)((char*)inbuf[i] - lb);
				}
			}
			
			/* get own data into inbuf[0] */
			MPICH_Irecv( inbuf[0], count, datatype->self, rank, MPIR_REDUCE_TAG, comm->self, &(req[0]) );
			MPICH_Send( buf, count, datatype->self, rank, MPIR_REDUCE_TAG, comm->self );
			MPICH_Waitall( 1, &(req[0]), &status );

			/* fill the other intermediate buffers */
			source = relrank + lastdist;
			source_proc_nbr = 1;
			while( (source < size) && (source < relrank + newdist) ) {
				MPICH_Irecv( inbuf[source_proc_nbr], count, datatype->self, LRANK(source),
							 MPIR_REDUCE_TAG, comm->self, &(req[source_proc_nbr])); 

				source_proc_nbr++;
				source += lastdist;
			}

			/* reduce in order */
			for( i = 1; i < source_proc_nbr; i++ ) {
				MPICH_Waitall( 1, &req[i], &status );
				CALL_OP_FUNC( inbuf[0], inbuf[i], count, op_ptr, datatype );
			}

			buf = inbuf[0];
		}
		else {
			MPICH_Send( buf, count, datatype->self, LRANK(dest), MPIR_REDUCE_TAG, comm->self );
			break;
		}

		lastdist = newdist;
		newdist *= FANIN;
	}

	if( (!(op_ptr->commute)) && (root != 0) ) {
		if( relrank == 0 )
			mpi_errno = MPICH_Send( inbuf[0], count, datatype->self, root, MPIR_REDUCE_TAG, comm->self );
		if( rank == root )
			mpi_errno = MPICH_Recv( recvbuf, count, datatype->self, LRANK(0), MPIR_REDUCE_TAG, comm->self, &status );
	}

	if( inbuf[0] != NULL ) {
		for( i = firstproc; i < FANIN; i++ )
			ALIGNEDFREE((char *)inbuf[i] + lb);
	}

	FREE( req );
	FREE( inbuf );

#undef LRANK
#undef FANIN

	return( mpi_errno );
}



/* This is the pipelined MPI_Reduce variant. */
static int MPID_SMI_Rpipe (void *sendbuf, void *recvbuf, int count, struct MPIR_DATATYPE *dtype_ptr,
						   MPI_Op op, int root, struct MPIR_COMMUNICATOR *comm, int rmode)
{
	MPID_SMI_comm_info_t *ci = (MPID_SMI_comm_info_t *)comm->adiCollCtx;
    int        mylrank, size;
    int        relative_rank, to_grank, from_grank;
    int        mpi_errno = MPI_SUCCESS;
    int        contig_size, dtypesize;
    ulong      sgmt_offset;
	struct     MPIR_OP *op_ptr;
    static char myname[] = "MPID_SMI_RPIPE";
	int        do_dma;

    /* See the overview in Collection Operations for why this is ok */
    if (count == 0) 
		return MPI_SUCCESS;

	size = comm->local_group->np;
    /* If there is only one process */
    if (size == 1) 
		return MPI_SUCCESS;
	
#ifndef MPIR_NO_ERROR_CHECKING
    if (root >= size) 
		mpi_errno = MPIR_Err_setmsg( MPI_ERR_ROOT, MPIR_ERR_ROOT_TOOBIG, 
									 myname, (char *)0, (char *)0, root, size );
    if (mpi_errno)
		return MPIR_ERROR(comm, mpi_errno, myname );
#endif
	op_ptr = MPIR_GET_OP_PTR(op);
	MPIR_TEST_MPI_OP(op,op_ptr,comm,myname);

    dtypesize = MPIR_GET_DTYPE_SIZE(datatype,dtype_ptr);
    contig_size = count * dtypesize;
	
	/* Determine optimal blocksize when dynamicblocksize is set. */
	if ( MPID_SMI_cfg.COLL_PIPE_DYNAMIC ) {
		/* Really big packets (greater BLOCK_INC_LIMIT) need a blocksize of at least PIPE_BIG_BLOCK for
		   good DMA performance. This is more important than many blocks. */
		if ( contig_size >= PIPE_BLOCK_INC_LIMIT && MPID_SMI_cfg.COLL_PIPE_BLOCKSIZE < PIPE_BIG_BLOCK ) {
			mpid_smi_pipe_nbrblocks = MPID_SMI_RNDVSIZE / 4 / PIPE_BIG_BLOCK;
			mpid_smi_pipe_bsize = PIPE_BIG_BLOCK;
			if ( mpid_smi_pipe_nbrblocks < 4 ) {
				/* Number of blocks must be at least 4.
				   If it isn't possible to get enough blocks (with size PIPE_BIG_BLOCK) forget it,
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

	/* set correct root for MPID_Scan - start with lrank=0, end with lrank=size-1 */
	if ( rmode == RMODE_SCAN )
		root = size - 1;

    /* Get my rank and switch communicators to the hidden collective */
    mylrank = comm->local_group->local_rank;
    comm = comm->comm_coll;

    relative_rank = (mylrank - root + size) % size;
	from_grank = comm->lrank_to_grank[(relative_rank - 1 + root + size) % size];
	to_grank   = comm->lrank_to_grank[(relative_rank + 1 + root + size) % size];

    /* initialize rhandle */
    MPID_Recv_init( &mpid_smi_pipe_rhandle );
	mpid_smi_pipe_rhandle.recv_handle = &mpid_smi_pipe_rhandle_recv_handle;
	memset(mpid_smi_pipe_rhandle.recv_handle, 0, sizeof(mpid_smi_pipe_rhandle_recv_handle));
	mpid_smi_pipe_rhandle.buf = recvbuf;

	/* relative rank 0 (root) has to save the result when finished.
	   So we start with root+1 and end with root */
    /* Send PIPE_READY to rank - 1 (except first proc = right neighbour of root) */
    if (relative_rank != 1) 
		mpid_smi_send_pipeready_pkt (from_grank, comm, contig_size, &sgmt_offset);
	
	/* Wait for PIPE_READY packet. (except root = last process) */
    if (relative_rank != 0) {
		while (mpid_smi_pipe_ready_arrived == 0)
			MPID_DeviceCheck( MPID_NOTBLOCKING );
		mpid_smi_pipe_ready_arrived = 0;
    }
	/* The shandle was initialized when the PIPE_READY packet arrived, we only need to 
	   set the recv buffer here. */
	mpid_smi_pipe_shandle[0].start = recvbuf;

 	do_dma = mpid_smi_pipe_do_dma(contig_size, comm, ci);

    /* Now we can start receive and/or send. Root + 1 only send data,
	   root only receives, all other processes send and receive. */
	if (relative_rank == 1) {
		mpi_errno = rpipe_send (to_grank, sendbuf, op_ptr, dtype_ptr);
  		if (rmode == RMODE_SCAN)
			MEMCPY_R(recvbuf, sendbuf, contig_size);
	} else 
		if (relative_rank == 0) {
			mpi_errno = rpipe_recv (from_grank, sendbuf, op_ptr, dtype_ptr);
		} else {
			mpi_errno = rpipe_sendrecv (from_grank, to_grank, sendbuf, op_ptr, 
										dtype_ptr, rmode, do_dma);
		}

	if (relative_rank != 1)
		mpid_smi_pipe_free_recvbuf (sgmt_offset, from_grank);

	return mpi_errno;
}



/*
  DMA-based pipelined reduction operations. Very similar to pcast functions.
  
  This is a first (simple) version of the protocol which has strict buffer size constraints
  and does not perform with the maximum degree of concurrency. Also, it has quite al long 
  pipeline startup and will thus perform best for really long messages. 
*/

/* Recv data and place it into the user recv buffer after applying reduction operation
   from user send buffer. */
static int rpipe_recv (int from_grank, char *sendbuf, struct MPIR_OP *op_ptr, 
					   struct MPIR_DATATYPE *dtype_ptr)
{
	char *recv_base_addr, *recv_databuf_addr, *recv_source_addr, *recv_target_addr;
    long * volatile recv_woffset_ptr, * volatile recv_roffset_ptr;
    volatile long recv_woffset, recv_roffset;
    int recv_databuf_size, recv_avail_bufsize;
	int bufblocks_avail, total_blocks, blocks_in, blocks_out;
	int remaining, cpy_len, elmt_count;
    uint ptrmem_size, misalign_size;
	
	MPID_STAT_ENTRY(rdcpipe_recv);

	/* receive: initalize addresses & sizes */
    recv_base_addr = (char *)mpid_smi_pipe_rhandle.start;
    misalign_size = (size_t)recv_base_addr % MPID_SMI_STREAMSIZE;
    recv_base_addr += (misalign_size == 0) ? 0 : (MPID_SMI_STREAMSIZE - misalign_size);

    recv_woffset_ptr = (long *)(recv_base_addr + MPID_SMI_STREAMSIZE - sizeof(long *));
    recv_roffset_ptr = recv_woffset_ptr + MPID_SMI_STREAMSIZE/sizeof(long);
	recv_woffset = 0;
    recv_roffset = 0;

    ptrmem_size = 2 * MPID_SMI_STREAMSIZE;
    recv_databuf_addr = recv_base_addr + ptrmem_size;
    recv_databuf_size = (mpid_smi_pipe_rhandle.recv_handle->len_local - ptrmem_size)
		/ mpid_smi_pipe_bsize * mpid_smi_pipe_bsize;
	recv_avail_bufsize  = 0;
	recv_target_addr    = (char *)mpid_smi_pipe_rhandle.buf;

	/* How many elements are contained in one pipeline block ? */
	elmt_count = mpid_smi_pipe_bsize / dtype_ptr->extent;
	MPID_ASSERT (mpid_smi_pipe_bsize % dtype_ptr->extent == 0, "Unaligned blocksize in reduce pipeline.");
	
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
		while (blocks_out + 1 < blocks_in|| blocks_out + 2 == total_blocks) {
			MPID_STAT_CALL(rdcpipe_rcopy);
		
			recv_source_addr = recv_databuf_addr + (blocks_out % bufblocks_avail)*mpid_smi_pipe_bsize;
            /* Data has arrived - apply reduction operation and then copy to
			   user receive buffer. */
 			CALL_OP_FUNC(recv_source_addr, sendbuf, elmt_count, op_ptr, dtype_ptr);
			MEMCPY_R( recv_target_addr, recv_source_addr, mpid_smi_pipe_bsize);
			recv_target_addr   += mpid_smi_pipe_bsize;
			sendbuf            += mpid_smi_pipe_bsize;
			recv_avail_bufsize -= mpid_smi_pipe_bsize;
			recv_roffset = (recv_roffset + mpid_smi_pipe_bsize) % recv_databuf_size;
			blocks_out++;
			
			/* Update read ptr. */
			if (blocks_out + 1 < total_blocks)
				*recv_roffset_ptr = recv_roffset;

			MPID_STAT_RETURN(rdcpipe_rcopy);
		}
	}

	/* Copy the remaining part of the data: wait until all data has arrived. */
	remaining = mpid_smi_pipe_rhandle.len - (total_blocks-1)*mpid_smi_pipe_bsize;
	do {
		READ_LOCAL_SHARED_PTR(recv_woffset_ptr, recv_woffset, recv_databuf_size);
		recv_avail_bufsize = (recv_woffset >= recv_roffset) ? 
			recv_woffset - recv_roffset : recv_woffset + (recv_databuf_size - recv_roffset);
	} while (recv_avail_bufsize < remaining);
	
	/* Copy the remainder, but take care for wrap-around! */
	MPID_STAT_CALL(rdcpipe_rcopy);
	recv_source_addr = recv_databuf_addr + (blocks_out*mpid_smi_pipe_bsize) % recv_databuf_size;
	cpy_len = MPID_MIN( recv_databuf_size - (blocks_out % bufblocks_avail * mpid_smi_pipe_bsize),
						remaining );
	/* This will always be a no-remainder division becaues we only copy multiples of 
	   the rpipe-blocksize (see assert above) *or* any remaining number of elements. */
 	elmt_count = cpy_len / dtype_ptr->extent;
 	CALL_OP_FUNC(recv_source_addr, sendbuf, elmt_count, op_ptr, dtype_ptr);
	MEMCPY_R (recv_target_addr, recv_source_addr, cpy_len);

	remaining -= cpy_len;
	recv_target_addr += cpy_len;	
	if (remaining > 0) {
 		elmt_count = remaining / dtype_ptr->extent;
 		CALL_OP_FUNC(recv_databuf_addr, sendbuf, elmt_count, op_ptr, dtype_ptr);
		MEMCPY_R (recv_target_addr, recv_databuf_addr, remaining);
	}	
	MPID_STAT_RETURN(rdcpipe_rcopy);

    COMPLETE_RHANDLE( &mpid_smi_pipe_rhandle );
	MPID_STAT_EXIT(rdcpipe_recv);

    return MPI_SUCCESS;   
}


static int rpipe_send (int to_grank, char *sendbuf, struct MPIR_OP *op_ptr, 
					   struct MPIR_DATATYPE *dtype_ptr)
{
    unsigned int ptrmem_size, misalign_size;

    /* variables for sending data */
	char *send_base_addr, *send_databuf_addr;
	char *send_target_addr, *send_source_addr;
    volatile long send_roffset, send_woffset;
	long * volatile send_woffset_ptr, * volatile send_roffset_ptr;
    int send_avail_bufsize, min_avail_bufsize, send_databuf_size;
	int bufblocks_avail, total_blocks;
	int blocks_in, blocks_out;
	int remaining, cpy_len;

	MPID_STAT_ENTRY(rdcpipe_send);

	/* send: initalize addresses & sizes */
    ptrmem_size = 2 * MPID_SMI_STREAMSIZE;
    send_databuf_size = (mpid_smi_pipe_shandle[0].recv_handle->len_local - ptrmem_size)
		/ mpid_smi_pipe_bsize * mpid_smi_pipe_bsize;
	min_avail_bufsize = send_databuf_size;
	send_source_addr  = (char*)sendbuf;
	bufblocks_avail   = send_databuf_size / mpid_smi_pipe_bsize;
	total_blocks      = mpid_smi_pipe_shandle[0].bytes_as_contig / mpid_smi_pipe_bsize;
	blocks_out = 0;

	send_base_addr = (char *)mpid_smi_pipe_shandle[0].recv_handle->dest_addr;
	misalign_size = (size_t)send_base_addr % MPID_SMI_STREAMSIZE;
	send_base_addr += (misalign_size == 0) ? 0 : (MPID_SMI_STREAMSIZE - misalign_size);
	send_woffset_ptr = (long *)(send_base_addr + MPID_SMI_STREAMSIZE - sizeof(long *));
	send_roffset_ptr = send_woffset_ptr + MPID_SMI_STREAMSIZE/sizeof(long);
	send_woffset = 0;
	send_roffset = 0;
	send_databuf_addr  = send_base_addr + ptrmem_size;
	send_avail_bufsize = send_databuf_size;

    while (blocks_out + 1 < total_blocks) {
		/* Loop over copy-operations for complete mpid_smi_pipe_bsize blocks. */
		while (min_avail_bufsize / mpid_smi_pipe_bsize > 1
			   && blocks_out + 1 < total_blocks) {
			MPID_STAT_CALL(rdcpipe_scopy);

			send_target_addr = send_databuf_addr + (blocks_out % bufblocks_avail)*mpid_smi_pipe_bsize;
			MEMCPY_W (send_target_addr, send_source_addr, mpid_smi_pipe_bsize, to_grank);
			send_woffset = (send_woffset + mpid_smi_pipe_bsize) % send_databuf_size;
			
			WRITE_RMT_PTR(send_woffset_ptr, send_woffset, to_grank);

			blocks_out++;
			send_source_addr  += mpid_smi_pipe_bsize; 
			min_avail_bufsize -= mpid_smi_pipe_bsize;

			MPID_STAT_RETURN(rdcpipe_scopy);
		}

		/* Check if remote process has read data by reading the remote ptr. */
		min_avail_bufsize = send_databuf_size + 1;
		READ_RMT_PTR(send_roffset_ptr, send_roffset, to_grank);
		send_avail_bufsize = (send_roffset >= send_woffset) ? 
			send_roffset - send_woffset : send_roffset + (send_databuf_size - send_woffset);
		min_avail_bufsize = MPID_MIN(send_avail_bufsize, min_avail_bufsize);
	}

	/* Copy the remaining part of the data: wait until the outbuffer has enough room. */
	remaining = mpid_smi_pipe_shandle[0].bytes_as_contig - (total_blocks-1)*mpid_smi_pipe_bsize;
	while (send_avail_bufsize <= 2*mpid_smi_pipe_bsize) {
		READ_RMT_PTR(send_roffset_ptr, send_roffset, to_grank);
		send_avail_bufsize = (send_roffset > send_woffset) ? 
			send_roffset- send_woffset : send_roffset + (send_databuf_size - send_woffset);
	}
	
	/* Copy the remainder, but take care for wrap-around! */
	MPID_STAT_CALL(rdcpipe_scopy);
	cpy_len = MPID_MIN( send_databuf_size - (blocks_out % bufblocks_avail * mpid_smi_pipe_bsize),
						remaining );
	send_target_addr = send_databuf_addr + (blocks_out*mpid_smi_pipe_bsize) % send_databuf_size;
	MEMCPY_W (send_target_addr, send_source_addr, cpy_len, to_grank);
	send_woffset = (send_woffset + cpy_len) % send_databuf_size;			
	WRITE_RMT_PTR(send_woffset_ptr, send_woffset, to_grank);

	send_source_addr += cpy_len;
	remaining -= cpy_len;
	if (remaining > 0) {
		MEMCPY_W (send_databuf_addr, send_source_addr, remaining, to_grank);
		send_woffset = (send_woffset + cpy_len) % send_databuf_size;			
		WRITE_RMT_PTR(send_woffset_ptr, send_woffset, to_grank);
	}
	MPID_STAT_RETURN(rdcpipe_scopy);

	/* Only release remote SCI memory. */
	if (!MPID_SMI_use_localseg[to_grank]) 
		MPID_SMI_Rmt_mem_release (NULL, mpid_smi_pipe_shandle[0].recv_handle->smi_regid_dest, 
								  MPID_SMI_RSRC_CACHE);	
	COMPLETE_SHANDLE( &mpid_smi_pipe_shandle[0] );
	MPID_STAT_EXIT(rdcpipe_send);

    return MPI_SUCCESS;   
}


static int rpipe_sendrecv (int from_grank, int to_grank, char *sendbuf, struct MPIR_OP *op_ptr, 
						   struct MPIR_DATATYPE *dtype_ptr, int rmode, int do_dma)
{
	smi_memcpy_handle dma_handle;
    unsigned int ptrmem_size, misalign_size;
	int bufblocks_avail, total_blocks, blocks_in, blocks_out;
	int remain_len, cpy_len, elmt_count;

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

	/* reducing data */
	int blocks_rdcd;
	char *rdc_addr;

	MPID_STAT_ENTRY(rdcpipe_sendrecv);

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
	rdc_addr = recv_databuf_addr;

	/* How many elements are contained in one pipeline block ? */
	elmt_count = mpid_smi_pipe_bsize / dtype_ptr->extent;
	MPID_ASSERT (mpid_smi_pipe_bsize % dtype_ptr->extent == 0, "Unaligned blocksize in reduce pipeline.");
	
	bufblocks_avail = send_databuf_size / mpid_smi_pipe_bsize;
	total_blocks    = mpid_smi_pipe_rhandle.len / mpid_smi_pipe_bsize;
	blocks_in = 0; blocks_rdcd = 0; blocks_out = 0;
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

		/* Initiate the pipeline by reducing the first block that has come in - 
		   reducing a blocks needs to be done before sending it to the next process! */
		if (blocks_in > 0 && blocks_rdcd == 0) {
 			CALL_OP_FUNC(rdc_addr, sendbuf, elmt_count, op_ptr, dtype_ptr);
			sendbuf += mpid_smi_pipe_bsize;
			blocks_rdcd++;
		}
		
		/* Loop over DMA-operations for complete mpid_smi_pipe_bsize blocks. 
		   While reading, always stay one block behind the writer except when
		   reaching the end of the transmission when the danger of ptr-catching 
		   does lo longer exist. */
		while (blocks_in > 1 && (blocks_out + 1 < blocks_in || blocks_out + 2 == total_blocks)
			   && send_avail_bufsize / mpid_smi_pipe_bsize > 1) {
			MPID_STAT_CALL(rdcpipe_pcopy);

			addr_offset = (blocks_out % bufblocks_avail) * mpid_smi_pipe_bsize;
			send_target_addr = send_databuf_addr + addr_offset;
			send_source_addr = recv_databuf_addr + addr_offset;
			rdc_addr = recv_databuf_addr + (blocks_rdcd % bufblocks_avail)*mpid_smi_pipe_bsize;

			/* First, launch the DMA transfer for the block that has already been reduced. */
			if (do_dma) {
				dma_handle = NULL;
				SMIcall(SMI_Imemcpy(send_target_addr, send_source_addr, mpid_smi_pipe_bsize, 
									SMI_MEMCPY_LS_RS, &dma_handle));
			} else {
				MEMCPY_W (send_target_addr, send_source_addr, mpid_smi_pipe_bsize, to_grank);
				blocks_out++;
			}

			/* While DMA is busy, do the PIO incoming copy operation 
			   (copy to user receive buffer for MPI_Scan). and apply reduction 
			   operation on the next block which already has arrived. */
			if (rmode == RMODE_SCAN) {
				MEMCPY_R(recv_target_addr, send_source_addr, mpid_smi_pipe_bsize);
				recv_target_addr += mpid_smi_pipe_bsize;
			}

 			CALL_OP_FUNC(rdc_addr, sendbuf, elmt_count, op_ptr, dtype_ptr);
			sendbuf += mpid_smi_pipe_bsize;
			blocks_rdcd++;
			
			if (do_dma) {
				/* Wait for DMA to complete; then update remote ptr to indicate new
				   data at the receiver. (SCI remote write) */
				SMIcall(SMI_Memwait(dma_handle));
				blocks_out++;
			}
			
			/* Update read and write ptrs. and adjust buffer sizes. */
			send_avail_bufsize -= mpid_smi_pipe_bsize;
			recv_avail_bufsize -= mpid_smi_pipe_bsize;
			send_woffset = (send_woffset + mpid_smi_pipe_bsize) % send_databuf_size;
			recv_roffset = (recv_roffset + mpid_smi_pipe_bsize) % recv_databuf_size;
			WRITE_RMT_PTR(send_woffset_ptr, send_woffset, to_grank);
			if (blocks_out + 1 < total_blocks)
				*recv_roffset_ptr = recv_roffset;

			MPID_STAT_RETURN(rdcpipe_pcopy);
		}
		
		/* Check if remote process has read data by reading the remote ptr. */
		READ_RMT_PTR( send_roffset_ptr, send_roffset, to_grank);
		send_avail_bufsize = (send_roffset > send_woffset) ? 
			send_roffset - send_woffset : send_roffset + (send_databuf_size - send_woffset);
	}

	/* When we get here, we have 'blocks_out = total_blocks - 1', and 
	   'blocks_reduced = total_blocks'.
	   This means, we need to transmit one last block plust a possible remainder, and 
	   we need to reduce the remainder. To do this, we wait until 
	   - the data has completely arrived in the inbuffer, and
	   - the outbuffer has enough room
	   This will always happen because the pipeline buffers are at least sized 
	   3*mpid_smi_pipe_bsize. It basiscally means that the pipeline block may be 
	   somewhat larger, (but still < 2*mpid_smi_pipe_bsize. */
	final_recv_woffset = mpid_smi_pipe_rhandle.len % (bufblocks_avail*mpid_smi_pipe_bsize);
	do {
		READ_LOCAL_SHARED_PTR(recv_woffset_ptr, recv_woffset, recv_databuf_size);
	} while (recv_woffset != final_recv_woffset);	
	do {
		READ_RMT_PTR( send_roffset_ptr, send_roffset, to_grank);
		send_avail_bufsize = (send_roffset >= send_woffset) ? 
			send_roffset - send_woffset : send_roffset + (send_databuf_size - send_woffset);
	} while (send_avail_bufsize <= 2*mpid_smi_pipe_bsize);

	/* Now, we can copy the last full block, and do the reduction of the remainder
	   (if there is any). */
	addr_offset = (blocks_out % bufblocks_avail) * mpid_smi_pipe_bsize;
	send_target_addr = send_databuf_addr + addr_offset;
	send_source_addr = recv_databuf_addr + addr_offset;
	MPID_STAT_CALL(rdcpipe_pcopy);
	if (do_dma) {
		dma_handle = NULL;
		SMIcall(SMI_Imemcpy(send_target_addr, send_source_addr, mpid_smi_pipe_bsize, 
							SMI_MEMCPY_LS_RS, &dma_handle));
	} else {
		MEMCPY_W (send_target_addr, send_source_addr, mpid_smi_pipe_bsize, to_grank);
	}
	if (rmode == RMODE_SCAN) {
		MEMCPY_R(recv_target_addr, send_source_addr, mpid_smi_pipe_bsize);
		recv_target_addr += mpid_smi_pipe_bsize;
	}
	blocks_out++;

	remain_len = mpid_smi_pipe_rhandle.len % (total_blocks*mpid_smi_pipe_bsize);
	addr_offset = (blocks_out % bufblocks_avail) * mpid_smi_pipe_bsize;
	send_target_addr = send_databuf_addr + addr_offset;
	send_source_addr = recv_databuf_addr + addr_offset;
	rdc_addr = send_source_addr;
	elmt_count = remain_len / dtype_ptr->extent;
	CALL_OP_FUNC(rdc_addr, sendbuf, elmt_count, op_ptr, dtype_ptr);	

	/* Finally, copy the remainder - we use PIO for this, because we can't do any overlapping and
	   DMA is usually slower for this blocksize. Then, update the write ptrs. after DMA is done. */
	MEMCPY_W (send_target_addr, send_source_addr, remain_len, to_grank);
	if (do_dma) {
		SMIcall (SMI_Memwait(dma_handle));
	}
	send_woffset = (send_woffset + mpid_smi_pipe_bsize + remain_len) % send_databuf_size;			
	WRITE_RMT_PTR(send_woffset_ptr, send_woffset, to_grank);
	if (rmode == RMODE_SCAN) {
		MEMCPY_R(recv_target_addr, send_source_addr, remain_len);
		recv_target_addr += remain_len;
	}
	MPID_STAT_RETURN(rdcpipe_pcopy);

	if (!MPID_SMI_use_localseg[to_grank]) 
		/* Only release remote SCI memory. */
		MPID_SMI_Rmt_mem_release (NULL, mpid_smi_pipe_shandle[0].recv_handle->smi_regid_dest, 
								  MPID_SMI_RSRC_CACHE);

    COMPLETE_RHANDLE(&mpid_smi_pipe_rhandle);
	MPID_STAT_EXIT(rdcpipe_sendrecv);

    return MPI_SUCCESS;   
}


/*
 * Optimized Reduce: Rabenseifner for big messages, special tree for small ones.
 * Always use optimized assembler reduce-operations (MMX, SSE, ...)
 */

/* This is the original MPICH-Reduce for the cases in which the
   Rabenseifner-Reduce is less efficient. See src/coll/intra_fns.c for
   fully commented version. 
   
   Optimized for SCI:
   - a fan-in greater than 2 is appropiate for small messages. 
   - only send to upper processes 
*/
static int MPID_SMI_Tree_reduce ( void *sendbuf, void *recvbuf, int count, 
								  struct MPIR_DATATYPE *datatype, MPI_Op op, 
								  int root, struct MPIR_COMMUNICATOR *comm )
{
	MPI_Status status;
	MPI_Request *req; 
	MPI_Aint   lb, ub, m_extent;  /* Extent in memory */
	struct MPIR_OP *op_ptr;
	int        size, rank;
	int		 upper, lower, upper_size, nbr_recvs, dstnc, i;
	int        mask, mask_shift, rdc_rank, dest, source, lroot;
	int        mpi_errno = MPI_SUCCESS;
	void       *buffer, **inbuf;
	static char myname[] = "MPID_SMI_Tree_reduce";

	MPIR_ERROR_DECL;
	mpi_comm_err_ret = 0;

#define FANIN MPID_SMI_cfg.COLL_REDUCE_FANIN
#define LRANK(r) (((r) + size - (size - 1 - lroot)) % size)

	/* See the overview in Collection Operations for why this is ok */
	if (count == 0) 
		return MPI_SUCCESS;

	ALLOCATE(req, MPI_Request *, FANIN*sizeof(MPI_Request));

	ALLOCATE(inbuf, void **, FANIN*sizeof(void *));

	/* If the operation is predefined, we could check that the datatype's
	   type signature is compatible with the operation. */

	/* Get my rank and switch communicators to the hidden collective */
	size = comm->local_group->np;
	rank = comm->local_group->local_rank;
	comm = comm->comm_coll;

	op_ptr = MPIR_GET_OP_PTR(op);
	MPIR_TEST_MPI_OP(op,op_ptr,comm,myname);

	/* Get size of data for temporary buffers. */
	MPIR_Type_get_limits( datatype, &lb, &ub );
	m_extent = ub - lb;

	MPIR_Op_errno = MPI_SUCCESS;

	/* Rank transformation: for commutative operations, the root does not
	   really matter. For non-commutative, we always want the root to be proc size-1
	   (sending to upper procs) to get identical results for different roots.
	   The result will be transmitted to the real root afterwards.*/
	lroot   = op_ptr->commute ? root : (size - 1);


	/* Here the algorithm for arbitrary 2^n fan-in, and with sending only
	   to upper ranks (this is more efficient for SCI-connected system. 
	   Therefore, lroot will be assigned the reduce_rank 'size - 1', all other
	   ranks transformed accordingly. */
	/* NOTE:
	   The ranks in the process group of the communicator which we are using here
	   may of course be completely different than the ranks in MPI_COMM_WORLD, so
	   it may happen that we don't send to "upper" ranks (regarding MPI_COMM_WORLD)
	   at all.
	*/
	rdc_rank = (rank + (size - 1 - lroot)) % size;

	mask = FANIN;
	mask_shift = MPID_SMI_ld(mask);
	for (upper_size = 1; upper_size < size; upper_size <<= 1);

	if (rank != root) 
		recvbuf = NULL;
	for (i = 0; i < FANIN; i++) 
		inbuf[i] = NULL;

	dstnc = 1;
	do {
		dest = rdc_rank | (mask - 1);
		if (dest >= size)
			dest = size - 1;

		if (dest == rdc_rank) {
			/* This process is a receiver in this round. First, allocate
			   receive buffers if required. Then, post all recvs and process
			   the incoming messages in order. */
			if (inbuf[0] == NULL) {
				for (i = 0; i < FANIN; i++) {
					ALIGNEDALLOCATE(inbuf[i], void *, m_extent*count, 16);
					/* XXX: Is this strange ptr op correct? Got it from the orig. algorithm. */
					inbuf[i] = (void *)((char*)inbuf[i] - lb);
				}
			}

			if (dstnc == 1) {
				/* If I'm not the root, then my recvbuf may not be valid, therefore
				   I have to allocate a temporary one */
				if (rank != root) {

					ALIGNEDALLOCATE(recvbuf, void *, m_extent * count, 16);	/* must be aligned for SSE-optimized operations */
					recvbuf = (void *)((char*)recvbuf - lb);
				}

				/* If this process is to receive in the first round, it needs to
				   initialize its recv buffer (the send buffer must not be modified) */
				MPICH_Irecv (recvbuf, count, datatype->self, rank, MPIR_REDUCE_TAG, comm->self, &req[0]);
				MPICH_Send (sendbuf, count, datatype->self, rank, MPIR_REDUCE_TAG, comm->self);
				MPICH_Waitall (1, &req[0], &status);
			}

			/* Upper and lower ranks which are a 'regular' multiple of the current mask. */
			upper = ((rdc_rank + 1) % mask == 0) ? rdc_rank : 
				(rdc_rank / mask + 1) * mask - 1;
			lower = upper - mask;

			/* Which processes will send data to me? */
			nbr_recvs = FANIN - ((upper - rdc_rank)/dstnc) - 1;
			for (i = 0; i < nbr_recvs; i++) {
				source = lower + (i+1)*dstnc;
				MPICH_Irecv (inbuf[i], count, datatype->self, LRANK(source), MPIR_REDUCE_TAG, comm->self, &req[i]);
			}

			/* Apply the reduce operation: Because we received data "from the left" (i.e. from procs with lower ranks),
			   we don't need to swap the first two parameters for the reduce operation, which is done when sending to
			   lower ranks. The second function parameter (i.e. the second operation argument) is the buffer
			   with our data and that's where the result is stored.
			   Of course, we have to work through the received data from upper to lower ranks. */
			for (i = nbr_recvs - 1; i >= 0; i--) {
				MPICH_Waitall(1, &req[i], &status);
 				CALL_OP_FUNC_NOSWAP(inbuf[i], recvbuf, count, op_ptr, datatype);
 			}
		} else {
			/* This process sends data in this round; then its done. Most processes (at least
			   50%) do only send once. Therefore, we avoid allocating & copying a separate
			   send buffer for those. Processes, who have received in one round, use the
			   according recv buffer which contains the data to send. */
			buffer = (dstnc == 1) ? sendbuf : recvbuf;
			MPICH_Send (buffer, count, datatype->self, LRANK(dest), MPIR_REDUCE_TAG, comm->self);
			break;
		}

		mask <<= mask_shift;
		dstnc <<= mask_shift;
	} while ((mask >> mask_shift) <= upper_size);

	for (i = 0; i < FANIN; i++) {
		if (inbuf[i] != NULL)
			ALIGNEDFREE((char *)inbuf[i] + lb);
	}

	if (recvbuf != NULL && rank != root)
		ALIGNEDFREE( (char *)recvbuf + lb );

	/* Send the result to the real root if the operation has been non-commtative. */
	if (!op_ptr->commute && root != size - 1) {
		if (rank == size - 1)
			mpi_errno = MPICH_Send( recvbuf, count, datatype->self, root, MPIR_REDUCE_TAG, comm->self );
		if (rank == root) 
			mpi_errno = MPICH_Recv ( recvbuf, count, datatype->self, size - 1, MPIR_REDUCE_TAG, comm->self, &status);
	}

	if (mpi_errno == MPI_SUCCESS && MPIR_Op_errno) {
		mpi_errno = MPIR_Op_errno;
	}
	FREE(req);
	FREE(inbuf);

#undef LRANK
#undef FANIN
	return (mpi_errno);
}

/* This variant of Tree_reduce does avoid the copying of the incoming data from
   the incoming buffer into the (temporary) recv buffer by replacing the
   related default memcpy() function with the reduction operation, and supplying
   the other reduction vector in the receive buffer. This is especially helpful
   for eager messages. 

   This technique needs to use internal interfaces to post the recv requests - 
   nevertheless, it must not be considered as a 'hack'. For optimal performance,
   the order in which the incoming messages for a fan-in > 2 will be processed can
   be relaxed to operate on a first-come-first-served basis. This may, however, lead 
   to different reduction results for mulitple reduction with the same operands and
   processes. This dependends on the operation choosen and the  characteristics of the
   operands and is caused by the usual arithmetic inaccuracies of the FPU. 
*/
static int MPID_SMI_Direct_reduce ( void *sendbuf, void *recvbuf, int count, 
									struct MPIR_DATATYPE *datatype, MPI_Op op, 
									int root, struct MPIR_COMMUNICATOR *comm )
{
	MPI_Status status;
	MPIR_RHANDLE *rhandles[MPID_SMI_REDUCE_FANIN_MAX];
	MPI_User_function *uop;
	MPI_Aint   lb, ub, m_extent;  /* Extent in memory */
	struct MPIR_OP *op_ptr;
	int        size, rank;
	int		 upper, lower, upper_size, nbr_recvs, dstnc, i;
	int        mask, mask_shift, rdc_rank, dest, source, lroot;
	int        mpi_errno = MPI_SUCCESS;
	void       *buffer, *rdc_buf = NULL;
	static char myname[] = "MPID_SMI_Direct_reduce";

	MPIR_ERROR_DECL;
	mpi_comm_err_ret = 0;

#define FANIN MPID_SMI_cfg.COLL_REDUCE_FANIN
#define LRANK(r) (((r) + size - (size - 1 - lroot)) % size)

	/* See the overview in Collection Operations for why this is ok */
	if (count == 0) 
		return MPI_SUCCESS;

	/* If the operation is predefined, we could check that the datatype's
	   type signature is compatible with the operation. */

	/* Get my rank and switch communicators to the hidden collective */
	size = comm->local_group->np;
	rank = comm->local_group->local_rank;
	comm = comm->comm_coll;

	/* Get size of data for temporary buffers. */
	MPIR_Type_get_limits( datatype, &lb, &ub );
	m_extent = ub - lb;

	op_ptr = MPIR_GET_OP_PTR(op);
	MPIR_TEST_MPI_OP(op,op_ptr,comm,myname);
	uop  = op_ptr->op;

	/* The direct reduce algorithm doesn't work neither with the short protocol nor with
	   non-commutative reduce operations. */
	if( (m_extent*count <= MPID_SMI_cfg.MAX_SHORT_PAYLOAD ) || !(op_ptr->commute) )
		return MPID_SMI_Tree_reduce (sendbuf, recvbuf, count, datatype, op, root, comm);


	/* Rank transformation: for commutative operations, the root does not
	   really matter. For non-commutative, we always want the root to be proc 0
	   to get identical results for different roots. The result will be transmitted
	   to the real root afterwards. */
	lroot   = op_ptr->commute ? root : 0;

	MPIR_Op_errno = MPI_SUCCESS;

	/* Here the algorithm for arbitrary 2^n fan-in, and with sending only
	   to upper ranks (this is more efficient for SCI-connected system. 
	   Therefore, lroot will be assigned the reduce_rank 'size - 1', all other
	   ranks transformed accordingly. */
	rdc_rank = (rank + (size - 1 - lroot)) % size;

	mask = FANIN;
	mask_shift = MPID_SMI_ld(mask);
	for (upper_size = 1; upper_size < size; upper_size <<= 1);

	if (rank != root) 
		recvbuf = NULL;
	else {
		rdc_buf = recvbuf;
		MEMCPY(rdc_buf, sendbuf, m_extent*count);
	}

	dstnc = 1;
	do {
		dest = rdc_rank | (mask - 1);
		if (dest >= size)
			dest = size - 1;

		if (dest == rdc_rank) {
			/* This process is a receiver in this round. First, allocate receive buffer 
			   if not yet done, and copy the local vector into it. Because reduce-operations
			   always operate on contiguous data, a memcpy() is safe to use here. 

			   Then, post all recvs and process the incoming messages in (or out-of) order. */
			if (rdc_buf == NULL) {
				ALLOCATE(rdc_buf, void *, m_extent*count);
				MEMCPY(rdc_buf, sendbuf, m_extent*count);
			}

			/* Upper and lower ranks which are a 'regular' multiple of the current mask. */
			upper = ((rdc_rank + 1) % mask == 0) ? rdc_rank : 
				(rdc_rank / mask + 1) * mask - 1;
			lower = upper - mask;
			
			/* Which processes will send data to me? We will post a receive request with *the same*
			   recv buffer for all of them - upon each receive that takes place, the incoming 
			   data will be reduced with the data already in the recv buffer. This does of course mean,
			   that these operations need to be sync'ed in case of an active device thread. */
			nbr_recvs = FANIN - ((upper - rdc_rank)/dstnc) - 1;
			for (i = 0; i < nbr_recvs; i++) {
				source = lower + (i+1)*dstnc;

				MPID_Recv_alloc(rhandles[i]);
				MPID_Request_init ((MPI_Request)rhandles[i], MPIR_RECV );
				rhandles[i]->op_ptr = op_ptr;
				rhandles[i]->op_cnt = count;
				rhandles[i]->datatype = datatype;
				MPID_SMIstub_IrecvContig (comm, rdc_buf, m_extent*count, LRANK(source), MPIR_REDUCE_TAG, 
										  comm->recv_context, (MPI_Request)rhandles[i], &mpi_errno);
				/* XXX check for errors */
			}
			/* The reduction operation will be applied within the 'test'/'wait'. */
#if ORDERED_REDUCE
			for (i = 0; i < nbr_recvs; i++) 
				MPICH_Waitall(1, req[i], &status);
#else
			{
				int n_cmplt = 0, is_cmplt = 0;
				ulong test_it = 0; /* bitvector: bit == 0 means "test for this process" */

				i = 0;
				while (n_cmplt < nbr_recvs) {
					if ((1 << i) & ~test_it) {
						MPI_Request req = (MPI_Request)rhandles[i];

						MPICH_Test (&req, &is_cmplt, &status);
						if (is_cmplt) {
							test_it |= (1 << i);
							n_cmplt++;
							is_cmplt = 0;
						}
					}
					i = (i + 1) % nbr_recvs;
				}
			}
#endif
		} else {
			/* This process sends data in this round; then it's done. Most processes (at least
			   50%) do only send once. Therefore, we avoid allocating & copying a separate
			   send buffer for those. Processes, who have received in one round, use the
			   according recv buffer which contains the data to send. */
			buffer = (dstnc == 1) ? sendbuf : rdc_buf;
			MPICH_Send (buffer, count, datatype->self, LRANK(dest), MPIR_REDUCE_TAG, comm->self);
			break;
		}

		mask <<= mask_shift;
		dstnc <<= mask_shift;
	} while ((mask >> mask_shift) <= upper_size);

	/* Send the result to the real root if the operation has been non-commutative. */
	if (!op_ptr->commute && root != 0) {
		if (rank == 0)
			mpi_errno = MPICH_Send( rdc_buf, count, datatype->self, root, MPIR_REDUCE_TAG, comm->self );
		if (rank == root) 
			mpi_errno = MPICH_Recv ( recvbuf, count, datatype->self, 0, MPIR_REDUCE_TAG, comm->self, &status);
	}

	if (mpi_errno == MPI_SUCCESS && MPIR_Op_errno) {
		mpi_errno = MPIR_Op_errno;
	}
	if (rdc_buf != NULL && rank != root)
		FREE (rdc_buf);

#undef LRANK
#undef FANIN
	return (mpi_errno);
}

/* copyright for the code in MPID_SMI_Rab_reduce and use_rabenseifner: 
   /*  Rolf Rabenseifner, 1997
   *  Computing Center University of Stuttgart
   *  rabenseifner@rus.uni-stuttgart.de 
   *
   *  Adaptions for use of this code in MP-MPICH (together with plug-in 
   *  reduce operations) by Karsten Scholtyssik, Boris Bierbaum, Joachim Worringen
   *  of Lehrstuhl f’r Betriebbssysteme, RWTH Aachen
   */ 

/* Check if to use Rabenseifner algorithm: First, check the data size and number of processes 
   to decide if the Rabenseifner-algorithm should be used. This new protocol
   is only efficient for vectors of a minimum size, depending on the actual
   ratio of computing to communication speed. This is platform- and
   datatype-dependant.
   
   Therefore, the thresholds are defined in easily changeable datastructures
   (see top of this file). */
static int use_rabenseifner (int count, struct MPIR_DATATYPE *dtype_ptr, 
							 MPI_Op op, struct MPIR_COMMUNICATOR *comm, int rmode) 
{
	int prot = REDUCE_TREE, size;
	register int ss;
	
	size = comm->local_group->np;
	
	/* Different thresholds exist for 2, 3, 2^n and arbitrary 
	   number of processes. */
	if (size == 2)
		ss = 0;
	else 
		if (size == 3)
			ss = 1;
		else {
			register int s = size;
			
			while (!(s & 1)) s = s >> 1;
			if (s == 1) 
				/* size == power of 2 */
				ss = 2;
			else 
				/* size != power of 2 */
				ss = 3;
		}
	switch(op) {
	case MPI_MAX:   case MPI_MIN: case MPI_SUM:  case MPI_PROD:
	case MPI_LAND:  case MPI_LOR: case MPI_LXOR:
	case MPI_BAND:  case MPI_BOR: case MPI_BXOR:
		switch(dtype_ptr->dte_type) {
		case MPIR_SHORT:  case MPIR_USHORT:
			prot = (count >= Lsh[rmode][ss]) ? REDUCE_RABENSEIFNER : REDUCE_TREE; 
			break;
		case MPIR_INT:    case MPIR_UINT:
			prot = (count >= Lin[rmode][ss]) ? REDUCE_RABENSEIFNER : REDUCE_TREE; 
			break;
		case MPIR_LONG:   case MPIR_ULONG:
			prot = (count >= Llg[rmode][ss]) ? REDUCE_RABENSEIFNER : REDUCE_TREE;
			break;
		}
	}
	switch(op) {
	case MPI_MAX:  case MPI_MIN: case MPI_SUM: case MPI_PROD:
		switch(dtype_ptr->dte_type) {
		case MPIR_FLOAT:
			prot = (count >= Lfp[rmode][ss]) ? REDUCE_RABENSEIFNER : REDUCE_TREE; 
			break;
		case MPIR_DOUBLE:
			prot = (count >= Ldb[rmode][ss]) ? REDUCE_RABENSEIFNER : REDUCE_TREE;
			break;
		}
	}
	switch(op) {
	case MPI_BAND:  case MPI_BOR: case MPI_BXOR:
		switch(dtype_ptr->dte_type) {
		case MPIR_BYTE:
			prot = (count >= Lby[rmode][ss]) ? REDUCE_RABENSEIFNER : REDUCE_TREE; 
			break;
		}
	}
	return prot;
}


#define ALLREDUCE_DEBUG 0

static int MPID_SMI_Rab_reduce (void *sendbuf, void *recvbuf, int count, struct MPIR_DATATYPE *datatype,
								MPI_Op op, int root, struct MPIR_COMMUNICATOR *comm, int rmode) {
	char *scrbas,*scrbuf, *scr2buf, *xxx, *sbuf, *rbuf;
	int myrank, size, x_base, x_size, idx, computed = 0;
	int x_start, x_count, r, n, mynewrank, newroot, partner;
	int start_even[20], start_odd[20], count_even[20], count_odd[20];
	int count_lowerhalf, count_upperhalf;
	MPI_Aint typelng;
	MPI_Status status; MPI_Request req;
	size_t scrlng;
	struct MPIR_OP *op_ptr;
	MPI_User_function *uop;
	int mpi_errno;


	myrank = comm->local_group->local_rank;
	size = comm->local_group->np;
	/* get pointer to operation */
	comm = comm->comm_coll;
	op_ptr = MPIR_GET_OP_PTR(op);
	MPIR_TEST_MPI_OP(op,op_ptr,comm, "MPID_SMI_anyReduce");
	uop = op_ptr->op;
	count_lowerhalf = count/2;
	count_upperhalf = count - count_lowerhalf;

	typelng = datatype->extent;
	scrlng  = typelng * count;

	sbuf = (char*) sendbuf;
	rbuf = (char*) recvbuf;
	if (rmode == RMODE_ALLREDUCE)
		root = myrank; /* for correct rbuf handling */
	if( root == myrank ) { /* rbuf is valid and can be used */
		ALIGNEDALLOCATE( scrbas, char *, scrlng, 16 );
		scr2buf = scrbas;
		scrbuf = rbuf;
	}
	else { /* we need one more temporary buffer */
		ALIGNEDALLOCATE( scrbas, char *, 2*scrlng, 16 );
		scr2buf = scrbas;
		scrbuf = scr2buf + scrlng;
	}
    
	/*...step 1 */

	n = 0; x_size = 1;
	while (2*x_size <= size) {
		n++; x_size = x_size * 2;
	}
	/* x_sixe == 2**n */
	r = size - x_size;

	/*...step 2 */

	if (myrank < 2*r) {
		if ((myrank % 2) == 0) { /* even */
			MPICH_Send(sbuf + (count/2)*typelng,
					   count - count/2, datatype->self, myrank+1, 1220, comm->self);
			MPICH_Recv(scrbuf,
					   count/2, datatype->self, myrank+1, 1221, comm->self, &status);
#ifdef WIN32
			if( op_ptr->stdcall )
				op_ptr->op_s( sbuf, scrbuf, &count_lowerhalf, &datatype->self );
			else
#endif 
				(*uop)( sbuf, scrbuf, &count_lowerhalf, &datatype->self );

			MPICH_Recv(scrbuf + (count/2)*typelng, count - count/2,
					   datatype->self, myrank+1, 1223, comm->self, &status);
			computed = 1;
		}
		else { /* odd */
			MPICH_Recv(scrbuf + (count/2)*typelng,
					   count - (count/2), datatype->self, myrank-1, 1220, comm->self, &status);
			MPICH_Send(sbuf,
					   count/2, datatype->self, myrank-1, 1221, comm->self);

#ifdef WIN32
			if( op_ptr->stdcall )
				op_ptr->op_s( sbuf + count_lowerhalf*typelng,
							  scrbuf + count_lowerhalf*typelng,
							  &count_upperhalf, &datatype->self );
			else
#endif 
				(*uop)( sbuf + count_lowerhalf*typelng,
						scrbuf + count_lowerhalf*typelng,
						&count_upperhalf, &datatype->self );
			MPICH_Send(scrbuf + (count/2)*typelng, count - count/2,
					   datatype->self, myrank-1, 1223, comm->self);
		}
	}

	/*...step 3+4 */

	if ((myrank >= 2*r) || ((myrank%2 == 0)  &&  (myrank < 2*r)))
		mynewrank = (myrank < 2*r ? myrank/2 : myrank-r);
	else
		mynewrank = -1;
      
	if (mynewrank >= 0) {    /* begin -- only for nodes with new rank */

#define OLDRANK(new)   ((new) < r ? (new)*2 : (new)+r)

		/*...step 5 */

		x_start = 0;
		x_count = count;
		for (idx=0, x_base=1; idx<n; idx++, x_base=x_base*2) {
			start_even[idx] = x_start;
			count_even[idx] = x_count / 2;
			start_odd [idx] = x_start + count_even[idx];
			count_odd [idx] = x_count - count_even[idx];
			if (((mynewrank/x_base) % 2) == 0) {  
#if ALLREDUCE_DEBUG
				fprintf (stderr, "[%d|%d] rab rdc_even: to %d, rcount %d, scount %d\n",
						 MPID_SMI_myid, mynewrank, mynewrank+x_base, x_count, count_odd[idx]);
#endif
				/* even */
				x_start = start_even[idx];
				x_count = count_even[idx];

				MPICH_Send((computed ? scrbuf : sbuf) + start_odd[idx]*typelng,
						   count_odd[idx], datatype->self, OLDRANK(mynewrank+x_base), 1231, comm->self);
				MPICH_Recv(scr2buf + x_start*typelng,
						   x_count, datatype->self, OLDRANK(mynewrank+x_base), 1232, comm->self, &status);

#ifdef WIN32
				if( op_ptr->stdcall )
					op_ptr->op_s( (computed ? scrbuf : sbuf) + x_start*typelng,
								  scr2buf + x_start*typelng,
								  &x_count, &datatype->self );
				else
#endif
					(*uop)( (computed ? scrbuf : sbuf) + x_start*typelng,
							scr2buf + x_start*typelng,
							&x_count, &datatype->self );
			} else { 
				/* odd */
#if ALLREDUCE_DEBUG
				fprintf (stderr, "[%d|%d] rab rdc_odd: to %d, rcount %d, scount %d\n",
						 MPID_SMI_myid, mynewrank, mynewrank-x_base, x_count, count_even[idx]);
#endif
				x_start = start_odd[idx];
				x_count = count_odd[idx];

				MPICH_Recv(scr2buf + x_start*typelng,
						   x_count, datatype->self, OLDRANK(mynewrank-x_base), 1231, comm->self, &status);
				MPICH_Send((computed ? scrbuf : sbuf) + start_even[idx]*typelng,
						   count_even[idx], datatype->self, OLDRANK(mynewrank-x_base), 1232, comm->self);

#ifdef WIN32
				if( op_ptr->stdcall )
					op_ptr->op_s( (computed ? scrbuf : sbuf) + x_start*typelng,
								  scr2buf + x_start*typelng,
								  &x_count, &datatype->self );
				else
#endif
					(*uop)( (computed ? scrbuf : sbuf) + x_start*typelng,
							scr2buf + x_start*typelng,
							&x_count, &datatype->self );
			}
			computed = 1;
			xxx = scrbuf; scrbuf = scr2buf; scr2buf = xxx;
		} /*for*/

		/* valid data should now be in scrbuf, but maybe scrbuf is != rbuf, so we have to copy some data
		   (unfortunately); we want to copy as little as possible, so we copy only that data that we have correct;
		   this is the one on which the last calculation has been taken place */
	     
		if( root == myrank ) {
			if( scrbuf != rbuf )
				memcpy( rbuf + x_start*typelng, scrbuf + x_start*typelng, x_count*typelng );
		}
#undef OLDRANK
	} /* end -- only for nodes with new rank */
      
	if (rmode == RMODE_ALLREDUCE) {
		/*...steps 6.1 to 6.n */
		if (mynewrank >= 0) {   /* begin -- only for nodes with new rank */
#define OLDRANK(new)   ((new) < r ? (new)*2 : (new)+r)

			/* to get data in rbuf without memcpy */
			for(idx=n-1, x_base=x_size/2; idx>=0; idx--, x_base=x_base/2) {
				if (((mynewrank/x_base) % 2) == 0) {    
#if ALLREDUCE_DEBUG
					fprintf (stderr, "[%d|%d] rab gather_even: to %d, rcount %d, scount %d\n",
							 MPID_SMI_myid, mynewrank, mynewrank+x_base, count_odd[idx], count_even[idx]);
#endif
					/* even */
					MPICH_Send(rbuf + start_even[idx]*typelng,
							   count_even[idx], datatype->self, OLDRANK(mynewrank+x_base), 1241, comm->self);
					MPICH_Recv(rbuf + start_odd[idx]*typelng,
							   count_odd[idx], datatype->self, OLDRANK(mynewrank+x_base), 1242, comm->self, &status);
				} else { 
#if ALLREDUCE_DEBUG
					fprintf (stderr, "[%d|%d] rab gather_odd: to %d, rcount %d, scount %d\n",
							 MPID_SMI_myid, mynewrank, mynewrank+x_base, count_even[idx], count_odd[idx]);
#endif
					/* odd */
					MPICH_Recv(rbuf + start_even[idx]*typelng,
							   count_even[idx], datatype->self, OLDRANK(mynewrank-x_base), 1241, comm->self, &status);
					MPICH_Send(rbuf + start_odd[idx]*typelng,
							   count_odd[idx], datatype->self, OLDRANK(mynewrank-x_base), 1242, comm->self);
				}
			} /* for */
#undef OLDRANK
	      
		} /* end -- only for nodes with new rank */
	  
		/*...step 7 */
		if (myrank < 2*r) {
			if (myrank%2 == 0)  /* even */
				MPICH_Send(rbuf, count, datatype->self, myrank+1, 1253, comm->self);
			else   /* odd */
				MPICH_Recv(rbuf, count, datatype->self, myrank-1, 1253, comm->self, &status);
		}

	} else {    /* not ALLREDUCE, i.e. Reduce */

		/*...step 6.0 */
		if ((root < 2*r) && (root%2 == 1)) {
			if (myrank == 0) { /* then mynewrank==0, x_start==0
								  x_count == count/x_size  */
				MPICH_Send(scrbuf,x_count,datatype->self,root,1241,comm->self);
				mynewrank = -1;
			}

			if (myrank == root) {
				mynewrank = 0;
				x_start = 0;
				x_count = count;
				for (idx=0, x_base=1; idx<n; idx++, x_base=x_base*2) {
					start_even[idx] = x_start;
					count_even[idx] = x_count / 2;
					start_odd [idx] = x_start + count_even[idx];
					count_odd [idx] = x_count - count_even[idx];
					/* code for always even in each bit of mynewrank: */
					x_start = start_even[idx];
					x_count = count_even[idx];
				}
				MPICH_Recv(rbuf,x_count,datatype->self,0,1241,comm->self,&status);
			}
			newroot = 0;
		} else
			newroot = (root < 2*r ? root/2 : root-r);

		/*...steps 6.1 to 6.n */
		if (mynewrank >= 0) { /* begin -- only for nodes with new rank */

#define OLDRANK(new) ((new)==newroot ? root				\
					  : ((new)<r ? (new)*2 : (new)+r) )

			for(idx=n-1, x_base=x_size/2; idx>=0; idx--, x_base=x_base/2) {
				if ((mynewrank & x_base) != (newroot & x_base)) {
					if (((mynewrank/x_base) % 2) == 0) {   /* even */
						x_start = start_even[idx]; x_count = count_even[idx];
						partner = mynewrank+x_base;
					} else {
						x_start = start_odd[idx]; x_count = count_odd[idx];
						partner = mynewrank-x_base; }
					MPICH_Send(scrbuf + x_start*typelng, x_count, datatype->self,
							   OLDRANK(partner), 1244, comm->self);
				} else {   
					/* odd */
					if (((mynewrank/x_base) % 2) == 0) { /* even */
						x_start = start_odd[idx]; x_count = count_odd[idx];
						partner = mynewrank+x_base;
					} else {
						x_start = start_even[idx]; x_count = count_even[idx];
						partner = mynewrank-x_base;
					}
					MPICH_Recv((myrank == root ? rbuf : scrbuf)
							   + x_start*typelng, x_count, datatype->self,
							   OLDRANK(partner), 1244, comm->self, &status);
				}
			} /*for*/
#undef OLDRANK
		} /* end -- only for nodes with new rank */
	}

	ALIGNEDFREE( scrbas );

	return MPI_SUCCESS;

}


/* Initialize recv buffer with the local vector. */
#define INIT_RBUF(rbuf, sbuf, count, dtype, comm, req, status, is_init) \
	if (!is_init) {														\
        is_init = 1;													\
		if (dtype->is_contig) {											\
			MEMCPY (rbuf, sbuf, count*dtype->extent);					\
		} else {														\
			MPICH_Irecv(rbuf, count, dtype->self, comm->local_rank, MPIR_ALLREDUCE_TAG, comm->self, req); \
			MPICH_Send(sendbuf, count, dtype->self, comm->local_rank, MPIR_ALLREDUCE_TAG, comm->self); \
			MPICH_Waitall(1, req, status);								\
		} }

/* 16 steps are sufficient for 65536 procs */
#define MAX_RDC_STEPS    16
/* Have only one process per SMP-node do inter-node communication? */
#define SMP_CONCENTRATE  0
/* send head-to-head or ping-pong ? (n/a for direct reduce) */
#define DO_HEADTOHEAD    1

/* This is basically the Rabenseifner algorithm for MPI_Allreduce, but optimized for SCI and 
   SMP usage. And it is much cleaner written, as I think.... */
/* XXX For now, we only support 2**n number of processes. For other communicator sizes,
   this algorithm does perform relatively worse, because we reduce the number of active
   processes to the next lower 2**n-number by having the remaining processes send their
   complete vectors, and getting the complete vectors back at the end, adding two 
   additional full-lenth communication steps. */
static int MPID_SMI_RabSCI_allreduce (void *sendbuf, void *recvbuf, int count, struct MPIR_DATATYPE *datatype,
									  MPI_Op op, struct MPIR_COMMUNICATOR *comm)
{
	MPID_SMI_comm_info_t *ci = (MPID_SMI_comm_info_t *)comm->adiCollCtx;
	MPI_Status status;
	MPI_Request req;
	MPI_User_function *uop;
	MPIR_RHANDLE *direct_rhandle;
	MPI_Aint   lb, ub, m_extent;  /* Extent in memory */
	struct MPIR_OP *op_ptr;
	char      *sbuf, *rbuf, *rdc_src, *invec = NULL;
	int        size, rsize, rank, rrank, extent, i, is_init, partner;
	int        sdispls, rdispls, tdispls, scount, rcount, tcount, rank_scale;
	int        mpi_errno = MPI_SUCCESS;
	int        displs_sent[MAX_RDC_STEPS], count_sent[MAX_RDC_STEPS], rdc_steps; 
	static char myname[] = "MPID_SMI_RabSCI_allreduce";

	/* See the overview in Collection Operations for why this is ok */
	if (count == 0) 
		return MPI_SUCCESS;

	/* If the operation is predefined, we could check that the datatype's
	   type signature is compatible with the operation. */

	/* Get my rank and switch communicators to the hidden collective */
	rsize = size = comm->np;
	rrank = rank = comm->local_rank;
	comm = comm->comm_coll;
	extent = datatype->extent;
	is_init = 0;

	if (size == 1) {
		INIT_RBUF(recvbuf, sendbuf, count, datatype, comm, &req, &status, is_init);
		return MPI_SUCCESS;
	}

	if (comm->group->N2_prev != size)
		return MPID_SMI_Rab_reduce (sendbuf, recvbuf, count, datatype, op, 0, comm, RMODE_ALLREDUCE);

	op_ptr = MPIR_GET_OP_PTR(op);
	MPIR_TEST_MPI_OP(op, op_ptr, comm, myname);
	uop = op_ptr->op;

	/* SMP optimization: do a local reduction first to reduce network traffic. */ 
	if (SMP_CONCENTRATE && ci->lprocs_per_node >= 2 && ci->nbr_lnodes > 1) {
		rsize = size / ci->lprocs_per_node;

		if (ci->nrank == 0) {
			/* Init recv buffer with this operation. */
			MPICH_Recv (recvbuf, count, datatype->self, rank+1, MPIR_ALLREDUCE_TAG, comm->self, &status);
			CALL_OP_FUNC (recvbuf, sendbuf, count, op_ptr, datatype);
			is_init = 1;
			if (ci->nsize > 2)
				ALIGNEDALLOCATE (invec, char *, count*extent, 16);
			for (i = 2; i < ci->nsize; i++) {
				/* XXX optimize: direct reduce */
				MPICH_Recv (invec, count, datatype->self, rank+i, MPIR_ALLREDUCE_TAG, comm->self, &status);
				CALL_OP_FUNC(recvbuf, invec, count, op_ptr, datatype);
			}
		} else {
			/* SMP slave process! */
			MPICH_Send (sendbuf, count,  datatype->self, rank - ci->nrank, MPIR_ALLREDUCE_TAG, comm->self);
			MPICH_Recv (recvbuf, count,  datatype->self, rank - ci->nrank, MPIR_ALLREDUCE_TAG, 
						comm->self, &status);
			
			return MPI_SUCCESS;
		}
	}

	rank_scale = size/rsize;
	rrank = rank/rank_scale;
	rdc_steps = MPID_SMI_ld(rsize);
	MPID_ASSERT (rdc_steps <= MAX_RDC_STEPS, "Passed maximum number of reduce steps!");

	if (rdc_steps != 0) {
		if (invec == NULL)
			ALIGNEDALLOCATE (invec, char *, count*extent, 16);

		/* Now, do the actual reductions. Each process pair exchanges the upper and lower half of
		   the vector it calculated last -> vector size to exchange and reduce decreases logarithmically
		   with each step. */
		partner = (rrank ^ 1);
		sdispls = (rrank < partner) ? 0 : count/2*extent; 
		rdispls = (rrank < partner) ? count/2*extent : 0; 
		rcount  = (rrank < partner) ? count - count/2 : count/2;
		scount  = count - rcount;
		/* Depending on whether the recvbuffer is already initialized (this is the case for
		   SMP-optimization), we can avoid a memcpy by setting the buffer ptrs accordingly 
		   for the first exchange. */
		rbuf    = is_init ? (char *)invec : (char *)recvbuf;
		sbuf    = is_init ? (char *)recvbuf : (char *)sendbuf;
		rdc_src = is_init ? (char *)invec :  (char *)sendbuf;
		for (i = 0; i < rdc_steps; i++) {
			/* Perfect shuffle, here we go...*/
			count_sent[i] = scount;
			displs_sent[i] = sdispls;

#if ALLREDUCE_DEBUG
			fprintf (stderr, "[%d|%d] rdc: partner %d, rcount %d, scount %d, rdispls %d, sdispls %d\n",
					 MPID_SMI_myid, rank, partner*rank_scale, rcount, scount, rdispls, sdispls);
#endif
			/* Different ways to transmit the data...*/
			if (MPID_SMI_cfg.COLL_DIRECT_REDUCE_OK
				&& extent*rcount > MPID_SMI_cfg.MAX_SHORT_PAYLOAD) {
				/* Direct reduce is not possible for the short protocol. Also, with direct 
				   reduce, we always need to explicitly init the recv buffer! */
				if (!is_init) 
					INIT_RBUF(recvbuf, sendbuf, count, datatype, comm, &req, &status, is_init);			
				MPID_Recv_alloc(direct_rhandle);
				MPID_Request_init ((MPI_Request)direct_rhandle, MPIR_RECV);
				direct_rhandle->op_ptr = op_ptr;
				direct_rhandle->op_cnt = rcount;
				direct_rhandle->datatype = datatype;
				MPID_SMIstub_IrecvContig (comm, (char *)recvbuf + rdispls, extent*rcount, partner*rank_scale,
										  MPIR_ALLREDUCE_TAG, comm->recv_context, (MPI_Request)direct_rhandle, 
										  &mpi_errno);
				MPICH_Send (sbuf + sdispls, scount, datatype->self, partner*rank_scale, MPIR_ALLREDUCE_TAG, 
							comm->self);
				MPICH_Waitall (1, (MPI_Request *)&direct_rhandle, &status);
			} else {
#if DO_HEADTOHEAD
				MPICH_Irecv (rbuf + rdispls, rcount, datatype->self, partner*rank_scale, MPIR_ALLREDUCE_TAG, 
							 comm->self, &req);
				MPICH_Send (sbuf + sdispls, scount, datatype->self, partner*rank_scale, MPIR_ALLREDUCE_TAG, 
							comm->self);
				MPICH_Waitall (1, &req, &status);
#else
				if (rrank < partner) {
					MPICH_Recv (rbuf + rdispls, rcount, datatype->self, partner*rank_scale, MPIR_ALLREDUCE_TAG, 
								comm->self, &status);
					MPICH_Send (sbuf + sdispls, scount, datatype->self, partner*rank_scale, MPIR_ALLREDUCE_TAG, 
								comm->self);
				} else {
					MPICH_Send (sbuf + sdispls, scount, datatype->self, partner*rank_scale, MPIR_ALLREDUCE_TAG, 
								comm->self);
					MPICH_Recv (rbuf + rdispls, rcount, datatype->self, partner*rank_scale, MPIR_ALLREDUCE_TAG, 
								comm->self, &status);
				}
#endif /* DO_HEADTOHEAD */
				CALL_OP_FUNC((char *)recvbuf + rdispls, rdc_src + rdispls, rcount, op_ptr, datatype);
			}

			/* For all than the first step, this is the only valid setting of buffers. */
			if (i == 0) {
				rbuf = (char *)invec;
				sbuf = (char *)recvbuf;
				rdc_src = (char *)invec;
			}

			/* We need these values after the loop, therefore don't touch them if we
			   are going to leave it. */
			if (i + 1 < rdc_steps) {
				/* Calculate counts and displacements for the next exchange. We send one
				   half of the vector that we have just 'reduced', and we know that the
				   next partner has received the same amount of data in the last step. */
				partner = (rrank ^ (1<<(i+1)));
				scount = (rrank < partner) ? rcount/2 : rcount - rcount/2;
				rcount = (rrank < partner) ? rcount - rcount/2 : rcount/2;
				sdispls = (rrank < partner) ? rdispls : rdispls + rcount*extent; 
				rdispls = (rrank < partner) ? rdispls + scount*extent : rdispls; 
			}
		}
		
		/* For uneven distribution of part-vector lenghts: some part-
		   vectors may contain one more element. The nice thing is that these count%rsize
		   longer vectors are located at the count%rsize lower processes, because the longer
		   vector halfes are always sent "downstream". */

		/* 'allgatherv' the results into the single result vector. This is just the other way
		   round than the reduction. */
		partner = (rrank ^ (1<<(rdc_steps - 1)));
		tcount = rcount; rcount = scount; scount = tcount;
		tdispls = rdispls; rdispls = sdispls; sdispls = tdispls;
		rbuf = (char *)recvbuf;

		for (i = rdc_steps - 1; i >= 0; i--) {
#if ALLREDUCE_DEBUG
			fprintf (stderr, "[%d|%d] gather: partner %d, rcount %d, scount %d, rdispls %d, sdispls %d\n",
					 MPID_SMI_myid, rank, partner*rank_scale, rcount, scount, rdispls, sdispls);
#endif
#if DO_HEADTOHEAD
			MPICH_Irecv (rbuf + rdispls, rcount, datatype->self, partner*rank_scale, MPIR_ALLREDUCE_TAG, 
						 comm->self, &req);
			MPICH_Send (rbuf + sdispls, scount, datatype->self, partner*rank_scale, MPIR_ALLREDUCE_TAG, 
						comm->self);
			MPICH_Waitall (1, &req, &status);
#else
			if (rrank < partner) {
				MPICH_Recv (rbuf + rdispls, rcount, datatype->self, partner*rank_scale, MPIR_ALLREDUCE_TAG, 
							comm->self, &status);
				MPICH_Send (rbuf + sdispls, scount, datatype->self, partner*rank_scale, MPIR_ALLREDUCE_TAG, 
							comm->self);
			} else {
				MPICH_Send (rbuf + sdispls, scount, datatype->self, partner*rank_scale, MPIR_ALLREDUCE_TAG, 
							comm->self);
				MPICH_Recv (rbuf + rdispls, rcount, datatype->self, partner*rank_scale, MPIR_ALLREDUCE_TAG, 
							comm->self, &status);
			}
#endif
			
			if (i > 0) {
				scount += rcount;
				sdispls = (rdispls < sdispls) ? rdispls : sdispls;

				partner = (rrank ^ (1<<(i-1)));
				rcount  = count_sent[i-1];
				rdispls = displs_sent[i-1];
			}
		}

		ALIGNEDFREE(invec);
	}

	/* If necessary, distribute data to local process(es). 
	   XXX For larger SMPs, a smarter bcast algorithm is recommended. */
	if (size != rsize) {
		for (i = 1; i < size/rsize; i++) 
			MPICH_Send (rbuf, count, datatype->self, rank + i, MPIR_ALLREDUCE_TAG, comm->self);
	}

	return MPI_SUCCESS;
}

#undef MAX_RDC_STEPS
#undef SMP_CONCENTRATE
#undef DO_DIRECT_REDUCE
#undef DO_HEADTOHEAD



/* This is another optimization for MPI_Allreduce: an MPI_Allreduce is nearly identical
   to an MPI_Allgather, except for the reduction operation: each process has a vector
   which is sent "as is" to all other processes - that's allgahering. Having this,
   all-reducing simply requires an additional reduction operation instead of a copy 
   operation. Because the custom-allgather for SCI performs much better than the
   other custom allreduce-variants, we choose this approach. Combined with the 
   direct-reduce technique, we need no addtional copy operation, but really just
   replace the copy-op with the reduction-op - which should be only slightly slower,
   if at all.

   Further thoughts could be spent to do some global overlapping of computation and
   communication as it was done in reduce_scatter. */
int MPID_SMI_Allgather_allreduce (void *sendbuf, void *recvbuf, int count, struct MPIR_DATATYPE *datatype,
								  MPI_Op op, struct MPIR_COMMUNICATOR *comm)
{
	MPI_Status status;
	MPI_Request req, *wait_req;
	MPI_User_function *uop;
	MPI_Aint   lb, ub, m_extent;  /* Extent in memory */
	struct MPIR_OP *op_ptr;
	int        size, rank, extent;
	int		   i, rsize, n2_prev, diff, gr_steps, is_init = 0;
	int        first_rmbr_vec, nbr_rmbr_vec, rmbr_vec_cnt;
	int       *recvcnt = NULL, *displs;
	int        mpi_errno = MPI_SUCCESS;
	void       **rmbr_vecs, *invec;
	static char myname[] = "MPID_SMI_Allgather_allreduce";

	/* See the overview in Collection Operations for why this is ok */
	if (count == 0) 
		return MPI_SUCCESS;

	/* If the operation is predefined, we could check that the datatype's
	   type signature is compatible with the operation. */

	/* Get my rank and switch communicators to the hidden collective */
	size = comm->np;
	rank = comm->local_rank;
	comm = comm->comm_coll;
	extent = datatype->extent;

	op_ptr = MPIR_GET_OP_PTR(op);
	MPIR_TEST_MPI_OP(op,op_ptr,comm,myname);
	uop  = op_ptr->op;

	if (size == 1) {
		INIT_RBUF(recvbuf, sendbuf, count, datatype, comm, &req, &status, is_init);
		return MPI_SUCCESS;
	}
 
	/* Determine how many "gather-reductions" we can do before we need to exchange
	   these vectors. The ideal number of processes for this is of course 2**n,
	   requiring n steps only. Other even numbers of processes require the same number
	   of steps than the next 2**n. For odd process numbers, the odd process will cause
	   two addtional steps, which however may at least partly overlap with the default 
	   steps: it sends it's data to process 0, and recvs a part of the vector from each 
	   other process (MPI_Gatherv). */
	rsize = (size % 2 != 0) ? size - 1 : size;
	n2_prev = comm->group->N2_prev;
	gr_steps = MPID_SMI_ld(n2_prev);
	if (n2_prev == rsize) {
		first_rmbr_vec = gr_steps + 1; /* don't need to remember any step in this case */
		nbr_rmbr_vec = 0;
	} else {
		/* The (number of) steps in which we need to remember the reduction vectors for the 
		   non-exp(2)-reductions at the end depends on which "block sizes" we need to 
		   do this. I.e.:
		   - 8 procs:  (((1+1)+2)+4) (nice!)
		   - 10 procs: (((1+1)+2)+4)+2 -> first_rmbr_vec = 0, nbr_rmbr_vec = 1
		   - 12 procs: (((1+1)+2)+4)+4 -> first_rmbr_vec = 1, nbr_rmbr_vec = 1
		   - 14 procs: (((1+1)+2)+4)+2+4 -> first_rmbr_vec = 0, nbr_rmbr_vec = 2
		   - 16 procs is nice again, as it is: ((((1+1)+2)+4)+8) */
		diff = (rsize - n2_prev) >> 1;
		nbr_rmbr_vec = 0;
		first_rmbr_vec = 0;
		for (i = 0; i < 2*n2_prev - n2_prev - 1; i++) {
			if ((diff >> 1) << 1 == diff && nbr_rmbr_vec == 0)
				first_rmbr_vec++;
			if ((diff >> 1) << 1 != diff)
				nbr_rmbr_vec++;
			diff >>= 1;
		}
	}
#if ALLREDUCE_DEBUG
	fprintf (stderr, "[%d|%d] gather_allrdc with %d procs; gr_steps = %d, nbr_rmbr= %d, f_rmbr = %d\n",
			 MPID_SMI_myid, comm->local_rank, comm->np, gr_steps, nbr_rmbr_vec, first_rmbr_vec);
#endif

	if (nbr_rmbr_vec > 0) {
		ALLOCATE(rmbr_vecs, void *, nbr_rmbr_vec*sizeof(void*));
		ALLOCATE (rmbr_vecs[0], void *, nbr_rmbr_vec*count*extent);
		for (i = 1; i < nbr_rmbr_vec; i++)
			rmbr_vecs[i] = (char *)rmbr_vecs[0] + i*count*extent;
	}
	ALIGNEDALLOCATE (invec, void *, count*extent, 16);

	if (rsize != size) {
		/* Handling of the odd process which will be separated from the others. */
		ZALLOCATE (recvcnt, int *, size*sizeof(int));
		ZALLOCATE (displs, int *, size*sizeof(int));
		for (i = 0; i < rsize; i++) 
			recvcnt[i] = count/rsize;
		for (i = 0; i < count % rsize; i++) 
			recvcnt[i]++;
		for (i = 1; i < rsize; i++) 
			displs[i] = recvcnt[i-1] + displs[i-1];

		if (rank == 0) {
			/* Allocate an additional buffer and recv vector from last process, then
			   reduce it. */
			MPICH_Recv(invec, count, datatype->self, size - 1, MPIR_ALLREDUCE_TAG, 
					   comm->self, &status);
			INIT_RBUF(recvbuf, sendbuf, count, datatype, comm, &req, &status, is_init);
			CALL_OP_FUNC(recvbuf, invec, count, op_ptr, datatype);
		}  
		if (rank == size - 1) {
			/* Send vector to process 0, than start with a 'gatherv' to recv the
			   reduced result-vector in parts from all other processes. */
			MPICH_Send (sendbuf, count, datatype->self, 0, MPIR_ALLREDUCE_TAG, comm->self);

			INIT_RBUF(recvbuf, sendbuf, count, datatype, comm, &req, &status, is_init);
			comm->collops->Gatherv (sendbuf, 0, datatype, recvbuf, recvcnt, displs, 
									datatype, rank, comm);
			FREE (displs); FREE (recvcnt); FREE(invec);
			if (nbr_rmbr_vec > 0) {
				FREE (rmbr_vecs[0]); FREE (rmbr_vecs);
			}
			return (mpi_errno);
		}
	}
	
	/* An even number of processes does their exchanging. This is synchronous for all processes, and the
	   next communication to take place always depends on the prior computation - that makes it hard 
	   to employ global overlapping. */
	/* Direct reduce can not help easily , as we can not modify the 'recvbuf' while
	   sending it at the same time. Instead, we would need to do another local copy of the buffer -
	   it depends on the memcyp-vs-network bandwidth if this pays of. */
	if (1 || (rsize/2) % 2 != 0 || nbr_rmbr_vec != 0) {
		INIT_RBUF(recvbuf, sendbuf, count, datatype, comm, &req, &status, is_init);
		
		for (i = 0, rmbr_vec_cnt = 0; i < gr_steps; i++) {
			MPICH_Irecv(invec, count, datatype->self, (rank + rsize - (1<<i)) % rsize, MPIR_ALLREDUCE_TAG, 
						comm->self, &req);
			MPICH_Send(recvbuf, count, datatype->self, (rank + (1<<i)) % rsize, 
					   MPIR_ALLREDUCE_TAG, comm->self);
			MPICH_Waitall(1, &req, &status);
			
			CALL_OP_FUNC(recvbuf, invec, count, op_ptr, datatype);
			if (i >= first_rmbr_vec && i < first_rmbr_vec + nbr_rmbr_vec) {
				MEMCPY (rmbr_vecs[rmbr_vec_cnt], recvbuf, count*extent);
				rmbr_vec_cnt++;
			}
		}
	} else {
		/* One possibility is to divide the processes into two independent groups (even and odd), which 
		   take turns in communicating and computing.  However, this will only pay off if there's really 
		   a significant congestion on the network as this  approach includes some wait states and 
		   synchronization. This is helpful for SMP usage with two processes per node which take turns in 
		   using the adapter. 
		   To help avoid SCI segment saturation in larger configurations, the two groups use different
		   communication distances (increasing vs. decreasing). */
		/* XXX: For now, only do this for the 2**n case */
		/* XXX: use barrier sync to increase synchronizity? */ 
		if (rank % 2 == 0) {
			INIT_RBUF(recvbuf, sendbuf, count, datatype, comm, &req, &status, is_init);

			for (i = 1; i < gr_steps; i++) {
				MPICH_Irecv(invec, count, datatype->self, (rank + rsize - (1<<i)) % rsize, MPIR_ALLREDUCE_TAG, 
							comm->self, &req);
				MPICH_Send(recvbuf, count, datatype->self, (rank + (1<<i)) % rsize, 
						   MPIR_ALLREDUCE_TAG, comm->self);
				MPICH_Waitall(1, &req, &status);
				
				CALL_OP_FUNC(recvbuf, invec, count, op_ptr, datatype);
			}
		} else {
			void *rdc_buf = sendbuf;
			for (i = gr_steps - 1; i > 0; i--) {
				MPICH_Irecv(invec, count, datatype->self, (rank + rsize - (1<<i)) % rsize, MPIR_ALLREDUCE_TAG, 
							comm->self, &req);
				MPICH_Send(rdc_buf, count, datatype->self, (rank + (1<<i)) % rsize, 
						   MPIR_ALLREDUCE_TAG, comm->self);
				MPICH_Waitall(1, &req, &status);
				
				/* We do init the recvbuffer later to achieve some relaxation. */
				if (i == gr_steps - 1) {
					INIT_RBUF(recvbuf, sendbuf, count, datatype, comm, &req, &status, is_init);
					rdc_buf = recvbuf;
				}

				CALL_OP_FUNC(rdc_buf, invec, count, op_ptr, datatype);
			}
		}
		/* Finally, merge the vectors of the two sub-groups. */
		MPICH_Irecv(invec, count, datatype->self, (rank + rsize - 1) % rsize, MPIR_ALLREDUCE_TAG, 
					comm->self, &req);
		MPICH_Send(recvbuf, count, datatype->self, (rank + 1) % rsize, MPIR_ALLREDUCE_TAG, comm->self);
		MPICH_Waitall(1, &req, &status);
		
		CALL_OP_FUNC(recvbuf, invec, count, op_ptr, datatype);
	}

	/* For process number of 2**n, we are finished now. For other process numbers, we need to 
	   exchange the "remember vectors" to achieve the full reduction. */
	for (diff = 0, i = first_rmbr_vec; i < first_rmbr_vec + nbr_rmbr_vec; i++) {
#if ALLREDUCE_DEBUG
		fprintf (stderr, "[%d|%d] rmbr_exhange: from %d, to %d\n",
				 MPID_SMI_myid, comm->local_rank, (rank + diff + (2<<i)) % rsize, (rank - (diff + (2<<i))) % rsize);
#endif
		MPICH_Irecv(invec, count, datatype->self, (rank + diff + (2<<i)) % rsize, MPIR_ALLREDUCE_TAG, 
					comm->self, &req);
		MPICH_Send(rmbr_vecs[i], count, datatype->self, (rank - (diff + (2<<i)) + rsize) % rsize, 
				   MPIR_ALLREDUCE_TAG, comm->self);
		MPICH_Waitall(1, &req, &status);
		diff += (2<<i);

		CALL_OP_FUNC(recvbuf, invec, count, op_ptr, datatype);
	}

	/* Finally, send a part of the result vector towards the waiting odd process. */
	if (rsize != size) {
		comm->collops->Gatherv ((char*)recvbuf + displs[rank]*extent, recvcnt[rank], datatype, NULL, 
								NULL, NULL, datatype, size - 1, comm);
		FREE (recvcnt);
		FREE (displs);
	}

	ALIGNEDFREE (invec);
	if (nbr_rmbr_vec > 0) {
		FREE (rmbr_vecs[0]);
		FREE (rmbr_vecs);
	}
	
	return (mpi_errno);
}
#undef INIT_RBUF


/* We employ a specific all-to-all-like communiation pattern for reduce_scatter of 
   "small" messsages (short or eager) which allows for overlapping of communication 
   and computation between the *processes* (not within an process). This reduces 
   the load on the network without effectively loosing cycles in computation or communication.

   For "big" messages (rendez-vous), a pipeline-reduce followed by a uni-process scatter
   will probably perform better although it involves more data to be moved if regarded
   sequentially (but less data if the total amount is regarded). 
   
   Alternatively, the technique below may be effective for big messages, too, in case of 
   true asynchronous communication which allows, just as short/eager, to receive data w/o
   polling of the MPI-libary (but instead, in this case, reduce another vector).
*/
int MPID_SMI_Reduce_scatter (void *sendbuf, void *recvbuf, int *recvcnts, 
							 struct MPIR_DATATYPE *datatype, MPI_Op op, 
							 struct MPIR_COMMUNICATOR *comm )
{
	MPI_Request req, self_req;
	MPI_Status status;
	MPI_Aint   m_extent;  /* Extent in memory */
	MPI_User_function *uop;
	struct MPIR_OP *op_ptr;

	int   rank, size, rsize, i, j, max_count = 0, count = 0; 
	int   to, from, ncnt = 0, *displs;
	void *buffer;
	int   mpi_errno = MPI_SUCCESS, rc;

	static char myname[] = "MPID_SMI_REDUCE_SCATTER";

	/* Determine the "count" of items to reduce and set the displacements */
	m_extent = datatype->extent;

	/* Get my rank and switch communicators to the hidden collective */
	size = comm->local_group->np;
	rank = comm->local_group->local_rank;
	comm = comm->comm_coll;

	op_ptr = MPIR_GET_OP_PTR(op);
	MPIR_TEST_MPI_OP(op,op_ptr,comm,myname);
	uop  = op_ptr->op;

	/* Allocate the displacements and initialize them */
	ALLOCATE (displs, int *,  size*sizeof(int));
	for (i = 0; i < size; i++) {
		displs[i] = count;
		if (recvcnts[i] > max_count)
			max_count = recvcnts[i];
		
		count += recvcnts[i];
		if (recvcnts[i] < 0) {
			FREE( displs );
			mpi_errno = MPIR_Err_setmsg( MPI_ERR_COUNT, MPIR_ERR_COUNT_ARRAY_NEG,
										 myname, (char *)0, (char *)0, i, recvcnts[i] );
			return mpi_errno;
		}
	}
	
	/* Sanity checks for trivial null-data or single-proces cases.  */
	if (max_count == 0) {
		FREE (displs);
		return mpi_errno;
	}		
	if (size == 1) {
		MPICH_Irecv (recvbuf, recvcnts[rank], datatype->self, rank, MPIR_REDUCE_SCATTER_TAG, 
					 comm->self, &self_req);
		MPICH_Send (sendbuf, recvcnts[rank], datatype->self, rank, MPIR_REDUCE_SCATTER_TAG, comm->self);
		MPICH_Waitall (1, &self_req, &status);

		FREE (displs);
		return mpi_errno;
	}

	/* Check for the maximum sub-vector to communicate - we require that all of them can 
	   be delivered eagerly. If this condition is not met, we do the standard
	   reduce-scatterv-combionation. 
	   Also, this algorithm does not work with only 3 processes due to the loop counter setup. */
	if (max_count*m_extent <= MPID_SMI_min_EAGERSIZE && size != 3) {
		ALIGNEDALLOCATE (buffer, void *, m_extent*recvcnts[rank], 16);
#define DEBUG_REDSCAT 0
		/* Exchange data with the other processes. The processes are split into even and
		   odd ones, which have their own communication patterns:
		   - even: send messages to upper odd processes, then to upper even processes, both in
		   ascending order. Interleave these sends with reduction operation from data received
		   from these same processes. Start with *two consecutive* sends before doing the 
		   first reduction. 
		   - odd: send messages to lower odd processes, then to lower even processes, both in 
		   descending order. Interleave these sends with reduction operation from data received
		   from these same processes. Start with *one* send.

		   This algorithm results in the followin send-to (S-X) and reduce-from (R-X) order
		   for 8 active processes:
		   proc 0: S-1 S-0 S-3 R-1 S-5 R-3 S-7 R-5 S-6 R-7 S-4 R-2 S-2 R-4 R-6
		   proc 1: S-1 S-0 R-0 S-6 R-6 S-4 R-4 S-2 R-2 S-7 R-3 S-5 R-5 S-3 R-7
		   ... and so on - the communication pattern for the procs 2 through 7 is left as an
		   exercise for the reader.
		*/
		rsize = (size % 2 != 0 && size > 4) ? size - 1 : size;
#if DEBUG_REDSCAT
		fprintf (stderr, "[%d] custom reduce_scatter for len = %d, size = %d, rsize = %d\n", rank, 
				 max_count*m_extent, size, rsize);
#endif
		if (rank % 2 != 0) {
			/* Thse are the odd-ranked processes. 
			   First, send data to myself and initialize the recv buffer by this. */
			MPICH_Irecv (recvbuf, recvcnts[rank], datatype->self, rank, MPIR_REDUCE_SCATTER_TAG, 
						 comm->self, &req);
			MPICH_Send ((char *)sendbuf + displs[rank]*m_extent, recvcnts[rank], datatype->self, 
						rank, MPIR_REDUCE_SCATTER_TAG, comm->self); ncnt++;
			MPICH_Waitall (1, &req, &status);
		
			for (i = 1; i < 3; i++) {
				for (j = i; j < rsize; j += 2) {
					to = (rank + rsize - j) % rsize;
					/* Optimization for even number of processes (see above) */
					from = (rsize % 2 == 0) ? (j % 2 != 0) ? to : (rank + j) % rsize
						: /* simple pattern for odd sized groups */ (rank + j) % rsize;
#if DEBUG_REDSCAT
					fprintf (stderr, "[%d] to: %d, from: %d\n", rank, to, from);
#endif
					/* XXX optimize: direct reduce */
					MPICH_Irecv (buffer, recvcnts[rank], datatype->self, from, MPIR_REDUCE_SCATTER_TAG, 
								 comm->self, &req);
					MPICH_Send ((char *)sendbuf + displs[to]*m_extent, recvcnts[to], datatype->self, 
								to, MPIR_REDUCE_SCATTER_TAG, comm->self); ncnt++;
					MPICH_Waitall (1, &req, &status);

					CALL_OP_FUNC (recvbuf, buffer, recvcnts[rank], op_ptr, datatype);

					/* Special case for the case of odd number of processes */
					if (size != rsize && ncnt >= rank && ncnt < rsize) {
#if DEBUG_REDSCAT
						fprintf (stderr, "[%d] from: %d\n", rank, rsize);
#endif
						/* XXX optimize: direct reduce */
						MPICH_Recv (buffer, recvcnts[rank], datatype->self, rsize, MPIR_REDUCE_SCATTER_TAG, 
									comm->self, &status);
#if DEBUG_REDSCAT
						fprintf (stderr, "[%d] to: %d\n", rank, rsize);
#endif
						MPICH_Send ((char *)sendbuf + displs[rsize]*m_extent, recvcnts[rsize], 
									datatype->self, rsize, MPIR_REDUCE_SCATTER_TAG, comm->self);
						CALL_OP_FUNC (recvbuf, buffer, recvcnts[rank], op_ptr, datatype);
						ncnt = rsize;
					}
				}
			}
		} else 
			if (rank != rsize) {
				/* Even-ranked process, but not the one with the highest rank in case 
				   the total number of processes is not even. */
				from = (rank + 1) % rsize;
				to   = (rank + 1) % rsize;
				/* First, send data to my odd partner process (while he sends data to himself); 
				   then send to myself and initialize the recv buffer by this. */
				MPICH_Irecv (buffer, recvcnts[rank], datatype->self, from, MPIR_REDUCE_SCATTER_TAG, 
							 comm->self, &req);
#if DEBUG_REDSCAT
				fprintf (stderr, "[%d] to: %d\n", rank, to, from);
#endif
				MPICH_Send ((char *)sendbuf + displs[to]*m_extent, recvcnts[to], datatype->self, 
							to, MPIR_REDUCE_SCATTER_TAG, comm->self); ncnt++;
				
				MPICH_Irecv (recvbuf, recvcnts[rank], datatype->self, rank, MPIR_REDUCE_SCATTER_TAG, 
							 comm->self, &self_req);
				MPICH_Send ((char *)sendbuf + displs[rank]*m_extent, recvcnts[rank], datatype->self, 
							rank, MPIR_REDUCE_SCATTER_TAG, comm->self); ncnt++;
				MPICH_Waitall (1, &self_req, &status);
				
				for (i = 1; i >= 0; i--) {
					for (j = i; j < rsize - (2-i); j += 2) {
						/* For an even number of processes, we can use an even better balanced
						   communication pattern which reduces potential waiting times. */
						to = (rsize % 2 == 0) ? 
							(j % 2 == 0) ? (rank + rsize - (j+2)) % rsize : (rank + j+2) % rsize : 
							/* simple pattern for odd number of processes */ (rank + j+2) % rsize;
#if DEBUG_REDSCAT
						fprintf (stderr, "[%d] to: %d, from: %d\n", rank, to, from);
#endif
						
						MPICH_Send ((char *)sendbuf + displs[to]*m_extent, recvcnts[to], datatype->self, 
									to, MPIR_REDUCE_SCATTER_TAG, comm->self); ncnt++; 						
						MPICH_Waitall (1, &req, &status);
						CALL_OP_FUNC (recvbuf, buffer, recvcnts[rank], op_ptr, datatype);
						
						/* Special case for the case of odd number of processes */
						if (rsize != size && ncnt >= rank && ncnt < rsize) {
#if DEBUG_REDSCAT
							fprintf (stderr, "[%d] from: %d\n", rank, rsize);
#endif
							MPICH_Recv (buffer, recvcnts[rank], datatype->self, rsize, MPIR_REDUCE_SCATTER_TAG, 
										comm->self, &status);
#if DEBUG_REDSCAT
							fprintf (stderr, "[%d] to: %d\n", rank, rsize);
#endif
							MPICH_Send ((char *)sendbuf + displs[rsize]*m_extent, recvcnts[rsize], 
										datatype->self, rsize, MPIR_REDUCE_SCATTER_TAG, comm->self);
							CALL_OP_FUNC (recvbuf, buffer, recvcnts[rank], op_ptr, datatype);
							ncnt = rsize;
						}
						from = (rank + j+2) % rsize;
						/* XXX optimize: direct reduce */
						MPICH_Irecv (buffer, recvcnts[rank], datatype->self, from, MPIR_REDUCE_SCATTER_TAG, 
									 comm->self, &req);
					}
				}
#if DEBUG_REDSCAT
				fprintf (stderr, "[%d] from: %d\n", rank, from);
#endif
				MPICH_Waitall (1, &req, &status);
				CALL_OP_FUNC (recvbuf, buffer, recvcnts[rank], op_ptr, datatype);
			} else {
				/* The remaining process for an uneven number of processes in the group
				   will be treated differently. He sends his messages out linearly, and then waits
				   for the others to send. */
				MPICH_Irecv (recvbuf, recvcnts[rank], datatype->self, rank, MPIR_REDUCE_SCATTER_TAG, 
							 comm->self, &self_req);
				MPICH_Send ((char *)sendbuf + displs[rank]*m_extent, recvcnts[rank], datatype->self, 
							rank, MPIR_REDUCE_SCATTER_TAG, comm->self);
				MPICH_Waitall (1, &self_req, &status);

				for (i = 0; i < size; i++) {
					if (i == rank)
						continue;
#if DEBUG_REDSCAT
					fprintf (stderr, "[%d] to: %d\n", rank, i);
#endif
					MPICH_Send ((char *)sendbuf + displs[i]*m_extent, recvcnts[i], datatype->self, 
								i, MPIR_REDUCE_SCATTER_TAG, comm->self);
				}
				for (i = 0; i < size; i++) {
					if (i == rank)
						continue;
#if DEBUG_REDSCAT
					fprintf (stderr, "[%d] from: %d\n", rank, i);
#endif
					/* XXX optimize: direct reduce */
					MPICH_Recv (buffer, recvcnts[rank], datatype->self, i, MPIR_REDUCE_SCATTER_TAG, 
								comm->self, &status);
					CALL_OP_FUNC (recvbuf, buffer, recvcnts[rank], op_ptr, datatype);
				}
			}		
	} else {
		ALIGNEDALLOCATE (buffer, void *, m_extent*count, 16);

		/* Reduce to 0, then scatter */
		mpi_errno = comm->collops->Reduce (sendbuf, buffer, count, datatype, op, 0, comm);
		if (mpi_errno == MPI_SUCCESS) {
			rc = comm->collops->Scatterv (buffer, recvcnts, displs, datatype, recvbuf, 
										  recvcnts[rank], datatype, 0, comm);
			if (rc) 
				mpi_errno = rc;
		}
	}
	
	/* Free the temporary buffers */
	ALIGNEDFREE( buffer );
	FREE( displs );
	
	return (mpi_errno);
}


/* This is an improved and more scalable version of MPI_Scan, contributed 
   originally by Jesper Larsson Traeff <traff@ccrl-nece.technopark.gmd.de> */
static int MPID_SMI_Traeff_scan (void *sendbuf, void *recvbuf, int count, struct MPIR_DATATYPE *datatype, 
								 MPI_Op op, struct MPIR_COMMUNICATOR *comm )
{
	MPI_Status status;
	MPI_Request req;
	int        rank, size;
	int        mpi_errno = MPI_SUCCESS;
	MPI_Aint   lb, ub, m_extent;  /* Extent in memory */
	MPI_User_function   *uop;
	struct MPIR_OP *op_ptr;
	MPIR_ERROR_DECL;

	int dd; /* displacement, no of hops to send (power of 2) */
	int rr; /* "round rank" */
	void *tmpbuf;
	
	mpi_comm_err_ret = 0;
	
	/* Nov. 98: Improved O(log(size)) algorithm */
	
	/* See the overview in Collection Operations for why this is ok */
	if (count == 0) 
		return MPI_SUCCESS;
	
	/* Get my rank & size and switch communicators to the hidden collective */
	size = comm->np;
	rank = comm->local_rank;
	comm	   = comm->comm_coll;

	MPIR_Type_get_limits( datatype, &lb, &ub );
	m_extent = datatype->extent;

	op_ptr = MPIR_GET_OP_PTR(op);
	MPIR_TEST_MPI_OP(op,op_ptr,comm,"MPI_SCAN");
	uop	   = op_ptr->op;
	
	MPIR_Op_errno = MPI_SUCCESS;
	
	if (rank > 0) {
		/* allocate temporary receive buffer
		   (needed both in commutative and noncommutative case) */
		ALIGNEDALLOCATE(tmpbuf, void *, m_extent*count, 16);
		tmpbuf = (void *)((char*)tmpbuf-lb);
	}
	if (datatype->is_contig) {
		MEMCPY (recvbuf, sendbuf, m_extent*count);
	} else {
		MPICH_Irecv (recvbuf, count, datatype->self, rank, MPIR_SCAN_TAG, comm->self, &req);
		MPICH_Send (sendbuf, count, datatype->self, rank, MPIR_SCAN_TAG, comm->self);
		MPICH_Waitall (1, &req, &status);
	}
	
	/* compute partial scans */
	rr = rank; dd = 1;
	while ((rr&1) == 1) {
		/* odd "round rank"s receive */
		
		mpi_errno = MPICH_Recv(tmpbuf,count,datatype->self,rank-dd, MPIR_SCAN_TAG,comm->self,&status);
		if (mpi_errno) return mpi_errno;
#ifdef WIN32
		if(op_ptr->stdcall) 
			op_ptr->op_s(tmpbuf, recvbuf, &count, &datatype->self);
		else
#endif
			(*uop)(tmpbuf, recvbuf, &count, &datatype->self);
		
		dd <<= 1; /* dd*2 */
		rr >>= 1; /* rr/2 */
		
		/* Invariant: recvbuf contains the scan of
		   (rank-dd)+1, (rank-dd)+2,..., rank */
	}
	/* rr even, rank==rr*dd+dd-1, recvbuf contains the scan of
	   rr*dd, rr*dd+1,..., rank */
	
	/* send partial scan forwards */
	if (rank+dd<size) {
		mpi_errno = MPICH_Send(recvbuf,count,datatype->self,rank+dd,MPIR_SCAN_TAG, comm->self);
		if (mpi_errno) 
			return mpi_errno;
	}
	
	if (rank-dd >= 0) {
		mpi_errno = MPICH_Recv(tmpbuf,count,datatype->self,rank-dd, MPIR_SCAN_TAG,comm->self,&status);
		if (mpi_errno) 
			return mpi_errno;
#ifdef WIN32
		if(op_ptr->stdcall) 
			op_ptr->op_s(tmpbuf, recvbuf, &count, &datatype->self);
		else
#endif
			(*uop)(tmpbuf, recvbuf, &count, &datatype->self);
		/* recvbuf contains the scan of 0,..., rank */
	}
	
	/* send result forwards */
	do {
		dd >>= 1; /* dd/2 */
	} while (rank+dd>=size);
	
	while (dd>0) {
		mpi_errno = MPICH_Send(recvbuf,count,datatype->self,rank+dd,MPIR_SCAN_TAG, comm->self);
		if (mpi_errno) 
			return mpi_errno;
		dd >>= 1; /* dd/2 */
	}
	
	if (rank>0) {
		/* free temporary receive buffer */
		ALIGNEDFREE((char*)tmpbuf+lb);
	}
	
	/* If the predefined operation detected an error, report it here */
	if (mpi_errno == MPI_SUCCESS && MPIR_Op_errno)
		mpi_errno = MPIR_Op_errno;
	
	return (mpi_errno);
}





/*
 * Oerrides for XEmacs and vim so that we get a uniform tabbing style.
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
