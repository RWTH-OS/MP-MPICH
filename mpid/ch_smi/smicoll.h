/* $Id$ */

#ifndef _MPID_SMI_COLL
#define _MPID_SMI_COLL

#ifndef WIN32
#include <dlfcn.h>
#include <limits.h> /* for INT_MAX, gotta check if this header is present on Windows (I suppose it is) */
#else

#endif

#include "mpiimpl.h"

#include "comm.h"
#include "coll.h"
#include "mpicoll.h"

#include "sbcnst2.h"

#include "mpid.h"
#include "smidev.h"
#include "smimem.h"
#include "smidef.h"

#include "smipackets.h"
#include "reqrndv.h"
#include "smirndv.h"


/*
 * relevant parameters of the custom implemenations 
 */
/* Message-size threshold to perform a barrier between phases in MPI_Allgather() */
#if 0
#define ALLGATHER_BARRIER_LIMIT (128*1024)
#else
#define ALLGATHER_BARRIER_LIMIT INT_MAX
#endif
#define USE_TOPOLOGY_ALLTOALL    1
#define USE_CUSTOM_ALLTOALL      1
#define BARRIER_POLL_CHECKDEV    (128)
/* packets greater BLOCK_INC_LIMIT get a blocksize BIG_BLOCK */
#define PIPE_BLOCK_INC_LIMIT 2097152
#define PIPE_BIG_BLOCK 65536

/* external interface to pipeline memory buffer setup */
int MPID_SMI_Pipe_setup(void);
int MPID_SMI_Pipe_delete(void);
int MPID_SMI_Pipe_init (MPID_PKT_T *, int);
int mpid_smi_pipe_get_dmabuf_size (int );
int mpid_smi_send_pipeready_pkt (int, struct MPIR_COMMUNICATOR *, int, ulong *);
void mpid_smi_pipe_free_recvbuf(ulong , int);
int mpid_smi_pipe_get_recvbuf (int*, ulong*, int*, int*, int);
void mpid_smi_pipe_map_remote_mem (int, MPID_PKT_PIPE_T*, MPIR_SHANDLE*);


#define PIPE_INIT_ALIGNBUFS  3
#define PIPE_INCR_ALIGNBUFS  2


/* calling reduce operations "made easy" by these macros;
   these macros swap the first two functions parameters, which is necessary when
   running a tree-based reduce algorithm which sends data from upper to lower ranks */
#ifdef WIN32

#define CALL_OP_FUNC(_outbuf, _inbuf, _count, _op_ptr, _dtype_ptr) \
         if ( (_op_ptr)->commute ) { \
                  if ( (_op_ptr)->stdcall) \
                   (_op_ptr)->op_s( (_inbuf), (_outbuf), &(_count), &(_dtype_ptr)->self); \
              else \
                    (*(_op_ptr)->op)( (_inbuf), (_outbuf), &(_count), &((_dtype_ptr)->self)); \
         } else { \
                    if ( (_op_ptr)->stdcall)  \
                    (_op_ptr)->op_s( (_outbuf), (_inbuf), &(_count), &(_dtype_ptr)->self); \
              else \
                    (*(_op_ptr->op))( (_outbuf), (_inbuf), &(_count), &((_dtype_ptr)->self)); \
                    MEMCPY( (_outbuf), (_inbuf), ( (_dtype_ptr)->ub - (_dtype_ptr)->lb ) *(_count)); \
        }

#else

#define CALL_OP_FUNC(_outbuf, _inbuf, _count, _op_ptr, _dtype_ptr) \
         if ( (_op_ptr)->commute ) { \
                    (_op_ptr)->op( (_inbuf), (_outbuf), (int *)&(_count), &((_dtype_ptr)->self)); \
         } else { \
                    (_op_ptr)->op( (_outbuf), (_inbuf), (int *)&(_count), &((_dtype_ptr)->self)); \
                    MEMCPY( (_outbuf), (_inbuf), ( (_dtype_ptr)->ub - (_dtype_ptr)->lb ) *(_count)); \
        }

#endif /* WIN32 */


/* the macros don't swap parameters and may be used when sending from lower to upper ranks */

#ifdef WIN32

#define CALL_OP_FUNC_NOSWAP(_outbuf, _inbuf, _count, _op_ptr, _dtype_ptr) \
	if ( (_op_ptr)->stdcall)											\
		(_op_ptr)->op_s( (_outbuf), (_inbuf), &(_count), &(_dtype_ptr)->self); \
	else																\
		(*(_op_ptr->op))( (_outbuf), (_inbuf), &(_count), &((_dtype_ptr)->self));

#else

#define CALL_OP_FUNC_NOSWAP(_outbuf, _inbuf, _count, _op_ptr, _dtype_ptr) \
                    (_op_ptr)->op( (_outbuf), (_inbuf), (int *)&(_count), &((_dtype_ptr)->self));


#endif /* WIN32 */



#if !defined(WIN32) && defined(MPI_SHARED_LIBS)
/* these are function ponters to the required MPI pt2pt functions - we can not call them 
   directly as this would result in cross-dependecies of the libraries */
extern int (*MPICH_Send)(void*, int, MPI_Datatype, int, int, MPI_Comm);
extern int (*MPICH_Recv)(void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Status *);
extern int (*MPICH_Isend)(void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
extern int (*MPICH_Irecv)(void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
extern int (*MPICH_Waitall)(int, MPI_Request *, MPI_Status *);
extern int (*MPICH_Test)(MPI_Request *, int *, MPI_Status *);
extern int (*MPICH_Barrier)(MPI_Comm);
#else
/* This is only applicable possible if everything is in one big library. 
   For Unix, we only can use custom collective functions with dynamic linking. */
#define MPICH_Send     MPI_Send		      
#define MPICH_Recv     MPI_Recv
#define MPICH_Isend    MPI_Isend
#define MPICH_Irecv    MPI_Irecv
#define MPICH_Waitall  MPI_Waitall
#define MPICH_Test     MPI_Test
#define MPICH_Barrier  MPI_Barrier
#endif

/* XXX: These globals (from smicoll.c) make the Pipe non-threadsafe! This doesn't matter
   as long as MP-MPICH itself isn't threadsafe anyway. */
extern MPIR_SHANDLE mpid_smi_pipe_shandle[MAX_SCI_DIMS];
extern MPIR_RHANDLE mpid_smi_pipe_rhandle;
extern MPID_SBHeader mpid_smi_pipe_alignbuf_allocator;

extern MPID_SMI_RNDV_info bla[MAX_SCI_DIMS];
extern MPID_SMI_RNDV_info mpid_smi_pipe_shandle_recv_handle[MAX_SCI_DIMS];
extern MPID_SMI_RNDV_info mpid_smi_pipe_rhandle_recv_handle;

extern int mpid_smi_pipe_bsize;
extern int mpid_smi_pipe_nbrblocks;
extern volatile int mpid_smi_pipe_ready_arrived;

/*
 * prototypes
 */
int MPID_SMI_Collops_init(struct MPIR_COMMUNICATOR*, MPIR_COMM_TYPE);
int MPID_SMI_Comm_init( struct MPIR_COMMUNICATOR *oldcomm, 
			struct MPIR_COMMUNICATOR *newcomm );
int MPID_SMI_Comm_free (struct MPIR_COMMUNICATOR *comm );

/* these are the device-specific collective functions */
int MPID_SMI_Barrier(struct MPIR_COMMUNICATOR *);
int MPID_SMI_Alltoall_straight(void*, int, struct MPIR_DATATYPE *, void*, int, 
							   struct MPIR_DATATYPE *, struct MPIR_COMMUNICATOR *);
int MPID_SMI_Alltoallv_straight(void*, int *, int *, struct MPIR_DATATYPE *, void*, int *, 
								int *, struct MPIR_DATATYPE *, struct MPIR_COMMUNICATOR *);
int MPID_SMI_Alltoall_plain(void*, int, struct MPIR_DATATYPE *, void*, int, 
							struct MPIR_DATATYPE *, struct MPIR_COMMUNICATOR *);
int MPID_SMI_Alltoallv_plain(void*, int *, int *, struct MPIR_DATATYPE *, void*, int *, 
							 int *, struct MPIR_DATATYPE *, struct MPIR_COMMUNICATOR *);
int MPID_SMI_Alltoall_topology(void*, int, struct MPIR_DATATYPE *, void*, int, 
							   struct MPIR_DATATYPE *, struct MPIR_COMMUNICATOR *);
int MPID_SMI_Alltoallv_topology(void*, int *, int *, struct MPIR_DATATYPE *, void*, int *, 
								int *, struct MPIR_DATATYPE *, struct MPIR_COMMUNICATOR *);
int MPID_SMI_Allgather(void*, int, struct MPIR_DATATYPE *, void *, int, 
					   struct MPIR_DATATYPE *, struct MPIR_COMMUNICATOR *);
int MPID_SMI_Allgatherv(void*, int, struct MPIR_DATATYPE *, void *, int *, int *, 
						struct MPIR_DATATYPE *, struct MPIR_COMMUNICATOR *);
int MPID_SMI_Allgatherv_sendrecv (void *sendbuf, int sendcnt, struct MPIR_DATATYPE *sendtype,
								  void *recvbuf, int *recvcnts, int *displs, struct MPIR_DATATYPE *recvtype,
								  struct MPIR_COMMUNICATOR *comm);
int MPID_SMI_Pcast (void *buffer, int count, struct MPIR_DATATYPE *datatype, 
					int root, struct MPIR_COMMUNICATOR *comm );
int MPID_SMI_Reduce (void *sendbuf, void *recvbuf, int count, struct MPIR_DATATYPE *datatype,
					 MPI_Op op, int root, struct MPIR_COMMUNICATOR *comm);
int MPID_SMI_Allreduce (void *sendbuf, void *recvbuf, int count, struct MPIR_DATATYPE *datatype,
						MPI_Op op, struct MPIR_COMMUNICATOR *comm);
int MPID_SMI_Reduce_scatter (void *sendbuf, void *recvbuf, int *recvcnts, 
							 struct MPIR_DATATYPE *datatype, MPI_Op op, 
							 struct MPIR_COMMUNICATOR *comm );
int MPID_SMI_Scan (void *sendbuf, void *recvbuf, int count, struct MPIR_DATATYPE *datatype,
				   MPI_Op op, struct MPIR_COMMUNICATOR *comm);
int MPID_SMI_Scatter (void *sendbuf, int sendcnt, struct MPIR_DATATYPE *sendtype, 
					  void *recvbuf, int recvcnt, struct MPIR_DATATYPE *recvtype, int root, 
					  struct MPIR_COMMUNICATOR *comm );
int MPID_SMI_Scatter_seq (void *sendbuf, int sendcnt, struct MPIR_DATATYPE *sendtype, 
						  void *recvbuf, int recvcnt, struct MPIR_DATATYPE *recvtype, int root, 
						  struct MPIR_COMMUNICATOR *comm );
int MPID_SMI_Scatterv (void *sendbuf, int *sendcnts, int *displs, struct MPIR_DATATYPE *sendtype, 
					   void *recvbuf, int recvcnt,  struct MPIR_DATATYPE *recvtype, 
					   int root, struct MPIR_COMMUNICATOR *comm );
int MPID_SMI_Scatterv_seq (void *sendbuf, int *sendcnts, int *displs, struct MPIR_DATATYPE *sendtype, 
						   void *recvbuf, int recvcnt,  struct MPIR_DATATYPE *recvtype, 
						   int root, struct MPIR_COMMUNICATOR *comm );
int MPID_SMI_Gather_allreduce (void *sendbuf, void *recvbuf, int count, struct MPIR_DATATYPE *datatype,
						       MPI_Op op, struct MPIR_COMMUNICATOR *comm);
	
/* this is the original MPICH-internal Barrier function for fallback */
extern int (*MPICH_msg_barrier)(struct MPIR_COMMUNICATOR *);


/*
 * types 
 */
struct barrier_info {
	/* The counter value of the next barrier synchronization. */
	uint bcnt;
	
	/* Where to check-in at upper node, and where to receive check-out from upper node. */
	uint *checkin_node_up;
	uint *checkout_node_up;
	/* Where to wait for check-ins of lower nodes, and of local processes */
	uint nbr_nodes_down;
	uint **checkin_nodes_down;
	uint **checkin_local_procs;
	/* Where to signal check-out for lower nodes and local processes */
	uint **checkout_nodes_down;
	uint *checkout_local_procs;

	/* Slaves need only this: where to check-in and -out with local master process. */
	uint *checkin_local;
	uint *checkout_local;

	/* Node-local block of global memory. */
	char *nodemem;
};

struct alltoall_info {
  /* vector containing the node with which to exchange data in each round (plain) */
  int *comm_partner;

  /* vector containing the nodes with which to exchange data in each round,
     separated for send & recv operations (topology & scampi) */
  int *send_to;
  int *recv_from;

  /* number of rounds in which nodes exchange their data in MPI_SMI_Alltoall*/
  int nbr_rounds;
};

struct bcast_info {
	/* For multidimensional pipelined broadcast, the communication partners are 
	   not simply myrank+1 and myrank-1. We need to store this information here:
	   for each different process being a potential root, we need lsize different
	   routing schemes which are calculated on demand and then "cached" in these
	   arrays of local ranks which are indexed by the root. Entries of "-1" mean 
	   that the routing for this process being root has not yet been calculated. */
	int *send_to;
	int *recv_from;

	/* If this process is root, it sends to these processes. */
	int root_send_to[MAX_SCI_DIMS];
};

/* General topology information needed in communicator and datastructures for collective. 
   Used prefixes: 
     l = communicator-relative ("local")
	 g = MPI_COMM_WORLD-relative ("global"), also device-relative for single-device
	 n = node-relative */
typedef struct {
    int lsize;                 /* size of communicator */
    int lrank;                 /* my rank in communicator (lrank) */

    int *procs_on_gnode;       /* number of processes on each node which are part of this communicator */
    int *master_on_gnode;      /* lrank of "master process" on a gnode;  
								  -1 if no process of communicator is running on this node */
    int **lranks_on_gnode,     /* For each gnode, an array with the lranks of the procs on it. */
		*topology_buffer;

    int lnode;                 /* communicator-local rank of this node */
    int nbr_lnodes;            /* number of nodes on which communicator processes are running */
    int *lnode_to_gnode;       /* mapping from local to global node rank */
    int *lproc_to_lnode;       /* mapping from local proc rank to local node rank */
	int lprocs_per_node;       /* > 0: number of processes of this comm on any active node (symetric distribution)
								  == 0: asymetric distribution */
    
    int nsize;                 /* number of processes on this process' node */
    int nrank;                 /* local rank on this node (0 .. nsize-1) */

    /* virtual carthesian topology (sub-network of physical SCI network) */
    int sci_ndims;             /* number of SCI dimensions */
    int active_ndims;          /* number of active SCI/ring dimension */
    int *dim_distnc_lrank[MAX_SCI_DIMS]; /* distance towards other process for each of sci_ndims dimension */
    int *distnc_lrank;                   /* dimension-independant distance to process */
    int lnodes_in_dim[MAX_SCI_DIMS];     /* max. number of processes in each SCI dimension */
    int *lnode_to_grid[MAX_SCI_DIMS];    /* coordinate triple (X, Y, Z) for each lnode */
    int *grid_to_lnode;                  /* grid->lnode information for get_lnode_at() function */
    
    /* Specific information for individual collective operations. */
    struct alltoall_info alltoall;
    struct barrier_info  barrier;
	struct bcast_info    bcast;
} MPID_SMI_comm_info_t;

/* More prototypes (which require some of the types above) */
void MPID_SMI_Pcast_coll_init (MPID_SMI_comm_info_t *ci);
void MPID_SMI_Pcast_coll_destroy (MPID_SMI_comm_info_t *ci);
int mpid_smi_pipe_do_dma (int, struct MPIR_COMMUNICATOR*, MPID_SMI_comm_info_t*);

/* Get the rank of an lnode located at a specified position in the 
   virtual grid (subgrid) of a communicator. < 0 means "no active node
   at this position" */
#define GRID_TO_LNODE(ci, x, y, z) (ci)->grid_to_lnode[\
   (z)*ci->lnodes_in_dim[Y_DIM]*(ci)->lnodes_in_dim[X_DIM] * ((ci)->lnodes_in_dim[Z_DIM] > 0) \
   + (y)*ci->lnodes_in_dim[X_DIM] * ((ci)->lnodes_in_dim[Y_DIM] > 0) + (x)]
/* The other direction. */
#define LNODE_TO_GRID(ci, lnode, x, y, z) x = (ci)->lnode_to_grid[X_DIM][lnode]; \
	y = (ci)->active_ndims > 1 ? (ci)->lnode_to_grid[Y_DIM][lnode] : -1; \
	z = (ci)->active_ndims > 2 ? (ci)->lnode_to_grid[Z_DIM][lnode] : -1; 

/* Get the address of a symbol from a currently loaded DLL. If dlopen() fails,
   this means we have compiled with shared libs, but the active executable was
   linked with the big static lib. In this case, we can just assign the ptrs 
   because they are all in one library. */
#if MPI_SHARED_LIBS
#if (defined MPI_solaris) || (defined MPI_solaris86)
#define GET_DLL_FCTNPTR(symbol, ptr, type) ptr = (type)dlsym(RTLD_DEFAULT, symbol); 
#elif (defined MPI_LINUX) || (defined MPI_LINUX_ALPHA) || (defined MPI_LINUX_IA64) || (defined MPI_LINUX_X86_64)


#define GET_DLL_FCTNPTR(symbol, ptr, type) { void *dll_handle; \
	dll_handle = dlopen (NULL, RTLD_LAZY); \
    if(!dll_handle) \
    ptr = (type)symbol; \
    else { \
  	ptr = (type)dlsym (dll_handle, symbol); \
	dlclose (dll_handle); } }


#elif (defined WIN32)
/* not required for WIN32 because of monolithic library */
#endif
#else
#define GET_DLL_FCTNPTR(symbol, ptr, type) ptr = symbol;
#endif

#endif




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
