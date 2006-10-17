/* $Id$ */

/* MPI_Alltoall/MPI_Alltoallv optimized for SCI torus
   topologies (of any dimension 1, 2, 3, ...).

   A semi-heuristic approach (dynamic programming) is chosen:
   for each communicator, the complete communication pattern 
   is pre-calculated on the creation of the communicator. Each
   process calculates the full communication in the network
   and remembers its part in it. 

   The calculation is based on a reprenstation of the nodes,
   each with an arbitraty number of active processes, within
   the n-dimensional torus topology. Two different communication
   algorithms have been implemented:
   - "plain" algorithm which build communication pairs as they
     come, without considering the SCI topology.
   - "topology" algorithm which considers the given SCI topology
      between the processes involved and tries to distribute the traffic
      evenly across the network when building the communication
	  pattern.

   Next to these both, there is of course the "dumb" MPICH-algorithm,
   which is used for small messages.
  */

#include "smidef.h"
#include "smicoll.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#define PAIR_DEBUG     0


/*
 * Generic auxilary functions for both algorithms.
 */
static int *actual_choice, **already_chosen;

/* Check if a specified node has already been chosen as communication 
   partner in this round. Return values:
   -1     : node has not yet been chosen;
   N >= 0 : node has already been chosen by node N */
static int node_was_chosen( MPID_SMI_comm_info_t *ci, int lnode )
{
	int n = 0, retval;
    
	while (n < ci->nbr_lnodes && actual_choice[n] != lnode)
		n++;

	retval = (n < ci->nbr_lnodes) ? n : -1;

	return retval;
}

/* Check if any two nodes are left to build a pair for this round. 
   'do_SMP' specified if intra-node communication (SMP) is to be
   considered or not.
   Return values:
   0 :  all possible pairs have been built;
   1 :  at least one pair is left to be built */
static int some_node_left(MPID_SMI_comm_info_t *ci, boolean allow_SMP)
{
	int retval = 0, m, n;
    
	for (m = 0; m < ci->nbr_lnodes; m++) {
		for (n = 0; n < ci->nbr_lnodes; n++) {
			if ((allow_SMP || m != n) && !already_chosen[m][n])
				retval = 1;
		}
	}
	return retval;
}


/*
 * "Plain" alltoall-algorithm (doesn't consider the SCI topology) below.
 */

 /* Pre-calculate the communication pattern for the alltoall-communication
   the simple way (plain = without considering topology). */
void MPID_SMI_alltoall_init_plain( MPID_SMI_comm_info_t *ci )
{
	int i, j, round = 0, choice, node, power_of_two;

	/* calculate number of rounds in which to communicate between nodes */
	power_of_two = MPID_SMI_msb( ci->nbr_lnodes );
	power_of_two = ( power_of_two == ci->nbr_lnodes ) ? power_of_two : power_of_two * 2;
	ci->alltoall.nbr_rounds = power_of_two - 1;
	/* allocate array to store information about with which node to communicate in each round */
	if (ci->alltoall.nbr_rounds > 0) {
		ALLOCATE( ci->alltoall.send_to, int *, ci->alltoall.nbr_rounds * sizeof(int));
		ALLOCATE( ci->alltoall.recv_from, int *, ci->alltoall.nbr_rounds * sizeof(int));
	}

	/* Allocate & initialize temporary arrays that we need to calculate 
	   content of ci->send_to ( ci->recv_from */
	ALLOCATE( actual_choice, int *, ci->nbr_lnodes * sizeof(int));
	ALLOCATE( already_chosen, int **, ci->nbr_lnodes * sizeof(int *));
	for (i = 0; i < ci->nbr_lnodes; i++ )
		ALLOCATE( already_chosen[i], int *, ci->nbr_lnodes * sizeof(int *));

	for (i = 0; i < ci->nbr_lnodes; i++) {
		actual_choice[i] = -1;
		for (j = 0; j < ci->nbr_lnodes; j++)
			already_chosen[i][j] = 0;
	}

	/* Loop until all possible pairs have been built: */
	while (some_node_left(ci, false)) {
		/* Each node has to choose its communication partner: */
		for (node = 0; node < ci->nbr_lnodes; node++) {
			/* Has this node already been chosen by another node ? */
			if ((choice = node_was_chosen(ci, node)) == -1) {
				/* This node has no partner yet, so we look for a possible partner: */
				i = 0;
				while ((i < ci->nbr_lnodes) && (choice == -1)) {
					if (i != node) {  /* node shall not communcate with itself */
						/* Has this node already communicated with node i? */
						if (!already_chosen[node][i]) {
							/* Has node i already been chosen by another node ? */
							if (node_was_chosen(ci, i) == -1) {
								/* No, so this is our partner. */
								already_chosen[node][i] = 1;
								choice = i;
								actual_choice[node] = i;
							}
						}
					}
					i++;
				}
			} else {
				/* This node has already been chosen, so we match it with the partner
				   who has chosen this node: */
				already_chosen[node][choice] = 1;
				actual_choice[node] = choice;
			}
		}
		/* Save my own choice for this round and initialize actual_choice for next round: */
		ci->alltoall.send_to[round]   = actual_choice[ci->lnode];
		ci->alltoall.recv_from[round] = actual_choice[ci->lnode];
		for (i = 0; i < ci->nbr_lnodes; i++)
			actual_choice[i] = -1;
		round++;
	}

	for ( i = 0; i < ci->nbr_lnodes; i++)
		FREE( already_chosen[i] );
	FREE( already_chosen );
	FREE( actual_choice );
}

void MPID_SMI_alltoall_destroy_plain (MPID_SMI_comm_info_t *ci)
{
	if (ci->alltoall.send_to != NULL)
		FREE( ci->alltoall.send_to );	
	if (ci->alltoall.recv_from != NULL)
		FREE( ci->alltoall.recv_from );	

	return;
}


int MPID_SMI_Alltoall_plain (void *sendbuf, int sendcount, struct MPIR_DATATYPE *sendtype, 
							 void *recvbuf, int recvcount, struct MPIR_DATATYPE *recvtype, 
							 struct MPIR_COMMUNICATOR *comm)
{
	MPID_SMI_comm_info_t *ci;
	int *int_array, *sdispls, *rdispls, *scounts, *rcounts;
	int mpi_errno, i;

	ci = (MPID_SMI_comm_info_t *)(comm->adiCollCtx);
	ALLOCATE (int_array, int *, 4 * ci->lsize * sizeof(int));
	sdispls = &int_array[0];
	rdispls = &int_array[ci->lsize];
	scounts = &int_array[2*ci->lsize];
	rcounts = &int_array[3*ci->lsize];
	for (i = 0; i < ci->lsize; i++) {
		sdispls[i] = i*sendcount;
		rdispls[i] = i*recvcount;
		scounts[i] = sendcount;
		rcounts[i] = recvcount;
	}

	mpi_errno = MPID_SMI_Alltoallv_plain (sendbuf, scounts, sdispls, sendtype, 
										  recvbuf, rcounts, rdispls, recvtype, comm);

	FREE (int_array);
	return (mpi_errno);
}


int MPID_SMI_Alltoallv_plain (void *sendbuf, int *sendcnts, int *sdispls, struct MPIR_DATATYPE *sendtype, 
							  void *recvbuf, int *recvcnts, int *rdispls, struct MPIR_DATATYPE *recvtype,
							  struct MPIR_COMMUNICATOR *comm)
{
	MPI_Aint     send_extent, recv_extent;
	MPI_Request  rem_req[2], *reqarray;
	MPI_Status   rem_stat[2], *statarray;
	MPID_SMI_comm_info_t *ci;
	int p, partner, proc, round, comm_rank, mpi_errno = MPI_SUCCESS;
	int msg_size = 0;
	static char myname[] = "MPID_SMI_Alltoallv_plain";
	char *sbuf, *rbuf;

	MPID_STAT_ENTRY(alltoall);

	ci = (MPID_SMI_comm_info_t *)(comm->adiCollCtx);

    /* get char pointers to recv- and sendbuffer, this seems to be more comfortable */
	sbuf = (char *)sendbuf;
	rbuf = (char *)recvbuf;
  
    /* Get extent of send and recv types. We could use MPI_Type_extent, but 
       this also only returns this value. */
	send_extent = sendtype->extent;
	recv_extent = recvtype->extent;
  
	/* Determine the average message size from all send/recv-cnts/extents to 
		   decide for a communication pattern to use. */
	for (p = 0; p < ci->lsize; p++) {
		msg_size += recv_extent*recvcnts[p];
	}
	msg_size /= ci->lsize;
	if (msg_size >= MPID_SMI_cfg.COLL_ALLTOALL_MIN) {
		/* processes running on the same node now exchange their data for each other, 
		   this is done in just the same way as the whole thing is done in intra_fnsc.c: 
		   just post all irecvs and isend, then wait for everything */
		ALLOCATE( reqarray, MPI_Request *, 2 * ci->nsize * sizeof(MPI_Request) );
		ALLOCATE( statarray, MPI_Status *, 2 * ci->nsize * sizeof(MPI_Status) );

		for (proc = 0; proc < ci->nsize; proc++) {
			comm_rank = ci->lranks_on_gnode[MPID_SMI_myNode][proc];
			MPICH_Irecv( rbuf + recv_extent*rdispls[comm_rank], recvcnts[comm_rank], recvtype->self,
						 comm_rank, MPIR_ALLTOALLV_TAG, comm->self, &reqarray[2*proc+1] );
			MPICH_Isend( sbuf + send_extent*sdispls[comm_rank], sendcnts[comm_rank], sendtype->self,
						 comm_rank, MPIR_ALLTOALLV_TAG, comm->self, &reqarray[2*proc] );
		}
      
		MPICH_Waitall( 2 * ci->nsize, reqarray, statarray );
      
		FREE( reqarray );
		FREE( statarray );
      
		/* If all processes run on the same node, we are done now. Usually, this is not 
		   the case, and the show begins: */
		if (ci->nbr_lnodes > 1 ) {
			for (round = 0; round < ci->alltoall.nbr_rounds; round++) {
#if 0
				/* This barrier improves the performance by letting contentions
				   and head-of-line blockings be resolved before new traffic is generated. */
				MPICH_Barrier( comm->self );
#endif
					
				if ((partner = ci->alltoall.send_to[round]) != -1) {
					/* In each round, a process exchanges data with all processes on the 
					   remote node. */
					for  (p = 0; p < ci->procs_on_gnode[partner]; p++) {
						/* We need to avoid that one process on the remote node
						   communicates with more than one processes on the local node, while 
						   other processes remain idle. Of course, this will only work well 
						   if the number of procs per node is distributed evenly - but this is
						   usually the case for our setups. If not, we would need to calculate
						   process-related communication patterns (instead of node-related). */
						proc = (p + ci->nrank +1) % ci->procs_on_gnode[partner];
						comm_rank = ci->lranks_on_gnode[ci->lnode_to_gnode[partner]][proc];

						/* Instead of using MPI_Sendrecv, we split up the communication for test 
						   how the send/recv-ordering influences the performance (may also be
						   dependand on whether using PIO or DMA). */
						if (partner < ci->lnode) {
							MPICH_Irecv (rbuf + recv_extent*rdispls[comm_rank], recvcnts[comm_rank], 
										 recvtype->self, comm_rank, MPIR_ALLTOALLV_TAG, comm->self, &rem_req[1] );
							MPICH_Isend (sbuf + send_extent*sdispls[comm_rank], sendcnts[comm_rank], 
										 sendtype->self, comm_rank, MPIR_ALLTOALLV_TAG, comm->self, &rem_req[0] );
							MPICH_Waitall (2, rem_req, rem_stat );
						} else {
							MPICH_Irecv (rbuf + recv_extent*rdispls[comm_rank], recvcnts[comm_rank], 
										 recvtype->self, comm_rank, MPIR_ALLTOALLV_TAG, comm->self, &rem_req[0]);
							MPICH_Isend (sbuf + send_extent*sdispls[comm_rank], sendcnts[comm_rank], 
										 sendtype->self, comm_rank, MPIR_ALLTOALLV_TAG, comm->self, &rem_req[1] );
							MPICH_Waitall( 2, rem_req, rem_stat );
						}
					}
				}
			}
		}
	} else {
		/* Perform alltoall the standard way (faster for small data sizes which can
			   be delivered eagerly and will cause no severe contention on the network, 
			   no matter how they are distributed). */
		ALLOCATE( reqarray, MPI_Request *, 2*ci->lsize * sizeof(MPI_Request) );
		ALLOCATE( statarray, MPI_Status *, 2*ci->lsize * sizeof(MPI_Status) );

		for (p = 0; p < ci->lsize; p++) 
			MPICH_Irecv (rbuf + recv_extent*rdispls[p], recvcnts[p], recvtype->self,
						 p, MPIR_ALLTOALLV_TAG, comm->self, &reqarray[2*p] );

		for (p = 0; p < ci->lsize; p++) {
			/* Avoid contention by distributing the send order evenly. Although we use
			   non-blocking send, the messages will (usually) be delivered immedeately
			   because they (usually) are short- or eager-messages. */
			proc = (2*ci->lrank - 1 + p + ci->lsize) % ci->lsize;
			MPICH_Isend (sbuf + send_extent*sdispls[proc], sendcnts[proc], sendtype->self,
						 proc, MPIR_ALLTOALLV_TAG, comm->self, &reqarray[2*p+1] );
		}

		MPICH_Waitall (2*ci->lsize, reqarray, statarray);

		FREE( reqarray );
		FREE( statarray );
	}

	MPID_STAT_EXIT(alltoall);
	return (mpi_errno);
}
	
/*
 * ScaMPIs alltoall-communication pattern as described by Lars-Paul Huse
 * in the EuroPVMPI '99 paper.
 */

void MPID_SMI_alltoall_init_scampi( MPID_SMI_comm_info_t *ci )
{
	int round, node, send_to, recv_from;

	ALLOCATE( ci->alltoall.send_to, int *, ci->nbr_lnodes * sizeof(int) );
	ALLOCATE( ci->alltoall.recv_from, int *, ci->nbr_lnodes * sizeof(int) );

	ci->alltoall.nbr_rounds = ci->nbr_lnodes;
	for (round = 0; round < ci->nbr_lnodes; round++) {
#if PAIR_DEBUG			
		printf ("[%d] round %d:", ci->lrank, round);
#endif
		for (node = 0; node < ci->nbr_lnodes; node++) {
			send_to = (node + round) % ci->nbr_lnodes;
			recv_from = (node + ci->nbr_lnodes - round) % ci->nbr_lnodes;
			
			if (node == ci->lnode) {
				ci->alltoall.send_to[round] = send_to;
				ci->alltoall.recv_from[round] = recv_from;
			}
#if PAIR_DEBUG			
			printf (" [%d -> %d]", node, send_to);
#endif
		}
#if PAIR_DEBUG
	printf ("\n");
#endif		
	}	
		
	return;
}


void MPID_SMI_alltoall_init_straight( MPID_SMI_comm_info_t *ci )
{
	int round;

	ALLOCATE( ci->alltoall.send_to, int *, ci->lsize * sizeof(int) );
	ALLOCATE( ci->alltoall.recv_from, int *, ci->lsize * sizeof(int) );

	ci->alltoall.nbr_rounds = ci->lsize;
	for (round = 0; round < ci->lsize; round++) {
		ci->alltoall.send_to[round] = (ci->lrank + round) % ci->lsize;
		ci->alltoall.recv_from[round] = (ci->lrank + ci->lsize - round) % ci->lsize;
	}
		
	return;
}
	

/* "Straightforward" alltoall which just executes the process-related send/recv oder
   given in the communicator. */
int MPID_SMI_Alltoall_straight (void *sendbuf, int sendcount, struct MPIR_DATATYPE *sendtype, 
								void *recvbuf, int recvcount, struct MPIR_DATATYPE *recvtype, 
								struct MPIR_COMMUNICATOR *comm)
{
	MPID_SMI_comm_info_t *ci;
	int *int_array, *sdispls, *rdispls, *scounts, *rcounts;
	int mpi_errno, i;

	ci = (MPID_SMI_comm_info_t *)(comm->adiCollCtx);
	ALLOCATE (int_array, int *, 4 * ci->lsize * sizeof(int));
	sdispls = &int_array[0];
	rdispls = &int_array[ci->lsize];
	scounts = &int_array[2*ci->lsize];
	rcounts = &int_array[3*ci->lsize];
	for (i = 0; i < ci->lsize; i++) {
		sdispls[i] = i*sendcount;
		rdispls[i] = i*recvcount;
		scounts[i] = sendcount;
		rcounts[i] = recvcount;
	}

	mpi_errno = MPID_SMI_Alltoallv_straight (sendbuf, scounts, sdispls, sendtype, 
											 recvbuf, rcounts, rdispls, recvtype, comm);

	FREE (int_array);
	return (mpi_errno);
}


int MPID_SMI_Alltoallv_straight (void *s_buf, int *s_cnts, int *sdispls, struct MPIR_DATATYPE *s_type, 
								 void *r_buf, int *r_cnts, int *rdispls, struct MPIR_DATATYPE *r_type,
								 struct MPIR_COMMUNICATOR *comm)
{
	MPI_Aint     s_ext, r_ext;
	MPI_Request  request[2];
	MPI_Status   status[2];
	MPID_SMI_comm_info_t *ci;
	int r, f, t, mpi_errno = MPI_SUCCESS;
	static char myname[] = "MPID_SMI_Alltoallv_straight";
	char *sbuf, *rbuf;

	MPID_STAT_ENTRY(alltoall);

	ci = (MPID_SMI_comm_info_t *)(comm->adiCollCtx);

    /* get char pointers to recv- and sendbuffer, this seems to be more comfortable */
	sbuf = (char *)s_buf;
	rbuf = (char *)r_buf;
  
    /* Get extent of send and recv types. We could use MPI_Type_extent, but 
       this also only returns this value. */
	s_ext = s_type->extent;
	r_ext = r_type->extent;
  
	for (r = 0; r < ci->alltoall.nbr_rounds; r++) {
		f = ci->alltoall.recv_from[r];
		t = ci->alltoall.send_to[r];

		MPICH_Irecv( rbuf + r_ext*rdispls[f], r_cnts[f], r_type->self,
					 f, MPIR_ALLTOALLV_TAG, comm->self, &request[0] );
		MPICH_Isend( sbuf + s_ext*sdispls[t], s_cnts[t], s_type->self,
					 t, MPIR_ALLTOALLV_TAG, comm->self, &request[1] );
		MPICH_Waitall( 2, request, status );
		/* XXX check status */
	}

	return mpi_errno;
} 


/*
	 * Topology-oriented alltoall-algorithm below.
	 */


/* Pair-building function type - 
   Input:
    comm_info : communicator information
    from_node : lrank of node for which a partner is to be found
    dstnc     : distance to choose 
   Return value:
    >= 0      : lrank of partner-node for from_node 
	-1        : no node found to communicate with using this function */
typedef int(*get_pair_fcn_t)(MPID_SMI_comm_info_t *ci, int from_lnode);

typedef enum { X_comm = 0, Y_comm, XY_comm, SMP_comm } comm_direction;
#define COMM_TYPES 4

typedef enum { d_Xmin, d_Xmax, d_Ymin, d_Ymax, 
			   d_XY_Xmin, d_XY_Ymin, d_XY_Xmax, d_XY_Ymax } dsntc_idx_t;

static int dstnc[d_XY_Ymax + 1];

/* Dynamically increase/decrease then minimum/maximum distance to find 
   receiving nodes at? Doing this may increase number of conflicts when routing 
   (thus increasing number of rounds), but decrease the number of conflicts
   when actually communicating. */
#define DYNAMIC_MINMAX 0
#define ROUTE_DEBUG    0

int get_pair_x_max (MPID_SMI_comm_info_t *ci, int from_lnode)
{
	int to_lnode = -1, x = 0, y = 0, z = 0, j;
	
#if ROUTE_DEBUG			
	printf (" X");
#endif
	LNODE_TO_GRID(ci, from_lnode, x, y, z);
#if DYNAMIC_MINMAX
	for (j = dstnc[d_Xmax]; j > 0; j--) {
#else
	for (j = ci->lnodes_in_dim[X_DIM] - 1; j > 0; j--) {
#endif
		to_lnode = GRID_TO_LNODE(ci, (x + j) % ci->lnodes_in_dim[X_DIM], y, z);
		if (to_lnode >= 0 && !already_chosen[from_lnode][to_lnode] 
			&& actual_choice[to_lnode] == -1) {
			/* Found a new communication pair - mark this choice, and
			   decrement distance counter. */
			already_chosen[from_lnode][to_lnode] = 1;
			actual_choice[to_lnode] = from_lnode;

			dstnc[d_Xmax]--;
			if (dstnc[d_Xmax] == 0)
				dstnc[d_Xmax] = ci->lnodes_in_dim[X_DIM] - 1;

			break;
		} else
			to_lnode = -1;
	}
		
	return to_lnode;
}

int get_pair_y_max (MPID_SMI_comm_info_t *ci, int from_lnode)
{
	int to_lnode = -1, x = 0, y = 0, z = 0, j;
	
#if ROUTE_DEBUG			
	printf (" Y");
#endif
	LNODE_TO_GRID(ci, from_lnode, x, y, z);
#if DYNAMIC_MINMAX
	for (j = dstnc[d_Ymax]; j > 0; j--) {
#else
	for (j =  ci->lnodes_in_dim[Y_DIM] - 1; j > 0; j--) {
#endif
		to_lnode = GRID_TO_LNODE(ci, x, (y + j) % ci->lnodes_in_dim[Y_DIM], z);
		if (to_lnode >= 0 && !already_chosen[from_lnode][to_lnode] 
			&& actual_choice[to_lnode] == -1) {
			/* Found a new communication pair - mark this choice, and
			   decrement distance counter. */
			already_chosen[from_lnode][to_lnode] = 1;
			actual_choice[to_lnode] = from_lnode;

			dstnc[d_Ymax]--;
			if (dstnc[d_Ymax] == 0)
				dstnc[d_Ymax] = ci->lnodes_in_dim[Y_DIM] - 1;

			break;
		} else
			to_lnode = -1;
	}
		
	return to_lnode;
}

int get_pair_xy_max (MPID_SMI_comm_info_t *ci, int from_lnode)
{
	int to_lnode = -1, x = 0, y = 0, z = 0, d_x, d_y;
	
#if ROUTE_DEBUG			
	printf (" XY");
#endif
	LNODE_TO_GRID(ci, from_lnode, x, y, z);
#if DYNAMIC_MINMAX
	for (d_x = dstnc[d_XY_Xmax]; d_x > 0; d_x--) {
		for (d_y = dstnc[d_XY_Ymax]; d_y > 0; d_y--) {
#else
	for (d_x = ci->lnodes_in_dim[X_DIM] - 1; d_x > 0; d_x--) {
		for (d_y = ci->lnodes_in_dim[Y_DIM] - 1; d_y > 0; d_y--) {
#endif
			to_lnode = GRID_TO_LNODE(ci, (x + d_x) % ci->lnodes_in_dim[X_DIM], 
									 (y + d_y) % ci->lnodes_in_dim[Y_DIM], z);
			if (to_lnode >= 0 && !already_chosen[from_lnode][to_lnode] 
				&& actual_choice[to_lnode] == -1) {
				/* Found a new communication pair - mark this choice, and
				   decrement distance counter. */
				already_chosen[from_lnode][to_lnode] = 1;
				actual_choice[to_lnode] = from_lnode;
				
				dstnc[d_XY_Xmax]--;
				if (dstnc[d_XY_Xmax] == 0)
					dstnc[d_XY_Xmax] = ci->lnodes_in_dim[X_DIM] - 1;
				dstnc[d_XY_Ymax]++;
				if (dstnc[d_XY_Ymax] == 0)
					dstnc[d_XY_Ymax] = ci->lnodes_in_dim[Y_DIM] - 1;

				return to_lnode;
			} else
				to_lnode = -1;
		}
	}

	return to_lnode;
}

int get_pair_x_min (MPID_SMI_comm_info_t *ci, int from_lnode)
{
	int to_lnode = -1, x = 0, y = 0, z = 0, j;
	
#if ROUTE_DEBUG			
	printf (" x");
#endif
	LNODE_TO_GRID(ci, from_lnode, x, y, z);
#if DYNAMIC_MINMAX
	for (j = dstnc[d_Xmin]; j < ci->lnodes_in_dim[X_DIM]; j++) {
#else
	for (j = 1; j < ci->lnodes_in_dim[X_DIM]; j++) {
#endif
		to_lnode = GRID_TO_LNODE(ci, (x + j) % ci->lnodes_in_dim[X_DIM], y, z);
		if (to_lnode >= 0 && !already_chosen[from_lnode][to_lnode] 
			&& actual_choice[to_lnode] == -1) {
			/* Found a new communication pair - mark this choice, and
			   decrement distance counter. */
			already_chosen[from_lnode][to_lnode] = 1;
			actual_choice[to_lnode] = from_lnode;

			dstnc[d_Xmin]++;
			if (dstnc[d_Xmin] == ci->lnodes_in_dim[X_DIM])
				dstnc[d_Xmin] = 1;

			break;
		} else
			to_lnode = -1;
	}
		
	return to_lnode;
}

int get_pair_y_min (MPID_SMI_comm_info_t *ci, int from_lnode)
{
	int to_lnode = -1, x = 0, y = 0, z = 0, j;
	
#if ROUTE_DEBUG			
	printf (" y");
#endif
	LNODE_TO_GRID(ci, from_lnode, x, y, z);
#if DYNAMIC_MINMAX
	for (j = dstnc[d_Ymin]; j < ci->lnodes_in_dim[Y_DIM]; j++) {
#else
	for (j = 1; j < ci->lnodes_in_dim[Y_DIM]; j++) {
#endif
		to_lnode = GRID_TO_LNODE(ci, x, (y + j) % ci->lnodes_in_dim[Y_DIM], z);
		if (to_lnode >= 0 && !already_chosen[from_lnode][to_lnode] 
			&& actual_choice[to_lnode] == -1) {
			/* Found a new communication pair - mark this choice, and
			   decrement distance counter. */
			already_chosen[from_lnode][to_lnode] = 1;
			actual_choice[to_lnode] = from_lnode;

			dstnc[d_Ymin]++;
			if (dstnc[d_Ymin] == ci->lnodes_in_dim[Y_DIM])
				dstnc[d_Ymin] = 1;

			break;
		} else
			to_lnode = -1;
	}
		
	return to_lnode;
}

int get_pair_xy_min (MPID_SMI_comm_info_t *ci, int from_lnode)
{
	int to_lnode = -1, x = 0, y = 0, z = 0, d_x, d_y;
	
#if ROUTE_DEBUG			
	printf (" xy");
#endif
	LNODE_TO_GRID(ci, from_lnode, x, y, z);
#if DYNAMIC_MINMAX
	for (d_x = dstnc[d_XY_Xmin]; d_x < ci->lnodes_in_dim[X_DIM]; d_x++) {
		for (d_y = dstnc[d_XY_Ymin]; d_y <= ci->lnodes_in_dim[Y_DIM]; d_y++) {
#else
	for (d_x = 1; d_x <= ci->lnodes_in_dim[X_DIM]; d_x++) {
		for (d_y = 1; d_y < ci->lnodes_in_dim[Y_DIM]; d_y++) {
#endif
			to_lnode = GRID_TO_LNODE(ci, (x + d_x) % ci->lnodes_in_dim[X_DIM], 
									 (y + d_y) % ci->lnodes_in_dim[Y_DIM], z);
#if 0		
			printf ("xy_min from %d: (%d, %d) = %d\n", from_lnode, (x + d_x) % ci->lnodes_in_dim[X_DIM], 
					(y + d_y) % ci->lnodes_in_dim[Y_DIM], to_lnode);
#endif
			if (to_lnode >= 0 && !already_chosen[from_lnode][to_lnode] 
				&& actual_choice[to_lnode] == -1) {
				/* Found a new communication pair - mark this choice, and
				   decrement distance counter. */
				already_chosen[from_lnode][to_lnode] = 1;
				actual_choice[to_lnode] = from_lnode;
				
				dstnc[d_XY_Xmin]++;
				if (dstnc[d_XY_Xmin] == ci->lnodes_in_dim[X_DIM])
					dstnc[d_XY_Xmin] = 1;
				dstnc[d_XY_Ymin]++;
				if (dstnc[d_XY_Ymin] == ci->lnodes_in_dim[Y_DIM])
					dstnc[d_XY_Ymin] = 1;

				return to_lnode;
			} else
				to_lnode = -1;
		}
	}

	return to_lnode;
}

int get_pair_smp (MPID_SMI_comm_info_t *ci, int from_lnode)
{	
	int to_lnode = -1; 

#if ROUTE_DEBUG			
	printf (" SMP");
#endif
	if (!already_chosen[from_lnode][from_lnode] && actual_choice[from_lnode] == -1) {
		to_lnode = from_lnode;
		actual_choice[from_lnode] = to_lnode;
		already_chosen[from_lnode][to_lnode] = 1;
	}

	return to_lnode;
}


static get_pair_fcn_t pair_fcns_2D_avail[2*COMM_TYPES] = { get_pair_x_min, get_pair_x_max,
														get_pair_y_min, get_pair_y_max,
														get_pair_xy_min, get_pair_xy_max,
														get_pair_smp, get_pair_smp };


/* Pre-calculate the communication pattern for the alltoall-communication
   the complex way by carefully considering the SCI topology. */
/* XXX: For now, we only implement this for 1- and 2-dimensional torus topologies.
   Higher dimensions are not a problem, but will become quite excessize to set up. */
void MPID_SMI_alltoall_init_topology_2D( MPID_SMI_comm_info_t *ci )
{
	get_pair_fcn_t *pair_fcn;
	int nbr_comms[COMM_TYPES] = { 0, 0, 0, 0 }, comm_weight[COMM_TYPES], nbr_comm_types;
	int min_max_comm[COMM_TYPES] = { 0, 0, 0, 0 }; /* toggle between min-max variants */
	int consider_smp, node_doing_smp;
	int total_comms, match_try, max_comms, last_fcn_idx;
	int i, j, idx, from_node, to_node, fcn_idx, fcn_offset;

	/* Worst-case allocation */
	ALLOCATE( ci->alltoall.send_to, int *, 2*MPID_SMI_msb(ci->nbr_lnodes) * sizeof(int) );
	ALLOCATE( ci->alltoall.recv_from, int *, 2*MPID_SMI_msb(ci->nbr_lnodes) * sizeof(int) );
	for (i = 0; i <  2*MPID_SMI_msb(ci->nbr_lnodes); i++) {
		ci->alltoall.send_to[i]   = -1;
		ci->alltoall.recv_from[i] = -1;
	}
	ZALLOCATE (pair_fcn, get_pair_fcn_t *, 2*ci->nbr_lnodes * sizeof(get_pair_fcn_t));

	/* Build table of pair-building functions to call in round-robin fashion
	   while determining the communication pairs. The appearance of each type of
	   pair building-function (X, Y, X-Y, SMP (for one or more processes on a node), 
	   each with max and min attribute) is weighted by the requirements of the 
	   topology of the virtual network (number of time each of the directions will 
	   need to be used). */
	total_comms = ci->nbr_lnodes - 1;
	nbr_comms[X_comm] = comm_weight[X_comm] = ci->lnodes_in_dim[X_DIM] > 1 ? ci->lnodes_in_dim[X_DIM] - 1 : 0;
	nbr_comms[Y_comm] = comm_weight[Y_comm] = ci->lnodes_in_dim[Y_DIM] > 1 ? ci->lnodes_in_dim[Y_DIM] - 1 : 0;
	nbr_comms[XY_comm] = comm_weight[XY_comm] = total_comms - nbr_comms[X_comm] - nbr_comms[Y_comm];
	nbr_comms[SMP_comm] = 1;
	/* Sanity check for higly irregular topologies (like 2x2 with 3 nodes) */
	if (nbr_comms[XY_comm] == 0 && ci->lnodes_in_dim[X_DIM] > 1 && ci->lnodes_in_dim[Y_DIM] > 1) {
		nbr_comms[XY_comm] = comm_weight[XY_comm] = (ci->lnodes_in_dim[X_DIM]-1)*(ci->lnodes_in_dim[Y_DIM]-1);
		total_comms += nbr_comms[XY_comm];
	}
	dstnc[d_Xmax] = dstnc[d_XY_Xmax] = ci->lnodes_in_dim[X_DIM] - 1;
	dstnc[d_Ymax] = dstnc[d_XY_Ymax] = ci->lnodes_in_dim[Y_DIM] - 1;
	dstnc[d_Xmin] = dstnc[d_Ymin] = dstnc[d_XY_Xmin] = dstnc[d_XY_Ymin] = 1;

	/* Only consider SMP communication if nbr of nodes is odd. */
	consider_smp = (ci->nbr_lnodes % 2);
	nbr_comm_types = COMM_TYPES - 1;

#if PAIR_DEBUG			
	printf ("[%d] grid_to_lode: ", ci->lrank);
	for (i = 0; i < ci->lnodes_in_dim[Y_DIM]; i++)
		for (j= 0; j < ci->lnodes_in_dim[X_DIM]; j++)
			printf ("(%d,%d) = %d - ", j, i, GRID_TO_LNODE(ci, j, i, 0));
	printf ("\n[%d] total_comms = %d - x = %d - y = %d - xy = %d\n", ci->lrank, total_comms, 
			nbr_comms[X_comm], nbr_comms[Y_comm], nbr_comms[XY_comm]);
#endif

	/* Choose the pair-building functions which will be used to calculate all
	   the communication pairs. */
	fcn_idx = 0;
#if PAIR_DEBUG			
	printf ("[%d] comm_type vector: ", ci->lrank);
#endif
	for (i = 0; i < total_comms; i++) {
		/* Find the communication direction with the max. weight. and store
		   the related min/max variant of this direction in the function ptr array. 
		   Distribution algorithm based on the relative occurence of a communication  type:
		   - choose the comm_type with the max. current weight; then reset the weight value
		   - add the respective original weight to all other weights */
		max_comms = 0;
		last_fcn_idx = fcn_idx;
		/* Find the comm_type with the biggest weight. */
		for (j = 0; j < nbr_comm_types; j++) {
			idx = (last_fcn_idx + j) % nbr_comm_types;
			if (comm_weight[idx] > max_comms) {
				max_comms = comm_weight[idx];
				fcn_idx = idx;
			}
		}

		/* Select the according comm_type routing function and reset/increment comm_weights. */		
#if PAIR_DEBUG			
		printf (" %d", 2*fcn_idx + min_max_comm[fcn_idx]);
#endif
		pair_fcn[i] = pair_fcns_2D_avail[2*fcn_idx + min_max_comm[fcn_idx]];
		min_max_comm[fcn_idx] = (min_max_comm[fcn_idx] + 1) % 2;
		for (j = 0; j < nbr_comm_types; j++) {
			comm_weight[j] = (j == fcn_idx) ? nbr_comms[j] : comm_weight[j] + nbr_comms[j];
		}
	}
#if PAIR_DEBUG			
		printf ("\n");
#endif

	/* Allocate & initialize temporary arrays that we need to calculate 
	   content of ci->send_to / ci->recv_from */
	ALLOCATE( actual_choice, int *, ci->nbr_lnodes * sizeof(int));
	ALLOCATE( already_chosen, int **, ci->nbr_lnodes * sizeof(int *));
	for (i = 0; i < ci->nbr_lnodes; i++ )
		ALLOCATE( already_chosen[i], int *, ci->nbr_lnodes * sizeof(int *));
	for (i = 0; i < ci->nbr_lnodes; i++) {
		actual_choice[i] = -1;
		for (j = 0; j < ci->nbr_lnodes; j++)
			already_chosen[i][j] = 0;
	}

	/* Now calculate the communication using the function determined above. */
	fcn_offset = 0;
	node_doing_smp = consider_smp ? ci->nbr_lnodes - 1 : -1;
	ci->alltoall.nbr_rounds = 0;
	while (some_node_left(ci, false)) {
		for (from_node = 0; from_node < ci->nbr_lnodes; from_node++) {
			actual_choice[from_node] = -1;
		}
		/* Assign the SMP communication to a node (if appplicable). This
		   is required due to the assymetric nature of SMP (identical receiver and
		   sender) which would cause problems if not considered. */
		if (node_doing_smp >= 0) {
			actual_choice[node_doing_smp] = node_doing_smp;
		}

#if PAIR_DEBUG			
		printf ("[%d] round %d:", ci->lrank, ci->alltoall.nbr_rounds);
#endif

		fcn_idx = fcn_offset;
		for (i = 0; i < ci->nbr_lnodes; i++) {
			from_node = i;

			if (from_node != node_doing_smp) {
				match_try = 0;
				do {					
					to_node = pair_fcn[fcn_idx](ci, from_node);

					fcn_idx = (fcn_idx + 1) % total_comms;
					match_try++;
				} while (to_node < 0 && match_try < total_comms);
			} else {
				to_node = node_doing_smp;
			}

			/* Remember this choice if it concerns my node. */
			if (ci->lnode == from_node) 
				ci->alltoall.send_to[ci->alltoall.nbr_rounds] = to_node;
			if (ci->lnode == to_node) 
				ci->alltoall.recv_from[ci->alltoall.nbr_rounds] = from_node;
#if PAIR_DEBUG			
			printf (" [%d -> %d]", from_node, to_node);
#endif
		}
#if PAIR_DEBUG
		printf ("\n");
#endif		
		node_doing_smp--;
		fcn_offset = (fcn_offset + 1) % total_comms;
		ci->alltoall.nbr_rounds++;
	}

	/* free temporary arrays */
	for( i = 0; i < ci->nbr_lnodes; i++ )
		FREE( already_chosen[i] );
	FREE( already_chosen );
	FREE( actual_choice );
	FREE( pair_fcn );

	return;
}


int get_pair_1D_max (MPID_SMI_comm_info_t *ci, int from_lnode)
{
	int to_lnode = -1, j;
	
#if ROUTE_DEBUG			
	printf (" M");
#endif
	for (j = ci->nbr_lnodes - 1; j > 0; j--) {
		to_lnode = (from_lnode + j) % ci->nbr_lnodes;
		if (!already_chosen[from_lnode][to_lnode] && actual_choice[to_lnode] == -1) {
			/* Found a new communication pair - mark this choice, and
			   decrement distance counter. */
			already_chosen[from_lnode][to_lnode] = 1;
			actual_choice[to_lnode] = from_lnode;

			break;
		} else
			to_lnode = -1;
	}
		
	return to_lnode;
}

int get_pair_1D_min (MPID_SMI_comm_info_t *ci, int from_lnode)
{
	int to_lnode = -1, j;
	
#if ROUTE_DEBUG			
	printf (" M");
#endif
	for (j = 1; j < ci->nbr_lnodes; j++) {
		to_lnode = (from_lnode + j) % ci->nbr_lnodes;
		if (!already_chosen[from_lnode][to_lnode] && actual_choice[to_lnode] == -1) {
			/* Found a new communication pair - mark this choice, and
			   decrement distance counter. */
			already_chosen[from_lnode][to_lnode] = 1;
			actual_choice[to_lnode] = from_lnode;
			
			break;
		} else
			to_lnode = -1;
	}
		
	return to_lnode;
}

static get_pair_fcn_t pair_fcns_1D_avail[2] = { get_pair_1D_min, get_pair_1D_max };


/* Pre-calculate the communication pattern for the alltoall-communication
   the complex way by carefully considering the SCI topology. 
   This is the simpler 1D-variant of above 2D-variant which does only use a 1D-model
   of the SCI topology. This may lead to worse load distribution, but is more
   deterministic concerning the number of required communication steps. */
void MPID_SMI_alltoall_init_topology_1D( MPID_SMI_comm_info_t *ci )
{
	get_pair_fcn_t *pair_fcn;
	int nbr_comms[COMM_TYPES] = { 0, 0, 0, 0 }, comm_weight[COMM_TYPES], nbr_comm_types;
	int min_max_comm[COMM_TYPES] = { 0, 0, 0, 0 }; /* toggle between min-max variants */
	int consider_smp, node_doing_smp;
	int total_comms, match_try;
	int i, j, from_node, to_node, fcn_idx, fcn_offset;

	/* Worst-case allocation */
	ALLOCATE( ci->alltoall.send_to, int *, 2*MPID_SMI_msb(ci->nbr_lnodes) * sizeof(int) );
	ALLOCATE( ci->alltoall.recv_from, int *, 2*MPID_SMI_msb(ci->nbr_lnodes) * sizeof(int) );
	for (i = 0; i <  2*MPID_SMI_msb(ci->nbr_lnodes); i++) {
		ci->alltoall.send_to[i]   = -1;
		ci->alltoall.recv_from[i] = -1;
	}
	ALLOCATE (pair_fcn, get_pair_fcn_t *, 2*ci->nbr_lnodes * sizeof(get_pair_fcn_t));

	/* Build table of pair-building functions to call in round-robin fashion
	   while determining the communication pairs. Very simple for this 1D-case. */
	total_comms = ci->nbr_lnodes;
	nbr_comms[X_comm] = comm_weight[X_comm] = ci->nbr_lnodes - 1;
	nbr_comms[SMP_comm] = 1;

	/* Only consider SMP communication if nbr of nodes is odd. */
	consider_smp = (ci->nbr_lnodes % 2);
	nbr_comm_types = 1;

#if PAIR_DEBUG			
	printf ("[%d] grid_to_lode: ", ci->lrank);
	for (i = 0; i < ci->lnodes_in_dim[Y_DIM]; i++)
		for (j= 0; j < ci->lnodes_in_dim[X_DIM]; j++)
			printf ("(%d,%d) = %d - ", j, i, GRID_TO_LNODE(ci, j, i, 0));
	printf ("\n[%d] total_comms = %d - x = %d - y = %d - xy = %d\n", ci->lrank, total_comms, 
			nbr_comms[X_comm], nbr_comms[Y_comm], nbr_comms[XY_comm]);
#endif

	/* Choose the pair-building functions which will be used to calculate all
	   the communication pairs. Very simple, too. */
	fcn_idx = 0;
#if PAIR_DEBUG			
	printf ("[%d] comm_type vector: ", ci->lrank);
#endif
	fcn_idx = 0;
	for (i = 0; i < total_comms; i++) {
		/* Select the according comm_type routing function and reset/increment comm_weights. */		
#if PAIR_DEBUG			
		printf (" %d", 2*fcn_idx + min_max_comm[fcn_idx]);
#endif
		pair_fcn[i] = pair_fcns_1D_avail[min_max_comm[fcn_idx]];
		min_max_comm[fcn_idx] = (min_max_comm[fcn_idx] + 1) % 2;
	}
#if PAIR_DEBUG			
	printf ("\n");
#endif

	/* Allocate & initialize temporary arrays that we need to calculate 
	   content of ci->send_to / ci->recv_from */
	ALLOCATE( actual_choice, int *, ci->nbr_lnodes * sizeof(int));
	ALLOCATE( already_chosen, int **, ci->nbr_lnodes * sizeof(int *));
	for (i = 0; i < ci->nbr_lnodes; i++ )
		ALLOCATE( already_chosen[i], int *, ci->nbr_lnodes * sizeof(int *));
	for (i = 0; i < ci->nbr_lnodes; i++) {
		actual_choice[i] = -1;
		for (j = 0; j < ci->nbr_lnodes; j++)
			already_chosen[i][j] = 0;
	}

	/* Now calculate the communication using the function determined above. */
	fcn_offset = 0;
	node_doing_smp = consider_smp ? ci->nbr_lnodes - 1 : -1;
	ci->alltoall.nbr_rounds = 0;
	while (some_node_left(ci, false)) {
		for (from_node = 0; from_node < ci->nbr_lnodes; from_node++) {
			actual_choice[from_node] = -1;
		}
		/* Assign the SMP communication to a node (if appplicable). This
		   is required due to the assymetric nature of SMP (identical receiver and
		   sender) which would cause problems if not considered. */
		if (node_doing_smp >= 0) {
			actual_choice[node_doing_smp] = node_doing_smp;
		}

#if PAIR_DEBUG			
		printf ("[%d] round %d:", ci->lrank, ci->alltoall.nbr_rounds);
#endif

		fcn_idx = fcn_offset;
		for (i = 0; i < ci->nbr_lnodes; i++) {
			from_node = i;

			if (from_node != node_doing_smp) {
				match_try = 0;
				do {					
					to_node = pair_fcn[fcn_idx](ci, from_node);

					fcn_idx = (fcn_idx + 1) % total_comms;
					match_try++;
				} while (to_node < 0 && match_try < total_comms);
			} else {
				to_node = node_doing_smp;
			}

			/* Remember this choice if it concerns my node. */
			if (ci->lnode == from_node) 
				ci->alltoall.send_to[ci->alltoall.nbr_rounds] = to_node;
			if (ci->lnode == to_node) 
				ci->alltoall.recv_from[ci->alltoall.nbr_rounds] = from_node;
#if PAIR_DEBUG			
			printf (" [%d -> %d]", from_node, to_node);
#endif
		}
#if PAIR_DEBUG
		printf ("\n");
#endif		
		node_doing_smp--;
		fcn_offset = (fcn_offset + 1) % total_comms;
		ci->alltoall.nbr_rounds++;
	}

	/* free temporary arrays */
	for( i = 0; i < ci->nbr_lnodes; i++ )
		FREE( already_chosen[i] );
	FREE( already_chosen );
	FREE( actual_choice );
	FREE( pair_fcn );

	return;
}


void MPID_SMI_alltoall_destroy_topology (MPID_SMI_comm_info_t *ci)
{
	if (ci->alltoall.send_to != NULL)
		FREE( ci->alltoall.send_to );
	if (ci->alltoall.recv_from != NULL)
		FREE( ci->alltoall.recv_from );	
	
	return;
}


int MPID_SMI_Alltoall_topology (void *sendbuf, int sendcount, struct MPIR_DATATYPE *sendtype, 
								void *recvbuf, int recvcount, struct MPIR_DATATYPE *recvtype, 
								struct MPIR_COMMUNICATOR *comm)
{
	MPID_SMI_comm_info_t *ci;
	int *int_array, *sdispls, *rdispls, *scounts, *rcounts;
	int mpi_errno, i;

	ci = (MPID_SMI_comm_info_t *)(comm->adiCollCtx);
	ALLOCATE (int_array, int *, 4 * ci->lsize * sizeof(int));
	sdispls = &int_array[0];
	rdispls = &int_array[ci->lsize];
	scounts = &int_array[2*ci->lsize];
	rcounts = &int_array[3*ci->lsize];
	for (i = 0; i < ci->lsize; i++) {
		sdispls[i] = i*sendcount;
		rdispls[i] = i*recvcount;
		scounts[i] = sendcount;
		rcounts[i] = recvcount;
	}

	mpi_errno = MPID_SMI_Alltoallv_topology (sendbuf, scounts, sdispls, sendtype, 
											 recvbuf, rcounts, rdispls, recvtype, comm);

	FREE (int_array);
	return (mpi_errno);}


int MPID_SMI_Alltoallv_topology (void *sendbuf, int *sendcnts, int *sdispls, struct MPIR_DATATYPE *sendtype, 
								 void *recvbuf, int *recvcnts, int *rdispls, struct MPIR_DATATYPE *recvtype,
								 struct MPIR_COMMUNICATOR *comm)
{
	MPI_Aint     sextent, rextent;
	MPI_Request  request, rem_req[2], *reqarray;
	MPI_Status   status, rem_stat[2], *statarray;
	MPID_SMI_comm_info_t *ci;
	int msg_size, p, proc, round, comm_rank, mpi_errno = MPI_SUCCESS;
	static char myname[] = "MPID_SMI_Alltoallv_topology";
	char *sbuf, *rbuf;

	MPID_STAT_ENTRY(alltoall);

	ci = (MPID_SMI_comm_info_t *)(comm->adiCollCtx);

    /* get char pointers to recv- and sendbuffer, this seems to be more comfortable */
	sbuf = (char *)sendbuf;
	rbuf = (char *)recvbuf;
  
    /* Get extent of send and recv types. We could use MPI_Type_extent, but 
       this also only returns this value. */
	sextent = sendtype->extent;
	rextent = recvtype->extent;
  
	/* Determine the average message size from all send/recv-cnts/extents to 
		   decide for a communication pattern to use. */
	msg_size = 0;
	for (p = 0; p < ci->lsize; p++) {
		msg_size += rextent*recvcnts[p];
	}
	msg_size /= ci->lsize;
	if (msg_size >= MPID_SMI_cfg.COLL_ALLTOALL_MIN) {
		void *saddr, *raddr;
		int scnt, rcnt, rnode, snode, rproc, sproc;
		
		/* For an even number of nodes, the intra-node exchange is a separate step. */
		if (ci->nbr_lnodes % 2 == 0) {
			ALLOCATE( reqarray, MPI_Request *, 2 * ci->nsize * sizeof(MPI_Request) );
			ALLOCATE( statarray, MPI_Status *, 2 * ci->nsize * sizeof(MPI_Status) );
			
			for (proc = 0; proc < ci->nsize; proc++) {
				comm_rank = ci->lranks_on_gnode[MPID_SMI_myNode][proc];
				MPICH_Irecv( rbuf + rextent*rdispls[comm_rank], recvcnts[comm_rank], recvtype->self,
							 comm_rank, MPIR_ALLTOALLV_TAG, comm->self, &reqarray[2*proc+1] );
				MPICH_Isend( sbuf + sextent*sdispls[comm_rank], sendcnts[comm_rank], sendtype->self,
							 comm_rank, MPIR_ALLTOALLV_TAG, comm->self, &reqarray[2*proc] );
			}
			
			MPICH_Waitall( 2 * ci->nsize, reqarray, statarray );
			
			FREE( reqarray );
			FREE( statarray );			
		}
		
		for (round = 0; round < ci->alltoall.nbr_rounds; round++) {
			/* In each round, a process exchanges data with all processes on the 
			   remote node. */
			rnode = ci->alltoall.recv_from[round] >= 0 ? 
				ci->lnode_to_gnode[ci->alltoall.recv_from[round]] : -1;
			snode = ci->alltoall.send_to[round] >= 0 ?
				ci->lnode_to_gnode[ci->alltoall.send_to[round]] : -1;
			
			p = 1;
			do {
				/* We need to avoid that one process on the remote node
				   communicates with more than one processes on the local node, while 
				   other processes remain idle. Of course, this will only work well 
				   if the number of procs per node is distributed evenly - but this is
				   usually the case for our setups. If not, we would need to calculate
				   process-related communication patterns (instead of node-related). */				
				if (rnode >= 0 && ci->procs_on_gnode[rnode] >= p) {
					rproc = (p + ci->nrank) % ci->procs_on_gnode[rnode];
					rproc = ci->lranks_on_gnode[rnode][rproc];
					raddr = rbuf + rextent*rdispls[rproc];
					rcnt  = recvcnts[rproc];
				} else {
					rproc = MPI_PROC_NULL;
					raddr = NULL;
					rcnt  = 0;
				}
				if (snode >= 0 && ci->procs_on_gnode[snode] >= p) {
					sproc = (p + ci->nrank) % ci->procs_on_gnode[snode];
					sproc = ci->lranks_on_gnode[snode][sproc];
					saddr = sbuf + sextent*sdispls[sproc];
					scnt  = sendcnts[sproc];
				} else {
					sproc = MPI_PROC_NULL;
					saddr = NULL;
					scnt  = 0;
				}
					
				MPICH_Irecv (raddr, rcnt, recvtype->self, rproc, MPIR_ALLTOALLV_TAG, comm->self, &rem_req[1]);
				MPICH_Isend (saddr, scnt, sendtype->self, sproc, MPIR_ALLTOALLV_TAG, comm->self, &rem_req[0]);
				MPICH_Waitall (2, rem_req, rem_stat );
				
				p++;
			} while ((rnode >= 0 && p <= ci->procs_on_gnode[rnode]) || 
					 (snode >= 0 && p <= ci->procs_on_gnode[snode]));
#if 0
			/* This barrier improves the performance by letting contentions
			   and head-of-line blockings be resolved before new traffic is generated.
			   However, this only applies to LC-2 based systems; LC-3 has a working priority
			   scheduling for retry traffic and thus doesn't need such tricks. */
			if (round < ci->alltoall.nbr_rounds -1)
				MPICH_Barrier( comm->self );
#endif
		}
	} else {
		/* Perform alltoall the standard way (faster for small data sizes which can
		   be delivered eagerly and will cause no severe contention on the network, 
		   no matter how they are distributed). */
		ALLOCATE( reqarray, MPI_Request *, 2*ci->lsize * sizeof(MPI_Request) );
		ALLOCATE( statarray, MPI_Status *, 2*ci->lsize * sizeof(MPI_Status) );

		for (p = 0; p < ci->lsize; p++) {
			proc = (ci->lrank - (p + 1) + ci->lsize) % ci->lsize;

			MPICH_Irecv (rbuf + rextent*rdispls[proc], recvcnts[proc], recvtype->self,
						 proc, MPIR_ALLTOALLV_TAG, comm->self, &reqarray[ci->lsize + p] );
		}

		for (p = 0; p < ci->lsize; p++) {
			/* Avoid contention by distributing the send order evenly. Although we use
			   non-blocking send, the messages will (usually) be delivered immedeately
			   because they (usually) are short- or eager-messages. */
			proc = (ci->lrank + 1 + p) % ci->lsize;

			MPICH_Isend (sbuf + sextent*sdispls[proc], sendcnts[proc], sendtype->self,
						 proc, MPIR_ALLTOALLV_TAG, comm->self, &reqarray[p] );
		}

		MPICH_Waitall (2*ci->lsize, reqarray, statarray);

		FREE( reqarray );
		FREE( statarray );
	}

	MPID_STAT_EXIT(alltoall);
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
 * vim:tw=0:ts=4:wm=0:sw=4:
 */
