/* $Id$ */

/*	The SCI segment resources are described by a MPID_SMI_region_info entry.
	To manage these resource, a number of data structures are maintained:
    
	- A hash table for all segment resources which are currently in use. If
	  a connection/mapping/registering is requested, it is first checked
	  with this hash table if the resource has already been established. If
	  yes, the required information can directly be returned.
	- Another hash table for all segment resources which had been in use,
	  but are currently not established. If a new resource has to be made
	  available, it is first checked with this hash table if it had been
	  in use. This ensures that information like 'access_count' is not lost 
	  once a resource is de-allocated.
	- Depending on the scheduling strategy for de-allocating resources if
	  a new segment resource can not be established (due to resource shortage),
	  one or more data structures are required to manage the resources which
	  are currently established, but have are not currently used ('in_use' is 
	  0) and are thus subject to de-allocation. These resources are kept in 
	  a sorted list, with a suitable sorting key for the chosen scheduling
	  strategy. Possible strategies are:
	  - LRU: the least recently used resource is de-allocated 
	    => 'last_used' entry is primary sort criterium (FIFO-list)
	  - LFU: the least frequently used resource is de-allocated
	    => 'access_count' is primary sort criterium
	  - Best-Fit: the resource which best fits the new resource request
	    is de-allocated (here: relevant for mapped remote segments)
		=> 'size' is primary sort criterium
	  - Random: the resource to be de-allocated is determined by random choice.
	    => 'random_key' is primary sort criterium
	  - Immediate: the resource is de-allocated as soon as it is no longer in use.
	    This strategy is only for performance-comparison.
		=> no list required

	  If mulitple strategies are used concurrently, multiple sort lists need
	  to be maintained. Also, a list can have multiple, ranked sorting keys.
*/

#define USE_INTERNAL_PROTOTYPES
#include "smiregionmngmt.h"
#undef USE_INTERNAL_PROTOTYPES
#ifdef WIN32
#define srandom srand
#define random rand
#endif

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#ifdef MPID_USE_DEVTHREADS
static MPID_SMI_LOCK_T MPID_SMI_regionmngmt;
#endif

static MPID_hash_table_t regions_in_use;      /* currently used, not to be de-allocated */
static MPID_hash_table_t usable_regions;      /* currently unused, to be de-allocated or re-used */
static MPID_hash_table_t deallocated_regions; /* have been de-allocated, stored to preserve scheduling info */ 
static MPID_FIFO_t regions_to_destruct;       /* have to be de-allocated due to callback */ 
static MPID_SBHeader reginfo_allocator;

static MPID_SMI_rsrc_sched_strategy_t sched_strat = LRU;
static MPID_tree_t LRU_tree, LFU_tree, RANDOM_tree;

/* For inter-process resource-balancing. */
static int first_proc_on_node, last_proc_on_node, next_req;
static volatile int rmt_rsrc_released;

/* Compare functions for the tree insertion/search */
static int _mpid_smi_lfu_cmp (void *data1, void *data2) 
{
	MPID_SMI_region_info_t ri_1 = (MPID_SMI_region_info_t)data1;
	MPID_SMI_region_info_t ri_2 = (MPID_SMI_region_info_t)data2;
	int retval;

	retval = (ri_1->access_count < ri_2->access_count) ? -1 :
		((ri_1->access_count > ri_2->access_count) ? 1 : 0);

	return retval;
}

static int _mpid_smi_lru_cmp (void *data1, void *data2) 
{
	MPID_SMI_region_info_t ri_1 = (MPID_SMI_region_info_t)data1;
	MPID_SMI_region_info_t ri_2 = (MPID_SMI_region_info_t)data2;
	int retval;

	retval = (ri_1->last_used < ri_2->last_used) ? -1 :
		((ri_1->last_used > ri_2->last_used) ? 1 : 0);

	return retval;
}

static int _mpid_smi_random_cmp (void *data1, void *data2) 
{
	MPID_SMI_region_info_t ri_1 = (MPID_SMI_region_info_t)data1;
	MPID_SMI_region_info_t ri_2 = (MPID_SMI_region_info_t)data2;
	int retval;

	retval = (ri_1->random_key < ri_2->random_key) ? -1 :
		((ri_1->random_key > ri_2->random_key) ? 1 : 0);

	return retval;
}

/* Generate a hash key for a region_info. */
static size_t _mpid_smi_keygen (void *data)
{
  MPID_SMI_region_info_t reginfo = (MPID_SMI_region_info_t)data;
  size_t key;

  key = (reginfo == NULL) ? 0 : KEY_GEN(reginfo->sci_sgmtid, reginfo->owner, reginfo->type);

  return key;
}


static void schedule_rsrc_for_destruction (int smi_regid)
{
	int *id;
	
	MPID_ALLOCATE (id, int *, sizeof(int));
	*id = smi_regid;

	MPID_FIFO_push (regions_to_destruct, id);
	
	return;
}


static int _mpid_smi_region_cb (int region_id, int cb_reason) 
{
	RSRC_MNGMT_DEBUG (fprintf(stderr, "[%d] region_cb: was called for region %d, reason is %d\n", 
							  MPID_SMI_myid, region_id, cb_reason););

	if (cb_reason == SMI_CB_REASON_DISCONNECT) {
		/* A remote region has been removed, we need to de-allocate the local
		   region which is/was connected to it. */
		MPID_STAT_COUNT( rsrc_cb_disconnect );
#if 1
		release_resource (0, region_id, ANY, MPID_SMI_RSRC_DESTROY);	
#else
		schedule_rsrc_for_destruction (region_id);
#endif
	}

	RSRC_MNGMT_DEBUG (fprintf(stderr, "[%d] region_cb: terminating\n", MPID_SMI_myid););
	return SMI_CB_ACTION_TERMINATE;
}


/* Free the next resource according to the current scheduling strategy. 

   Input: 
   type         optional specificication of the resource type
                XXX: the type is currently not considered!
   flags        is the request a local or a remote request?
				
   Return Value:
   0            no resource of the specified type could be free'd
   1            free'd a resource of the specified type
*/
static int free_resource (MPID_SMI_rsrc_type_t type, int flags)
{
	MPID_SMI_region_info_t reginfo;
	int rsrc_freed = 0, give_up = 0, found, rsrc_req_sent = 0;
	smi_error_t smi_err;

	while (!rsrc_freed && !give_up) {
		do {
			switch (sched_strat) {
			case IMMEDIATE:
				/* not applicable */
				break;
			case LRU:
				reginfo = (MPID_SMI_region_info_t)MPID_tree_remove_smallest (LRU_tree);
				break;
			case LFU:
				reginfo = (MPID_SMI_region_info_t)MPID_tree_remove_smallest (LFU_tree);
				break;
			case BEST_FIT:
				/* XXX not yet implemented */
				break;
			case RANDOM:
				reginfo = (MPID_SMI_region_info_t)MPID_tree_remove_smallest (RANDOM_tree);
				break;
			case NONE:
				MPID_SMI_UNLOCK(&MPID_SMI_regionmngmt);
				MPID_ABORT ("free_resource(): Insufficent SCI resources.\n\
NONE scheduling strategy does not allow to free resources.");
				break;
			}	
			/* a region that we get from the queue might already be in use 
			   again -> check for 'in_use' counter! */
			if ((reginfo != NULL) && !reginfo->is_SCI && (reginfo->in_use == 0) && reginfo->is_valid) {
				smi_err = SMI_Free_shreg(reginfo->smi_regid);
				
				if (smi_err == SMI_SUCCESS) {
					MPID_STAT_COUNT(rsrc_destroy);
					reginfo->is_valid = 0;
					rsrc_freed = 1;
					break;
				}
			} 
		} while (reginfo != NULL);
		
		/* this region is no longer usable */
		if (rsrc_freed) {
			MPID_hash_data_remove (usable_regions, (void *)reginfo, &found);
			MPID_ASSERT (found == 1, "free_resource(): could not remove region from usable region table.");
#if STORE_DEALLOCATED
			MPID_hash_store (deallocated_regions, (void *)reginfo);
#else
			MPID_SBfree (reginfo_allocator, reginfo);
#endif
		} else {
			/* XXX: Open problem: How often should we ask each process to release 
			   a resource? Once, twice, forever? */
			if ((MPID_SMI_numProcsOnNode[MPID_SMI_myNode] > 1) && (flags & LOCAL_FREE_REQ) 
				&& (rsrc_req_sent < 4*MPID_SMI_numProcsOnNode[MPID_SMI_myNode])
				&& MPID_SMI_is_initialized) {
				/* We could not free a resource in the local process. If more than one 
				   process of this application is running on the local node, we request
				   it to release a resource for us. This avoids the situation that one
				   process "steals" all resources from another one. */
				MPID_SMI_CTRLPKT_T pkt_desc;
				MPID_PKT_RSRC_T    prepkt;
				int p;
				
				for (p = 0; p < MPID_SMI_numProcsOnNode[MPID_SMI_myNode]; p++) {
					MPID_STAT_COUNT(rsrc_release_req);
					rsrc_req_sent++;

					rmt_rsrc_released = RSRC_REQ_PENDING;
					
					/* Send a reqeust to the next process on this node */
					MPID_GETSENDPKT (pkt_desc, sizeof(MPID_PKT_RSRC_T), &prepkt, 0, NULL, next_req, 0);
					MPID_INIT_RSRC_REQ_PREPKT(prepkt, 0, MPID_SMI_myid, 0, ANY);
					
					MPID_SMI_DEBUG_PRINT_SEND_PKT("S Sending rsrc_req message", &prepkt, next_req);
					while (MPID_SMI_SendControl(&pkt_desc) != MPI_SUCCESS)
						;
					
					/* Now wait until the response of the remote process has been received. */
					MPID_SMI_UNLOCK(&MPID_SMI_regionmngmt);
					while (rmt_rsrc_released == RSRC_REQ_PENDING)
						MPID_DeviceCheck (MPID_NOTBLOCKING);
					MPID_SMI_LOCK(&MPID_SMI_regionmngmt);
					
					next_req++;
					if (next_req > last_proc_on_node)
						next_req = first_proc_on_node;
					if (next_req == MPID_SMI_myid) {
						next_req++;						
						if (next_req > last_proc_on_node)
							next_req = first_proc_on_node;
					}
					
					if (rmt_rsrc_released == RSRC_RELEASE_ACK)
						/* The other process released a resource => we can try again. */
						break;
				}
			} else {
				give_up = 1;
			}
		}	
	}

	RSRC_MNGMT_DEBUG (if (reginfo != NULL)
					  fprintf (stderr, "[%d] free_resource: free'd resource from (%d), SMI regid %d (type %d)\n", 
							   MPID_SMI_myid, reginfo->owner, reginfo->smi_regid, type);
					  else 
					  fprintf (stderr, "[%d] free_resource: could not free any resource\n", MPID_SMI_myid));
	return rsrc_freed;
}


/* Process an incoming response to a rsrc_releases-request by setting the internal state
   accordingly. */
void MPID_SMI_rsrc_ack( void *in_pkt, int from_grank )
{
	MPID_PKT_RSRC_T *recv_pkt = (MPID_PKT_RSRC_T *)in_pkt;
	
	rmt_rsrc_released = recv_pkt->have_released ? RSRC_RELEASE_ACK : RSRC_RELEASE_NACK;
	MPID_SMI_FreeRecvPkt( (MPID_PKT_T *)recv_pkt, from_grank, IS_CTRL_MSG);

	return;
}


/* Process an incoming rsrc_releases-request by trying to free an resource. */
void MPID_SMI_rsrc_req( void *in_pkt, int from_grank )
{
	MPID_SMI_CTRLPKT_T pkt_desc;
	MPID_PKT_RSRC_T    prepkt;
	MPID_PKT_RSRC_T   *recv_pkt = (MPID_PKT_RSRC_T *)in_pkt;
	int have_freed;

	MPID_SMI_LOCK(&MPID_SMI_regionmngmt);
	
	/* Try to release a resource, then communicate the result to the requester. */
	have_freed = free_resource (recv_pkt->rsrc_type, REMOTE_FREE_REQ);
	if (have_freed) {
		MPID_STAT_COUNT(rsrc_release_ack);
	} else {
		MPID_STAT_COUNT(rsrc_release_nack);
	}

	MPID_SMI_FreeRecvPkt( (MPID_PKT_T *)recv_pkt, from_grank, IS_CTRL_MSG);

	MPID_GETSENDPKT (pkt_desc, sizeof(MPID_PKT_RSRC_T), &prepkt, 0, NULL, from_grank, 0);
	MPID_INIT_RSRC_OK_PREPKT(prepkt, 0, MPID_SMI_myid, 0, ANY, have_freed);
	
	MPID_SMI_DEBUG_PRINT_SEND_PKT("S Sending rsrc_ok message", &prepkt, from_grank);
	while (MPID_SMI_SendControl(&pkt_desc) != MPI_SUCCESS)
		;
	
	MPID_SMI_UNLOCK(&MPID_SMI_regionmngmt);
	return;
}


/* Add a resource to the pool of available resources 
   according to the current scheduling strategy. 

   Input: 
   reginfo      the resource to be added
   type         optional specificication of the resource type   

   No Return Value.
*/
static int cache_resource (MPID_SMI_region_info_t reginfo, MPID_SMI_rsrc_type_t type)
{
	int retval = MPI_SUCCESS;

	switch (sched_strat) {
	case IMMEDIATE:
		/* no resource-reusing at all */
		if ((reginfo != NULL) && !reginfo->is_SCI && (reginfo->in_use == 0)) {
			SMI_Free_shreg(reginfo->smi_regid);

			MPID_STAT_COUNT(rsrc_destroy);
		} else 
			retval = MPIR_ERR_NOMATCH;
		break;
	case LRU:
		MPID_hash_store (usable_regions, reginfo);
		MPID_tree_insert (LRU_tree, reginfo);
		break;
	case LFU:
		MPID_hash_store (usable_regions, reginfo);
		MPID_tree_insert (LFU_tree, reginfo);
		break;
	case BEST_FIT:
		/* XXX not yet implemented */
		break;
	case RANDOM:		
		MPID_hash_store (usable_regions, reginfo);
		MPID_tree_insert (RANDOM_tree, reginfo);
		break;
	case NONE:
		(void)MPID_hash_store (usable_regions, reginfo);
		/* Nothing else required as no cache replacement policy is given. */
		break;
	}	

	RSRC_MNGMT_DEBUG (fprintf (stderr, "[%d] cache_resource: cached resource from (%d), SMI regid %d (type %d)\n", 
							   MPID_SMI_myid, reginfo->owner, reginfo->smi_regid, reginfo->type););
	return retval;
}

/* Destroy a specific resource (and remove related information from all scheduling
   data structures). 

   Input: 
   reginfo      the resource to be destroyed
   type         optional specificication of the resource type   

   No Return Value.
*/
static int destroy_resource (MPID_SMI_region_info_t reginfo, MPID_SMI_rsrc_type_t type)
{
	int retval = MPI_SUCCESS;
	smi_error_t smi_err = SMI_SUCCESS;

	if ((reginfo != NULL) /* && !reginfo->is_SCI */ && (reginfo->in_use == 0)) {
		smi_err = SMI_Free_shreg(reginfo->smi_regid);
		MPID_ASSERT (smi_err == SMI_SUCCESS || smi_err == SMI_ERR_BUSY, "Invalid return code of SMI_Free_shreg()");

		RSRC_MNGMT_DEBUG (fprintf (stderr, "[%d] destroy_resource: free'd SMI region %d (type %d)\n", 
								   MPID_SMI_myid, reginfo->smi_regid, type););
		reginfo->deallocated++;
		reginfo->is_valid = 0;
#if STORE_DEALLOCATED
		(void)MPID_hash_store (deallocated_regions, reginfo);			
#else
		MPID_SBfree (reginfo_allocator, reginfo);
#endif
		MPID_STAT_COUNT(rsrc_destroy);

		switch (sched_strat) {
		case IMMEDIATE:
			/* Nothing to do. */
			break;
		case LRU:
			/* Nothing to do. */
			break;
		case LFU:
			/* Nothing to do. */
			break;
		case BEST_FIT:
			/* XXX not yet implemented */
			break;
		case RANDOM:		
			/* Nothing to do. */
			break;
		case NONE:
			/* Nothing to do. */
			break;
		}	
	} else 
		retval = MPIR_ERR_NOMATCH;

	return retval;
}


static void sched_init (MPID_SMI_rsrc_sched_strategy_t strategy)
{
	sched_strat = strategy;
	
	switch (strategy) {
	case IMMEDIATE:
		/* no management data structures required */
		break;
	case LRU:
		LRU_tree = MPID_tree_init (_mpid_smi_lru_cmp, 
								   MPID_UTIL_THREADSAFE|MPID_UTIL_IGNORE_DUPLICATES);
		break;
	case LFU:
		LFU_tree = MPID_tree_init (_mpid_smi_lfu_cmp, 
								   MPID_UTIL_THREADSAFE|MPID_UTIL_IGNORE_DUPLICATES);
		break;
	case BEST_FIT:
		/* XXX not yet implemented */
		MPID_SMI_UNLOCK(&MPID_SMI_regionmngmt);
		MPID_ABORT ("sched_init(): BEST_FIT strategy not yet implemented");
		break;
	case RANDOM:
		RANDOM_tree = MPID_tree_init (_mpid_smi_random_cmp, 
									  MPID_UTIL_THREADSAFE|MPID_UTIL_IGNORE_DUPLICATES);
		break;
	case NONE:
		/* no management data structures required */
		break;
	}
	
	return;
}


static void sched_finalize (void)
{
	switch (sched_strat) {
	case IMMEDIATE:
		/* no management data structures required */
		break;
	case LRU:
		MPID_tree_destroy (LRU_tree);
		break;
	case LFU:
		MPID_tree_destroy (LFU_tree);
		break;
	case BEST_FIT:
		/* XXX not yet implemented */
		break;
	case RANDOM:
		MPID_tree_destroy (RANDOM_tree);
		break;
	case NONE:
		/* no management data structures required */
		break;
	}

	return;
}


/* We need to check if the found region really satisfies all our requirements */
static int region_fits (int reg_type, void **buf, ulong sgmt_offset, 
						size_t len, MPID_SMI_region_info_t reginfo)
{
	RSRC_MNGMT_DEBUG (fprintf (stderr,
		"[%d] region_fits: parameters are:\n \t regtype=%d\n \t buf=%p\n \t sgmt_offset=%ul\n \t len=%d\n \t reginfo=%p\n",
		MPID_SMI_myid,reg_type,buf,sgmt_offset,len,reginfo););
	if (reginfo == NULL) {
		RSRC_MNGMT_DEBUG (fprintf (stderr, "[%d] region_fits: Reginfo is NULL! -> returning false...\n", MPID_SMI_myid););
		return 0;
	}

	/* is the complete buffer contained ? */
	if (reginfo->len < len) {
		RSRC_MNGMT_DEBUG (fprintf (stderr, "[%d] region_fits: region buffer is not long enough!\n", MPID_SMI_myid););
		return 0;		
	}

	if (*buf != NULL && (size_t)reginfo->address <= (size_t)*buf
		&& (size_t)reginfo->address + reginfo->len >= (size_t)*buf + len) {
		/* The user buffer is completely contained in the mapped remote SCI segment. */
		RSRC_MNGMT_DEBUG (fprintf (stderr, "[%d] region_fits: The user buffer is completely contained in the mapped remote SCI segment.\n", MPID_SMI_myid););
		return 1;
	}
	if (*buf == NULL && reginfo->offset <= sgmt_offset
		&& reginfo->offset + reginfo->len >= sgmt_offset + len) {
		/* The connected remote SCI segment is large enough for this transfer. */
		RSRC_MNGMT_DEBUG (fprintf (stderr, "[%d] region_fits: The connected remote SCI segment is large enough for this transfer.\n", MPID_SMI_myid););
		return 1;
	}
	RSRC_MNGMT_DEBUG (fprintf (stderr, "[%d] region_fits: tested region does not fit!\n", MPID_SMI_myid););
	return 0;
}


/* Allocate a resource (= segment). Unified function, specific action depends on 
   the requested reg_type */
static int acquire_resource (int procrank, int *sci_sgmtid, int rmt_adptr, size_t len, 
							 size_t offset, int reg_type, void **buf, int *smi_regid, int flags)
{
	MPID_SMI_region_info_t reginfo = NULL;
	MPID_SMI_rsrc_type_t reginfo_type;
	smi_region_info_t smi_reginfo;
	smi_region_callback_t cb_fcn = NULL;
	smi_error_t smi_error;
	int is_unique, found, region_size, region_offset, reg_flags = 0, *destroy_id;
	int retval = MPIR_ERR_EXHAUSTED;

	MPID_STAT_ENTRY(acquire_rsrc);

	/* First, check for regions to be destroyed. */
	while ((destroy_id = MPID_FIFO_pop(regions_to_destruct)) != NULL) {
		RSRC_MNGMT_DEBUG (fprintf (stderr, "[%d] acquire_resource: freeing SMI region %d (delayed)\n" , 
								   MPID_SMI_myid, *destroy_id));
		release_resource (NULL, *destroy_id, ANY, MPID_SMI_RSRC_DESTROY);
		FREE (destroy_id);
	}

	MPID_SMI_LOCK(&MPID_SMI_regionmngmt);
	RSRC_MNGMT_DEBUG (fprintf (stderr, "[%d] acquire_resource: proc %d, sci_id %d, len %d, off %d, type %d, addr 0x%p, smi_id %d\n", 
							   MPID_SMI_myid, procrank, *sci_sgmtid, len, offset, reg_type, *buf, *smi_regid));
	
	/* Creationg a local segment by registering user allocated memory is done by
	   supplying an address, not a remote SCI id. Need to check for this. */
	if (*buf != NULL) {
		if (SMI_Range_to_region ((char *)*buf, len, smi_regid) == SMI_SUCCESS)
			SMIcall (SMI_Query(SMI_Q_SMI_REGION_SGMT_ID, *smi_regid, (void *)sci_sgmtid));
	}
	/* O.k., this is not beautiful, but it is correct. */
	reginfo_type = (reg_type == (SMI_SHM_LOCAL|SMI_SHM_REGISTER)) ? LOCAL : 
		(reg_type == SMI_SHM_LOCAL) ? LOCAL : 
		(reg_type == SMI_SHM_REMOTE) ? RMT_MAP :
		(reg_type == SMI_SHM_RDMA) ? RMT_CNCT : ANY;

	/* If no SCI segment id could be determined (< 0), there is no chance that the requested
	   region is already allocated. */
	if (*sci_sgmtid >= 0) {
		RSRC_MNGMT_DEBUG (fprintf (stderr, "[%d] acquire_resource: probing SCI segments with id %d\n", 
								   MPID_SMI_myid, *sci_sgmtid););
		/* Most of the time, a region is not in use, but it might be already connected. */
		MPID_hash_lock (usable_regions);
		reginfo = (MPID_SMI_region_info_t)
			MPID_hash_find (usable_regions, KEY_GEN(*sci_sgmtid, procrank, reginfo_type), &is_unique);
		while (reginfo != NULL) {
			if (region_fits(reg_type, buf, offset, len, reginfo)) {
				MPID_STAT_COUNT(rsrc_reuse);
				INCR_USE (reginfo);
				*smi_regid = reginfo->smi_regid;
				if (*buf == NULL){
					*buf = (char *)reginfo->address - reginfo->offset + offset;
					RSRC_MNGMT_DEBUG (fprintf (stderr, "[%d] buffer address now 0x%p\n",MPID_SMI_myid,*buf));
				}

				MPID_hash_data_remove (usable_regions, (void *)reginfo, &found);
				MPID_hash_store (regions_in_use, (void *)reginfo);

				retval = reginfo->is_SCI ? MPID_SMI_ISSCI : MPI_SUCCESS;
				if (flags & INCR_USE_ONLY)
					return MPI_SUCCESS;

				break;
			} else {
				if (is_unique) {
					reginfo = NULL;
					break;
				} else {
					reginfo = (MPID_SMI_region_info_t)
						MPID_hash_next (usable_regions, KEY_GEN(*sci_sgmtid, procrank, reginfo_type));
				}
			}
		}
		MPID_hash_unlock (usable_regions);

		if (retval == MPIR_ERR_EXHAUSTED) {
			/* Check if the region is currently in use. */
			MPID_hash_lock (regions_in_use);
			reginfo = (MPID_SMI_region_info_t)
				MPID_hash_find (regions_in_use, KEY_GEN(*sci_sgmtid, procrank, reginfo_type), &is_unique);
			while (reginfo != NULL) {
				if (region_fits(reg_type, buf, offset, len, reginfo)) {
					MPID_STAT_COUNT(rsrc_reuse);
					INCR_USE (reginfo);
					*smi_regid = reginfo->smi_regid;
					if (*buf == NULL)
						*buf = (char *)reginfo->address - reginfo->offset + offset;
					
					retval = reginfo->is_SCI ? MPID_SMI_ISSCI : MPI_SUCCESS;
					if (flags & INCR_USE_ONLY)
						return MPI_SUCCESS;
					break;
				} else {
					if (is_unique) {
						reginfo = NULL;
						break;
					} else {
						reginfo = (MPID_SMI_region_info_t)
							MPID_hash_next (regions_in_use, KEY_GEN(*sci_sgmtid, procrank, reginfo_type));
					}
				}
			}
			MPID_hash_unlock (regions_in_use);
		}
	}
	if (flags & INCR_USE_ONLY)
		return MPIR_ERR_NOMATCH;
	
	if (reginfo == NULL) {
		if ((*sci_sgmtid >= 0) && (reg_type != SMI_SHM_REMOTE) && (reg_type != SMI_SHM_RDMA)) {
			/* The memory address belongs to a local SMI region, but no matching region info 
			   could not be found in the resource tables. Two possible reasons for this:
			   - it was not created via this interface, but via MPI_Alloc_mem() or other means.
			   - it belongs to a user-allocated buffer of which only parts have been registered so far */
			int reg_size;
			char *reg_adr;

			SMIcall (SMI_Query (SMI_Q_SMI_REGION_SIZE, *smi_regid, &reg_size));
			SMIcall (SMI_Query (SMI_Q_SMI_REGION_ADDRESS, *smi_regid, &reg_adr));
			if ((reg_size < len) || 
				((*buf != NULL) && ((size_t)reg_adr > (size_t)*buf)
				 || ((size_t)reg_adr + reg_size < (size_t)*buf + len))) {
				/* We need do register this user buffer. It will contain part of the
				   already registered area. But because this new, bigger registered area
				   will be found first if a region info is searched which matches a 
				   given address, it will be used more frequently and the old, smaller 
				   region will be displaced by the scheduler if necessary.
				*/
				retval = MPIR_ERR_EXHAUSTED;
			} else {
				/* Remember the complete SMI region. */
				MPID_STAT_COUNT(rsrc_reuse);
				
				RSRC_MNGMT_DEBUG (fprintf (stderr, "[%d] acquire_resource: hashing full SMI region, address 0x%p\n", 
										   MPID_SMI_myid, *buf));

				reginfo = (MPID_SMI_region_info_t) MPID_SBalloc(reginfo_allocator);
				reginfo->smi_regid  = *smi_regid;
				SMIcall (SMI_Query(SMI_Q_SMI_REGION_OWNER, reginfo->smi_regid, &reginfo->owner));
				reginfo->sci_sgmtid = *sci_sgmtid;
				reginfo->address    = *buf;
				reginfo->offset     = offset;
				reginfo->is_SCI     = 1;
				reginfo->len        = reg_size;
				/* Not (yet) possible to decide if local SCI segment or registered buffer,
				   but this isn't required, too.*/
				reginfo->type       = LOCAL;
				reginfo->is_valid   = 1;

				INIT_USE(reginfo);
				MPID_hash_store (regions_in_use, (void *)reginfo);
				retval = MPID_SMI_ISSCI;
			}
		} 
		if (retval != MPID_SMI_ISSCI) {
			RSRC_MNGMT_DEBUG (fprintf (stderr, "[%d] acquire_resource: try to create resource from (%d) (type %d)\n", 
									   MPID_SMI_myid, procrank, reginfo_type););

			/* do not register a user-buffer if de-activated in the device configuration */
			if (!((reg_type & SMI_SHM_REGISTER) && !MPID_SMI_cfg.REGISTER)) {
				/* We need to  create a new region - but we check if the region 
				   has ever been connected */
#if STORE_DEALLOCATED
				reginfo = (MPID_SMI_region_info_t) MPID_hash_find (deallocated_regions, 
																   KEY_GEN(*sci_sgmtid, procrank, reginfo_type), 
																   &is_unique);
#endif
				if ((reginfo != NULL) && !is_unique) {
					/* More regions with the same key exist - this shouldn't happen
					   because the hash-keys are unigue !*/
					MPID_SMI_UNLOCK(&MPID_SMI_regionmngmt);
					MPID_ABORT("acquire_resource(): region is not unique");
				}
				/* If we map a remote region, we first try to map it entirely to 
				   avoid multiple part-mapping of a single remote region. Only if the full-mapping
				   fails, we should map it partly as demanded by the call. */
				region_size = (reg_type == SMI_SHM_REMOTE) ? 
					(flags & MPID_SMI_RSRC_PARTIAL_MAP) ? len : 0
					: len;
				region_offset = (reg_type == SMI_SHM_REMOTE) ? 
					(flags & MPID_SMI_RSRC_PARTIAL_MAP) ? offset : 0
					: offset;
				/* For remote maps and remote connects: establish a callback function that removes the 
				   region in case the remote segment is withdrawn. This is especially important for remote
				   connects to segments of registered memory because such a segment needs to be 
				   withdrawn as soon as the related memory buffer gets un-registered (i.e. by free()ing
				   malloc()ed memory. Only with callbacks, caching of remote connections is possible. */
				if ((reg_type == SMI_SHM_REMOTE || reg_type == SMI_SHM_RDMA) 
					&& MPID_SMI_cfg.CACHE_CONNECTED) {
					cb_fcn = (smi_region_callback_t)_mpid_smi_region_cb;
					reg_flags |= SMI_SHM_CALLBACK;
				}
				SMI_Init_reginfo (&smi_reginfo, region_size, region_offset, procrank,  SMI_ADPT_DEFAULT,
								  rmt_adptr, *sci_sgmtid, cb_fcn);

				MPID_SMI_LOCK(&MPID_SMI_connect_lck);
				smi_error = SMI_Create_shreg (reg_type|reg_flags, &smi_reginfo, smi_regid, buf);
				if (smi_error != SMI_SUCCESS) {
					while (free_resource(ANY, LOCAL_FREE_REQ) && (smi_error != SMI_SUCCESS)) {
						smi_error = SMI_Create_shreg (reg_type|reg_flags, &smi_reginfo, smi_regid, buf);
					}
				}
#if TRY_PARTIAL_MAPPING
				if (smi_error != SMI_SUCCESS) {
					/* Even after freeing all freeable ressources, the full mapping failed
					   Now try one more time with partial mapping. */
					smi_reginfo.size   = len;
					smi_reginfo.offset = offset;
					smi_error = SMI_Create_shreg (reg_type|reg_flags, &smi_reginfo, smi_regid, buf);
				}
#endif
				MPID_SMI_UNLOCK(&MPID_SMI_connect_lck);
				
				if (smi_error == SMI_SUCCESS) {
					MPID_STAT_COUNT(rsrc_create);
					if (reginfo == NULL) {
						/* we have never been connected to this segment -> complete initialization */
						reginfo = (MPID_SMI_region_info_t) MPID_SBalloc(reginfo_allocator);
						reginfo->smi_regid = *smi_regid;
						reginfo->owner     = procrank;
						reginfo->is_SCI    = (reg_type == SMI_SHM_LOCAL) ? 1 : 0;
						reginfo->address   = *buf;
						reginfo->offset    = offset;
						if (reg_type == SMI_SHM_REMOTE) {
							/* The offset/size that was actually used for the mapping is not necessarily
							   the actual offset/size supplied by the user */
							SMIcall (SMI_Query(SMI_Q_SMI_REGION_OFFSET, reginfo->smi_regid, 
											   &reginfo->offset));
							*(char **)buf += offset - reginfo->offset;
						}						
						SMIcall (SMI_Query(SMI_Q_SMI_REGION_SGMT_ID, reginfo->smi_regid, &reginfo->sci_sgmtid));
						*sci_sgmtid = reginfo->sci_sgmtid;
						SMIcall (SMI_Query(SMI_Q_SMI_REGION_SIZE, reginfo->smi_regid, &reginfo->len));
						reginfo->type = reginfo_type;
						reginfo->is_valid   = 1;

						INIT_USE(reginfo);
						RSRC_MNGMT_DEBUG (fprintf (stderr, "[%d] acquire_resource: created new resource, SMI regid %d (type %d)\n", 
												   MPID_SMI_myid, reginfo->smi_regid, reginfo->type););
					} else {
#if STORE_DEALLOCATED
						MPID_hash_data_remove (deallocated_regions, (void *)reginfo, &found);
#endif
						INCR_USE(reginfo);
						RSRC_MNGMT_DEBUG (fprintf (stderr, "[%d] acquire_resource: re-created a resource, SMI regid %d (type %d)\n", 
												   MPID_SMI_myid, reginfo->smi_regid, reginfo->type););
					}
					MPID_hash_store (regions_in_use, (void *)reginfo);
					retval = MPI_SUCCESS;
				} else {
					retval = MPIR_ERR_EXHAUSTED;
					MPID_STAT_COUNT(rsrc_failed);
					RSRC_MNGMT_DEBUG (fprintf (stderr, "[%d] acquire_resource: could not create resource\n", 
											   MPID_SMI_myid););
				}
			}
		}
	} else {
		/* we are re-using an existing resource */
		RSRC_MNGMT_DEBUG (fprintf (stderr, "[%d] acquire_resource: re-using a resource, SMI regid %d (type %d)\n", 
								   MPID_SMI_myid, reginfo->smi_regid, reginfo->type););
	}

	MPID_SMI_UNLOCK(&MPID_SMI_regionmngmt);
	MPID_STAT_EXIT(acquire_rsrc);

	return retval;	
}


static int release_resource (void *regaddr, int smi_regid, MPID_SMI_rsrc_type_t type, int flag)
{
	MPID_SMI_region_info_t reginfo;
	MPID_hash_table_t hash_table;
	ulong hash_key;
	int sci_sgmtid, owner, is_unique, found;
	int retval = MPI_SUCCESS;
	char err_msg[256];

	MPID_STAT_ENTRY(release_rsrc);
	RSRC_MNGMT_DEBUG (fprintf (stderr, "[%d] release_resource: addr 0x%p, smi_id %d, type %d - need lock\n", 
							   MPID_SMI_myid, regaddr, smi_regid, type));
	MPID_SMI_LOCK(&MPID_SMI_regionmngmt);
	
	RSRC_MNGMT_DEBUG (fprintf (stderr, "[%d] got lock.\n", MPID_SMI_myid));
	/* generate the key */
	if (regaddr != NULL) {
		SMIcall (SMI_Adr_to_region ((char *)regaddr, &smi_regid));
	} else {
		RSRC_MNGMT_DEBUG (fprintf (stderr, "[%d] regaddr is NULL so query for it\n", MPID_SMI_myid));
		SMIcall (SMI_Query(SMI_Q_SMI_REGION_ADDRESS, smi_regid, (void *)&regaddr));
	} 
	SMIcall (SMI_Query(SMI_Q_SMI_REGION_OWNER, smi_regid, (void *)&owner));
	SMIcall (SMI_Query(SMI_Q_SMI_REGION_SGMT_ID, smi_regid, (void *)&sci_sgmtid));

	if (type == ANY) {
		if (owner == MPID_SMI_myid) {
			type = LOCAL;
		} else {
			SMIcall (SMI_Query(SMI_Q_SMI_REGION_ADDRESS, smi_regid, (void *)&regaddr));
			type = (regaddr != NULL) ? RMT_MAP : RMT_CNCT;
		}
	}
	
	hash_key = KEY_GEN(sci_sgmtid, owner, type);

	/* get the region information entry */
	hash_table = regions_in_use;
	reginfo = (MPID_SMI_region_info_t) MPID_hash_find (hash_table, hash_key, &is_unique);
	if (reginfo == NULL && (flag & MPID_SMI_RSRC_DESTROY)) {
		/* This *can* happen when caching multiple regions of registered memory 
		   which has been malloced as private memory via MPI_Alloc_mem() (or wrapped malloc()).
		   If the region is to be destroyed, we need to check for it in the "unused" hash table
		   and destroy it from there.*/
		hash_table = usable_regions;
		reginfo = (MPID_SMI_region_info_t) MPID_hash_find (usable_regions, hash_key, &is_unique);
	}
	/* We assume that no two SMI regions provide the same memory address! If this assumption
	   holds true (it should!), this code is correct. */
	while (reginfo != NULL && !region_fits(0, &regaddr, 0, 1 /* dummy length */, reginfo)) {
		RSRC_MNGMT_DEBUG (fprintf (stderr, "[%d] release_resource: looking for region that fits\n", MPID_SMI_myid););
		reginfo = (MPID_SMI_region_info_t) MPID_hash_next (hash_table, hash_key);
	}
	if (reginfo != NULL) {
			MPID_STAT_COUNT(rsrc_release);
			DECR_USE (reginfo);
			RSRC_MNGMT_DEBUG (fprintf (stderr, "[%d] release_resource: from (%d), usecnt = %d, flag %d, addr 0x%p, size 0x%x, SMI regid %d (type %d)\n", 
									   MPID_SMI_myid, reginfo->owner, reginfo->in_use, flag, reginfo->address, reginfo->len, 
									   reginfo->smi_regid, reginfo->type););
			if (reginfo->in_use <= 0) {
				(void)MPID_hash_key_remove (hash_table, hash_key, &found);
				switch (flag & MPID_SMI_RSRC_FLAGS) {
				case MPID_SMI_RSRC_CACHE:
					/* store in scheduler-specific data structure */
					if (reginfo->in_use == 0)
						retval = cache_resource (reginfo, ANY);
					else
						/* This means in_use < 0; this situation is possible if a region which
						   was currently in use was subject to a deallocation by a callback. 
						   May only occur as a race condition after a transfer
						   if the receiver deallocates its region faster than the sending process
						   releases the resource, and the callback is first. It means that the resource
						   can not be cached, but needs to be destroyed. */
						retval = destroy_resource (reginfo, reginfo->type);
					MPID_STAT_COUNT(rsrc_unused);
					break;
				case MPID_SMI_RSRC_DESTROY:
					/* definitely remove/destroy the resource */
					retval = destroy_resource (reginfo, reginfo->type);
					break;
				}
			}
		} else {
		retval = MPIR_ERR_NOMATCH;
		/* If we do not find the region in our hash tables, something is really wrong! */
		sprintf (err_msg, "release_resource(): region not found (SMI id %d, addr 0x%p)\n", 
				 smi_regid, regaddr);
		MPID_SMI_UNLOCK(&MPID_SMI_regionmngmt);

		/* Final resort - or what else should we do? */
		MPID_ABORT (err_msg);
	}

	MPID_SMI_UNLOCK(&MPID_SMI_regionmngmt);
	MPID_STAT_EXIT(release_rsrc);

	return retval;
}


void MPID_SMI_Rsrc_sched_init (MPID_SMI_rsrc_sched_strategy_t strategy)
{
	int secs, usecs;

	SMI_Get_timer (&secs, &usecs);
	srandom ((unsigned int)secs);

    MPID_SMI_INIT_LOCK(&MPID_SMI_regionmngmt);

	/* Set up the hash tables. They do not need to be configured threadsafe because
	   here, they are always used within a threadsafe context. */
	regions_in_use = MPID_hash_init (HASHTABLE_SIZE, sizeof(MPID_SMI_region_info), _mpid_smi_keygen, 0);
	usable_regions = MPID_hash_init (HASHTABLE_SIZE, sizeof(MPID_SMI_region_info), _mpid_smi_keygen, 0);
	deallocated_regions = MPID_hash_init (HASHTABLE_SIZE, sizeof(MPID_SMI_region_info), _mpid_smi_keygen, 0);

	/* fixed-size-block memory manager */
	reginfo_allocator = MPID_SBinit (sizeof(MPID_SMI_region_info), INIT_REGION_INFOS, INCR_REGION_INFOS);

	/* XXX: Insert all existing SMI regions into the hash-tables - they might be accessed
	   lateron and would unecessarily be reconnected if not in the hash tables */
	

	sched_init(strategy);

	/* Which processes of this application are located on this node ? */
	SMIcall( SMI_First_proc_on_node (MPID_SMI_myNode, &first_proc_on_node));
	if (MPID_SMI_myNode == MPID_SMI_numNodes - 1) 
		last_proc_on_node = MPID_SMI_numids - 1;
	else {
		SMIcall( SMI_First_proc_on_node (MPID_SMI_myNode+1, &last_proc_on_node));
		last_proc_on_node--;
	}
	next_req = (first_proc_on_node == MPID_SMI_myid) ? MPID_SMI_myid + 1 : first_proc_on_node;

	regions_to_destruct = MPID_FIFO_init(MPID_UTIL_THREADSAFE);

	return;
}


int MPID_SMI_Rsrc_sched_finalize (void)
{
	MPID_SMI_region_info_t reginfo;
	int rsrcs_in_use = 0;

	MPID_SMI_LOCK(&MPID_SMI_regionmngmt);
	MPID_SMI_UNLOCK(&MPID_SMI_regionmngmt);
	MPID_SMI_DESTROY_LOCK(&MPID_SMI_regionmngmt);

	/* The hash tables need to be deleted freeing all allocated resources */
	while ((reginfo = (MPID_SMI_region_info_t)MPID_hash_empty(deallocated_regions)) != NULL) {
		MPID_SBfree (reginfo_allocator, reginfo);
	}
	MPID_hash_destroy (deallocated_regions);

	while ((reginfo = (MPID_SMI_region_info_t)MPID_hash_empty(usable_regions)) != NULL) {
		if (!reginfo->is_SCI) {
			SMI_Free_shreg (reginfo->smi_regid) ;
		}
		MPID_SBfree (reginfo_allocator, reginfo);
	}
	MPID_hash_destroy (usable_regions);

	while ((reginfo = (MPID_SMI_region_info_t)MPID_hash_empty(regions_in_use)) != NULL) {
		if (!reginfo->is_SCI) {
			SMI_Free_shreg (reginfo->smi_regid) ;
		}
		rsrcs_in_use++;
		MPID_SBfree (reginfo_allocator, reginfo);
	}
	MPID_SBdestroy (reginfo_allocator);

	MPID_hash_destroy (regions_in_use);

	return rsrcs_in_use;
}


int MPID_SMI_Rmt_region_connect (int rmt_procrank, int sci_sgmtid, int rmt_adptr, int *smi_regid)
{
	void *dummy = NULL;
	int retval;
	MPID_STAT_ENTRY(rmt_reg_connect);

	retval = acquire_resource (rmt_procrank, &sci_sgmtid, rmt_adptr, 0, 0, 
							   SMI_SHM_RDMA, &dummy, smi_regid, 0);
	
	MPID_STAT_EXIT(rmt_reg_connect);
	return retval;
}


int MPID_SMI_Rmt_region_release (int smi_regid, int flag)
{	
	int retval;
	MPID_STAT_ENTRY(rmt_reg_release);
	
	retval = release_resource(NULL, smi_regid, RMT_CNCT, flag);

	MPID_STAT_EXIT(rmt_reg_release);
	return retval;
}


int MPID_SMI_Local_mem_use (void *buf)
{
	int retval, sci_sgmtid, smi_regid;

	retval = acquire_resource (MPID_SMI_myid, &sci_sgmtid, 0, 1, 0, 
							   SMI_SHM_LOCAL, &buf, &smi_regid, INCR_USE_ONLY);
	
	return retval;
}


int MPID_SMI_Local_mem_register (void *buf, size_t len, int *smi_regid, int *sci_sgmtid)
{
	int retval;
	MPID_STAT_ENTRY(loc_mem_register);

	*sci_sgmtid = -1;
	retval = acquire_resource (MPID_SMI_myid, sci_sgmtid, 0, len, 0, 
							   SMI_SHM_LOCAL|SMI_SHM_REGISTER, &buf, smi_regid, 0);

	MPID_STAT_EXIT(loc_mem_register);
	return retval;
}


int MPID_SMI_Local_mem_create (size_t *size, size_t min_size, void **buf, 
  							   int *smi_regid,  int *sci_sgmtid)
{
	int retval;
	size_t try_size;
	MPID_STAT_ENTRY(loc_mem_create);

	*sci_sgmtid = -1;
	try_size = *size;
	while ((retval = acquire_resource (MPID_SMI_myid, sci_sgmtid, 0, try_size, 0, SMI_SHM_LOCAL,
									  buf, smi_regid, 0)) != MPI_SUCCESS) {
		if ((min_size > 0) && (try_size > min_size)) {
			try_size /= 2;
			MPID_SMI_PAGESIZE_ALIGN(try_size);
			if (try_size < min_size)
				try_size = min_size;
		} else {
			break;
		}
	}

	*size = (retval == MPI_SUCCESS) ? try_size : 0;

	MPID_STAT_EXIT(loc_mem_create);
	return retval;
}


int MPID_SMI_Local_mem_release (void *buf, int smi_regid, int flag)
{
	int retval; 
	MPID_STAT_ENTRY(loc_mem_release);

	retval = release_resource(buf, smi_regid, LOCAL, flag);

	MPID_STAT_EXIT(loc_mem_release);
	return retval;
}


int MPID_SMI_Rmt_mem_map (int rmt_procrank, int sci_sgmtid, size_t len, size_t offset, 
						  int rmt_adptr, void **buf, int *smi_regid, int flags)
{
	int retval; 
	MPID_STAT_ENTRY(rmt_mem_map);

	*buf = NULL;
	retval =  acquire_resource (rmt_procrank, &sci_sgmtid, rmt_adptr, len, offset, 
							SMI_SHM_REMOTE, buf, smi_regid, flags);

	MPID_STAT_EXIT(rmt_mem_map);
	return retval;
}


int MPID_SMI_Rmt_mem_release (void *buf, int smi_regid, int flag)
{
	int retval; 
	MPID_STAT_ENTRY(rmt_mem_release);

	retval = release_resource(buf, smi_regid, RMT_MAP, flag);

	MPID_STAT_EXIT(rmt_mem_release);
	return retval;
}


int MPID_SMI_Rsrc_destroy (void *buf, int smi_regid) 
{
	return release_resource(buf, smi_regid, ANY, MPID_SMI_RSRC_DESTROY);
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
