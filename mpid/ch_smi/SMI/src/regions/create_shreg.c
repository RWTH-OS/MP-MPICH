/* $Id$ */

#include <unistd.h>
#include <stddef.h>

#include "env/smidebug.h"

#include "create_shreg.h"
#include "free_shreg.h"
#include "memory/shmem.h"
#include "memory/sci_shmem.h"
#include "utility/general.h"
#include "utility/query.h"
#include "segment_address.h"
#include "proc_node_numbers/first_proc_on_node.h"
#include "proc_node_numbers/proc_size.h"
#include "utility/statistics.h"
#include "message_passing/lowlevelmp.h"
#include "regions/idstack.h"
#include "proper_shutdown/resource_list.h"
#include "memtree.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* align size by calculating the next bigger multiple of smi_page_size */
#define SMI_ALIGN_SIZE(size) { size = (size / _smi_page_size + 1) * _smi_page_size; }

static int adapter_last_used = 0;

#if defined DOLPHIN_SISCI
static void *_smi_sgmt_cb(void *arg)
{
    DSECTION ("_smi_sgmt_cb");
    region_t *shreg;
    sci_segment_cb_reason_t sgmt_event;
    sci_error_t sgmt_status, sci_err;
    unsigned int src_node_id, adapt_nbr;
    int cb_arg = 0, cb_action;
    pthread_t my_tid;
	
    SMI_LOCK(&_smi_mis_lock);
	shreg = (region_t *)_smi_mis.region[*(int *)arg];
	SMI_UNLOCK(&_smi_mis_lock);
    my_tid = shreg->cb_thread;
	
    while (1) {
		DNOTICEI ("waiting for segment event for region", shreg->id);
		if (shreg->seg[0]->owner == _smi_my_proc_rank)
			sgmt_event = SCIWaitForLocalSegmentEvent (shreg->seg[0]->localseg, &src_node_id, &adapt_nbr,
													  SCI_INFINITE_TIMEOUT, 0, &sci_err);
		else
			sgmt_event = SCIWaitForRemoteSegmentEvent (shreg->seg[0]->segment, &sgmt_status,
													   SCI_INFINITE_TIMEOUT, 0, &sci_err);
		
		DNOTICEI ("Callback event occured for region", shreg->id);
		DNOTICEP ("                             type", sgmt_event);
		
		cb_arg = 0;
		if (sgmt_event == SCI_CB_CONNECT)
			cb_arg = SMI_CB_REASON_CONNECT;
		if (sgmt_event == SCI_CB_DISCONNECT)
			cb_arg = SMI_CB_REASON_DISCONNECT;

		if (sgmt_event == SCI_CB_LOST) {
			/* XXX What to do here? This indicates a serious problem, maybe
			   we should abort. */
#if 0
			DPROBLEM("connection lost: freeing region and terminating callback thread");			
			shreg->cb_thread = 0;
			_smi_free_shreg(shreg->id);
#else 
			DPROBLEM("connection lost: aborting application");			
			SMI_Abort(1);			
#endif
			return NULL;
		}

		if (sgmt_event == SCI_CB_NOT_OPERATIONAL) {
			DPROBLEM("connection to remote segment not operational, supending callback thread");
			sleep(SCI_NOT_OPERATIONAL_DELAY);
		}
		if (sgmt_event == SCI_CB_OPERATIONAL) {
			DNOTICE("connection to remote segment operational again");
		}
		
		if (cb_arg == SMI_CB_REASON_DISCONNECT || cb_arg == SMI_CB_REASON_CONNECT) {
			if (shreg->cb_fcn != NULL) {
				/* avoid canceling myself! */
				shreg->cb_thread = 0;
				
				DNOTICEI ("Invoking callback function with arg", cb_arg);
				cb_action = shreg->cb_fcn (shreg->id, cb_arg);
				
				/* allow canceling by other again */
				shreg->cb_thread = my_tid;
			} else {
				if (shreg->seg[0]->owner != _smi_my_proc_rank && sgmt_event == SCI_CB_DISCONNECT) 
					/* If a remote region has been disconnected, we need to free it in this process, too. */
					cb_action = SMI_CB_ACTION_FREEREG;
				else
					cb_action = SMI_CB_ACTION_NOTHING;
			}
			
			switch (cb_action) {
			case SMI_CB_ACTION_FREEREG:
				shreg->cb_thread = 0;
				_smi_free_shreg(shreg->id);
				return NULL;
			case SMI_CB_ACTION_TERMINATE:
				return NULL;
			case SMI_CB_ACTION_NOTHING:
				break;
			}
		}
    }
}
#endif

smi_error_t SMI_Set_region_callback (int region_id, smi_region_callback_t cb_fcn)
{
    DSECTION ("SMI_Set_region_callback");

    ASSERT_R(_smi_initialized, "SMI library not initialized", SMI_ERR_NOINIT);
    ASSERT_R(IS_VALID_ID(region_id), "Invalid region id", SMI_ERR_PARAM);
    
    SMI_LOCK(&_smi_region_lock);
    SMI_LOCK(&_smi_mis_lock);
    _smi_mis.region[region_id]->cb_fcn = cb_fcn;
    SMI_UNLOCK(&_smi_mis_lock);
    SMI_UNLOCK(&_smi_region_lock);

    return SMI_SUCCESS;
}

/*****************************************************************************/
/*** This function allocates the required segments for a region and maps   ***/
/*** them. If a single segment is too large such that it can be allocated, ***/
/*** it is split in two.                                                   ***/
/*****************************************************************************/
static smi_error_t _smi_allocate_contig(region_t* region)
{
    smi_error_t   error;
    shseg_t** tmp_sgmt;
    int sgmt = 0;
    int j;
	size_t tmp, region_size = 0;

    DSECTION("_smi_allocate_contig");
    SMI_STAT_ENTRY(allocate);
    DSECTENTRYPOINT;

    do { 
		DNOTICEI("Allocate segment nbr",sgmt);
		DNOTICEI("...of size",region->seg[sgmt]->size);
		DNOTICEI("...on machine",region->seg[sgmt]->machine);
		DNOTICEP("...at address",region->seg[sgmt]->address);
      
		error = _smi_create_shared_segment(region->seg[sgmt]); 

		if (error == SMI_SUCCESS)	{
			if (!(region->seg[sgmt]->flags & SHREG_NOMAP)) {
		
				error = _smi_map_shared_segment(region->seg[sgmt]);
				if (error != SMI_SUCCESS) {
					_smi_remove_shared_segment(region->seg[sgmt]); 
					DPROBLEM("could not map segment");
					DSECTLEAVE; SMI_STAT_EXIT(allocate);
					return(SMI_ERR_MAPFAILED);
				}  
				DNOTICEI("Allocated segment of size",region->seg[sgmt]->size);
			}
			else {
				DNOTICEI("Created unmapped segment of size",region->seg[sgmt]->size);		
			}
			if (region->size == 0)
				region_size += region->seg[sgmt]->size;
	    
			sgmt++;
		} else {
			if ((error != SMI_ERR_NOMEM) && (error != SMI_ERR_NOSEGMENT)) {
				/* in this case, we have a really serious problem which can not */
				/* be overcome by splitting the segment into two                */
				DPROBLEM("Could not create segment: unknown reason");
				DSECTLEAVE; SMI_STAT_EXIT(allocate);
				return(error);
			}	
	    
			if (region->seg[sgmt]->flags & SHREG_NONFIXED) {
				DPROBLEM("Could not create NON_FIXED segment: not enough continous SCI memory available");
				DSECTLEAVE; SMI_STAT_EXIT(allocate);
				return(error);
			}

			if (region->seg[sgmt]->device == DEV_SMP) {
				DPROBLEM("Could not create SMP segment: not enough IPC shared memory available");
				DSECTLEAVE; SMI_STAT_EXIT(allocate);
				return(error);
			}

			/* We try to construct the region from multiple smaller segments which 
			   we map one behind the other. */
			ZALLOCATE(tmp_sgmt, shseg_t**, (region->no_segments+1) * sizeof(shseg_t*));
			for (j = 0; j <= sgmt; j++)
				tmp_sgmt[j] = region->seg[j];
			for (j = sgmt+1; j < region->no_segments; j++)
				tmp_sgmt[j+1] = region->seg[j];
			ZALLOCATE(tmp_sgmt[sgmt+1], shseg_t*, sizeof(shseg_t));
			free(region->seg);
			region->seg = tmp_sgmt;
			(region->no_segments)++;
	  
			region->seg[sgmt+1]->size = region->seg[sgmt]->size;
			region->seg[sgmt]->size  /= 2;
			/* round to the next multiple of 512 kB to save ATT entries */
			tmp = (region->seg[sgmt]->size) % (512*1024);
			if (tmp <= 256*1024 && region->seg[sgmt]->size > tmp)
				region->seg[sgmt]->size -=tmp;
			else 
				if (region->seg[sgmt]->size > (512*1024)-tmp)
					region->seg[sgmt]->size += (512*1024)-tmp;

			region->seg[sgmt]->size   = (region->seg[sgmt]->size / _smi_page_size) * _smi_page_size;
			region->seg[sgmt+1]->size = region->seg[sgmt+1]->size - region->seg[sgmt]->size;
	    
			DNOTICEI("Splitting segment into sub-segments of size ",region->seg[sgmt]->size);
			if (region->seg[sgmt]->size < (size_t)_smi_page_size || region->seg[sgmt+1]->size < (size_t)_smi_page_size) {
				/* if this occures, we can  assume that there is 
				   really no possiblity to allocate this segment. */
				DPROBLEM("Could not create segment: not enough SCI memory available");
				DSECTLEAVE; return(SMI_ERR_NOSEGMENT);
			}
	  
			region->seg[sgmt+1]->address = (char*)((size_t)(region->seg[sgmt]->address)
												   + (size_t)(region->seg[sgmt]->size));
			region->seg[sgmt+1]->machine = region->seg[sgmt]->machine;
			region->seg[sgmt+1]->device  = region->seg[sgmt]->device;
			region->seg[sgmt+1]->connect_flag = region->seg[sgmt]->connect_flag;
		}
    } while (sgmt < region->no_segments);
  
    region->addresses[0] = region->seg[0]->flags & SHREG_DELAYED ? 0 : region->seg[0]->address;
    if (region->size == 0)
		region->size = region_size;

    DSECTLEAVE; SMI_STAT_EXIT(allocate);
    return(SMI_SUCCESS);
}

static smi_error_t _smi_allocate_fragmented(region_t* region)
{
	smi_error_t error;
	int sgmt, node, proc, i;
	smi_sgmt_locator_t* sgmt_locators;
	smi_sgmt_locator_t sgmt_locator;
  
	DSECTION("_smi_allocate_fragmented");
	SMI_STAT_ENTRY(allocate);
	DSECTENTRYPOINT;
  
	ZALLOCATE(sgmt_locators, smi_sgmt_locator_t*, _smi_nbr_procs*sizeof(smi_sgmt_locator_t));

	/* create the local segment */
	sgmt = _smi_my_machine_rank;
	if (_smi_my_proc_rank == _smi_first_proc_on_node(sgmt)) {
		DNOTICEI("Allocate segment nbr",sgmt);
		DNOTICEI("   of size",region->seg[sgmt]->size);
		DNOTICEI("   on machine",region->seg[sgmt]->machine);
		DNOTICEP("   at address",region->seg[sgmt]->address);
      
		error = _smi_create_shared_segment(region->seg[sgmt]);
		if (error != SMI_SUCCESS) {
			DPROBLEM("could not create segment");
			region->seg[sgmt]->id = 0;
		}
		sgmt_locator.segment_id = region->seg[sgmt]->id;
		sgmt_locator.sci_id     = region->seg[sgmt]->sci_id;
	} else {
		DNOTICE ("Using local segment");
		sgmt_locator.segment_id = -1;
		sgmt_locator.sci_id     = -1;
	}
  
	/* exchange the segment IDs */
	_smi_ll_allgather((int *)&sgmt_locator, sizeof(smi_sgmt_locator_t)/sizeof(int), 
					  (int *)sgmt_locators, _smi_my_proc_rank);
 
	/* connect to remote segments */ 
	sgmt = 0;
	do { 
		proc = _smi_first_proc_on_node(sgmt);
		region->seg[sgmt]->id = sgmt_locators[proc].segment_id;
		region->seg[sgmt]->sci_id = sgmt_locators[proc].sci_id; 
		if (region->seg[sgmt]->id <= 0) {
			error = SMI_ERR_NOSEGMENT;
			break;
		}
    
		if (proc != _smi_my_proc_rank) {
			DNOTICEI("Allocate segment nbr", sgmt);
			DNOTICEI("   of size", region->seg[sgmt]->size);
			DNOTICEI("   on machine", region->seg[sgmt]->machine);
			DNOTICEP("   at address", region->seg[sgmt]->address);
			DNOTICEI("   segment id", region->seg[sgmt]->id);
			DNOTICEI("   sci id", region->seg[sgmt]->sci_id);
			DNOTICEI("   adapter", region->seg[sgmt]->adapter);
		  
			error = _smi_create_shared_segment(region->seg[sgmt]);
			if (error != SMI_SUCCESS) {
				DPROBLEM("could not create segment");
				break;
			}
		}
      
		sgmt++;
	} while (sgmt < _smi_nbr_machines);
    
	/* in case of error: remove all segments which have been created so far */
	if (sgmt < _smi_nbr_machines) {
		DNOTICE ("Removing segments");
		for (i = 0; i < sgmt; i++) 
			_smi_remove_shared_segment(region->seg[i]); 
	  
		free (sgmt_locators);
		DSECTLEAVE; SMI_STAT_EXIT(allocate);
		return error;
	}
  
	/* map all segments */
	sgmt = 0;
	do {
		error = _smi_map_shared_segment(region->seg[sgmt]);
		if (error != SMI_SUCCESS) {
			DPROBLEMI("could not map segment nbr", sgmt);
			break; 
		}  
    
		DNOTICEI("Allocated segment of size",region->seg[sgmt]->size);
		sgmt++;
	} while (sgmt < _smi_nbr_machines);
  
	/* in case of error: unmap all segments which have been mapped so far */
	if (sgmt < _smi_nbr_machines) {
		DNOTICE ("Unmapping and removing segments created so far");
		for (i = 0; i < sgmt; i++) 
			_smi_unmap_shared_segment(region->seg[i]); 
		for (i = 0; i < _smi_nbr_machines; i++) 
			_smi_remove_shared_segment(region->seg[i]); 
    
		free (sgmt_locators);
		DSECTLEAVE; SMI_STAT_EXIT(allocate);
		return error;
	}  
  
	/* store addresses */
	for (proc = 0, node = 0; node < _smi_nbr_machines; node++) {
		int local_proc = 0;
		while (proc < _smi_nbr_procs && _smi_machine_rank[proc] == node) {
			size_t per_proc_size = region->seg[node]->size / _smi_procs_on_node(node);
			region->addresses[proc] = region->seg[node]->flags & SHREG_DELAYED ? 0 : 
				region->seg[node]->address + per_proc_size*local_proc;
			local_proc++; 
			proc++;
		}
	}
  
	free (sgmt_locators);
	DSECTLEAVE; SMI_STAT_EXIT(allocate);
	return error;
}


/* establish a blocked shared memory region */
static smi_error_t _smi_mk_blocked(region_t* region, smi_region_info_t *region_desc, device_t device, 
						unsigned int sgmt_flag)
{
    int node;
	size_t nbr_pages;
    int pages_so_far = 0;
    int pages_on_this_machine;
    double tmp;
    smi_error_t error;
    boolean finish = false;

    DSECTION ("_smi_mk_blocked");
    DSECTENTRYPOINT;
  
    nbr_pages = region_desc->size / _smi_page_size; 
    region->no_segments = _smi_nbr_machines;
    region->collective = true;

    ZALLOCATE(region->seg,shseg_t**,_smi_nbr_machines*sizeof(shseg_t*));
    for (node = 0; node < _smi_nbr_machines && finish == false; node++) {
		ZALLOCATE(region->seg[node],shseg_t*,sizeof(shseg_t));
		region->seg[node]->machine = node;
		/* XXX block distribution over available adapters, too ? */
		region->seg[node]->adapter = region_desc->adapter;
		region->seg[node]->sci_id  = 0;
		region->seg[node]->owner   = _smi_first_proc_on_node(node);
		region->seg[node]->partner = -1;
		region->seg[node]->id      = 0;
      
		if (node == _smi_nbr_machines-1) {
			region->seg[node]->size = _smi_page_size * (nbr_pages - pages_so_far);
		} else {
			tmp = (double)(_smi_last_proc_on_node(node)+1) 
				* ((double)nbr_pages/(double)_smi_nbr_procs);
			tmp = tmp - (double)pages_so_far;
			pages_on_this_machine = (int)tmp;
			if (tmp - (double)pages_on_this_machine > 0.5)
				pages_on_this_machine++;
			pages_on_this_machine = imax(1, pages_on_this_machine);
			region->seg[node]->size = _smi_page_size * pages_on_this_machine;

			pages_so_far += pages_on_this_machine;
			if (pages_so_far == nbr_pages) {
				if (pages_on_this_machine==0)
					region->no_segments = node;
				else
					region->no_segments = node+1;
				finish = true;
			}
		}
      
		if (node == 0)
			region->seg[node]->address = region->addresses[0];
		else
			region->seg[node]->address = (char*)((size_t)(region->seg[node-1]->address)
												 + (size_t)(region->seg[node-1]->size));
      
		region->seg[node]->offset = 0;
		region->seg[node]->flags  = 0;
		region->seg[node]->device = device;     
		region->seg[node]->connect_flag = 0;
    }  
  
    error = _smi_allocate_contig(region);
    DSECTLEAVE;
    return(error);
}

/* esthablish a pt2pt shared memory region */
static smi_error_t _smi_mk_pt2pt(region_t* region, smi_region_info_t *region_desc, 
					  device_t device, unsigned int sgmt_flag)
{
    DSECTION ("_smi_mk_pt2pt");
    smi_error_t error;

    DSECTENTRYPOINT;

    region->no_segments = 1;
    region->collective = false;
    
    ZALLOCATE (region->seg, shseg_t**, sizeof(shseg_t*));
    ZALLOCATE (region->seg[0], shseg_t*, sizeof(shseg_t));

    region->seg[0]->size     = region->size;
    region->seg[0]->offset   = region_desc->offset;
    region->seg[0]->adapter  = region_desc->adapter;
    region->seg[0]->machine  = _smi_machine_rank[region_desc->owner];
    region->seg[0]->owner    = region_desc->owner;
    region->seg[0]->partner  = region_desc->partner;
    region->seg[0]->id       = region_desc->sgmt_id;
    region->seg[0]->address  = region->addresses[0];
    
    region->seg[0]->device = device;
    region->seg[0]->flags  = sgmt_flag|SHREG_PT2PT;
    region->seg[0]->connect_flag = 0;
    
    error = _smi_allocate_contig(region); 

    DSECTLEAVE; return(error);
}

/* esthablish a shared memory region in a singlesided manner (without 
   concurrent partication of another process) */
static smi_error_t _smi_mk_singlesided(region_t* region, smi_region_info_t *region_desc, 
							device_t device, unsigned int sgmt_flag)
{
    DSECTION ("_smi_mk_singlesided");
    smi_error_t error;
    size_t iSize = region->size;

    DSECTENTRYPOINT;

    region->no_segments = 1;
    region->collective = false;
    
    ZALLOCATE (region->seg, shseg_t**, sizeof(shseg_t*));
    ZALLOCATE (region->seg[0], shseg_t*, sizeof(shseg_t));

    do {	
		region->size = iSize;
		region->seg[0]->size     = region->size;
		region->seg[0]->offset   = region_desc->offset;
		region->seg[0]->adapter  = region_desc->adapter;
		DNOTICEI ("Using adapter", region->seg[0]->adapter);
		region->seg[0]->sci_id   = (device == DEV_SMP) ? 0 :
			_smi_adpt_sci_id[region_desc->owner].sci_id[region_desc->rmt_adapter];
		region->seg[0]->machine  = _smi_machine_rank[region_desc->owner];
		region->seg[0]->owner    = region_desc->owner;
		region->seg[0]->partner  = (region_desc->owner == _smi_my_proc_rank) ? 
			-1 : _smi_my_proc_rank;
		region->seg[0]->id       = (region_desc->owner == _smi_my_proc_rank) ? 
			0 : region_desc->sgmt_id;
		region->seg[0]->address  = (sgmt_flag & SHREG_REGISTER) ? 
			region->addresses[0] : NULL;
	
		region->seg[0]->device = device;
		region->seg[0]->flags  = sgmt_flag|SHREG_ASYNC|SHREG_NONFIXED;
		region->seg[0]->connect_flag = 0;
	
		error = _smi_allocate_contig(region); 
	
		if (region_desc->shrinkable && (error == SMI_ERR_NOMEM || error == SMI_ERR_NOSEGMENT)) {
			iSize = (int) (0.9 * (double) iSize);
			if (!(sgmt_flag & SHREG_REGISTER) && (iSize % _smi_page_size != 0)) 
				SMI_ALIGN_SIZE(iSize);
		}
    } while (region_desc->shrinkable && (error == SMI_ERR_NOMEM || error == SMI_ERR_NOSEGMENT));
    
    DSECTLEAVE; return(error);
}

/* Esthablish a fragmented shared memory region. One segment per node (not per process!)
   is created to reduce the resource consumption. Each process get's the same amount of 
   memory, though. */
static smi_error_t _smi_mk_fragmented(region_t* region, smi_region_info_t *region_desc, 
						   device_t device, unsigned int sgmt_flag)
{
    smi_error_t error;
    int sgmt;
	size_t nbr_pages;
    DSECTION ("_smi_mk_fragmented");
    DSECTENTRYPOINT;

    /* fragmented regions need to be sized specially */
    nbr_pages = region->size/_smi_page_size;
    if (nbr_pages % region_desc->nbr_sgmts) {
		nbr_pages = (nbr_pages/region_desc->nbr_sgmts + 1)*region_desc->nbr_sgmts;
		region->size = nbr_pages * _smi_page_size;
    }

    /* one segment per node */     
    region_desc->nbr_sgmts = _smi_nbr_machines;
    region->no_segments = _smi_nbr_machines;
    region->collective  = true;
    
    ZALLOCATE (region->seg, shseg_t**, sizeof(shseg_t*)*region->no_segments);
    for (sgmt = 0; sgmt < region->no_segments; sgmt++) {
		ZALLOCATE (region->seg[sgmt], shseg_t*, sizeof(shseg_t));
	
		region->seg[sgmt]->size    = region->size/_smi_nbr_procs * _smi_procs_on_node(sgmt);
		region->seg[sgmt]->offset  = 0;
		region->seg[sgmt]->machine = sgmt;
		region->seg[sgmt]->owner   = _smi_first_proc_on_node(sgmt);
		region->seg[sgmt]->partner = -1;
		region->seg[sgmt]->id      = 0;

		/* XXX This works, but is not exactly the way IMPEXP is meant to be. The problem
		   is, that only one adapter is specified in region_desc, but for fragmented 
		   regions, we do both import and export segments */
		if (region->seg[sgmt]->owner == _smi_my_proc_rank) {
			/* always use the default adapter to export segments */
			region->seg[sgmt]->adapter = _smi_adpt_export;
		} else {
			region->seg[sgmt]->adapter = _smi_adpt_import;
		}

		region->seg[sgmt]->sci_id  = 0;
		region->seg[sgmt]->address = NULL;
	
		region->seg[sgmt]->device   = device;
		region->seg[sgmt]->flags    = SHREG_NONFIXED|SHREG_ASYNC;
		region->seg[sgmt]->connect_flag = 0;
    }

    error = _smi_allocate_fragmented(region); 

    /* XXX hack: allocation can be done asynchronous, 
       but de-allocation needs to be synchronous */
    for (sgmt = 0; sgmt < region->no_segments; sgmt++)
		region->seg[sgmt]->flags = SHREG_NONFIXED;

    DSECTLEAVE; return (error);
}

/* esthablish an undivided shared memory region */
static smi_error_t _smi_mk_undivided(region_t* region, smi_region_info_t *region_desc, 
						  device_t device, unsigned int sgmt_flag)
{
    DSECTION ("_smi_mk_undivided");
    smi_error_t error;
    size_t iSize = region->size;

    DSECTENTRYPOINT;

    region->no_segments = 1;
    region->collective = true;
    
    ZALLOCATE (region->seg, shseg_t**, sizeof(shseg_t*));
    ZALLOCATE (region->seg[0], shseg_t*, sizeof(shseg_t));
    
    do {
		region->size = iSize;
		region->seg[0]->size     = region->size;
		region->seg[0]->offset   = region_desc->offset;
		region->seg[0]->adapter  = region_desc->adapter;
		region->seg[0]->sci_id   = 0;
		region->seg[0]->machine  = _smi_machine_rank[region_desc->owner];
		region->seg[0]->owner    = region_desc->owner;
		region->seg[0]->partner  = -1;
		region->seg[0]->id       = 0;
		region->seg[0]->address  = region->addresses[0];
	
		region->seg[0]->device   = device;
		region->seg[0]->flags    = sgmt_flag;
		region->seg[0]->connect_flag = 0;
	
		error = _smi_allocate_contig(region);

		/* if region is shrinkable and theres not enough memory available,
		   reduce size be 10 percent */
		if ((region_desc->shrinkable == TRUE) && ((error == SMI_ERR_NOMEM) ||
												  (error == SMI_ERR_NOSEGMENT))) {
			iSize = (int) (0.9 * (double) iSize);
			if (!(sgmt_flag & SHREG_REGISTER) && (iSize % _smi_page_size != 0)) 
				SMI_ALIGN_SIZE(iSize);
		}
    } while ((region_desc->shrinkable == TRUE) && ((error == SMI_ERR_NOMEM) || (error == SMI_ERR_NOSEGMENT)));
    
    DSECTLEAVE; return(error);
}

/* esthablish an customized shared memory region  */
static smi_error_t _smi_mk_customized(region_t* region, smi_region_info_t *region_desc, device_t device,
						   unsigned int sgmt_flag)
{
    int sgmt = 0;  
    size_t size_so_far = 0; /* accumulator for the size of the region constituted
							by the segmenst so far */

    DSECTION ("_smi_mk_customized");
    smi_error_t error;
    
    DSECTENTRYPOINT;
  
    ZALLOCATE (region->seg, shseg_t**, region_desc->nbr_sgmts*sizeof(shseg_t*));
    region->no_segments = region_desc->nbr_sgmts;
    region->collective = true;
  
    while (sgmt < region_desc->nbr_sgmts && size_so_far < region->size) {
		ZALLOCATE (region->seg[sgmt], shseg_t*, sizeof(shseg_t));
		region->seg[sgmt]->size = imin(region_desc->sgmt_size[sgmt], 
									   region->size-size_so_far);
		if (region->seg[sgmt]->size % _smi_page_size != 0)
			region->seg[sgmt]->size = (1 + (region->seg[sgmt]->size / _smi_page_size))*_smi_page_size;
		size_so_far += region->seg[sgmt]->size;
      
		/* XXX allow individual offsets ? */
		region->seg[sgmt]->offset  = 0;
		region->seg[sgmt]->adapter = region_desc->adapter;
		region->seg[sgmt]->machine = _smi_machine_rank[region_desc->sgmt_owner[sgmt]];
		region->seg[sgmt]->owner   = _smi_first_proc_on_node(region->seg[sgmt]->machine);
		region->seg[sgmt]->partner = -1;
		region->seg[sgmt]->id      = 0;
		region->seg[0]->sci_id     = 0;
		region->seg[sgmt]->address = (sgmt == 0) ?
			region->addresses[0] :
			(char*)((size_t)(region->seg[sgmt-1]->address) 
					+ (size_t)(region->seg[sgmt-1]->size));
		region->seg[sgmt]->device = device;
		region->seg[sgmt]->flags  = sgmt_flag;
		region->seg[sgmt]->connect_flag = 0;

		sgmt++;
    }
  
    region->size = 0;
    for(sgmt = 0; sgmt < region->no_segments; sgmt++)
		region->size += region->seg[sgmt]->size;

    error = _smi_allocate_contig(region);
    DSECTLEAVE; return(error);
}

static smi_error_t _smi_mk_private(region_t* region, smi_region_info_t *region_desc, device_t device,
						unsigned int sgmt_flag)
{
    DSECTION ("_smi_mk_private");
    smi_error_t error;

    DSECTENTRYPOINT;

    region->no_segments = 1;
    region->collective = false;
    
    ZALLOCATE (region->seg, shseg_t**, sizeof(shseg_t*));
    ZALLOCATE (region->seg[0], shseg_t*, sizeof(shseg_t));
  
    region->seg[0]->size     = region->size;
    region->seg[0]->offset   = 0;
    region->seg[0]->adapter  = region_desc->adapter;
    region->seg[0]->machine  = _smi_machine_rank[region_desc->owner];
    region->seg[0]->owner    = region_desc->owner;
    region->seg[0]->partner  = -1;
    region->seg[0]->id       = 0;
    region->seg[0]->sci_id   = (device == DEV_SMP) ? 0 :
		_smi_adpt_sci_id[region_desc->owner].sci_id[region_desc->adapter];
    region->seg[0]->address  = region->addresses[0];
    
    region->seg[0]->device   = device;
    region->seg[0]->flags    = SHREG_PRIVATE|SHREG_ASYNC;
    region->seg[0]->connect_flag = 0;

    error = _smi_allocate_contig(region);
    
    DSECTLEAVE; return(error);
}

/*****************************************************************************/
/*** Internal function: Find a free region identifier, enlarge the data    ***/
/*** structure that enumerates the existing shared regions fill into the   ***/
/*** call-by-reference parameters the found identifier and a pointer to a  ***/
/*** newly allocated region structure                                      ***/
/*****************************************************************************/
smi_error_t _smi_prepare_for_new_region(int* id, region_t** new_region)
{
    region_t** tmp;            /* used temporary to expand the array of existing regions */
    int i;
	
    REMDSECTION ("_smi_prepare_for_new_region");
    DSECTENTRYPOINT;
	
	SMI_LOCK(&_smi_mis_lock);
	
    /* see if the region table contains a free slot */
    *id = _smi_idstack_pop();
    if (*id == -1) {
		/* enlarge the region table to one more entry */
		ZALLOCATE(tmp,region_t**,(_smi_mis.no_regions+1)*sizeof(region_t*));
		for(i = 0; i < _smi_mis.no_regions; i++)
			tmp[i] = _smi_mis.region[i];
		free(_smi_mis.region);
		_smi_mis.region = tmp;
		
		/* allocate one more region struct */
		ZALLOCATE(*new_region, region_t*,sizeof(region_t));
		_smi_mis.region[_smi_mis.no_regions] = *new_region;
		_smi_mis.no_regions++;
		
		/* find a free identifier */
		(*new_region)->id = _smi_mis.no_regions - 1;
		*id = (*new_region)->id;
    } else {
		/* take first available recently used id, and the associated region structure */
		*new_region = _smi_mis.region[*id];
		(*new_region)->id = *id;	
    }
	
	SMI_UNLOCK(&_smi_mis_lock);
    DSECTLEAVE;
    return(SMI_SUCCESS);
}


/*****************************************************************************/
/*** undo the preparations for a new region if the creation of the region  ***/
/*** failed - otherwise, problems will arise in SMI_Finalize()             ***/
/*****************************************************************************/
static smi_error_t _smi_remove_region_data_structure (region_t* failed_region)
{
    int free_id = failed_region->id;

    /* keep the region data structures but mark unused slot as free */
    failed_region->id = -1;
    _smi_idstack_push(free_id);
	
    return(SMI_SUCCESS);
}

/*****************************************************************************/
/*** Generates and fills-in the complete data structure for a new shared   ***/
/*** region. The only thing that is missing compared to the                ***/
/*** SMI_Create_shreg function is the fill in of the used device and the   ***/
/*** mapping of memory to the segments. The address to the newly generated ***/
/*** region structure is returned.                                         ***/
/*****************************************************************************/
smi_error_t _smi_create_region_data_structure(int region_type, smi_region_info_t *region_desc,
										  int* id, char** address, region_t** region,
										  device_t device, unsigned int sgmt_flag)
{
    region_t* new_region, *busy_reg = NULL;
    int i;
    smi_error_t error;
  
    DSECTION ("_smi_create_region_data_structure");
    DSECTENTRYPOINT;

    DNOTICEI("sgmt_flag is ",sgmt_flag);
    DNOTICEI("region_type is ",region_type);

    /* Round 'total_size' to the next higher multiple of the page-size. This
       is not allowed when registering already allocated memory. */
    if (!(sgmt_flag & SHREG_REGISTER) && (region_desc->size % _smi_page_size != 0)) 
		SMI_ALIGN_SIZE(region_desc->size);

    /* Determine the start address for FIXED regions. */
    if ((region_type != SMI_SHM_PRIVATE) && !(sgmt_flag & SHREG_NONFIXED)) {
		error = _smi_determine_start_address(address, region_desc->size);
		if (error != SMI_SUCCESS) {
			DPROBLEM ("Could not determine a start address");
			DSECTLEAVE;
			return(error);
		}
    } else {
		/* For registered segments, we *have* to use the supplied address. */
		if (!(sgmt_flag & SHREG_REGISTER))
			*address = NULL;
    }
    
    /* enlarge region data structure and find a free identifier */
    error = _smi_prepare_for_new_region(id, &new_region);
    if (error != SMI_SUCCESS) {
		DPROBLEM ("Could not allocate region data structures");
		DSECTLEAVE; return(error);
    }

    ZALLOCATE(new_region->addresses, char **, region_desc->nbr_sgmts*sizeof(char *));
    new_region->addresses[0] = *address;
    new_region->size = region_desc->size;
    new_region->counterpart_id = -1;
    new_region->replication    = false;
    new_region->nbr_rmt_cncts  = 0;
    new_region->type           = region_type;
    new_region->cb_thread      = 0;
    new_region->cb_fcn         = NULL;
  
	/* Free all busy shared regions to make space for new ones... */
	_smi_free_busy_shregs();

	switch (region_type) {
	case SMI_SHM_UNDIVIDED:
		error = _smi_mk_undivided(new_region, region_desc, device, sgmt_flag);
		break;
	case SMI_SHM_BLOCKED:
		error = _smi_mk_blocked(new_region, region_desc, device, sgmt_flag);
		break;
	case SMI_SHM_CUSTOMIZED:
		error = _smi_mk_customized(new_region, region_desc, device, sgmt_flag);
		break;
	case SMI_SHM_CYCLIC:
		DWARNING("CYCLIC distribution not yet implemented");
		error = SMI_ERR_NOTIMPL;      
		break;
	case SMI_SHM_SMP:
		region_desc->owner = _smi_first_proc_on_node(_smi_my_machine_rank);
		error = _smi_mk_undivided(new_region, region_desc, device, sgmt_flag);
		break;
	case SMI_SHM_PRIVATE:
		region_desc->owner = _smi_my_proc_rank;
		error = _smi_mk_private(new_region, region_desc, device, sgmt_flag);
		break;
	case SMI_SHM_PT2PT:
		error = _smi_mk_pt2pt(new_region, region_desc, device, sgmt_flag);
		break;
	case SMI_SHM_FRAGMENTED:
		error = _smi_mk_fragmented(new_region, region_desc, device, sgmt_flag);
		break;
	case SMI_SHM_LOCAL:
	case SMI_SHM_REMOTE:
	case SMI_SHM_RDMA:
		error = _smi_mk_singlesided(new_region, region_desc, device, sgmt_flag);
		break;
	default:
		error = SMI_ERR_NOSEGMENT;
		DPROBLEM ("illegal distribution policy!");
	}

    if (error != SMI_SUCCESS) {
		_smi_remove_region_data_structure (new_region);	
		DPROBLEM ("Could not create region");
    } else {
		for (i = 0; i < region_desc->nbr_sgmts; i++)
			new_region->seg[i]->region_id = new_region->id;
    }

    *region = new_region;
  
    DSECTLEAVE; return(error);
}

smi_error_t SMI_Init_reginfo(smi_region_info_t* region_desc, size_t size, size_t offset, int owner, int loc_adpt, 
						 int rmt_adpt, int sgmt_id, smi_region_callback_t cb_fcn)
{
    DSECTION("SMI_Init_reginfo");

    DSECTENTRYPOINT;

	/* mark region description as 'invalid' */
    region_desc->size = -1;

    ASSERT_R(_smi_initialized, "SMI library not initialized", SMI_ERR_NOINIT);
    ASSERT_R(region_desc != NULL, "NULL ptr for region description", SMI_ERR_PARAM);
    ASSERT_R((owner >= 0) && (owner < _smi_nbr_procs), "Illegal owner rank specified", SMI_ERR_PARAM);
    
    memset(region_desc, 0, sizeof(smi_region_info_t));
    region_desc->size    = size;
    region_desc->offset  = offset;
    region_desc->owner   = owner;
    region_desc->adapter = loc_adpt;
    region_desc->rmt_adapter = rmt_adpt;    
    region_desc->sgmt_id = sgmt_id;
    region_desc->cb_fcn  = cb_fcn;
   
    DSECTLEAVE; 
    return(SMI_SUCCESS);
}

/*****************************************************************************/
/*** Create a shared memory region.                                        ***/
/*****************************************************************************/
smi_error_t SMI_Create_shreg(int region_type, smi_region_info_t *region_desc, int* id, void **address)
{ 
    DSECTION ("SMI_Create_shreg");
    region_t *new_region;      
    memtree_memarea_t memarea;
    device_t device;
    smi_error_t error/*, rmt_error, *all_errors*/;
    unsigned int flags = 0, i;
    int local_rank, retval;
    boolean delayed = false,
		nonfixed = false, 
		private = false, 
		regist = false,
		local = false,
		shrinkable = false,
		callback = false;  
    
    SMI_STAT_ENTRY(create_shreg);
    DSECTENTRYPOINT;
    ASSERT_R(_smi_initialized, "SMI library not initialized", SMI_ERR_NOINIT);

    DNOTICE ("Creating shared region:");
    DNOTICEI("  type:", region_type);
    DNOTICEI("  size:", region_desc->size);
  
    ASSERT_R(region_desc != NULL, "NULL ptr for region description", SMI_ERR_PARAM);
    SMI_LOCK(&_smi_region_lock);

    device = (_smi_all_on_one == true) ? DEV_SMP : DEV_GLOBAL;
  
    /* parameter extraction */
    if (region_type & SMI_SHM_DELAYED) {
		delayed = true;
		region_type = region_type & (~(int)SMI_SHM_DELAYED);
    }
    if (region_type & SMI_SHM_NONFIXED) {
		nonfixed = true;
		region_type = region_type & (~(int)SMI_SHM_NONFIXED);
    }
    if (region_type & SMI_SHM_PRIVATE) {
		private = true;
		region_type = region_type & (~(int)SMI_SHM_PRIVATE);
    }
    if (region_type & SMI_SHM_REGISTER) {
		regist = true;
		flags |= SHREG_NOMAP;
		region_type = region_type & (~(int)SMI_SHM_REGISTER);
    }
    if (region_type & SMI_SHM_INTERN) {
		local = true;
		device = DEV_SMP;
		region_type = region_type & (~(int)SMI_SHM_INTERN);
    }
    if (region_type & SMI_SHM_SHRINKABLE) {
		shrinkable = true;
		region_type = region_type & (~(int)SMI_SHM_SHRINKABLE);
    }
    if (region_type & SMI_SHM_CALLBACK) {
		callback = true;
		region_type = region_type & (~(int)SMI_SHM_CALLBACK);
    }
	
    if (!_smi_all_on_one) {
		/* Check adapter specification. */
		/* XXX: For now, region-specific adapter selection is not yet supported. This is mostly
		   a performance problem, as for each transfer check, the adapter which was used for this 
		   transfer needs to be determined. In most cases, this will be done via the addresss, which
		   requires checking the memory region tree each time.  With global adapter selection, 
		   the used adapter is always known in advance. */
		switch (region_desc->adapter) {
		case SMI_ADPT_DEFAULT:
			/* We need to determine if we do an export or import of memory. */
			/* XXX region_desc->owner is possibly uninitialized */
			/* region_desc->adapter = (_smi_machine_rank[region_desc->owner] == _smi_my_machine_rank) ?
			   _smi_adpt_export : _smi_adpt_import; */
			region_desc->adapter = 0;
			
			DNOTICEI("Setting adapter to default:", region_desc->adapter);
			break;
		case SMI_ADPT_CYCLIC:
			DWARNING ("Region-specific adapter selection not yet fully supported!");
	  
			for (i = adapter_last_used + 1; i < 2*MAX_ADAPTER; i++)
				if (_smi_adapter_available(i % MAX_ADAPTER))
					break;
			ASSERT_A(i < 2*MAX_ADAPTER, "there is no adapter available", SMI_ERR_OTHER);
			region_desc->adapter = i % MAX_ADAPTER;
			adapter_last_used = region_desc->adapter;
			DNOTICEI("Cyclic setting adapter to:", region_desc->adapter);
			break;
		case SMI_ADPT_SMP:
			DWARNING ("Region-specific adapter selection not yet fully supported!");

			SMI_Local_proc_rank(&local_rank);
			for (i = 0; i < (unsigned int)_smi_nbr_procs*MAX_ADAPTER; i++) {
				if (_smi_adapter_available(i % MAX_ADAPTER))
					local_rank--;
				if (local_rank < 0)
					break;
			}
			ASSERT_A(i < (unsigned int)_smi_nbr_procs*MAX_ADAPTER, "there is no adapter available", SMI_ERR_OTHER);
			region_desc->adapter = i % MAX_ADAPTER;
	  
			DNOTICEI("SMP setting adapter to:", region_desc->adapter);
			break;
		case SMI_ADPT_IMPEXP:
			DWARNING ("Region-specific adapter selection not yet fully supported!");

			if (region_desc->owner == _smi_my_proc_rank) {
				region_desc->adapter = _smi_DefAdapterNbr;
			} else {
				for (i = _smi_DefAdapterNbr + 1; i < 2*MAX_ADAPTER; i++)
					if (_smi_adapter_available(i % MAX_ADAPTER))
						break;
				ASSERT_A(i < 2*MAX_ADAPTER, "there is no adapter available", SMI_ERR_OTHER);
				region_desc->adapter  = i % MAX_ADAPTER;
			}
			DNOTICEI("EXPIMP setting adapter to:",region_desc->adapter);
			break;
		default:
			DWARNING ("Region-specific adapter selection not yet fully supported!");

			ASSERT_R_UNLOCK(region_desc->adapter < MAX_ADAPTER && region_desc->adapter >= 0,
							"Illegal adapter number", SMI_ERR_PARAM, _smi_region_lock);
#ifndef NO_SISCI
			SMI_Query(SMI_Q_SCI_VALIDADAPTER, region_desc->adapter, &i);
			ASSERT_R_UNLOCK(i != 0, "The specified adapter number does not exist", SMI_ERR_PARAM,
							_smi_region_lock);
#endif
		}
    }
    /* evaluate region specification and set relavant default values */
    ASSERT_R_UNLOCK(region_type >= 0 && region_type <= SHREG_MAX_REGION, "Illegal region_type", 
					SMI_ERR_PARAM, _smi_region_lock);
    switch(region_type) {
    case SMI_SHM_UNDIVIDED:
    case SMI_SHM_PT2PT:
		ASSERT_R_UNLOCK(region_desc->owner >= 0 && region_desc->owner < _smi_nbr_procs,
						"Illegal proc rank in region description", SMI_ERR_PARAM, _smi_region_lock);      
		
		if (nonfixed) 
			flags |= SHREG_NONFIXED;      
		if (delayed) {
			ASSERT_R_UNLOCK(nonfixed == TRUE, "DELAYED segments must be NONFIXED", SMI_ERR_PARAM,
							_smi_region_lock);
			flags |= SHREG_DELAYED;
		}
		region_desc->nbr_sgmts = 1;
		break;
    case SMI_SHM_SMP:
		if (nonfixed) 
			flags |= SHREG_NONFIXED;      
		region_desc->nbr_sgmts = 1;
		break;
    case SMI_SHM_FRAGMENTED:
		region_desc->nbr_sgmts = _smi_nbr_procs;
		flags |= SHREG_NONFIXED;
		break;
    case SMI_SHM_LOCAL:
		flags |= SHREG_NONFIXED;
	
		if (regist) {
			ASSERT_R_UNLOCK (*address != NULL, "Must provide shreg address", 
							 SMI_ERR_BADADR, _smi_region_lock);
			flags |= SHREG_REGISTER;
		}
		if (private) {
			ASSERT_R_UNLOCK (device == DEV_GLOBAL, "PRIVATE not possible for SMP usage", 
							 SMI_ERR_NODEVICE, _smi_region_lock);
			flags |= SHREG_PRIVATE;
		}
		region_desc->nbr_sgmts = 1;
		region_desc->rmt_adapter = 0;   /* not needed for local segment */
		break;
	
    case SMI_SHM_RDMA:
		flags |= SHREG_NOMAP;
		/* fall through */
    case SMI_SHM_REMOTE:
		flags |= SHREG_NONFIXED;
	
		ASSERT_R_UNLOCK (region_desc->sgmt_id > 0, "invalid segment ID (must be > 0)", 
						 SMI_ERR_PARAM, _smi_region_lock);
		ASSERT_R_UNLOCK (region_desc->owner != _smi_my_proc_rank, "invalid owner rank (can not map remote from myself)", 
						 SMI_ERR_PARAM, _smi_region_lock);
		ASSERT_R_UNLOCK (region_desc->owner < _smi_nbr_procs, "invalid owner rank (must be < numprocs", 
						 SMI_ERR_PARAM, _smi_region_lock);
		region_desc->nbr_sgmts = 1;
		break;
    case SMI_SHM_BLOCKED:
		region_desc->nbr_sgmts = _smi_nbr_machines;
		break;
    default:
		break;
    } 

    /* shrinkable only possible if theres only one segment */
    ASSERT_R_UNLOCK((region_desc->nbr_sgmts == 1) || (shrinkable == false),
					"SMI_SHM_SHRINKABLE implemented for one-segment regions only",
					SMI_ERR_NOTIMPL, _smi_region_lock);
    region_desc->shrinkable = shrinkable;

#ifdef SCALI_SISCI
    if (flags & SHREG_NONFIXED == 0 && !_smi_all_on_one && region_type != SMI_SHM_SMP) {
		DPROBLEM("fixed mapping is not supported by Scali SISCI");
		DSECTLEAVE; SMI_UNLOCK(&_smi_region_lock); return(SMI_ERR_NOTIMPL);
    }
#endif

    /* prepare and fill in the whole data structure in the 'mis' structure, 
       then create the segment */
    error = _smi_create_region_data_structure(region_type, region_desc, id, (char **)address, 
											  &new_region, device, flags);

#if 0
    /* verify successful creation of the region on all nodes which have participated */
    if (0) {
		rmt_error = SMI_SUCCESS;
		ZALLOCATE(all_errors, smi_error_t*, _smi_nbr_procs*sizeof(smi_error_t));
		_smi_ll_allgather((int *)&error, sizeof(smi_error_t)/sizeof(int), (int *)all_errors, _smi_my_proc_rank);
		for (i = 0; i < _smi_nbr_procs; i++) {
			if (all_errors[i] != SMI_SUCCESS)
				rmt_error = all_errors[i];
		}
		free (all_errors);
    }
	
    if ((rmt_error != SMI_SUCCESS) && (error == SMI_SUCCESS)) {
		DPROBLEM ("A remote process could not create the shared region");
		DNOTICE ("Deallocating local ressources");	
		error = rmt_error;
    }
#endif

    if (error == SMI_SUCCESS) {
		if (region_type != SMI_SHM_FRAGMENTED) {
			/* the region has only one address */
			*address = (error == SMI_SUCCESS) ? new_region->addresses[0] : NULL;
		} else {
			/* each segment of the region has its own address */
			for (i = 0; i < (unsigned int)_smi_nbr_procs; i++)
				address[i] = (error == SMI_SUCCESS) ? new_region->addresses[i] : NULL;
		}
      
		/* this assertion has been removed, you can reactivate it for test-purposes */ 
		/* ASSERT_R(*address != NULL,"something went wrong", SMI_ERR_OTHER); */	    
      
		/* SMI_SHM_RDMA will not have a memory mapping */
		if (region_type != SMI_SHM_RDMA) {
			/* store adress to memtree */
			memarea.pStart = *address;
			memarea.iSize = new_region->size;
			_smi_memtree_insert(_smi_memtree, &memarea, *id);
		}

#if defined DOLPHIN_SISCI && !defined DISABLE_THREADS
		/* Create a thread to wait for potential segment events and perform the callback. This 
		   is only possible for regions which consist of a single segment. */
		if (callback && new_region->no_segments == 1 && !_smi_all_on_one) {
			new_region->cb_fcn = region_desc->cb_fcn;
			/* XXX create a detached thread? */
			retval = rs_pthread_create (&new_region->cb_thread, NULL, _smi_sgmt_cb, (void *)&new_region->id);
			ASSERT_A(retval == 0, "Could not create segment callback thread.", -1);
		}
#endif
		SMI_LOCK(&_smi_mis_lock);
		_smi_mis.nbr_user_regions++;
		SMI_UNLOCK(&_smi_mis_lock);

		DNOTICE ("Shared region successfully created:");
		DNOTICEP(" address:", *address);
		DNOTICEI(" size   :", new_region->size);
		DNOTICEI(" SMI ID :", new_region->id);
    } else {
		DPROBLEM ("Could not create shared region");
    }

    DSECTLEAVE; SMI_STAT_EXIT(create_shreg);
    SMI_UNLOCK(&_smi_region_lock); return error;
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
