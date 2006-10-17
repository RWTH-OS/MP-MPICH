/* $Id$ */

#include "env/general_definitions.h"
#include "free_shreg.h"
#include "memory/shmem.h"
#include "env/smidebug.h"
#include "message_passing/lowlevelmp.h"
#include "memtree.h"
#include "idstack.h"


/****************************************************************************/
/*** This function frees a shared region: all it's segments are unmapped  ***/
/*** and destroyed.                                                       ***/
/****************************************************************************/

smi_error_t SMI_Free_shreg(int id)
{ 
  smi_error_t error;
  
  DSECTION ("SMI_Free_shreg");
  DSECTENTRYPOINT;
  
  DNOTICEI("Free'ing region with SMI ID", id);

  /* The user must not free any internal regions !*/
  if (id >= 0 && id < _smi_nbr_machines) {
    DWARNING ("User tries to free internal region - region not free'd.");
    DSECTLEAVE; return (SMI_ERR_NOSEGMENT);
  }

  error = _smi_free_shreg(id);

  DSECTLEAVE; return (error);
}

  

int _smi_free_shreg(int id)
{
    memtree_memarea_t memarea;
    smi_error_t error;
    int j, sync_in, sync_out, pt2pt_partner;
    boolean sync, pt2pt;
    
    DSECTION ("_smi_free_shreg");
    DSECTENTRYPOINT;
    SMI_LOCK(&_smi_region_lock);

    _smi_free_busy_shregs();

    if (!IS_VALID_ID(id)) {
	DPROBLEMI("Illegal region id:",id);
	DSECTLEAVE; SMI_UNLOCK(&_smi_region_lock); return (SMI_ERR_PARAM); 
    }

    if (_smi_mis.region[id]->type != SMI_SHM_RDMA) {
	/* if region had an address, remove adressrange from memtree */
	memarea.pStart = _smi_mis.region[id]->addresses[0];
	memarea.iSize = _smi_mis.region[id]->size;
	_smi_memtree_delete(_smi_memtree, &memarea);
    }

    /* If there is an active callback thread waiting for an event on this region, 
       we need to kill it first. */
    if (_smi_mis.region[id]->cb_thread != 0) {
	DNOTICE ("Cancelling callback thread");
#ifndef DISABLE_THREADS
	pthread_cancel (_smi_mis.region[id]->cb_thread);
	pthread_join (_smi_mis.region[id]->cb_thread, NULL);
#endif
	_smi_mis.region[id]->cb_thread = 0;
    }
  
    /* unmap, remove and free the 'shseg_t' struct of all segments' 
       of the shared memory region                                  */
    for (j = 0; j < _smi_mis.region[id]->no_segments; j++) {
	sync = !(_smi_mis.region[id]->seg[j]->flags & (SHREG_LOCAL|SHREG_PRIVATE|SHREG_ASYNC));
	pt2pt = _smi_mis.region[id]->seg[j]->flags & SHREG_PT2PT;
	pt2pt_partner = (_smi_my_proc_rank == _smi_mis.region[id]->seg[j]->owner) ?
	    _smi_mis.region[id]->seg[j]->partner : _smi_mis.region[id]->seg[j]->owner;
	
	if (!(_smi_mis.region[id]->seg[j]->flags & (SHREG_DELAYED|SHREG_NOMAP))) {
	    error = _smi_unmap_shared_segment(_smi_mis.region[id]->seg[j]);
	    if (error != SMI_SUCCESS) {
	        DSECTLEAVE; SMI_UNLOCK(&_smi_region_lock); return (error);
	    }
	}
	
	if (_smi_mis.region[id]->seg[j]->owner != _smi_my_proc_rank) {
	    /* it's a remote segment */
	    error = _smi_remove_shared_segment(_smi_mis.region[id]->seg[j]);
	    if (error != SMI_SUCCESS) {
	        DSECTLEAVE; SMI_UNLOCK(&_smi_region_lock); return (error);
	    }
	    if (sync) {
		pt2pt ? 
		    SMI_Sendrecv(&sync_out, &sync_in, sizeof(int), pt2pt_partner) : _smi_ll_barrier();
	    }
	} else {
	    /* it's a local segment */
	    if (sync) {
		pt2pt ?
		    SMI_Sendrecv(&sync_out, &sync_in, sizeof(int), pt2pt_partner) : _smi_ll_barrier();
	    }

	    error = _smi_remove_shared_segment(_smi_mis.region[id]->seg[j]);
	    if (error != SMI_SUCCESS) {
		if (error == SMI_ERR_BUSY) 
		    _smi_FIFO_push (_smi_busy_regions_fifo, _smi_mis.region[id]);
		DSECTLEAVE; SMI_UNLOCK(&_smi_region_lock); return (error);
	    }
	}
	
	free(_smi_mis.region[id]->seg[j]);
    }
    free(_smi_mis.region[id]->seg);
    free(_smi_mis.region[id]->addresses);

    /* keep the region data structures but mark unused slot as free */
    _smi_mis.region[id]->id = -1;
    _smi_mis.nbr_user_regions--;

    /* add region id to list of available recently used ids */
    _smi_idstack_push(id);
    
    DSECTLEAVE; SMI_UNLOCK(&_smi_region_lock); return (SMI_SUCCESS);
}


int _smi_free_busy_shregs()
{
    /* _smi_FIFO_t tmp_fifo; */
    smi_error_t error;
    region_t *busy_reg;
    int j;
    
    DSECTION ("_smi_free_busy_shregs");
    DSECTENTRYPOINT;
#if 1
    if ((busy_reg = _smi_FIFO_pop (_smi_busy_regions_fifo)) != NULL) {
	DNOTICEI("Trying to free busy shreg with SMI ID", busy_reg->id);

	/* Remove and free the segments of this region which could not be free'd before. */
	for (j = 0; j < _smi_mis.region[busy_reg->id]->no_segments; j++) {
	    if (_smi_mis.region[busy_reg->id]->seg[j] != NULL) {
		error = _smi_remove_shared_segment(_smi_mis.region[busy_reg->id]->seg[j]);
		if (error != SMI_SUCCESS) {
		    if (error == SMI_ERR_BUSY) 
			_smi_FIFO_push (_smi_busy_regions_fifo, _smi_mis.region[busy_reg->id]);
		    DWARNING("Still could not free busy shreg.");
		} else {
		    free(_smi_mis.region[busy_reg->id]->seg[j]);
		    _smi_mis.region[busy_reg->id]->seg[j] = NULL;
		}
	    }
	}

	if (error == SMI_SUCCESS) {
	    free(_smi_mis.region[busy_reg->id]->seg);
	    free(_smi_mis.region[busy_reg->id]->addresses);
	    /* keep the region data structures but mark unused slot as free */
	    _smi_mis.region[busy_reg->id]->id = -1;
	    _smi_mis.nbr_user_regions--;
	    
	    /* add region id to list of available recently used ids */
	    _smi_idstack_push(busy_reg->id);
	    
	    DNOTICE("Busy shreg was free'd.");
	}
    }
#endif

#if 0    
    tmp_fifo = _smi_busy_regions_fifo;
    _smi_busy_regions_fifo = _smi_busy_regions_fifo_2;
    _smi_busy_regions_fifo_2 = _smi_busy_regions_fifo;
#endif
    
    DSECTLEAVE; return (SMI_SUCCESS);
}


void _smi_free_remaining_region_structs(void)
{
    int id;
    
    while ((id = _smi_idstack_pop()) != -1) 
	free(_smi_mis.region[id]);
}
