/* $Id$ 

   collective operations performed by the device */

#include <assert.h>

#include "smimem.h"
#include "smicoll.h" 
#include "mpiops.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* XXX: These globals make the Pipe non-threadsafe! This doesn't matter
   as long as MP-MPICH itself isn't threadsafe anyway. */
/* Globals for Pipeline Functions */
MPIR_SHANDLE mpid_smi_pipe_shandle[MAX_SCI_DIMS];
MPIR_RHANDLE mpid_smi_pipe_rhandle;
MPID_SBHeader mpid_smi_pipe_alignbuf_allocator;
MPID_SMI_RNDV_info mpid_smi_pipe_shandle_recv_handle[MAX_SCI_DIMS];
MPID_SMI_RNDV_info mpid_smi_pipe_rhandle_recv_handle;
int mpid_smi_pipe_bsize;
int mpid_smi_pipe_nbrblocks;
volatile int mpid_smi_pipe_ready_arrived = 0;

#if !defined(WIN32) && defined(MPI_SHARED_LIBS)
/* these are function ponters to the required MPI pt2pt functions - we can not call them 
   directly as this would result in cross-dependecies of the libraries */
int (*MPICH_Send)(void*, int, MPI_Datatype, int, int, MPI_Comm);
int (*MPICH_Recv)(void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Status *);
int (*MPICH_Isend)(void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int (*MPICH_Irecv)(void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int (*MPICH_Waitall)(int, MPI_Request *, MPI_Status *);
int (*MPICH_Test)(MPI_Request *, int *, MPI_Status *);
int (*MPICH_Barrier)(MPI_Comm);
#endif

/* this is the original MPICH-internal Barrier function for fallback */
int (*MPICH_msg_barrier)(struct MPIR_COMMUNICATOR *);

static int barrier_init( struct MPIR_COMMUNICATOR *nc,  MPID_SMI_comm_info_t *ci);
static void barrier_destroy (MPID_SMI_comm_info_t *comm_info);


/* Install the SCI-optimized collective operations into the new communicator. */
/* XXX: We don't consider the comm_type, which means we only do supper intra-
   communicators. */
/* XXX: The custom-collectives are *not* multi-device aware. */
int MPID_SMI_Collops_init(struct MPIR_COMMUNICATOR *comm, MPIR_COMM_TYPE comm_type)
{
    MPID_SMI_comm_info_t *comm_info;

	MPID_STAT_ENTRY(coll_init);
    MPID_ASSERT(comm != NULL, "NULL communicator");

	/* We only employ custom collectives for *more* than a single process! */
	if (MPID_SMI_cfg.COLLOPS && comm->np > 1) {
		if ((comm_info = (MPID_SMI_comm_info_t *)(comm->adiCollCtx)) != NULL) {
			/* Calculate communication pairs for MPI_Alltoall() */
			switch (MPID_SMI_cfg.COLL_ALLTOALL_TYPE) {
			case ALLTOALL_SCI_1D:
				MPID_SMI_alltoall_init_topology_1D(comm_info);
				break;
			case ALLTOALL_SCI_2D:
				MPID_SMI_alltoall_init_topology_2D(comm_info);
				break;
			case ALLTOALL_PLAIN:
				MPID_SMI_alltoall_init_plain(comm_info);
				break;
			case ALLTOALL_SCAMPI:
				MPID_SMI_alltoall_init_scampi(comm_info);
				break;
			case ALLTOALL_STRAIGHT:
				MPID_SMI_alltoall_init_straight(comm_info);
				break;
			}
			
			MPID_SMI_Pcast_coll_init (comm_info);

			MPICH_msg_barrier = comm->collops->Barrier;
			if (MPID_SMI_cfg.COLL_BARRIER) {
				if (barrier_init(comm, comm_info) == MPI_SUCCESS) {
					comm->collops->Barrier = MPID_SMI_Barrier;
				}
			}
			comm->collops->Allgather  = MPID_SMI_Allgather;
			comm->collops->Allgatherv = MPID_SMI_Allgatherv;
			switch (MPID_SMI_cfg.COLL_ALLTOALL_TYPE) {
			case ALLTOALL_PLAIN:
				comm->collops->Alltoallv  = MPID_SMI_Alltoallv_plain;
				comm->collops->Alltoall   = MPID_SMI_Alltoall_plain;
				break;
			case ALLTOALL_SCI_1D:
			case ALLTOALL_SCI_2D:
			case ALLTOALL_SCAMPI:
				comm->collops->Alltoallv  = MPID_SMI_Alltoallv_topology;
				comm->collops->Alltoall   = MPID_SMI_Alltoall_topology;
				break;
			case ALLTOALL_STRAIGHT:
				comm->collops->Alltoallv  = MPID_SMI_Alltoallv_straight;
				comm->collops->Alltoall   = MPID_SMI_Alltoall_straight;
				break;
			}
			switch (MPID_SMI_cfg.COLL_BCAST_TYPE) {
				/* only this custom version is available for now */
			case BCAST_PIPELINE:
				comm->collops->Bcast = MPID_SMI_Pcast;
				break;
			}
			comm->collops->Allgather  = MPID_SMI_Allgather;
			comm->collops->Allgatherv = MPID_SMI_Allgatherv;
			comm->collops->Reduce     = MPID_SMI_Reduce;
			comm->collops->Reduce_scatter = MPID_SMI_Reduce_scatter;
			comm->collops->Allreduce  = MPID_SMI_Allreduce;
			comm->collops->Scan       = MPID_SMI_Scan;
			comm->collops->Scatter    = MPID_SMI_Scatter;
		}
	}

#if defined MPI_SHARED_LIBS && !defined WIN32
	/* We need some symbols from libmpich.so - we get them via dlsym() 
	   which only works for dynamically linked executables */
	/* We need to use some pt2pt-functions from the MPIR-part (mpich-library) - 
	   not only for collective operations.*/
	/* XXX merge this with "sendrecvstubs.c" */
	GET_DLL_FCTNPTR("MPI_Send", MPICH_Send, int (*)
					(void*, int, MPI_Datatype, int, int, MPI_Comm));
	if (MPICH_Send == NULL) MPICH_Send = MPI_Send;
	GET_DLL_FCTNPTR("MPI_Recv",MPICH_Recv, int(*)
					(void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Status *));
	if (MPICH_Recv == NULL) MPICH_Recv = MPI_Recv;
	GET_DLL_FCTNPTR("MPI_Isend", MPICH_Isend, int(*)
					(void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *));
	if (MPICH_Isend == NULL) MPICH_Isend = MPI_Isend;
	GET_DLL_FCTNPTR("MPI_Irecv", MPICH_Irecv, int(*)
					(void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *));
	if (MPICH_Irecv == NULL) MPICH_Irecv = MPI_Irecv;
	GET_DLL_FCTNPTR("MPI_Waitall", MPICH_Waitall, int(*)(int,MPI_Request*,MPI_Status*));
	if (MPICH_Waitall == NULL) MPICH_Waitall = MPI_Waitall;
	GET_DLL_FCTNPTR("MPI_Test", MPICH_Test, int(*)(MPI_Request*,int *,MPI_Status*));
	if (MPICH_Test == NULL) MPICH_Test = MPI_Test;
	GET_DLL_FCTNPTR("MPI_Barrier", MPICH_Barrier, int(*)(MPI_Comm));
	if (MPICH_Barrier == NULL) MPICH_Barrier = MPI_Barrier;
#endif	

	MPID_STAT_EXIT(coll_init);
    return MPI_SUCCESS;
}

/* This is for switched topologies or pure SMP. */
static void get_other_sci_topology (struct MPIR_COMMUNICATOR *newcomm )
{
	MPID_SMI_comm_info_t *ci = (MPID_SMI_comm_info_t *)newcomm->adiCollCtx;
	int grid_size, j, distance;

	ci->lnodes_in_dim[X_DIM] = grid_size = ci->nbr_lnodes;
	ci->active_ndims = 1;

	distance = (ci->nbr_lnodes > 1) ? 1 : 0;

	for (j = 0; j < MAX_SCI_DIMS; j++) {
		ALLOCATE(ci->lnode_to_grid[j], int *, ci->nbr_lnodes*sizeof(int));
	}
	ALLOCATE (ci->grid_to_lnode, int *, grid_size * sizeof(int));

	for (j = 0; j < ci->nbr_lnodes; j++) {
		ci->lnode_to_grid[X_DIM][j] = j;
		ci->grid_to_lnode[j] = j;
	}

	ZALLOCATE (ci->distnc_lrank, int *, ci->lsize * sizeof(int));
	for (j = 0; j < ci->lsize; j++) 
		ci->distnc_lrank[j] = distance;

	for (j = 0; j < ci->sci_ndims; j++) {
		ZALLOCATE (ci->dim_distnc_lrank[j], int *, ci->lsize * sizeof(int));
	}
	for (j = 0; j < ci->lsize; j++) {
		ci->dim_distnc_lrank[X_DIM][j] = distance;
	}	

	return;
}

/* The calculations done in here are basically the same as performed in 
   the SMI library (src/utility/query.c) - but in SMI, it's done for all
   processes, and here, it's done for the processes of a communicator which 
   is not necessarily the same. */
static void get_torus_sci_topology (struct MPIR_COMMUNICATOR *newcomm )
{
	MPID_SMI_comm_info_t *ci = (MPID_SMI_comm_info_t *)newcomm->adiCollCtx;
	uint dim, i, j, lproc, lnode;
	uint grid_size, grid_offset[MAX_SCI_DIMS] = { 16, 16, 16};
	uint sci_id, min_pos[MAX_SCI_DIMS], max_pos[MAX_SCI_DIMS], pos, x_pos, y_pos, z_pos;

	if (ci->nbr_lnodes > 1) {
		for (dim = 0; dim < ci->sci_ndims; dim++) {
			min_pos[dim] = 64;
			max_pos[dim] = 0;
			for (lproc = 0; lproc < ci->lsize; lproc++) {
				sci_id = MPID_SMI_grank_to_sciid[newcomm->lrank_to_grank[lproc]];
				switch (dim) {
				case X_DIM:
					pos = (sci_id % SCI_ID_YOFFSET)/SCI_ID_STRIDE - 1;
					break;
				case Y_DIM:
					pos = (sci_id - MIN_SCI_ID) / SCI_ID_YOFFSET;
					break;
				case Z_DIM:
					pos = (sci_id - MIN_SCI_ID) / SCI_ID_ZOFFSET;
					break;
				}
				min_pos[dim] = (pos < min_pos[dim]) ? pos : min_pos[dim];
				max_pos[dim] = (pos > max_pos[dim]) ? pos : max_pos[dim];
			}
			ci->lnodes_in_dim[dim] = (max_pos[dim] != min_pos[dim])? 
				max_pos[dim] - min_pos[dim] + 1 : 0;
		}
	} else {
		/* Special case of only a single active node/process. */
		ci->lnodes_in_dim[X_DIM] = 1;
		ci->lnodes_in_dim[Y_DIM] = ci->lnodes_in_dim[Z_DIM] = 0;
	}
	for (dim = 0, ci->active_ndims = 0; dim < ci->sci_ndims; dim++) {
		if (ci->lnodes_in_dim[dim] > 0)
			ci->active_ndims++;
	}

	/* Build (virtual) topology mapping of the nodes. First, calculate
	   the size of the sub-grid within the physical SCI grid. */
	grid_size = 1;
	for (j = 0; j < ci->sci_ndims; j++) {
		if (ci->lnodes_in_dim[j] > 0)
			grid_size *= ci->lnodes_in_dim[j];
	}
	for (j = 0; j < MAX_SCI_DIMS; j++) {
		ALLOCATE(ci->lnode_to_grid[j], int *, ci->nbr_lnodes*sizeof(int));
	}
	ALLOCATE (ci->grid_to_lnode, int *, grid_size * sizeof(int));

	/* Determine the position of each proc in the subgrid and the postions of the subgrid 
	   witthin the physical SCI grid (offset in each dimension). */
	for (lproc = 0; lproc < ci->lsize; lproc++) {
		/* On which lnode is this lproc running, and is it the master? */
		lnode = ci->lproc_to_lnode[lproc];
		sci_id = MPID_SMI_grank_to_sciid[newcomm->lrank_to_grank[lproc]] - MIN_SCI_ID;
	  
		/* X-dimension */
		pos = (sci_id % SCI_ID_YOFFSET) / SCI_ID_STRIDE;
		if (pos < grid_offset[X_DIM])
			grid_offset[X_DIM] = pos;
		ci->lnode_to_grid[X_DIM][lnode] = pos;

		/* Y-dimension */
		if (ci->sci_ndims >= 2) {
			pos = sci_id / SCI_ID_YOFFSET;
			if (pos < grid_offset[Y_DIM])
				grid_offset[Y_DIM] = pos;
			ci->lnode_to_grid[Y_DIM][lnode] = pos;
		}

		/* Z-dimension */
		if (ci->sci_ndims == 3) {
			pos = sci_id /(SCI_ID_YOFFSET + SCI_ID_STRIDE);
			if (pos < grid_offset[Z_DIM])
				grid_offset[Z_DIM] = pos;
			ci->lnode_to_grid[Z_DIM][lnode] = pos;
		}
	}
	/* Normalize the positions with the determined offsets. */
    for (lnode = 0; lnode < ci->nbr_lnodes; lnode++) {
		ci->lnode_to_grid[X_DIM][lnode] -= grid_offset[X_DIM];
		if (ci->sci_ndims >= 2)
			ci->lnode_to_grid[Y_DIM][lnode] -= grid_offset[Y_DIM];
		if (ci->sci_ndims == 3)
			ci->lnode_to_grid[Z_DIM][lnode] -= grid_offset[Z_DIM];		
	}  
	
	/* Normalize distribution and postion of nodes in dimensions to X if "empty" dimensions do 
	   exist. This has the same effect for SCI communication, but avoids confusion with
	   0-size dimensions. */
	for (dim = 0; dim < ci->sci_ndims - 1; dim++) {
		if (ci->lnodes_in_dim[dim] == 0) {
			ci->lnodes_in_dim[dim] = ci->lnodes_in_dim[dim+1];
			ci->lnodes_in_dim[dim + 1] = 0;
			
			for (lnode = 0; lnode < ci->nbr_lnodes; lnode++) {
				ci->lnode_to_grid[dim][lnode] = ci->lnode_to_grid[dim+1][lnode];
			}  
		}
	}

	/* Initialize all lnode-ranks in the grid to -1; then set the points in
	   the grid which represent active nodes to the real lnode ranks. */
	for (j = 0; j < grid_size; j++) {
		ci->grid_to_lnode[j] = -1;
	}
	for (lnode = 0; lnode < ci->nbr_lnodes; lnode++) {
		x_pos = ci->lnode_to_grid[X_DIM][lnode];
		y_pos = ci->active_ndims >= 2 ? ci->lnode_to_grid[Y_DIM][lnode] : 0;
		z_pos = ci->active_ndims == 3 ? ci->lnode_to_grid[Z_DIM][lnode] : 0;
		
		ci->grid_to_lnode[z_pos*ci->lnodes_in_dim[Y_DIM]*ci->lnodes_in_dim[X_DIM] +
						 y_pos*ci->lnodes_in_dim[X_DIM] + x_pos] = lnode;
	}	
	
	/* Determine distance to each rank */
	ZALLOCATE (ci->distnc_lrank, int *, ci->lsize * sizeof(int));
	sci_id = MPID_SMI_grank_to_sciid[MPID_SMI_myid];
	for (i = 0; i < ci->lsize; i++) {
		for (j = 0; j < ci->sci_ndims; j++) {
			if (MPID_SMI_gnodes_in_dim[j] > 0) {
				/* XXX is this calculation correct ? */
				ci->distnc_lrank[i] += ((MPID_SMI_grank_to_sciid[newcomm->lrank_to_grank[i]] 
										 + MPID_SMI_gnodes_in_dim[j]*(4 << (4*j)) - sci_id)
										/ (4 << (4*j))) % MPID_SMI_gnodes_in_dim[j];
			}
		}
	}
	
	/* Now calculate distance for each dimension separately. */
	for (i = 0; i < ci->sci_ndims; i++) {
		ZALLOCATE (ci->dim_distnc_lrank[i], int *, ci->lsize * sizeof(int));
	}
	for (i = 0; i < ci->lsize; i++) {
		sci_id = MPID_SMI_grank_to_sciid[newcomm->lrank_to_grank[i]];
		/* XXX to be done */
	}	
	
	return;
}

static void get_sci_topology (struct MPIR_COMMUNICATOR *newcomm )
{
	MPID_SMI_comm_info_t *ci = (MPID_SMI_comm_info_t *)newcomm->adiCollCtx;
	int topo_type;
  
	/* Determine dimensions of SCI network in which our processes operate */
	ci->sci_ndims = MPID_SMI_sci_dims;
	ci->lnodes_in_dim[X_DIM] = 0;
	ci->lnodes_in_dim[Y_DIM] = 0;
	ci->lnodes_in_dim[Z_DIM] = 0;

	/* Determine number of nodes per dimension with processes of this communicator running: 
	   Check in every dimension for most left and most right node. 
	   This range is equal to the number of nodes in this dimension */
	SMIcall (SMI_Query (SMI_Q_SCI_SCI_TOPOLOGY_TYPE, 0, &topo_type));
	switch (topo_type) {
#ifdef DOLPHIN_SCI
	/* XXX get_torus_sci_topology() is not yet done for Scali topology rules */
	case SMI_SCI_TOPOLOGY_TORUS:
		get_torus_sci_topology(newcomm);
		break;
#endif
	case SMI_SCI_TOPOLOGY_SWITCH:
	case SMI_SCI_TOPOLOGY_UNKNOWN:
	case SMI_SCI_TOPOLOGY_SMP:
		get_other_sci_topology(newcomm);
		break;
	default:
		get_other_sci_topology(newcomm);
		break;
	}
	
	return; 
}


/* Called for each new communicator.  */
int MPID_SMI_Comm_init( struct MPIR_COMMUNICATOR *oldcomm, struct MPIR_COMMUNICATOR *newcomm )
{
	struct MPIR_COMMUNICATOR *comm;
	MPID_SMI_comm_info_t *ci;
	int node, proc, i, lproc, lnode, gnode;
	int *topology_buffer, **process_topology, *masterOnNode;
	
	MPID_STAT_ENTRY(comm_init);
	MPID_ASSERT(newcomm != NULL, "NULL communicator");
	
	newcomm->msgform = MPID_MSG_OK;
	comm = newcomm;
	
	ZALLOCATE( comm->adiCollCtx, void *, sizeof( MPID_SMI_comm_info_t ) );
	ci = (MPID_SMI_comm_info_t *)comm->adiCollCtx;
    
	/* Get size of communicator and my rank in communicator */
	MPIR_Comm_size ( comm, &(ci->lsize) );
	ci->lrank = comm->local_rank;
    
	/* Allocate and initialize array for number of processes running on each node which
	   are in my communicator (this replaces MPID_SMI_numProcsOnNode for this variable
	   cannot be used if not all processes in MPI_COMM_WORLD are in my communicator) */
	ALLOCATE( ci->procs_on_gnode, int *, MPID_SMI_numNodes * sizeof(int));
	for (node = 0; node < MPID_SMI_numNodes; node++ )
		ci->procs_on_gnode[node] = 0;
	/* Loop over all processes in communicator and increment counter for node 
	   on which proc is running */
	for (proc = 0; proc < ci->lsize; proc++ ) {
		ci->procs_on_gnode[MPID_SMI_procNode[comm->lrank_to_grank[proc]]]++;
	}

	/* Check if we have an even process-to-node distribution, which is helpful to know for 
	   SMP-oriented optimization.  If 'lprocs_per_node' is > 0, it gives the number
	   of processes of this communicator running on any node (even/synmetric distribution). 
	   If we have an asymetric distribution,  'lprocs_per_node' is set to 0. */
	for (node = 0, ci->lprocs_per_node = 0; node < MPID_SMI_numNodes; node++) {      
		if (ci->procs_on_gnode[node] > 0) {
			if (ci->lprocs_per_node == 0)
				ci->lprocs_per_node = ci->procs_on_gnode[node];
			else 
				if (ci->procs_on_gnode[node] != ci->lprocs_per_node) {
					/* asymetric distribution! */
					ci->lprocs_per_node = 0;
					break;
				}
		}
	}
  
	/* The topology_buffer is a vector with one element per process of this
	   communicator.
	   The process_topology is a vector with one element per node
	   of the application. For a node not running a process of this commmunicator,
	   this element is NULL. Elements for nodes with processes of this communicator 
	   contain a vector with one element per process (part of the topology_buffer)
	   containing the ranks of the processes running on this node. */
	ALLOCATE( ci->topology_buffer, int *, ci->lsize * sizeof(int) );
	topology_buffer = ci->topology_buffer;
	ALLOCATE( ci->lranks_on_gnode, int **, MPID_SMI_numNodes * sizeof(int *) );
	process_topology = ci->lranks_on_gnode;

	/* find first node on which communicator processes are running and set according pointer */
	node = 0;
	while (ci->procs_on_gnode[node] == 0) {
		process_topology[node] = NULL;
		node++;
	}
	process_topology[node] = topology_buffer;
	/* Let the elements of the process topology point to the processes on each node. */
	for (i = node + 1; i < MPID_SMI_numNodes; i++) {
		if (ci->procs_on_gnode[i] == 0)
			process_topology[i] = NULL;
		else {
			process_topology[i] = process_topology[node] + ci->procs_on_gnode[node];
			node = i;
		}
	}

	{
		int last_proc, new_last_proc;
  
		/* Now we initialize the elements of the array: loop over all nodes 
		   on which communicator processes are running .*/
		for( node = 0; node < MPID_SMI_numNodes; node++ ) {
			if( process_topology[node] != NULL ) {
				/* Find process with lowest comm rank on this node. */
				last_proc = (ci->lsize);
				for (proc = 0; proc < ci->lsize; proc++ ) {
					if (MPID_SMI_procNode[comm->lrank_to_grank[proc]] == node && proc < last_proc)
						last_proc = proc;
				}
				process_topology[node][0] = last_proc;
				/* Now find the other procs in comm running on this node. */
				lproc = 1;
				while (lproc < ci->procs_on_gnode[node]) {
					new_last_proc = (ci->lsize);
					for( proc = 0; proc < (ci->lsize); proc++ ) {
						if( (MPID_SMI_procNode[comm->lrank_to_grank[proc]] == node) 
							&& ( proc > last_proc ) && ( proc < new_last_proc ) )
							new_last_proc = proc;
					}
					process_topology[node][lproc] = new_last_proc;
					last_proc = new_last_proc;
					lproc++;
				}
			}
		}
	}

	/* Calculate the number of nodes on which communicator processes are running. */
	ci->nbr_lnodes = 0;
	for (node = 0; node < MPID_SMI_numNodes; node++) {
		if (ci->procs_on_gnode[node] > 0)
			ci->nbr_lnodes++;
	}
    
	/* Build an array which holds the process ranks of the master processes for every node;
	   the master process is the process with the lowest comm rank on that node, i.e. the process
	   whos comm rank is in process_topology[node][0] */
	ALLOCATE( ci->master_on_gnode, int *, MPID_SMI_numNodes * sizeof(int) );
	masterOnNode = ci->master_on_gnode;
	for (node = 0; node < MPID_SMI_numNodes; node++) {
		masterOnNode[node] = (ci->procs_on_gnode[node] == 0)? -1 : process_topology[node][0];
	}
	
	ci->nsize = ci->procs_on_gnode[MPID_SMI_myNode];
	
	/* Now we can find our comm rank in process_topology and set our local_rank. */
	for (lproc = 0; lproc < ci->nsize; lproc++) {
		if (process_topology[MPID_SMI_myNode][lproc] == ci->lrank)
			ci->nrank = lproc;
	}
	
	/* Build mapping from local node number to global node number. */
	ALLOCATE( ci->lnode_to_gnode, int *, ci->nbr_lnodes * sizeof(int));
	node = 0;
	for (lnode = 0; lnode < ci->nbr_lnodes; lnode++) {
		while (ci->procs_on_gnode[node] == 0)
			node++;
		ci->lnode_to_gnode[lnode] = node;
		node++;
	}

	/* Calculate the local rank of my node. */
	for (lnode = 0; lnode < ci->nbr_lnodes; lnode++) {
		if (ci->lnode_to_gnode[lnode] == MPID_SMI_myNode)
			ci->lnode = lnode;
	}
    
	/* Build mapping from local proc number to local node number. */
	ALLOCATE( ci->lproc_to_lnode, int *, ci->lsize * sizeof(int));
	node = 0;
	for (lproc = 0; lproc < ci->lsize; lproc++) {
		/* Find global node number and do a reverse lookup in lnode_to_gnode. */
		gnode = MPID_SMI_procNode[newcomm->lrank_to_grank[lproc]];
		for (i = 0; i < ci->nbr_lnodes; i++) {
			if (ci->lnode_to_gnode[i] == gnode) {
				ci->lproc_to_lnode[lproc] = i;
				break;
			}
		}
	}

	get_sci_topology (newcomm);

	MPID_STAT_EXIT(comm_init);
	return MPI_SUCCESS;
} 

int MPID_SMI_Comm_free (struct MPIR_COMMUNICATOR *comm )
{
	MPID_SMI_comm_info_t *comm_info;
	int i;

	comm_info = (MPID_SMI_comm_info_t *)(comm->adiCollCtx);

	/* the collective shadow communicator has no comm_info */ 
	if (comm_info != NULL) {
		if (MPID_SMI_cfg.COLLOPS && comm->np > 1) {
			/* XXX: The xy_destroy functions should be in a function like MPID_SMI_Collops_free(). */
			if (comm->collops->Barrier == MPID_SMI_Barrier)
				barrier_destroy(comm_info);

			MPID_SMI_Pcast_coll_destroy(comm_info);

			switch (MPID_SMI_cfg.COLL_ALLTOALL_TYPE) {
			case ALLTOALL_SCI_1D:
			case ALLTOALL_SCI_2D:
			case ALLTOALL_SCAMPI:
				MPID_SMI_alltoall_destroy_topology(comm_info);
				break;
			case ALLTOALL_PLAIN:
				MPID_SMI_alltoall_destroy_plain(comm_info);
				break;
			}
		}
		
		FREE( comm_info->procs_on_gnode );
		FREE( comm_info->topology_buffer );
		FREE( comm_info->lranks_on_gnode );
		FREE( comm_info->master_on_gnode );
		FREE( comm_info->lnode_to_gnode );
		FREE( comm_info->lproc_to_lnode );

		FREE( comm_info->distnc_lrank );
		for (i = 0; i < comm_info->sci_ndims; i++) 
			FREE (comm_info->dim_distnc_lrank[i]);
		
		FREE( comm_info->grid_to_lnode );
		for (i = 0; i < MAX_SCI_DIMS; i++)
			FREE (comm_info->lnode_to_grid[i]);

		FREE( comm->adiCollCtx );
	}

	return MPI_SUCCESS;
}


/* called from MPID_CH_InitMsgPass */
int MPID_SMI_Pipe_setup() {
    /* fixed-size-block memory manager */
    mpid_smi_pipe_alignbuf_allocator = MPID_SBinit (MPID_SMI_STREAMSIZE, 
													PIPE_INIT_ALIGNBUFS, PIPE_INCR_ALIGNBUFS);

    return MPI_SUCCESS;
}

int MPID_SMI_Pipe_delete() {
    MPID_SBdestroy (mpid_smi_pipe_alignbuf_allocator);

    return MPI_SUCCESS;
}


/* Free the memory for the inbuffer of a pipelined rendez-vous recv. */
void mpid_smi_pipe_free_recvbuf(ulong sgmt_offset, int from_drank)
{
    char *free_addr;

    /* transform offset into a ptr */
    free_addr = MPID_SMI_use_localseg[from_drank] ?
		MPID_SMI_rndv_shmempool : MPID_SMI_rndv_scipool;
	free_addr += sgmt_offset;

	MPID_SMI_shfree( free_addr );
}

int mpid_smi_pipe_get_recvbuf (sgmt_id, sgmt_offset, adpt_nbr, len, from_grank)
ulong *sgmt_offset;
int   *len, from_grank, *sgmt_id, *adpt_nbr;
{
    char *addr;
    int tlen, shreg_id, ptr_len, ret = MPI_SUCCESS, use_local_shmem;

    if (*len == 0)
		return ret;
    
    /* The buffer we allocate is for the data, the two pointers for  flow-control 
       (both placed at the end of a streambuffer) and an additional streambuffer-sized
       piece of memory for alignment of the complete buffer. */
    ptr_len  = 2*MPID_SMI_STREAMSIZE;
    MPID_SMI_STREAMBUF_ALIGN (*len);
    tlen = *len + MPID_SMI_BRNDV_PTRMEM;

    /* determine the shared memory region to use */
    use_local_shmem = MPID_SMI_use_localseg[from_grank];
    *sgmt_id  = use_local_shmem ? -1 : MPID_SMI_Rndv_sgmt_id;
    *adpt_nbr = use_local_shmem ? -1 : MPID_SMI_Rndv_adpt_nbr;

    shreg_id = use_local_shmem ? MPID_SMI_rndv_shmempool_regid : MPID_SMI_rndv_scipool_regid;
    addr = (char *)MPID_SMI_shmalloc( tlen, shreg_id );
    while (!addr) {
		MPID_SMI_DEBUG_PRINT_MSG("Allocating partial space for long message");
		tlen = tlen / 2; 
		while ((tlen > 8 * MPID_SMI_STREAMSIZE) 
			   && !(addr = (char *)MPID_SMI_shmalloc( tlen, shreg_id )) )
	    tlen = tlen / 2;
		if (tlen <= (8 * MPID_SMI_STREAMSIZE)) {
			MPID_ASSERT (MPID_SMI_Rndvrecvs_in_progress > 0, 
						 "Could not allocate rendez-vous buffer although no recvs are active.");
			
			MPID_DeviceCheck( MPID_NOTBLOCKING );
			tlen = *len;
		}
    }
	
    /* transform ptr into an offset */
    *sgmt_offset = use_local_shmem ? (size_t)(addr - (size_t)MPID_SMI_rndv_shmempool) : 
	(size_t)(addr - (size_t)MPID_SMI_rndv_scipool);
    *len = tlen;
	
    /* Initialize the read- and write-ptrs which are located in the first 2 streambuffers. */
    if (tlen > MPID_SMI_BRNDV_PTRMEM) {
		memset (addr, 0, MPID_SMI_BRNDV_PTRMEM);
    }
	
    return ret;
}


void mpid_smi_pipe_map_remote_mem (int rmt_grank, MPID_PKT_PIPE_T *pkt, MPIR_SHANDLE *shandle)
{
    int map_size, map_offset, map_flags, map_tries, err;
	
	if (!MPID_SMI_use_localseg[rmt_grank]) {
		map_tries = 0;
		do {
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
				map_size = pkt->len_avail;
				map_offset = pkt->sgmt_offset;
				map_flags = 0;
		    }			
		    err = MPID_SMI_Rmt_mem_map (rmt_grank, pkt->sgmt_id, map_size, 
										map_offset, pkt->adpt_nbr, 
										&shandle->recv_handle->dest_addr, 
										&shandle->recv_handle->smi_regid_dest, map_flags);
		    if (err != MPIR_ERR_EXHAUSTED && MPID_SMI_numNodes == 1) {
				shandle->recv_handle->dest_addr = (char *)shandle->recv_handle->dest_addr
					+ pkt->sgmt_offset;
		    }
		    map_tries++;
		    while (err == MPIR_ERR_EXHAUSTED && MPID_SMI_Rndvsends_in_progress > 0)
				MPID_DeviceCheck (MPID_NOTBLOCKING);
		} while (err == MPIR_ERR_EXHAUSTED && map_tries <= MPID_SMI_RNDV_CNCT_MAX_RETRIES);
		
		/* No access to recv buffer possible -> we have to give up! */
		MPID_ASSERT (err != MPIR_ERR_EXHAUSTED, "Could not access recv buffer.");
	} else {
		/* use the local SMP region */
		shandle->recv_handle->dest_addr      = MPID_SMI_rndv_shmempool + pkt->sgmt_offset;
		shandle->recv_handle->smi_regid_dest = MPID_SMI_rndv_shmempool_regid;
	}
	
	return;
}


/* For the current version of the DMA transfer protocol, we need certain
 *identical* buffers between the participating processes. A more flexible 
 version of this protocol may handle other sizes, too. */
int mpid_smi_pipe_get_dmabuf_size (int msglen)
{
	int dmabuf_size;

	if (MPID_SMI_cfg.USE_DMA_COLL && msglen >= MPID_SMI_cfg.COLL_PIPE_DMA_MIN) {
		dmabuf_size = MPID_MIN(msglen, mpid_smi_pipe_bsize*mpid_smi_pipe_nbrblocks);
		if (dmabuf_size % mpid_smi_pipe_bsize != 0)
			dmabuf_size = (dmabuf_size/mpid_smi_pipe_bsize + 1) * mpid_smi_pipe_bsize;
		
		if (dmabuf_size == msglen)
			dmabuf_size += mpid_smi_pipe_bsize;
	} else {
		dmabuf_size = MPID_MIN(msglen, mpid_smi_pipe_bsize*mpid_smi_pipe_nbrblocks);
	}

	return dmabuf_size;
}

int mpid_smi_pipe_do_dma (int msglen, struct MPIR_COMMUNICATOR *comm, MPID_SMI_comm_info_t *ci)
{
#if 0
	int minbuf;
#endif
	/* Some good reasons why not to use DMA */
	if ( !MPID_SMI_cfg.USE_DMA_COLL ||
		 msglen < MPID_SMI_cfg.COLL_PIPE_DMA_MIN ||
		 comm->local_group->np != ci->nbr_lnodes  )
		return 0;

#if 0
	/* what is the smallest buffer anyone did get? */
	MPI_Allreduce (&mpid_smi_pipe_rhandle.recv_handle->len_local, &minbuf, 1, MPI_INT, MPI_MIN, comm->self);
	/* XXX: TO BE DONE: choose new buffersize */
#endif
	
	/* And finally (USE_IT) */
	return 1;
}

/* send pipe_ready and set recv_handler */
int mpid_smi_send_pipeready_pkt (int target_grank, struct MPIR_COMMUNICATOR *comm, 
								 int size, ulong *sgmt_offset) 
{
    MPID_PKT_PIPE_T prepkt;
    MPID_SMI_CTRLPKT_T	pkt_desc;
    int ret = MPI_SUCCESS;
	int sgmt_id, adpt_nbr, local_len;

    MPID_SMI_DEBUG_PRINT_MSG("S About to get pkt for pipe_ready");
    MPID_GETSENDPKT (pkt_desc, sizeof (MPID_PKT_PIPE_T), &prepkt, 0, NULL, target_grank, 0);

	/* Negative size to disable the allocation limit in get_recvbuf() */
	local_len = mpid_smi_pipe_get_dmabuf_size (size);

	/* This should always get at least some amount of memory! */
    ret = mpid_smi_pipe_get_recvbuf (&sgmt_id, sgmt_offset, &adpt_nbr, &local_len, target_grank);
	MPID_ASSERT	(ret == MPI_SUCCESS, "Not enough memory for pipeline operation!");
	
	MPID_INIT_PIPE_PREPKT(prepkt, comm->send_context, comm->local_rank, size, *sgmt_offset, 
						   sgmt_id, adpt_nbr, local_len, 0);
    
    MPID_SMI_DEBUG_PRINT_SEND_PKT("S Sending pipe_ready message", &prepkt, target_grank);
    while (MPID_SMI_SendControl (&pkt_desc) != MPI_SUCCESS)
		;

    /* save buffer information in rhandle */
    mpid_smi_pipe_rhandle.start = (MPID_SMI_use_localseg[target_grank]) ? 
		MPID_SMI_rndv_shmempool + *sgmt_offset : MPID_SMI_rndv_scipool + *sgmt_offset;
    mpid_smi_pipe_rhandle.len   = size;
    mpid_smi_pipe_rhandle.recv_handle->len_local = local_len;

    return (ret);
}


/* reveice pipe_ready and set send_handler. */
int MPID_SMI_Pipe_init (MPID_PKT_T *in_pkt, int from_grank) {
    MPID_PKT_PIPE_T recv_pkt;
    int ret = MPI_SUCCESS, dim;

	MPID_TRACE_CODE ("Pipe_init", from_grank);
	MPID_SMI_DEBUG_PRINT_MSG("Entering Pipe_init");

	dim = mpid_smi_pipe_ready_arrived;

    /* initialize shandle */
    MPID_Send_init( &mpid_smi_pipe_shandle[dim] );
	mpid_smi_pipe_shandle[dim].recv_handle = &mpid_smi_pipe_shandle_recv_handle[dim];
	memset(mpid_smi_pipe_shandle[dim].recv_handle, 0, 
		   sizeof(mpid_smi_pipe_shandle_recv_handle[dim]));

    MEMCPY (&recv_pkt, in_pkt, sizeof(MPID_PKT_PIPE_T));
    MPID_SMI_FreeRecvPkt( in_pkt, from_grank, IS_CTRL_MSG);

    /* make sure the segment is connected: If region id is invalid, we still need to
       connect/import the remote memory. */
	switch (recv_pkt.mode) {
	case MPID_PKT_PIPE_READY:
		mpid_smi_pipe_map_remote_mem (from_grank, &recv_pkt, &mpid_smi_pipe_shandle[dim]);
	    break;
	default:
	    /* no other modes supported by this protocol */
	    MPID_ABORT ("Illegal send mode in Pipe_init.");
	    break;
	}

    mpid_smi_pipe_shandle[dim].recv_handle->len_local = recv_pkt.len_avail;
    mpid_smi_pipe_shandle[dim].bytes_as_contig = recv_pkt.len;
	
	mpid_smi_pipe_ready_arrived++;

    return (ret);
}


/* 
 *  Shared-memory barrier with separate intra-node and inter-node check-in
 *  and checkout. 
 */

/* Allocate and initialize the shared-memory datastructures for the efficient
   shared-memory barrier. */
static int barrier_init( struct MPIR_COMMUNICATOR *nc,  MPID_SMI_comm_info_t *ci)
{
    struct barrier_info *B;
    int node_checkin_rank, down_node_min, up_node, i, drank;
    ulong nodemem_size, offset, down_node_offset, up_node_offset;
    char *nodemem_addr, *up_node_addr, *down_node_addr;
#define FANIN MPID_SMI_cfg.COLL_BARRIER_FANIN 
	
    if (nc->np == 1)
		return MPI_SUCCESS;
	
    B = &ci->barrier;
    memset (B, 0, sizeof(struct barrier_info));
    B->bcnt = 1;

    nodemem_addr = (char *)SMI_Nodemem_address(MPID_SMI_myNode);
    MPID_ASSERT(nodemem_addr != NULL, "Couldn't determine base address of node-local global memory!");

    if (ci->lrank == ci->master_on_gnode[MPID_SMI_myNode]) {
		/* How many other nodes will check in at this node? To know this, 
		   we need to calculate this nodes position in the whole topology. */		
		up_node = (ci->lnode == 0) ? -1 : (ci->lnode - 1)/FANIN;
		node_checkin_rank = (ci->lnode - 1) % FANIN;

		down_node_min = ci->lnode * FANIN + 1;
		B->nbr_nodes_down = down_node_min >= ci->nbr_lnodes ? 0 : 
			ci->nbr_lnodes - down_node_min;
		if (B->nbr_nodes_down > FANIN)
			B->nbr_nodes_down = FANIN;
		if (B->nbr_nodes_down > 0)
		{
			ALLOCATE(B->checkout_nodes_down, uint **, B->nbr_nodes_down*sizeof(uint *));
		}

		/* Allocate globally-accessable node-local memory and distribute the offset to
		   the master procs of the nodes which checkin/out from via this node. */
		nodemem_size = MPID_SMI_STREAMSIZE*(FANIN + 2) + ci->nsize*sizeof(uint);
		MPID_STAT_CALL(nodemem_alloc);
		B->nodemem = (char *)SMI_Nodemem_alloc(nodemem_size);
		MPID_STAT_RETURN(nodemem_alloc);
	} else {
		B->nodemem = (char *)0x1; /* dummy value != NULL */
	}

	{ 
		char *min_addr;
		struct MPIR_DATATYPE *ulong_type;

		/* make sure all nodemaster-processes could allocate the memory */
		ulong_type = MPIR_GET_DTYPE_PTR(MPI_UNSIGNED_LONG);
		nc->collops->Allreduce (&B->nodemem, &min_addr, 1, ulong_type, MPI_MIN, nc);
		if (min_addr == NULL) {
			if (B->nodemem != NULL && B->nodemem != (char *)0x1)
				SMI_Nodemem_free (B->nodemem);
			
			return MPI_ERR_NOMEM;
		}
	}

    if (ci->lrank == ci->master_on_gnode[MPID_SMI_myNode]) {
		memset (B->nodemem, 0, nodemem_size);
		offset = (ulong)((char *)(B->nodemem) - nodemem_addr);

		/* publish the offset of the barrier memory */
		do {
			for (i = 0; i < ci->lsize; i++) {
				MPID_SMI_Int_info_exp[nc->lrank_to_grank[i]]->Barrier_setup.bmem_offset = offset;
			}
		} while (SMI_Check_transfer(CHECK_MODE) != SMI_SUCCESS);

		nc->collops->Barrier(nc);

		/* After the barrier, it is safe to read the required information from the other nodes. */
		if (up_node >= 0) {
			/* First, from upper node... */
			drank = nc->lrank_to_grank[ci->master_on_gnode[ci->lnode_to_gnode[up_node]]];
			up_node_offset = MPID_SMI_Int_info_imp[drank]->Barrier_setup.bmem_offset;
			up_node_addr = (char *)SMI_Nodemem_address(ci->lnode_to_gnode[up_node]);
			B->checkin_node_up = (uint *)(up_node_addr + up_node_offset +
										  (node_checkin_rank+1)*MPID_SMI_STREAMSIZE - sizeof(uint));
		}
		/* ... then, from lower nodes */
		for (i = down_node_min; i < down_node_min + B->nbr_nodes_down; i++) {
			drank = nc->lrank_to_grank[ci->master_on_gnode[ci->lnode_to_gnode[i]]];
			down_node_offset = MPID_SMI_Int_info_imp[drank]->Barrier_setup.bmem_offset;
			down_node_addr = (char *)SMI_Nodemem_address(ci->lnode_to_gnode[i]);
			B->checkout_nodes_down[i - down_node_min] = (uint *)(down_node_addr + down_node_offset +
																 (FANIN + 1)*MPID_SMI_STREAMSIZE - sizeof(uint));
		}
		
		/* Now initialize the other pointers to local nodemem, where other processes do 
		   check-ins or check-outs. */
		if (up_node >= 0) {
			B->checkout_node_up = (uint *)(B->nodemem 
										   + (FANIN + 1)*MPID_SMI_STREAMSIZE - sizeof(uint));
		}

		if (B->nbr_nodes_down > 0) {
			ALLOCATE(B->checkin_nodes_down, uint **, B->nbr_nodes_down*sizeof(uint*));
			for (i = 0; i < B->nbr_nodes_down; i++) {
				B->checkin_nodes_down[i] = (uint *)(B->nodemem 
													+ (i+1)*MPID_SMI_STREAMSIZE - sizeof(uint));
			}
		}

		if (ci->nsize > 1) {
			ALLOCATE (B->checkin_local_procs, uint **, ci->nsize*sizeof(uint*));
			for (i = 0; i < ci->nsize - 1; i++) {
				B->checkin_local_procs[i] = (uint *)(B->nodemem 
													 + (FANIN + 2)*MPID_SMI_STREAMSIZE + i*sizeof(uint));
			}
			B->checkout_local_procs = (uint *)(B->nodemem 
											   + (FANIN + 1)*MPID_SMI_STREAMSIZE);
		}
    } else {
		/* This process is a node-slave in the barrier, just has to wait to get the 
		   check-in and -out pointers. It reads from local shared memory, thus no SCI
		   transfer checking. */
		nc->collops->Barrier(nc);

		drank = nc->lrank_to_grank[ci->master_on_gnode[MPID_SMI_myNode]];
		offset = MPID_SMI_Int_info_imp[drank]->Barrier_setup.bmem_offset;
		B->checkin_local = (uint *)(nodemem_addr + offset +
									(FANIN + 2)*MPID_SMI_STREAMSIZE + (ci->nrank - 1)*sizeof(uint));
		B->checkout_local = (uint *)(nodemem_addr + offset +
									 (FANIN + 1)*MPID_SMI_STREAMSIZE);
    }

#undef FANIN
    return MPI_SUCCESS;
}

static void barrier_destroy (MPID_SMI_comm_info_t *comm_info)
{
    if (comm_info->lrank == comm_info->master_on_gnode[MPID_SMI_myNode]
		&& comm_info->lsize > 1) {
		MPID_STAT_CALL(nodemem_free);
		SMI_Nodemem_free (comm_info->barrier.nodemem);
		MPID_STAT_RETURN(nodemem_free);
		if (comm_info->barrier.nbr_nodes_down > 0) {
			FREE (comm_info->barrier.checkout_nodes_down);
			FREE (comm_info->barrier.checkin_nodes_down);
		}
		if (comm_info->nsize > 1) {
			FREE (comm_info->barrier.checkin_local_procs);
		}
    }

    return;
}

int MPID_SMI_Barrier(struct MPIR_COMMUNICATOR *comm)
{
	MPID_SMI_comm_info_t *ci = (MPID_SMI_comm_info_t *)(comm->adiCollCtx);
	struct barrier_info *B = &ci->barrier;
	int checkin_cnt, i;
	ulong poll_cnt, poll_mask;

	MPID_STAT_ENTRY(barrier);    

	if (comm->np > 1) {
		if (ci->lrank == ci->master_on_gnode[MPID_SMI_myNode]) {
			/* First, wait for checkin of local processes */
			checkin_cnt = poll_cnt = poll_mask = 0;
			while (checkin_cnt < ci->nsize - 1) {
				for (i = 0; i < ci->nsize - 1; i++)
					if ((poll_mask & (1 << i)) == 0 && *B->checkin_local_procs[i] == B->bcnt) {
						checkin_cnt++;
						poll_mask |= (1 << i);
					}
				if (poll_cnt++ % BARRIER_POLL_CHECKDEV == 0) {
					MPID_DeviceCheck( MPID_NOTBLOCKING );
				}
			}
			/* Then, wait for checkin of lower nodes. */
			checkin_cnt = poll_cnt = poll_mask = 0;
			while (checkin_cnt < B->nbr_nodes_down) {
				for (i = 0; i < B->nbr_nodes_down; i++)
					if ((poll_mask & (1 << i)) == 0 && *B->checkin_nodes_down[i] == B->bcnt) {
						checkin_cnt++;
						poll_mask |= (1 << i);
					}
				if (poll_cnt++ % BARRIER_POLL_CHECKDEV == 0) {
					MPID_DeviceCheck( MPID_NOTBLOCKING );
				}
			}
			/* Now, we can checkin at upper node (if applicable) and wait for checkout. */
			if (B->checkin_node_up != NULL) {
				do {
					*B->checkin_node_up = B->bcnt;
				} while (SMI_Check_transfer(CHECK_MODE) != SMI_SUCCESS);
				poll_cnt = 0;
				while (*B->checkout_node_up < B->bcnt) {
					if (poll_cnt++ % BARRIER_POLL_CHECKDEV == 0) {
						MPID_DeviceCheck( MPID_NOTBLOCKING );
					}
				}
			}
			/* Finally, initiate the checkout for the other local procs and the
			   lower nodes. */
			if (ci->nsize > 1)
				*B->checkout_local_procs = B->bcnt;
			if (B->nbr_nodes_down > 0) {
				do {
					for (i = 0; i < B->nbr_nodes_down; i++) 
						*B->checkout_nodes_down[i] = B->bcnt;
				} while (SMI_Check_transfer(CHECK_MODE) != SMI_SUCCESS);
			}
		} else {
			/* A slave only checks in locally and then waits for the checkout. */
			*B->checkin_local = B->bcnt;
			poll_cnt = 0;
			while (*B->checkout_local < B->bcnt) {
				if (poll_cnt++ % BARRIER_POLL_CHECKDEV == 0) {
					MPID_DeviceCheck( MPID_NOTBLOCKING );
				}
			}
		}
		B->bcnt++;
	}

	MPID_STAT_EXIT(barrier);
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

