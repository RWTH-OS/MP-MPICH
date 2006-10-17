/* $Id$ */

#include <unistd.h>

#include "smi.h"
#include "env/general_definitions.h"
#include "env/smidebug.h"
#include "sci_shmem.h"
#include "env/error_count.h"
#include "proc_node_numbers/first_proc_on_node.h"
#include "utility/statistics.h"
#include "proper_shutdown/resource_list.h"
#include "message_passing/lowlevelmp.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#define USE_SEGMENT_CALLBACK 0

static int  max_sci_id;  /* maximum sci-segment identifier used so far */
       
static int nbr_locsgmts = 0;
static size_t size_locsgmts = 0;
static int nbr_rmtsgmts = 0;
static size_t size_rmtsgmts = 0;

#if USE_SEGMENT_CALLBACK
sci_callback_action_t _smi_rmt_sgmt_callback (void* arg, sci_remote_segment_t segment, 
										 sci_segment_cb_reason_t reason, sci_error_t error)
{
    DSECTION("rmt_sgmt_callback");
    int reg_id = *(int *)arg;
    int reg_type;

    DSECTENTRYPOINT;
    
    switch (reason) {
    case SCI_CB_DISCONNECT:
		DNOTICEI("received remote segment callback SCI_CB_DISCONNECT for region", reg_id);
		/* XXX Check if this is an internal region or an user region. 
		   A user region can be disconnected if it is of type REMOTE or RDMA. 
		   
		   All other regions can not be asynchronously withdrawn: a 
		   withdrawal is a fatal error for these. The same is true for an 
		   internal region. */
#if 0
		SMI_LOCK(&_smi_mis_lock);
		reg_type = _smi_mis.region[reg_id]->type;
		SMI_UNLOCK(&_smi_mis_lock);

		switch (reg_type) {
		case SMI_SHM_REMOTE:
		case SMI_SHM_RDMA:
			DNOTICEI("removing region", reg_id);
			SMI_Free_shreg(reg_id);
			break;
		default:
			DERRORI ("segment of collective region was withdrawn - region ID", reg_id);
			SMI_Abort (-1);
			break;
		}
#endif
		break;
    case SCI_CB_CONNECT:
		DNOTICEI("received remote segment callback SCI_CB_CONNECT for region", reg_id);
		break;
    default:
		DNOTICEI("unhandled segment callback reason", reason);
		break;
    }
    
	DSECTLEAVE; return (SCI_CALLBACK_CONTINUE);
}


sci_callback_action_t _smi_local_sgmt_callback (void* arg, sci_local_segment_t segment, 
												sci_segment_cb_reason_t reason, 
												unsigned int sci_node_id, unsigned int adapt_nbr,
												sci_error_t error)
{
	DSECTION("local_sgmt_callback");
    int reg_id = *(int *)arg;
    
    DSECTENTRYPOINT;
    
    switch (reason) {
    case SCI_CB_CONNECT:
		DNOTICEI("received local segment callback SCI_CB_CONNECT for region", reg_id);
		/* only do increment the connect counter 
		   XXX: threadsafety! But need to avoid deadlock. */
		SMI_LOCK(&_smi_mis_lock);
		_smi_mis.region[reg_id]->nbr_rmt_cncts++;
		SMI_UNLOCK(&_smi_mis_lock);
		break;
    case SCI_CB_DISCONNECT:
		DNOTICEI("received local segment callback SCI_CB_DISCONNECT for region", reg_id);
		/* XXX: threadsafety! But need to avoid deadlock. */
		SMI_LOCK(&_smi_mis_lock);
		_smi_mis.region[reg_id]->nbr_rmt_cncts--;
		SMI_UNLOCK(&_smi_mis_lock);

		/* XXX Maybe process segment that could not yet be de-allocated? */
		
		break;
    default:
		DNOTICEI("unhandled segment callback reason", reason);
		break;
    }
    
    DSECTLEAVE; return (SCI_CALLBACK_CONTINUE);
}
#endif


/* initialization of some internal data structures*/
smi_error_t _smi_init_sci_subsystem()
{
    DSECTION("_smi_init_sci_subsystem");
#ifndef NO_SISCI   
    /*sci_desc_t fd;*/
    /*sci_error_t sci_error; */
    int i, j, *tmp_ids, max_adptrs = 1;
    /*smi_error_t mpi_error;*/

    DSECTENTRYPOINT;
    
    DNOTICEP("sci_rank[] is at", _smi_sci_rank);
    DNOTICEI ("my SCI ID is", _smi_sci_rank[_smi_my_proc_rank]);

    ALLOCATE (tmp_ids, int *, _smi_nbr_procs*sizeof(int));
    ALLOCATE (_smi_adpt_sci_id, adpt_rank_t *, _smi_nbr_procs*sizeof(adpt_rank_t));

    /* distribute local ranks to all processes */
    _smi_ll_allgather(&_smi_sci_rank[_smi_my_proc_rank], 1, _smi_sci_rank, _smi_my_proc_rank);
    DNOTICEI("local proc rank =",_smi_my_proc_rank);
    for (i = 0 ; i < _smi_nbr_procs; i++) {
		_smi_adpt_sci_id[i].sci_id[0] = _smi_sci_rank[i];
		DNOTICEI("SCI ID of proc",i);
		DNOTICEI("                 =",_smi_sci_rank[i]);
    }

    /* exchange number of adapters and their SCI id's */
    SMI_Query (SMI_Q_SCI_NBRADAPTERS, 0, (void *)&_smi_nbr_adapters[_smi_my_proc_rank]);    
    for (i = 0; i < MAX_ADAPTER; i++) {
		if (i < _smi_nbr_adapters[_smi_my_proc_rank])
			SMI_Query (SMI_Q_SCI_ID, i, (void *)&_smi_adpt_sci_id[_smi_my_proc_rank].sci_id[i]);
		else
			_smi_adpt_sci_id[_smi_my_proc_rank].sci_id[i] = 0;
    }

    _smi_ll_allgather(&_smi_nbr_adapters[_smi_my_proc_rank], 1, _smi_nbr_adapters, _smi_my_proc_rank);

    for (i = 0 ; i < _smi_nbr_procs; i++) {
		if (_smi_nbr_adapters[i] > max_adptrs)
			max_adptrs = _smi_nbr_adapters[i];
    }
    ASSERT_R((max_adptrs <= MAX_ADAPTER), "too many SCI adapters used in one node", SMI_ERR_OTHER);
    DNOTICEI("max. nbr of PCI-SCI adapters in one node:", max_adptrs);
    
    for (i = 1; i < max_adptrs; i++) {
		_smi_ll_allgather(&_smi_adpt_sci_id[_smi_my_proc_rank].sci_id[i], 1, 
						  tmp_ids, _smi_my_proc_rank);
		for (j = 0; j < _smi_nbr_procs; j++)
			_smi_adpt_sci_id[j].sci_id[i] = tmp_ids[j];
    }
    free (tmp_ids);
    
    /* we have already one SCI segment for each node/machine */
    max_sci_id = 1;  
#endif
    DSECTLEAVE; return(SMI_SUCCESS);
}


/*** This functions maps a SCI shared segment with the specified id in the ***/
/*** calling processes address space.                                      ***/
smi_error_t _smi_map_sci_shared_segment(shseg_t* shseg)
{
    DSECTION ("_smi_map_sci_shared_segment");
#ifndef NO_SISCI
    sci_error_t sci_error; 
    volatile void* addr;
    int flags = 0;
    size_t offset = shseg->offset;
    size_t size = shseg->size;
#endif 
    
    SMI_STAT_ENTRY(map_sci_segment);
    DSECTENTRYPOINT;
    
#ifndef NO_SISCI
#ifdef DOLPHIN_SISCI
    if (shseg->flags & SHREG_REGISTER) {
		/* registerd segments consist of already-mapped memory */
		SMI_STAT_EXIT(map_sci_segment);
		DSECTLEAVE; return (SMI_SUCCESS);
    }
#endif 

    /* only map the segment if no delayed connection is desired 
       (or if the segment is local) */
    if ((shseg->flags & SHREG_DELAYED) && (shseg->owner != _smi_my_proc_rank)){
		DNOTICE("Segment is delayed & non-local, not mapping yet");
		SMI_STAT_EXIT(map_sci_segment);
		DSECTLEAVE; return (SMI_SUCCESS);
    }

    /* first, we just map the segment on all nodes without the MAP_FIXED flag. 
       Then we compare the resulting addresses: if they are equal, everything is 
       fine. If they differ, we verify if fixed mapping (= equal adresses) is
       required for this segment. If it is, we return an error, if not, it's fine. */
    DNOTICEP("Trying to map segment  w/O MAP_FIXED to addr", shseg->address);
    DNOTICEI(" size of mapping:  ", size);
    DNOTICEI(" offset of mapping:", offset);
    if (_smi_my_proc_rank == shseg->owner) {
		DNOTICE("Mapping as local segment");
		addr = rs_SCIMapLocalSegment(shseg->localseg, &shseg->map, offset, size,
									 0, flags, &sci_error);
    } else {
		DNOTICE("Mapping as remote segment");
		addr = rs_SCIMapRemoteSegment(shseg->segment, &shseg->map, offset, size, 
									  0, flags, &sci_error);
    }
    RETURN_IF_FAIL ("SCIMap...Segment() failed",sci_error,SMI_ERR_MAPFAILED);
    DNOTICEP("got adress:",addr);

    /* make sure we map to the desired address */
    if (!(shseg->flags & SHREG_NONFIXED) && (shseg->address != addr)) {
		rs_SCIUnmapSegment(shseg->map, 0, &sci_error);
		RETURN_IF_FAIL("SCIUnmapSegment() failed with error",sci_error,SMI_ERR_MAPFAILED);
		
		flags = SCI_FLAG_FIXED_MAP_ADDR;
#ifdef WIN32 
		VirtualFree(shseg->address, 0, MEM_RELEASE);
#endif
		DNOTICEP("Trying to map segment to addr with MAP_FIXED", shseg->address);
		if (_smi_my_proc_rank == shseg->owner) {
			DNOTICE("Mapping as local segment");
			addr = rs_SCIMapLocalSegment(shseg->localseg, &shseg->map, offset, size,
										 shseg->address, flags, &sci_error);
		} else {
			DNOTICE("Mapping as remote segment");
			addr = rs_SCIMapRemoteSegment(shseg->segment, &shseg->map, offset, size, 
										  shseg->address, flags, &sci_error);
		}
		RETURN_IF_FAIL ("SCIMap...Segment() failed",sci_error,SMI_ERR_MAPFAILED);
	
#ifdef WIN32
		/* NT has problems with the fixed mapping - workaround here until it is done in SISCI */
		if (!(shseg->flags & SHREG_NONFIXED) && (_smi_ll_all_equal_pointer(addr))) {
			DWARNING("Patching segmentadress (SISCI_API does not support FIXMAP correctly)");
			DNOTICEP("   old address:",shseg->address);
			DNOTICEP("   new address:",addr);
			shseg->address = (char *) addr;
		}
#endif 
		if (shseg->address != addr) {
			DPROBLEM ("could not map at requested address:");
			DPROBLEMP("   requested address:", shseg->address);
			DPROBLEMP("   returned address :", addr);
	    
			DSECTLEAVE; SMI_STAT_EXIT(map_sci_segment);
			return(SMI_ERR_PARAM);
		}
    }
#ifdef WIN32
    else 
		VirtualFree(shseg->address, 0, MEM_RELEASE);
#endif
    shseg->address = (char *)addr;
    shseg->flags &= ~SHREG_DELAYED;

    DNOTICEP("Mapped segment to address", shseg->address);
    DSECTLEAVE; SMI_STAT_EXIT(map_sci_segment);
    return(SMI_SUCCESS);
#else
    DSECTLEAVE; SMI_STAT_EXIT(map_sci_segment);
    return(SMI_ERR_NODEVICE);
#endif
}


/* release the mapping of the specified segment. Afterwards it is no longer 
   mapped into the address space of the calling process. */
smi_error_t _smi_unmap_sci_shared_segment(shseg_t *shseg)
{
    DSECTION("_smi_unmap_sci_shared_segment");
	smi_error_t retval = SMI_SUCCESS;

#ifndef NO_SISCI
    sci_error_t sci_error; 
    
    DSECTENTRYPOINT;

    if (shseg->flags & SHREG_REGISTER) {
		DNOTICE ("Segment is registered memory, unmapping not possible");
		DSECTLEAVE; return (SMI_SUCCESS);
    }

    rs_SCIUnmapSegment(shseg->map, 0, &sci_error);
	if (sci_error != SCI_ERR_OK) {
		DERRORP("SCIUnmapSegment() failed with error",sci_error);
		retval = SMI_ERR_MAPFAILED;
	}    
#endif
	
    DSECTLEAVE; return(retval);
}

  
/* create and export an SCI  shared segment of the specified size on the local machine. */
static smi_error_t _smi_export_sci_shared_segment(shseg_t *shseg)
{
    DSECTION("_smi_export_sci_shared_segment");
#ifndef NO_SISCI
    sci_error_t sci_error = SCI_ERR_OK; 
	sci_cb_local_segment_t local_cb_fcn = NULL;
	void *cb_arg  = NULL;
    smi_error_t error;
    /*int i, alignment;*/
    unsigned int sisci_flags = 0;
#endif 

    SMI_STAT_ENTRY(export_sci_segment);
    DSECTENTRYPOINT;

#ifndef NO_SISCI
    if (shseg->fd == NULL) {
		DPROBLEM ("Invalid SCI file descriptor");
		DSECTLEAVE; return (SMI_ERR_NODEVICE);
    }

    error = SMI_Query(SMI_Q_SCI_ID,shseg->adapter,&shseg->sci_id);
    ASSERT_R((error==SMI_SUCCESS),"could not get sci-id for adapter",SMI_ERR_OTHER);
   
    DNOTICEI("using local adapter",shseg->adapter);
    DNOTICEI("SCI id of local adapter is",shseg->sci_id);

#ifdef DOLPHIN_SISCI
    /* try to create segments until an unused id is found */	
	if (shseg->flags & SHREG_PRIVATE) 
		sisci_flags |= SCI_FLAG_PRIVATE;
	if (shseg->flags & SHREG_REGISTER) 
		sisci_flags |= SCI_FLAG_EMPTY;
#if USE_SEGMENT_CALLBACK
	local_cb_fcn = _smi_local_sgmt_callback;
	cb_arg = &shseg->region_id;
	sisci_flags |= SCI_FLAG_USE_CALLBACK;
#endif

    SMI_STAT_CALL(sci_create);
    do {
		shseg->id = ++max_sci_id;
		rs_SCICreateSegment(shseg->fd, &shseg->localseg, shseg->id, 
							(unsigned int) (shseg->size + SEGSIZE_ALIGNMENT), local_cb_fcn, cb_arg, sisci_flags, &sci_error);
    } while ((sci_error == SCI_ERR_SEGMENTID_USED) && (max_sci_id < INT_MAX));
    SMI_STAT_RETURN(sci_create);
    RETURN_IF_FAIL ("SCICreateSegment() failed",sci_error,SMI_ERR_NOSEGMENT);

    /* register local memory if desired */
    if (shseg->flags & SHREG_REGISTER) {
		DNOTICE("Registering local segment memory");
		SCIRegisterSegmentMemory(shseg->address, (unsigned int) shseg->size, shseg->localseg, 0, &sci_error);
		RETURN_IF_FAIL ("SCIRegisterSegmentMemory() failed",sci_error,SMI_ERR_NOSEGMENT);
    }

    /* PrepareSegment() is always necessary */
    SCIPrepareSegment(shseg->localseg, shseg->adapter, 0,&sci_error);
    if (sci_error != SCI_ERR_OK) {
		rs_SCIRemoveSegment(shseg->localseg, 0, &sci_error);
		DPROBLEMP ("SCIPrepareSegment() failed, SISCI error", sci_error);
		return SMI_ERR_NOSEGMENT;
    }

    /* Export the segment if it is not private. */
    if (!(shseg->flags & SHREG_PRIVATE)) {
		SCISetSegmentAvailable (shseg->localseg, shseg->adapter, 0,&sci_error);
		if (sci_error != SCI_ERR_OK) {
			rs_SCIRemoveSegment(shseg->localseg, 0, &sci_error);
			DPROBLEMP ("SCISetSegmentAvailable failed, SISCI error", sci_error);
			return SMI_ERR_NOSEGMENT;
		}
    }
#else
    /* Scali SISCI has, due to the underlying ScaSCI, another semantics for the SCI 
       segment IDs which makes this work-around necessary */

    /* registering of local memory is not supported in Scali SISCI */
    if (shseg->flags & SHREG_REGISTER) {
		sci_error =  SCI_ERR_NOT_IMPLEMENTED;
		RETURN_IF_FAIL ("SCIRegisterSegmentMemory() not implemented",sci_error,SMI_ERR_NOTIMPL);
    }

    /* try to create *and export* (work-around!) segments until an unused id is found */	
    if (shseg->flags & SHREG_PRIVATE) 
		sisci_flags |= SCI_FLAG_PRIVATE;
    SMI_STAT_CALL(sci_create);
    do {
		shseg->id = ++max_sci_id;
		rs_SCICreateSegment(shseg->fd, &shseg->localseg, shseg->id, 
							shseg->size + SEGSIZE_ALIGNMENT, NULL, NULL, sisci_flags, &sci_error);
      
		if (sci_error == SCI_ERR_OK) {
			/* PrepareSegment() is always necessary */
			SCIPrepareSegment(shseg->localseg, shseg->adapter, 0, &sci_error);
			RETURN_IF_FAIL ("SCIPrepareSegment() failed",sci_error,SMI_ERR_NOSEGMENT);

			if (!(shseg->flags & SHREG_PRIVATE)) {
				SCISetSegmentAvailable(shseg->localseg, shseg->adapter, 0, &sci_error);
			}
			if (sci_error != SCI_ERR_OK) {
				rs_SCIRemoveSegment(shseg->localseg, 0, &sci_error);
				/* we need to loop on... */
				sci_error = SCI_ERR_SEGMENTID_USED;
			}
		}
    } while ((sci_error != SCI_ERR_OK) && (max_sci_id < INT_MAX));
    SMI_STAT_RETURN(sci_create);

    RETURN_IF_FAIL ("SCICreateSegment() failed",sci_error,SMI_ERR_NOSEGMENT);
#endif

    nbr_locsgmts++;
    size_locsgmts += shseg->size + SEGSIZE_ALIGNMENT;
    DNOTICEI("new SCI segment set available, SCI ID", shseg->id);
    DNOTICEI("  total number of local segments:", nbr_locsgmts);
    DNOTICEI("  total size of local segments  :", size_locsgmts);

    SMI_STAT_EXIT(export_sci_segment);
    DSECTLEAVE; return SMI_SUCCESS;
#else
    SMI_STAT_EXIT(export_sci_segment);
    DSECTLEAVE; return SMI_ERR_NODEVICE;
#endif
}
  
#define CONNECT_SGMT_DELAY  100000

/* connect to ("import") a remote SCI shared segment */
static smi_error_t _smi_import_sci_shared_segment(shseg_t* shseg)
{
    DSECTION("_smi_import_sci_shared_segment");
#ifndef NO_SISCI
    sci_error_t sci_error; 
	sci_cb_remote_segment_t rmt_cb_fcn = NULL;
	void *cb_arg  = NULL;
	int   cb_flag = 0;
	/*int i, alignment;*/
    smi_error_t error;
    /*int* error_array;*/
#endif 
    
    SMI_STAT_ENTRY(import_sci_segment);
    DSECTENTRYPOINT;
    
#ifndef NO_SISCI
    if (shseg->fd == NULL) {
		DPROBLEM ("Invalid SCI device descriptor");
		DSECTLEAVE; return (SMI_ERR_NODEVICE);
    }

    SMI_STAT_CALL(sci_connect);
    if (!(shseg->flags & SHREG_DELAYED)) {
		DNOTICEI("trying to connect to new SCI segment", shseg->id);
		DNOTICEI("remote SCI id is:",shseg->sci_id);
		DNOTICEI("using local adapter:",shseg->adapter);

#if USE_SEGMENT_CALLBACK
		rmt_cb_fcn = _smi_rmt_sgmt_callback;
		cb_arg = &shseg->region_id;
		cb_flag = SCI_FLAG_USE_CALLBACK;
#endif
		rs_SCIConnectSegment(shseg->fd, &shseg->segment, shseg->sci_id, shseg->id, 
							 shseg->adapter, rmt_cb_fcn, cb_arg, CONNECT_SGMT_DELAY, 
							 shseg->connect_flag|cb_flag, &sci_error);
		while ((sci_error != SCI_ERR_OK) && (sci_error != SCI_ERR_API_NOSPC) 
			   && (sci_error != SCI_ERR_NOSPC) && (sci_error != SCI_ERR_NO_SUCH_SEGMENT)) {
			/* we do a small delay here to avoid overload of the exporting node */
			usleep (500 + _smi_my_proc_rank*300);
			rs_SCIConnectSegment(shseg->fd, &shseg->segment, shseg->sci_id, shseg->id, 
								 shseg->adapter, rmt_cb_fcn, cb_arg, CONNECT_SGMT_DELAY, 
								 shseg->connect_flag|cb_flag, &sci_error);
		}
		if (sci_error != SCI_ERR_OK) {
			DPROBLEMP ("SCIConnectSegment() failed with error", sci_error);
			DERROR ("SCIConnectSegment() failed.");
			error = SMI_ERR_NOSEGMENT;
		} else {
			unsigned int real_sgmt_size;
			/* the user requests a connect to a segment of a size he doesn't know */
			real_sgmt_size = SCIGetRemoteSegmentSize(shseg->segment);
			if (shseg->size == 0) 
				shseg->size = real_sgmt_size; 
			DNOTICEI ("connected, size of remote SCI segment =", real_sgmt_size);
	    
			error = SMI_SUCCESS;	
		}

		if (error == SMI_SUCCESS) {
			nbr_rmtsgmts++;
			size_rmtsgmts += shseg->size;
			DNOTICE ("connected to new SCI segment");
			DNOTICEI("  total number of remote segments:", nbr_rmtsgmts);
			DNOTICEI("  total size of remote segments  :", size_rmtsgmts);
		}
    } else {
		DNOTICE ("Delayed connection desired - did not connect yet");
		error = SMI_SUCCESS;
    }

	/* Save the sequence which needs to be used to check for tranfer errors of accesses 
	   towards this segment. */
	/* XXX This is only relevant if the adapter is specified on a per-segment base - not,
	   if SMI_Set_adapter() is used. The per-segment assignment is not yet supported. */
    shseg->node_sequence = &_smi_node_sequence[_smi_nbr_machines*shseg->adapter + shseg->machine];

    SMI_STAT_RETURN(sci_connect);
    SMI_STAT_EXIT(import_sci_segment);
    DSECTLEAVE; return(error);
#else
    SMI_STAT_EXIT(import_sci_segment);
    DSECTLEAVE; return(SMI_ERR_NODEVICE);
#endif
}


/* "wrapper" for _smi_[im|ex]port_sci_shared_segment */
smi_error_t _smi_create_sci_shared_segment(shseg_t* shseg)
{
	DSECTION("_smi_create_sci_shared_segment");
#ifndef NO_SISCI
	smi_error_t error;
	sci_error_t sci_error;
	smi_sgmt_locator_t sgmt_locator;

	if (shseg->owner == _smi_my_proc_rank) { 
		_smi_get_loc_scidesc(&shseg->smifd, &sci_error); 
		shseg->fd = (sci_error == SCI_ERR_OK) ?
			_smi_trans_scidesc(&shseg->smifd) : NULL;
    
		DNOTICEP("SCI device descriptor is", shseg->fd);	

		if (!(shseg->flags & SHREG_ASYNC)) {
			if ( !_smi_ll_all_true( shseg->fd != NULL) ) {
				DPROBLEM("no more sci-decriptors available");
				DSECTLEAVE return(SMI_ERR_NOSEGMENT);
			}
		}
		else {
			if ( ( shseg->fd == NULL) ) {
				DPROBLEM("no more sci-decriptors available");
				DSECTLEAVE return(SMI_ERR_NOSEGMENT);
			}
		}	    

		/* create & export the segment */
		error = _smi_export_sci_shared_segment(shseg);
		if (error != SMI_SUCCESS) {
			if (shseg->fd != NULL)
				_smi_free_loc_scidesc(&shseg->smifd, &sci_error); 
			shseg->id = 0;
		}
      
		/* required data for the other processes to connect to the new segment */
		sgmt_locator.sci_id = shseg->sci_id;
		sgmt_locator.segment_id = shseg->id;
      
		/* is the distribution of the segment id required ? */
		if (!(shseg->flags & SHREG_ASYNC)) {
			if (shseg->flags & SHREG_PT2PT) {
				SMI_Send (&sgmt_locator, sizeof(sgmt_locator), shseg->partner);
			} else {
				_smi_ll_bcast((int *)&sgmt_locator, 2, shseg->owner, _smi_my_proc_rank);
			}
		}
    } else {
		_smi_get_rmt_scidesc(&shseg->smifd, &sci_error); 
		shseg->fd = (sci_error == SCI_ERR_OK) ?
			_smi_trans_scidesc(&shseg->smifd) : NULL;
		DNOTICEP("SCI device descriptor is", shseg->fd);		
	
		if (!(shseg->flags & SHREG_ASYNC)) {
			if ( !_smi_ll_all_true( shseg->fd != NULL) ) {
				DPROBLEM("no more sci-decriptors available");
				DSECTLEAVE return(SMI_ERR_NOSEGMENT);
			}
		}
		else {
			if ( ( shseg->fd == NULL) ) {
				DPROBLEM("no more sci-decriptors available");
				DSECTLEAVE return(SMI_ERR_NOSEGMENT);
			}
		}	
	
		/* import segment - global or pt2pt segment?  */
      
		/* is the distribution of the segment id required ? */
		if (!(shseg->flags & SHREG_ASYNC)) {
			if (shseg->flags & SHREG_PT2PT) {
				SMI_Recv (&sgmt_locator, sizeof(sgmt_locator), shseg->owner);
			} else {
				_smi_ll_bcast((int *)&sgmt_locator, 2, shseg->owner, _smi_my_proc_rank);
			}
	    
			/* required data for the other processes to connect to the new segment */
			shseg->sci_id = sgmt_locator.sci_id;
			shseg->id = sgmt_locator.segment_id;	
		}
	
		if (shseg->id != 0)
			error = _smi_import_sci_shared_segment(shseg);
		else {
			error = SMI_ERR_NOSEGMENT;
		}
	
		if ((error != SMI_SUCCESS) && (shseg->fd != NULL))
			_smi_free_rmt_scidesc(&shseg->smifd, &sci_error);
    }
  
	DSECTLEAVE; return (error);
#else
	DSECTLEAVE; return(SMI_ERR_NODEVICE);
#endif 
}


/* removes the SCI shared segment with the specified identifier */
smi_error_t _smi_remove_sci_shared_segment(shseg_t* shseg)
{
    DSECTION("_smi_remove_sci_shared_segment");
#ifndef NO_SISCI
    sci_error_t sci_error; 
    smi_error_t ret = SMI_SUCCESS;

    if (!(shseg->flags & SHREG_DELAYED) && (_smi_my_proc_rank != shseg->owner)) {
		rs_SCIDisconnectSegment(shseg->segment, 0, &sci_error);
		if (sci_error != SCI_ERR_OK) {
			DPROBLEMI("could not disconnect from remote SCI segment wit ID", shseg->id);
			DPROBLEMP("SCIDisconnectSegment failed with SISCI error", sci_error);
			ret = SMI_ERR_OTHER;
		} else {
			nbr_rmtsgmts--;
			size_rmtsgmts -= shseg->size;
			DNOTICEI("disconnected from SCI segment with SCI ID",shseg->id);
			DNOTICEI("  total number of remote segments:", nbr_rmtsgmts);
			DNOTICEI("  total size of remote segments  :", size_rmtsgmts);
			_smi_free_rmt_scidesc(&shseg->smifd, &sci_error);	
		}
    }

#if 0
    if (!(shseg->flags & (SHREG_PRIVATE|SHREG_ASYNC|SHREG_PT2PT)))
		_smi_ll_barrier();
#endif

    if (_smi_my_proc_rank == shseg->owner) {
		int busy_retries = 0;

		DNOTICE("notifying connected nodes");
		SCISetSegmentUnavailable(shseg->localseg, shseg->adapter, SCI_FLAG_NOTIFY, &sci_error);
		if (sci_error != SCI_ERR_OK) {
			DNOTICEP("sci error:", sci_error);
			DWARNING("notifycation of remote node failed");
		}

		/* We need to loop until all remote connections are removed from this segment. */		
		do {
			rs_SCIRemoveSegment(shseg->localseg, 0, &sci_error);
			if (sci_error != SCI_ERR_OK) {
				DPROBLEMP ("SCIRemoveSegment returned error ", sci_error);
				busy_retries++;
				usleep (BUSY_RETRY_DELAY);
			}
		} while (sci_error == SCI_ERR_BUSY && busy_retries < MAX_BUSY_RETRY);
		if (sci_error != SCI_ERR_OK) {
			DPROBLEMI("could not remove local SCI segment wit ID", shseg->id);
			DPROBLEMP("SCIRemoveSegment() failed with SISCI error",sci_error);
			ret = (sci_error == SCI_ERR_BUSY) ? SMI_ERR_BUSY : SMI_ERR_OTHER;
			if (ret == SMI_ERR_BUSY) {
				DNOTICE ("-> segment was scheduled for later removal");
			}
		} else {
			nbr_locsgmts--;
			size_locsgmts -= shseg->size + SEGSIZE_ALIGNMENT;
			DNOTICEI("removed local SCI segment wit ID", shseg->id);
			DNOTICEI("  total number of local segments:", nbr_locsgmts);
			DNOTICEI("  total size of local segments  :", size_locsgmts);
			_smi_free_loc_scidesc(&shseg->smifd, &sci_error);
		}
    }
    
    DSECTLEAVE; return(ret);
#else
    DSECTLEAVE; return(SMI_ERR_NODEVICE);
#endif
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
