/* $Id$ */

#ifndef _MPID_SMI_REGIONMNGMT_H
#define _MPID_SMI_REGIONMNGMT_H

/*	Management of SCI memory ressources/connections via SMI regions.

	Zero-copy operations required that local communication buffers
	need to be registered, and that connections to remote communication
	buffers be set up dynamically to transfer data as required by
	an MPI communication call. When the communication is finished,
	the allocated resources need to be free'd again to avoid 
	the exhaustion of these resources.

	Both these operations do cost time, and to improve the effective 
	communication performance, it is desirable not to perform these
	operation for every MPI communication call, but instead only
	once (in the best case). However, just not de-allocating the
	involved resources (local registration, remote connections) is
	not a solution as some of the involved resources are limited;
	if one of these resources is exhausted, subsequent demands for
	zero-copy operations would fail, reducing the effective performance.

	Therefore, we install a layer between the device communication 
	protocols and the SMI library to manage the allocation and 
	possibly required de-allocation of the involved resources by
	resource monitoring. In a first approach, this is done via "in-use" 
	counters and "least-recently-used" 	replacement strategy. These
	strategies are subject to optimization, i.e. by considering the
	size (which is related to the overhead in time) of a resource. */

#include "smi.h"

#include "smidef.h"
#include "smitypes.h"
#include "sbcnst2.h"
#include "hash.h"
#include "fifo.h"
#include "tree.h"
#include "smidev.h"
#include "mpid.h"

/* Store the region info for de-allocated regions to save the related scheduling information? */
/* XXX: do not enable - erraeonous behaviour observed! */
#define STORE_DEALLOCATED    0
/* If the mapping of a complete remote region fails, try to map only the required part of it? 
   May save your day in low-ressource situations, but could also increase ressource usage in
   certain situations. */
#define TRY_PARTIAL_MAPPING  1
/* Set to '1' to trace region management activities. */
#define DO_RSRC_MNGMT_DEBUG  0
#if DO_RSRC_MNGMT_DEBUG
#define RSRC_MNGMT_DEBUG(dbg) dbg
#else
#define RSRC_MNGMT_DEBUG(dbg) 
#endif

#if 0
extern MPID_SMI_LOCK_T MPID_SMI_connect;
#endif

#define SCI_ID_SIZE       16  /* length in bit of an SCI node and segment id */
#define HASHTABLE_SIZE    53  /* size of the used hash-tables */

/* for fixed-size-block allocation */
#define INIT_REGION_INFOS 64
#define INCR_REGION_INFOS 32

/* flags for the function calls below */
#define MPID_SMI_RSRC_CACHE        1
#define MPID_SMI_RSRC_DESTROY      (1<<1) /*2*/
#define MPID_SMI_RSRC_PARTIAL_MAP  (1<<2) /*4*/

#define MPID_SMI_RSRC_FLAGS        (1+(1<<1)+(1<<2)) /*7*/

#define INCR_USE_ONLY              (1<<3) /*8*/

/* internal flags */
#define LOCAL_FREE_REQ             1
#define REMOTE_FREE_REQ            (1<<1) /*2*/

/* states for remote-release requests */
#define RSRC_REQ_PENDING           1
#define RSRC_RELEASE_ACK           (1<<1) /*2*/
#define RSRC_RELEASE_NACK          (1<<2) /*4*/

/* error/return code */
#define MPID_SMI_ISSCI    1

#define KEY_GEN(sgmt_id, owner, type) (((ulong)(sgmt_id)<<SCI_ID_SIZE) \
									   |((ushort)MPID_SMI_procNode[owner] << 2) \
                                       | type)

#define INIT_USE(reginfo) (reginfo)->in_use = 1; \
                          (reginfo)->access_count = 1; \
                          (reginfo)->last_used = SMI_Wticks(); \
                          (reginfo)->random_key = random(); \
                          (reginfo)->deallocated = 0;
#define INCR_USE(reginfo) (reginfo)->in_use++; \
                          (reginfo)->access_count++; 
#define DECR_USE(reginfo) if ((reginfo)->in_use > 0) { (reginfo)->in_use--;  \
                          (reginfo)->last_used = SMI_Wticks(); }


typedef struct {
	/* region properties */
	int smi_regid;
	int owner;
	int sci_sgmtid;
	void *address;
	size_t offset;
	int is_persistent;   /* for persistent communication (not yet used) */
	int is_SCI;          /* is a regular SCI segment which was not created by region mngmt. */
	MPID_SMI_rsrc_type_t type;
	int is_valid;

	/* scheduling information - 'len' is also a property */
	size_t len;
	int   in_use;        /* how many transfers are currently under way? */
	int   access_count;  /* total number of accesses */
	ulong last_used;     /* wtime of last access */
	long  random_key;    /* for random-scheduling */
	int   deallocated;   /* how often has this resource been de-allocated? */
} MPID_SMI_region_info;

typedef MPID_SMI_region_info* MPID_SMI_region_info_t;

/* Iniitialize the scheduler and specify the resource scheduling strategy to be used.

   Input:
   strategy		the resource scheduling strategy
*/
void MPID_SMI_Rsrc_sched_init (MPID_SMI_rsrc_sched_strategy_t strategy);

/* Terminate the scheduler. All allocated resources are de-allocated, no matter
   if they are in use or not.
   
   Return value:
   Number of resources that were still in use on finalization (should be zero
   for proper shutdown). */
int MPID_SMI_Rsrc_sched_finalize (void);

/* Provide a connection to the remote SCI segment specified by sci_sgmtid. 
   Returns the SMI region id which can be used for SMI_Put()/SMI_Get()
   operations.

   This function connects to the remote segment (via SCIConnectSegment())
   if necessary by installing a SMI region of type RDMA. 
   In any case, it increments the usage counter and updates the usage time-stamp.

   Input:
   rmt_procrank			rank of the process which exports the segment
   sci_sgmtid			SCI segment id of remote process

   Output:
   smi_regid			SMI region id related to the remote segment

   Return value:
   MPI_SUCCESS			smi_regid contains valid SMI region id
   MPI_ERR_EXHAUSTED	remote region could not be connected.
*/
int MPID_SMI_Rmt_region_connect (int rmt_procrank, int sci_sgmtid, int rmt_adptr, int *smi_regid);

/* Indicate that the connection to the specified remote SCI segment is 
   no longer needed.

   This function decrements the usage counter related to the region. It does
   not effectively disconnect from the remote region.

   Input:
   smi_regid			SMI region id of the remote segment
   flag                 inidicat if segment shold be cached or released

   Return value:
   MPI_SUCCESS			usage counter has been decremented
   MPI_ERR_EXHAUSTED	illeagal SMI region id
*/
int MPID_SMI_Rmt_region_release (int smi_regid, int flag);

/* Ensure that a user-allocated local memory region can be used for SCI
   communication.

   This function returns the reference to a local SCI segment, backed up 
   by the memory buffer supplied by the user. If necessary, it creates
   this SCI segment. In any case, the related buffer can subsequently be
   used to server as a DMA source region or as a target for memory transfers
   of remote processes which connect to this segment, the usage
   counter is incremented and the usage time-stamp updated.

   Input:
   buf				address of the user-allocated memory buffer
   len				size of this buffer in bytes

   Output:
   smi_regid		SMI region id related to the SCI segment
   sci_sgmtid		SCI segment id, which can be passed to remote processes

   Return value:
   MPI_SUCCESS		    buffer is registered, status has been updated
   MPID_SMI_ISSCI       buffer is an SCI region not under control of this interface
   MPIR_ERR_EXHAUSTED	buffer could not be registered and is no SCI segment
*/
int MPID_SMI_Local_mem_register (void *buf, size_t len, int *smi_regid, int *sci_sgmtid);

/* Indicate that the local buffer is in use by other means. */
int MPID_SMI_Local_mem_use (void *buf);

/* Indicate that the local buffer is no longer used for SCI communication.

   Decrements the usage counter of the related SMI region. Does not necessarily
   de-register the segment.

   Input:
   buf					address of the registered buffer 
						If a valid SMI region id is supplied via smi_regid, buf
						may pass a NULL ptr.
   smi_regid			SMI region id of the related SCI segment. This parameter is
						only evaluated if buf is NULL.
   flag                 inidicat if segment shold be cached or released

   Return value:
   MPI_SUCCESS			usage counter has been decremented
   MPI_ERR_EXHAUSTED	invalid buffer pointer or SMI region id
*/
int MPID_SMI_Local_mem_release (void *buf, int smi_regid, int flag);


/* Create a local shared memory region (shmem or SCI).

   Input:
   size					size of the local segment to be created
   min_size             allowed minimum size of the segment
                        (set to 0 if only exact size is allowed)

   Output:
   size                 actual size of the segment
   buf					address of a pointer which will contain the
						address of the local buffer in case of success
   smi_regid			SMI region id related to the local segment
   sci_sgmtid   		SCI segment id, which can be passed to remote processes

   Return value:
   MPI_SUCCESS			smi_regid contains valid SMI region id, and buf can
						be used to access the memory
   MPI_ERR_EXHAUSTED	remote region could not be mapped
*/
int MPID_SMI_Local_mem_create (size_t *len, size_t min_size, void **buf, 
  							   int *smi_regid,  int *sci_sgmtid);

/* Provide a mapped memory representation of remote SCI segment specified 
   by sci_sgmtid. Returns the SMI region id which can be used for 
   SMI_Put()/SMI_Get() operations and the memory address for arbitrary
   CPU driven memory operations.

   This function connects to the remote segment (via SCIConnectSegment())
   if necessary and maps it to the user address space (via SCIMapRemoreSegment())
   by creating a SMI region of type REMOTE. 
   In any case, it increments the usage counter and updates the usage time-stamp.

   Input:
   rmt_procrank			rank of the process which exports the segment
   sci_sgmtid			SCI segment id of remote process
   len					size of the mapping to be established
						(may be 0 to map the complete segment from 
						 'offset' to end of segment)
   offset				offset for the mapping (>= 0)
   flags                pass possible flags:
                        MPID_SMI_RSRC_PARTIAL_MAP  only map the specified part of 
                                                   the remote mem; do not tryp to map
                                                   the complete region.
   Output:
   buf					address of a pointer which will contain the
						address of the remote buffer in case of success
   smi_regid			SMI region id related to the remote segment

   Return value:
   MPI_SUCCESS			smi_regid contains valid SMI region id, and buf can
						be used to access the memory
   MPI_ERR_EXHAUSTED	remote region could not be mapped
*/
int MPID_SMI_Rmt_mem_map (int rmt_procrank, int sci_sgmtid, 
						  size_t len, size_t offset, int rmt_adptr,
						  void **buf, int *smi_regid, int flags);

/* Indicate that the mapped memory related to the specified address / SMI 
   region is no longer needed.

   This function decrements the usage counter related to the region. It does
   not effectively unmap the remote region.

   Input:
   buf					address of the remote region (may be NULL).
   smi_regid			SMI region id of the remote segment. This is only
						evaluted if buf is NULL.
   flag                 inidicat if segment shold be cached or released

   Return value:
   MPI_SUCCESS			usage counter has been decremented
   MPI_ERR_EXHAUSTED	invalid buffer adddress or SMI region id 
*/
int MPID_SMI_Rmt_mem_release (void *buf, int smi_regid, int flag);


/* Effectively destroy the indicated resource, no caching is done. The resource
   referenced by the address or the region id may be a local SCI segment,
   a remote map or a remote connection.

   Input:
   buf					address of the remote region (may be NULL).
   smi_regid			SMI region id of the remote segment. This is only
						evaluted if buf is NULL.

   Return value:
   MPI_SUCCESS			usage counter has been decremented
   MPI_ERR_EXHAUSTED	invalid buffer adddress or SMI region id 
*/
int MPID_SMI_Rsrc_destroy (void *buf, int smi_regid);


/*
 * internal prototypes
 */
#ifdef USE_INTERNAL_PROTOTYPES
static int free_resource (MPID_SMI_rsrc_type_t type, int flags);
static int cache_resource (MPID_SMI_region_info_t reginfo, MPID_SMI_rsrc_type_t type);
static int destroy_resource (MPID_SMI_region_info_t reginfo, MPID_SMI_rsrc_type_t type);
static void sched_init (MPID_SMI_rsrc_sched_strategy_t strategy);
static void sched_finalize (void);
static int region_fits (int reg_type, void **buf, ulong sgmt_offset, 
						size_t len, MPID_SMI_region_info_t reginfo);
static int acquire_resource (int procrank, int *sci_sgmtid, int rmt_adptr, size_t len, 
							 size_t offset, int reg_type, void **buf, int *smi_regid, int flags);
static int release_resource (void *regaddr, int smi_regid, MPID_SMI_rsrc_type_t type, int flag);
#endif /* USE_INTERNAL_PROTOTYPES */


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
