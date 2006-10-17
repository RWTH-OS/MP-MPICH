/* $Id$ */

/* Optimized scatter for small messages */
#include <stdio.h>

#include "smidef.h"
#include "smicoll.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* Gather is hard to optimize - for very small messages with constant latency, 
   message combinining can be done. Larger messages (eager) may benefit from
   a scheduled/serialized send to avoid overload of the destination node. For very
   large messages, the rendez-vous recv-count limitation alreads serves for this
   kind of optimization. */
/* XXX not yet implemented */
int MPID_SMI_Gather (void *sendbuf, int sendcnt, struct MPIR_DATATYPE *sendtype,
					 void *recvbuf, int recvcnt, struct MPIR_DATATYPE *recvtype, 
					 int root, struct MPIR_COMMUNICATOR *comm)
{

  
	return MPI_SUCCESS;
}


int MPID_SMI_Gatherv (void *sendbuf, int sendcnt, struct MPIR_DATATYPE *sendtype,
					  void *recvbuf, int *recvcnts, int *displs, struct MPIR_DATATYPE *recvtype, 
					  int root, struct MPIR_COMMUNICATOR *comm)
{
  return MPI_SUCCESS;
}


/*
 * Shifting Allgather with message-size-doubling (would be extentable to quad, ... )
 * See thesis for detailed algorithm & background
 */
int MPID_SMI_Allgather (void *sendbuf, int sendcnt, struct MPIR_DATATYPE *sendtype,
						void *recvbuf, int recvcnt, struct MPIR_DATATYPE *recvtype, 
						struct MPIR_COMMUNICATOR *comm)
{
    int i, size, retval, *recvcnts, *displcs;

    MPIR_Comm_size ( comm, &size );

    ALLOCATE (recvcnts, int *, size*sizeof(int));
    ALLOCATE (displcs, int *, size*sizeof(int));
    for (i = 0; i < size; i++) {
		recvcnts[i] = recvcnt;
		displcs[i]  = i*recvcnt;
    }

    retval = MPID_SMI_Allgatherv(sendbuf, sendcnt, sendtype, recvbuf, recvcnts, displcs,
								 recvtype, comm);
    FREE(recvcnts); FREE(displcs);
    return (retval);
}


int MPID_SMI_Allgatherv (void *sendbuf, int sendcnt, struct MPIR_DATATYPE *sendtype,
						 void *recvbuf, int *recvcnts, int *displs, struct MPIR_DATATYPE *recvtype,
						 struct MPIR_COMMUNICATOR *comm)
{
    MPID_SMI_comm_info_t *comm_info;
    MPI_Status   status[2];
    MPI_Request  request[2];
    MPI_Aint     send_extent, recv_extent;
    int          lower, upper, size, lrank, partner, i, recv_gaps;
    int          data_from, send_data, send_amount, recv_amount, procs_per_node;
    int          use_SMP, mpi_errno = MPI_SUCCESS;
    static char myname[] = "MPID_SMI_Allgatherv";
   
    MPID_STAT_ENTRY(allgather);

    /* Get size and switch to collective communicator */
    size  = comm->np;
    lrank = comm->local_rank;
    comm_info = (MPID_SMI_comm_info_t *)comm->adiCollCtx;
    comm = comm->comm_coll;

    /* Get extent of send and recv types. We could use MPI_Type_extent, but 
       this also only returns this value. */
    send_extent = sendtype->extent;
    recv_extent = recvtype->extent;

    /* Lock for collective operation 
       MPID_THREAD_LOCK(comm->ADIctx, comm); */

    /* First, send to myself: direct memcpy() for contiguous data, use MPI for non-contignous data */
    if (sendtype->is_contig) {
		MEMCPY ((char *)recvbuf + displs[lrank]*recv_extent, sendbuf, sendcnt*send_extent);
    } else {
		MPICH_Irecv((char *)recvbuf + displs[lrank]*recv_extent, recvcnts[lrank], 
					recvtype->self, lrank, MPIR_ALLGATHER_TAG, comm->self, &request[0]);
		MPICH_Send(sendbuf, sendcnt, sendtype->self, lrank, MPIR_ALLGATHER_TAG, comm->self);
		MPICH_Waitall(1, &request[0], &status[0]);
    }

    if (size == 1) {
		MPID_STAT_EXIT(allgather);
		return MPI_SUCCESS;
    }
  
    /* To use our optimization, we need to check if the recv are pairwise contiguous! 
       This does not need to be so for Allgatherv. */  
    for (i = 0, recv_gaps = 0; i < size; i += 2) {
		if (displs[i] + recvcnts[i] != displs[i+1]) {
			recv_gaps = 1;
			break;
		}
    }

    /* Check if we have an even process-to-node distribution for SMP optimization. For now, we
       only optimize for 2 processes per node. */
    /* XXX move to communicator initialization */
    for( i = 0, procs_per_node = 0, use_SMP = 0; i < MPID_SMI_numNodes; i++ ) {      
		if (comm_info->procs_on_gnode[i] > 0)
			procs_per_node = comm_info->procs_on_gnode[i];
		if (comm_info->procs_on_gnode[i] > 1)
			use_SMP = 1;
		if ((procs_per_node > 0 && comm_info->procs_on_gnode[i] != procs_per_node) 
			|| procs_per_node > 2 || comm_info->nbr_lnodes == 1) {
			use_SMP = 0;
			break;
		}
    }
  
    /* For an even number of procs, we can optimize very well! */
    if (size % 2 == 0 && !recv_gaps) {
		/* initial neighbor data exchange between even and odd processes */
		if (lrank % 2 == 0) { 
			partner = lrank + 1;
			/* I'm even: send to upper neighbor, then recv from him. */
			MPICH_Irecv((char *)recvbuf + displs[partner]*recv_extent, recvcnts[partner], 
						recvtype->self, partner, MPIR_ALLGATHER_TAG, comm->self, &request[0]);
			MPICH_Isend(sendbuf, sendcnt, sendtype->self, partner, MPIR_ALLGATHER_TAG, comm->self, 
						&request[1]);
			MPICH_Waitall (2, request, status);
		} else {
			partner = lrank - 1;
			/* I'm odd: first recv from lower neighbor, then send to him */
			MPICH_Irecv ((char *)recvbuf + displs[partner]*recv_extent, recvcnts[partner], 
						 recvtype->self, partner, MPIR_ALLGATHER_TAG, comm->self, &request[0]);
			MPICH_Isend(sendbuf, sendcnt, sendtype->self, partner, MPIR_ALLGATHER_TAG, comm->self,
						&request[1]);
			MPICH_Waitall (2, request, status);
		}

		if (!use_SMP) {
			/* now the loop begins: always send the last datablock (containing the data of 
			   two(!) processes) to the lower/upper neighbor and receive two datablocks from him */
			upper = (lrank + 2) % size;
			lower = (lrank + size - 2) % size;
	  
			for (i = 0 ; i < size/2 - 1; i++) {
				/* This barrier improves the performance. */
				if (recvcnts[0] * recv_extent > ALLGATHER_BARRIER_LIMIT) {
					MPICH_Barrier( comm->self );
				}
				/* Which data will I recv and send in this round? Determine the rank of the lower
				   process (we always transmit the data of two processes). */
				data_from = (lrank - (lrank%2) - 2*(i+1) + size) % size;
				send_data = (data_from + 2 + size) % size;
	      
				recv_amount = recvcnts[data_from] + recvcnts[data_from+1];
				send_amount = recvcnts[send_data] + recvcnts[send_data+1];
	      
				MPICH_Irecv((char *)recvbuf + displs[data_from]*recv_extent, recv_amount, 
							recvtype->self, lower, MPIR_ALLGATHER_TAG, comm->self, &request[0]);
				MPICH_Isend((char *)recvbuf + displs[send_data]*recv_extent, send_amount, 
							recvtype->self, upper, MPIR_ALLGATHER_TAG, comm->self, &request[1]);
				MPICH_Waitall (2, request, status);
			}
		} else {
#define CONCURRENT_LOCAL 0	  
			/* Apply SMP optimization - ony one process does the job. The even process is the master,
			   the odd process the "local slave". */
			MPI_Status   *smp_status;
			MPI_Request  *smp_request;
	  
			ALLOCATE (smp_status, MPI_Status *, (size/2 - 1)*sizeof(MPI_Status));
			ALLOCATE (smp_request, MPI_Request *, (size/2 - 1)*sizeof(MPI_Request));

			if (lrank % 2 == 0) {
				/* This is the local master, doing the inter-node communication. */
				upper = (lrank + 2) % size;
				lower = (lrank + size - 2) % size;

				for (i = 0 ; i < size/2 - 1; i++) {
					/* This barrier improves the performance. */
					if (recvcnts[0] * recv_extent > ALLGATHER_BARRIER_LIMIT) {
						MPICH_Barrier( comm->self );
					}
					/* Which data will I recv and send in this round? Determine the rank of the lower
					   process (we always transmit the data of two processes). */
					data_from = (lrank - 2*(i+1) + size) % size;
					send_data = (data_from + 2 + size) % size;
		  
					recv_amount = recvcnts[data_from] + recvcnts[data_from+1];
					send_amount = recvcnts[send_data] + recvcnts[send_data+1];
		  
					MPICH_Irecv((char *)recvbuf + displs[data_from]*recv_extent, recv_amount, 
								recvtype->self, lower, MPIR_ALLGATHER_TAG, comm->self, &request[0]);
					MPICH_Isend((char *)recvbuf + displs[send_data]*recv_extent, send_amount, 
								recvtype->self, upper, MPIR_ALLGATHER_TAG, comm->self, &request[1]);
					MPICH_Waitall (2, request, status);
#if CONCURRENT_LOCAL
					/* trigger local distribution of data */
					MPICH_Isend((char *)recvbuf + displs[data_from]*recv_extent, recv_amount, 
								recvtype->self, lrank + 1, MPIR_ALLGATHER_TAG, comm->self, &smp_request[i]);
#endif		  
				}
#if !CONCURRENT_LOCAL
				for (i = 0 ; i < size/2 - 1; i++) {
					send_data = (lrank - 2*(i+1) + size) % size;
					send_amount = recvcnts[send_data] + recvcnts[send_data+1];

					MPICH_Isend((char *)recvbuf + displs[send_data]*recv_extent, send_amount, 
								recvtype->self, lrank + 1, MPIR_ALLGATHER_TAG, comm->self, &smp_request[i]);	  
				}
#endif
				MPICH_Waitall (size/2 - 1, smp_request, smp_status);
			} else {
				/* This is a local slave. */
				upper = -1;
				lower = lrank - 1;

				for (i = 0 ; i < size/2 - 1; i++) {
					data_from = (lrank - 1 - 2*(i+1) + size) % size;
					recv_amount = recvcnts[data_from] + recvcnts[data_from+1];

					/* This process needs to take part in the barrier, too. */
					if (recvcnts[0] * recv_extent > ALLGATHER_BARRIER_LIMIT) {
						MPICH_Barrier( comm->self );
					} else {
#if CONCURRENT_LOCAL
						if (i > 0)
							MPICH_Waitall(1, &smp_request[i-1], &smp_status[i-1]);
#endif
					}
					/* Recv data from master process. */
					MPICH_Irecv((char *)recvbuf + displs[data_from]*recv_extent, recv_amount, 
								recvtype->self, lower, MPIR_ALLGATHER_TAG, comm->self, &smp_request[i]);
		  
				}
				/* If we took part in the barrier, we need to finish *all* the recvs here. */
#if !CONCURRENT_LOCAL
				if (recvcnts[0] * recv_extent > ALLGATHER_BARRIER_LIMIT) {
					MPICH_Waitall (size/2 - 1, smp_request, smp_status);
				} else {
					MPICH_Waitall (1, &smp_request[i-1], &smp_status[i-1]);
				}
#else 
				MPICH_Waitall (size/2 - 1, smp_request, smp_status);		  
#endif
			}
			FREE( smp_status ); FREE( smp_request );
		}
    } else {
	/* Uneven nbr of processes: this case can not be optimized this good. A modified
	   version of the algoritm used above could be devloped (which would need to treat
	   many exceptions), but for now, we just use the original MPICH algorithm. */
	upper = (lrank + 1) % size; 
	lower = (size + lrank - 1) % size;

	send_data = lrank;
	data_from = lower;
	for (i = 1; i < size; i++) {
	    MPICH_Irecv ((char *)recvbuf + displs[data_from]*recv_extent, recvcnts[data_from], recvtype->self, 
			 lower, MPIR_ALLGATHERV_TAG, comm->self, &request[0]);
	    MPICH_Isend ((char *)recvbuf+displs[send_data]*recv_extent, recvcnts[send_data], recvtype->self, 
			 upper, MPIR_ALLGATHERV_TAG,  comm->self, &request[1]);
	    MPICH_Waitall (2, request, status);

	    send_data = data_from;
	    data_from = (data_from - 1 + size) % size;
	}

	}    

    MPID_STAT_EXIT(allgather);
    return (mpi_errno);
}


/* This is the alogrithm from MPICH, which isn't too bad, either. */
int MPID_SMI_Allgatherv_sendrecv (void *sendbuf, int sendcnt, struct MPIR_DATATYPE *sendtype,
								  void *recvbuf, int *recvcnts, int *displs, struct MPIR_DATATYPE *recvtype,
								  struct MPIR_COMMUNICATOR *comm)
{
	MPI_Request req;
	MPI_Status  status;
	MPI_Aint    recv_extent;
	int        size, rank;
	int        mpi_errno = MPI_SUCCESS;
	int        j, jnext, i, right, left;
	
	/* Get the size of the communicator */
	size = comm->np;
	rank = comm->local_rank;
	recv_extent = recvtype->extent;
	comm = comm->comm_coll;
	
	/* Do a gather for each process in the communicator
	   This is the "circular" algorithm for allgatherv - each process sends to
	   its right and receives from its left.  This is faster than simply
	   doing size Gathervs. */

	/* First, load the "local" version in the recvbuf. */
	MPICH_Irecv ((char *)recvbuf + displs[rank]*recv_extent, recvcnts[rank], recvtype->self, rank,
				 MPIR_ALLGATHERV_TAG, comm->self, &req);
	MPICH_Send ( sendbuf, sendcnt, sendtype->self, rank, MPIR_ALLGATHERV_TAG, comm->self);
	MPICH_Waitall (1, &req, &status);

	left  = (size + rank - 1) % size;
	right = (rank + 1) % size;
	
	j = rank;
	jnext = left;
	for (i = 1; i < size; i++) {
		MPICH_Irecv ((char *)recvbuf + displs[jnext]*recv_extent, recvcnts[jnext], recvtype->self, left,
					 MPIR_ALLGATHERV_TAG, comm->self, &req);
		MPICH_Send ( (char *)recvbuf+displs[j]*recv_extent, recvcnts[j], recvtype->self, right, 
					 MPIR_ALLGATHERV_TAG, comm->self);
		MPICH_Waitall (1, &req, &status);

		j = jnext;
		jnext = (size + jnext - 1) % size;
	}
	
	return (mpi_errno);
}

/* This is the alogrithm from MPICH, optimized for short messages. The optimization is to
   do not perform a sequence check after *each* short messsage, but only once for all messages.
   The error probability, esp. for short messages, is small enough to justify this approach. */
int MPID_SMI_Allgatherv_short (void *sendbuf, int sendcnt, struct MPIR_DATATYPE *sendtype,
							   void *recvbuf, int *recvcnts, int *displs, struct MPIR_DATATYPE *recvtype,
							   struct MPIR_COMMUNICATOR *comm)
{
	MPI_Request req;
	MPI_Status  status;
	MPI_Aint    recv_extent;
	int        size, rank;
	int        mpi_errno = MPI_SUCCESS;
	int        j, jnext, i, right, left;
	
	/* Get the size of the communicator */
	size = comm->np;
	rank = comm->local_rank;
	recv_extent = recvtype->extent;
	comm = comm->comm_coll;
	
	/* Do a gather for each process in the communicator
	   This is the "circular" algorithm for allgatherv - each process sends to
	   its right and receives from its left.  This is faster than simply
	   doing size Gathervs. */

	/* First, load the "local" version in the recvbuf. */
	MPICH_Irecv ((char *)recvbuf + displs[rank]*recv_extent, recvcnts[rank], recvtype->self, rank,
				 MPIR_ALLGATHERV_TAG, comm->self, &req);
	MPICH_Send ( sendbuf, sendcnt, sendtype->self, rank, MPIR_ALLGATHERV_TAG, comm->self);
	MPICH_Waitall (1, &req, &status);

	left  = (size + rank - 1) % size;
	right = (rank + 1) % size;
	
	j = rank;
	jnext = left;
	for (i = 1; i < size; i++) {
		MPICH_Irecv ((char *)recvbuf + displs[jnext]*recv_extent, recvcnts[jnext], recvtype->self, left,
					 MPIR_ALLGATHERV_TAG, comm->self, &req);

		MPICH_Send ( (char *)recvbuf+displs[j]*recv_extent, recvcnts[j], recvtype->self, right, 
					 MPIR_ALLGATHERV_TAG, comm->self);
		MPICH_Waitall (1, &req, &status);

		j = jnext;
		jnext = (size + jnext - 1) % size;
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

