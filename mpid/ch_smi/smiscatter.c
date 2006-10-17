/* $Id$ */

/* Optimized scatter for small messages */
#include <stdio.h>

#include "smidef.h"
#include "smicoll.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* Optimization of Scatter(v): for small chunk sizes n and np processes, sending np*n 
   individual messages to each process can take considerably longer than sending the 
   same data in fewer messages. This is always true for the short protocol, and up to a 
   certain message size this holds true for the eager protocol, too. Therefore, we do a 
   broadcast-style binominal-tree-wise distribtion of the data - this involves 
   sending more data in total, but reduces the number of sequential messages from 
   np down to ld(np). The actual reduction of the latency depends on the latency ratio
   of the original (smaller sized) and the new (bigger sized) messages. */
int MPID_SMI_Scatter (void *sendbuf, int sendcnt, struct MPIR_DATATYPE *sendtype, 
					  void *recvbuf, int recvcnt, struct MPIR_DATATYPE *recvtype, int root, 
					  struct MPIR_COMMUNICATOR *comm )
{
    MPI_Status  status;
    MPI_Request req;
    MPI_Aint    send_size, recv_size, send_extent, recv_extent, count, displs;
    int         rank, rel_rank, size, mask, is_leaf, dst, src, msglen;
    int         mpi_errno = MPI_SUCCESS;
    static char myname[] = "MPID_SMI_SCATTER";
    char       *rbuf, *tbuf;
	MPI_Datatype ttype; 
	
    size = comm->np;
    rank = comm->local_rank;

    /* Check for invalid arguments */
#ifndef MPIR_NO_ERROR_CHECKING
    if (root >= size)
		mpi_errno = MPIR_Err_setmsg(MPI_ERR_ROOT, MPIR_ERR_ROOT_TOOBIG,
									myname,(char *)0,(char *)0,root,size);
    if (root < 0) 
		mpi_errno = MPIR_Err_setmsg(MPI_ERR_ROOT,MPIR_ERR_DEFAULT,myname,
									(char *)0,(char *)0,root);
    if (mpi_errno)
		return MPIR_ERROR(comm, mpi_errno, myname );
#endif

    /* Get size & extent of send and recv types. We could use MPI_Type_extent/_size, but 
       this also only returns this value. */
    send_size = sendtype->size;
    recv_size = recvtype->size;
    send_extent = sendtype->extent;
	recv_extent = recvtype->extent;

    /* Check for length of data to send - this optimization is only effective up 
       to a certain (system-dependant) message length. Use standard scatter()-function
       for message sizes above. */
    msglen = (rank == root) ? sendcnt*send_size : recvcnt*recv_size;
    if (msglen > MPID_SMI_cfg.COLL_SCATTER_MAX)
		return MPID_SMI_Scatter_seq (sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root, comm);
	
    /* Switch communicators to the hidden collective */
    comm = comm->comm_coll;
    rel_rank = (rank >= root) ? rank - root : rank - root + size;
#define SCATTER_DEBUG 0
#define ABS_RANK(r) (((r) + root) % size)
    is_leaf = (rel_rank != 0);
    /* If real rank of root is not 0, we have a problem: the second half of the
       vector being sent is not contiguous, but wraps around the end of the vector.
       We can only handle this by copying the data into a temporary buffer which makes
       this buffer fit well with the 'relative ranks'. Of course, this does have a negative
       impact on performance! */
    if (rel_rank == 0 && root != 0) {
		ALLOCATE (tbuf, char *, size*sendcnt*send_extent);
		MEMCPY(tbuf, (char *)sendbuf + rank*sendcnt*send_extent, (size - rank)*sendcnt*send_extent);
		MEMCPY(tbuf + (size - rank)*sendcnt*send_extent, sendbuf, rank*sendcnt*send_extent);
    } else 
		tbuf = (rel_rank == 0) ? (char *)sendbuf : NULL;

    mask = 0x1;
    while (mask < size) {
		if (rel_rank & mask) {
			/* If this process is not a leaf in the tree, then it needs to recv more data 
			   than its recv buffer can hold - we need to allocate a temporary buffer. */
			if (mask != 0x1 && rel_rank != size - 1) {
				is_leaf = 0;

				count = MPID_MIN(mask*recvcnt, (size - rel_rank)*recvcnt);
				ALLOCATE(tbuf, char *, recv_size*count);
				rbuf = tbuf;
			} else {
				rbuf = (char *)recvbuf;
				count = recvcnt;
			}
			src = (rel_rank + size - mask) % size; 
#if SCATTER_DEBUG
			fprintf (stderr, "[%d|%d] recv %d elmnts from %d (root = %d, am_leaf = %d)\n",
					 rank, rel_rank, count, src, root, is_leaf);
#endif
			mpi_errno = MPICH_Recv(rbuf, count, recvtype->self, ABS_RANK(src), 
								   MPIR_SCATTER_TAG, comm->self, &status);
			if (mpi_errno) 
				return mpi_errno;
			break;
		}
		mask <<= 1;
    }

    mask >>= 1;
    while (mask > 0) {
		if (rel_rank + mask < size) {
			dst = (rel_rank + mask) % size;
			/* Determine the data to send: it is 'mask' unless there are not
			   enough processes 'at the end' of the communicator.
			   The data layout in tbuf is sendcnt/sendtype for the root process and recvcnt/recvtype for the others */
			if( rel_rank == 0 ) {
				count = (dst + mask > size) ? (size - dst)*sendcnt : mask*sendcnt;
				ttype = sendtype->self;
				displs = (dst - rel_rank + size) % size;
				displs *= sendcnt * send_extent;
			}
			else {
				count = (dst + mask > size) ? (size - dst)*recvcnt : mask*recvcnt;
				ttype = recvtype->self;
				displs = (dst - rel_rank + size) % size;
				displs *= recvcnt * recv_extent;
			}
#if SCATTER_DEBUG
			fprintf (stderr, "[%d|%d] send %d elmnts, displs %d,  to %d (root = %d, am_leaf = %d)\n",
					 rank, rel_rank, count, displs, dst, root, is_leaf);
#endif
			mpi_errno = MPICH_Send (tbuf + displs, count, ttype, ABS_RANK(dst), 
									MPIR_SCATTER_TAG, comm->self);
			if (mpi_errno) 
				return mpi_errno;
		}
		mask >>= 1;
    }
#undef ABS_RANK

    /* Finally, extract the data for the local process from the temporary buffer for the 
       processes which are not leaves. */
    if (!is_leaf) {
		MPICH_Irecv (recvbuf, recvcnt, recvtype->self, rank, MPIR_SCATTER_TAG, comm->self, &req);
		/* The data layout in tbuf is sendcnt/sendtype for the root process and recvcnt/recvtype for the others */
		if( rel_rank == 0 )
			MPICH_Send (tbuf, sendcnt, sendtype->self, rank, MPIR_SCATTER_TAG, comm->self);
		else
			MPICH_Send (tbuf, recvcnt, recvtype->self, rank, MPIR_SCATTER_TAG, comm->self);
		mpi_errno = MPICH_Waitall (1, &req, &status);
    }

    if (tbuf != NULL && (rel_rank != 0 || (rel_rank == 0 && root != 0)))
		FREE(tbuf);

    return (mpi_errno);
}

/* Default sequential tree from MPICH code. */
int MPID_SMI_Scatter_seq (void *sendbuf, int sendcnt, struct MPIR_DATATYPE *sendtype, 
						  void *recvbuf, int recvcnt, struct MPIR_DATATYPE *recvtype, int root, 
						  struct MPIR_COMMUNICATOR *comm )
{
    MPI_Status status;
    int        rank, size;
    int        mpi_errno = MPI_SUCCESS;
    static char myname[] = "MPID_SMI_SCATTER_SEQ";
	
    size = comm->np;
    rank = comm->local_rank;
	
    /* Switch communicators to the hidden collective */
    comm = comm->comm_coll;
	
    if (rank == root) {
		MPI_Request req;
		MPI_Aint extent;
		int      dst;
		
		extent = sendtype->extent;

		for (dst = 0; dst < root; dst++) {
			mpi_errno = MPICH_Send((char *)sendbuf + dst*sendcnt*extent, sendcnt, sendtype->self, dst, 
								   MPIR_SCATTER_TAG, comm->self);
			if (mpi_errno) 
				return mpi_errno;
		}
		MPICH_Irecv (recvbuf, recvcnt, recvtype->self, rank, MPIR_SCATTER_TAG, comm->self, &req);
		MPICH_Send ((char *)sendbuf + rank*sendcnt*extent, sendcnt, sendtype->self, rank, 
					MPIR_SCATTER_TAG, comm->self);
		mpi_errno = MPICH_Waitall (1, &req, &status);
		if (mpi_errno) 
			return mpi_errno;
		
		for (dst = root+1; dst < size; dst++) {
			mpi_errno = MPICH_Send((char*)sendbuf + dst*sendcnt*extent, sendcnt, sendtype->self, dst, 
								   MPIR_SCATTER_TAG, comm->self);
			if (mpi_errno) 
				return mpi_errno;
		}
    } else
		mpi_errno = MPICH_Recv(recvbuf, recvcnt, recvtype->self, root, MPIR_SCATTER_TAG, 
							   comm->self, &status);
	
    return (mpi_errno);
}


/* The problem with Scatterv is, that only root knows the recvcnt of all other 
   processes; the individual processes do only know their own recvcnt. This means
   that root would need to send this information along with the data, which is 
   impractical. Two possible work-arounds:
   - root sends to j other processes {r1,..., rj} , which in turn send the data minus 
   their own data to process ri + 1. Result: j + np/(j-1) sequential messages
   instead of np sequential messages (MULTI_SEQTREE)
   - Send the relevant part of the count-array out-of-band, using shared memory via
   SMI_Send / SMI_Recv (very low latency - which is important to get any benefit)
   and proceed as with Scatter() for the algorithm. Problem: additional (minor)
   latency and size-limititation of SMI-messages.

   Furthermore, we need to guarantee that the blocks of elements that we send to 
   a process (to have it send it on towards the final destination) is correctly 
   described by a single displacement and element count. */
#define MULTI_SEQTREE 1

int MPID_SMI_Scatterv (void *sendbuf, int *sendcnts, int *displs, struct MPIR_DATATYPE *sendtype, 
					   void *recvbuf, int recvcnt,  struct MPIR_DATATYPE *recvtype, 
					   int root, struct MPIR_COMMUNICATOR *comm )
{
    MPI_Aint    send_size, recv_size, send_extent, recv_extent;
    int         rank, rel_rank, size, is_leaf, scnt, i, j;
    int         max_treelen, *treelen, treelen_rest, *tree_sndcnts=0, tree_recvcnt;
    int         mpi_errno = MPI_SUCCESS;
    static char myname[] = "MPID_SMI_SCATTERV";
    char       *tbuf;
  
    /* Get size and rank */
    size = comm->np;
    rank = comm->local_rank;

    /* Check for invalid arguments */
#ifndef MPIR_NO_ERROR_CHECKING
    if (root >= size)
		mpi_errno = MPIR_Err_setmsg(MPI_ERR_ROOT, MPIR_ERR_ROOT_TOOBIG,
									myname,(char *)0,(char *)0,root,size);
    if (root < 0) 
		mpi_errno = MPIR_Err_setmsg(MPI_ERR_ROOT,MPIR_ERR_DEFAULT,myname,
									(char *)0,(char *)0,root);
    if (mpi_errno)
		return MPIR_ERROR(comm, mpi_errno, myname );
#endif

    /* Get extent of send and recv types. We could use MPI_Type_extent, but 
       this also only returns this value. */
    send_extent = sendtype->extent;
    recv_extent = recvtype->extent;
    send_size = sendtype->size;
    recv_size = recvtype->size;

    /* Check if we can apply our optimiziation - if not, send the data sequentially.
       However, as this is only known to root, the non-root processes will recv 
       the same way in any case. They will know by checking the recv status - if they
       only the amount of data they expect for themselves, it is clear that no data
       has to be passed on. 
       Only root can do this checks - the other procs don't have the required information.
       See below how the get notified on the choice of the algorithm. 

       XXX We could limit the test to the part of the data we actually need to send as
       a contiguous block. */
    for (scnt = 0, i = 0; i < size-1; i++) {
		if (scnt + sendcnts[i] != displs[i+1]) 
			return MPID_SMI_Scatterv_seq (sendbuf, sendcnts, displs, sendtype, recvbuf,
										  recvcnt, recvtype, root, comm);
		scnt += sendcnts[i];
    }
    scnt += sendcnts[i];
    /* Concerning the length,  we decide upon the arithmetic average of the lenghts. This
       is not optimal, but does there exist an optimal solution? 
       XXX For now, limit the algorithm to root == 0 */
    if (scnt/size*send_size > MPID_SMI_cfg.COLL_SCATTER_MAX || root != 0)
		return MPID_SMI_Scatterv_seq (sendbuf, sendcnts, displs, sendtype, recvbuf,
									  recvcnt, recvtype, root, comm);
    
    /* Switch communicators to the hidden collective */
    comm = comm->comm_coll;
    rel_rank = (rank >= root) ? rank - root : rank - root + size;
#define SCATTERV_DEBUG 0
#define ABS_RANK(r) (((r) + root) % size)
    is_leaf = (rel_rank != 0);
    tbuf = (rel_rank == 0) ? (char *)sendbuf : NULL;

#if MULTI_SEQTREE
    /* This is the algorithm with multiple sequential trees. First, decide on how many 
       sequential tree's we want to create. We first do an estimation based upon 
       SUM(i=1...n) = n*(n+1)/2 and find the exakt number from there. */
    max_treelen = size;
    while (max_treelen*max_treelen > 2*size)
		max_treelen >>= 1;    
    do {
		for (j = 0, i = 1; i < max_treelen; i++)
			j += i;
		max_treelen++;
    } while (j + max_treelen < size);
    max_treelen--;
    treelen_rest = j;

    ZALLOCATE (treelen, int *, size*sizeof(int));
    for (i = 0, j = 1; max_treelen > 0; max_treelen--) {
		j += max_treelen;
		/* tree_sndcnts seems to be uninitialized .... JOACHIM ???? */
		tree_sndcnts[size - j] = max_treelen;
		if (treelen_rest > 0 && treelen_rest <= max_treelen) {
			treelen[size - j]++;
			treelen_rest--;
		}
    }

    /* From now on, we only work with relative ranks as we want the root to have rank 0. */
    if (rel_rank == 0) {
		for (i = size - 1; i > 0; i--) {
			if (treelen[i] > 0) {
				/* Calculate length of partial vector to send, and send it. */
			}
		}
    } else 
		if (rel_rank = size - 1 || treelen[rel_rank + 1] != 0) {
			/* This process is at the end of a sequential tree and only recvs it's own data. */
	
		} else {
			/* This process is starts or continuous a sequential tree. Calculate the amount of
			   data to recv, recv it and send on the part of the data excluding my own data. */
			if (treelen[rel_rank] != 0)
				tree_recvcnt = treelen[rel_rank];
			else {
				/* Determine the lenght of the seq. tree this proc belongs to, and determine the length
				   to recv by measuring the distance between this seq. tree's root and itself. */
				for (i = rel_rank-1; treelen[i] == 0; i--)
					;
				tree_recvcnt = treelen[i] - (rel_rank - i);
			}
		}
#else 
    mask = 0x1;
    while (mask < size) {
		if (rel_rank & mask) {
			src = (rel_rank + size - mask) % size; 
			/* If this process is not a leaf in the tree, than it needs to recv more data 
			   than it's recv buffer can hold - we need to allocate a temporary buffer. */
			if (mask != 0x1 && rel_rank != size - 1) {
				is_leaf = 0;

				MPICH_Recv (sendcnt_buf, mask, MPI_INT, src, MPIR_SCATTERV_TAG, comm->self, &req);

				count = MPID_MIN(mask*recvcnt, (size - rel_rank)*recvcnt);
				ALLOCATE(tbuf, char *, recv_size*count);
				rbuf = tbuf;
			} else {
				rbuf = (char *)recvbuf;
				count = recvcnt;
			}
#if SCATTERV_DEBUG
			fprintf (stderr, "[%d|%d] recv %d elmnts from %d (root = %d, am_leaf = %d)\n",
					 rank, rel_rank, count, src, root, is_leaf);
#endif
			mpi_errno = MPICH_Recv(rbuf, count, recvtype->self, ABS_RANK(src), 
								   MPIR_SCATTERV_TAG, comm->self, &status);
			if (mpi_errno) 
				return mpi_errno;
			/* If the root has decided to do a sequential-tree scatterv, it sends only
			   the data for a single process - check for this and exit if required. */
			if (status.count == recvcnt) {
				MPICH_Irecv (recvbuf, recvcnt, recvtype->self, rank, MPIR_SCATTERV_TAG, comm->self, &req);
				MPICH_Send (tbuf, recvcnt, recvtype->self, rank, MPIR_SCATTERV_TAG, comm->self);
				mpi_errno = MPICH_Waitall (1, &req, &status);

				return mpi_errno;
			}
			break;
		}
		mask <<= 1;
    }

    mask >>= 1;
    while (mask > 0) {
		if (rel_rank + mask < size) {
			dst = (rel_rank + mask) % size;
			/* Determine the data to send: it is 'mask' unless there are not
			   enough processes 'at the end' of the communicator. */
			for (count = 0, i = dst; i < size && i < dst + mask; i++)
				count += sendcnt[i];
			for (snd_displs = 0, i = rel_rank; i < dst; i++)
				snd_displs += sendcnt[i];
		  
#if SCATTERV_DEBUG
			fprintf (stderr, "[%d|%d] send %d elmnts, displs %d,  to %d (root = %d, am_leaf = %d)\n",
					 rank, rel_rank, count, displs, dst, root, is_leaf);
#endif
			mpi_errno = MPICH_Send (tbuf + snd_displs*sendcnt*send_extent, count, sendtype->self, ABS_RANK(dst), 
									MPIR_SCATTERV_TAG, comm->self);
			if (mpi_errno) 
				return mpi_errno;
		}
		mask >>= 1;
    }
#undef ABS_RANK

    /* Finally, extract the data for the local process from the temporary buffer for the 
       processes which are not leaves. */
    if (!is_leaf) {
		MPICH_Irecv (recvbuf, recvcnt, recvtype->self, rank, MPIR_SCATTERV_TAG, comm->self, &req);
		MPICH_Send (tbuf, sendcnt, sendtype->self, rank, MPIR_SCATTERV_TAG, comm->self);
		mpi_errno = MPICH_Waitall (1, &req, &status);
    }

    if (tbuf != NULL && (rel_rank != 0 || (rel_rank == 0 && root != 0)))
		FREE(tbuf);
#endif  /* MULTI_SEQTREE */
	
    return (mpi_errno);
}


/* Default sequential tree from MPICH code. */
int MPID_SMI_Scatterv_seq (void *sendbuf, int *sendcnts, int *displs, struct MPIR_DATATYPE *sendtype, 
						   void *recvbuf, int recvcnt,  struct MPIR_DATATYPE *recvtype, 
						   int root, struct MPIR_COMMUNICATOR *comm )
{
    MPI_Status status;	
    int        rank, size;
    int        mpi_errno = MPI_SUCCESS;
    static char myname[] = "MPID_SMI_SCATTERV_SEQ";
	
    size = comm->np;
    rank = comm->local_rank;
	
    /* Switch communicators to the hidden collective */
    comm = comm->comm_coll;
	
    if ( rank == root ) {
		MPI_Request req;
		MPI_Aint extent;
		int      i;
		
		extent = sendtype->extent;
		/* We could use Isend here, but since the receivers need to execute
		   a simple Recv, it may not make much difference in performance, 
		   and using the blocking version is simpler */
		for (i = 0; i < root; i++) {
			mpi_errno = MPICH_Send((char *)sendbuf+displs[i]*extent, sendcnts[i], 
								   sendtype->self, i, MPIR_SCATTERV_TAG, comm->self);
			if (mpi_errno) 
				return mpi_errno;
		}
		MPICH_Irecv (recvbuf, recvcnt, recvtype->self, rank, MPIR_SCATTERV_TAG, comm->self, &req);
		MPICH_Send ((char *)sendbuf + displs[rank]*extent, sendcnts[rank], sendtype->self, rank, 
					MPIR_SCATTERV_TAG, comm->self);
		mpi_errno = MPICH_Waitall (1, &req, &status);
		if (mpi_errno) 
			return mpi_errno;
		
		for (i = root+1; i < size; i++) {
			mpi_errno = MPICH_Send((char *)sendbuf + displs[i]*extent, sendcnts[i], 
								   sendtype->self, i, MPIR_SCATTERV_TAG, comm->self);
			if (mpi_errno) 
				return mpi_errno;
		}
    } else
		mpi_errno = MPICH_Recv(recvbuf, recvcnt, recvtype->self, root,
							   MPIR_SCATTERV_TAG, comm->self, &status);
	
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

