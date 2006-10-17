/* $Id$ */

/* create the internal SMI regions in a faster way than the usual 
   SMI_Create_Shreg() */

#include <unistd.h>

#define TEST_LOCAL_SYNC 0

#include "internal_regions.h"
#include "env/general_definitions.h"
#include "env/smidebug.h"
#include "message_passing/lowlevelmp.h"
#include "utility/query.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* use this to enable shutdown if a node is disconnected */
#define SMI_USE_SEGMENT_CALLBACK 0

#if SMI_USE_SEGMENT_CALLBACK
#define CB_FCN   internal_remote_callback
#define CB_FLAG  SCI_FLAG_USE_CALLBACK

static sci_callback_action_t 
internal_remote_callback(void* arg, 
		sci_remote_segment_t segment,
		sci_segment_cb_reason_t reason,
		sci_error_t error)
{
    DSECTION("internal_remote_callback");
    DSECTENTRYPOINT;
    
    switch (reason) {
    case SCI_CB_DISCONNECT:
	DPROBLEM("Abort due to disconnection of an internal segment");
	_smi_wd_request_shutdown(-1);
	break;
    default:
	DNOTICEI("received a callback which is not handled here:", reason);
	break;
    }

    DSECTLEAVE return(SCI_CALLBACK_CONTINUE);
}
#else
#define CB_FCN   NULL
#define CB_FLAG  0
#endif

#if TEST_LOCAL_SYNC
smi_error_t _smi_create_local_region(unsigned int int_segsize)
{
    char *addr;
    smi_error_t error; 
    smi_region_info_t int_region;
    
    DSECTION("_smi_create_local_region");
    DSECTENTRYPOINT;
        
    DNOTICEI("Creating local shared memory segment on machine",_smi_my_machine_rank);
    int_region.size   = int_segsize;
    int_region.owner  = _smi_first_proc_on_node(_smi_my_machine_rank);
    int_region.offset = 0;
    int_region.adapter= 0;
    
    error = SMI_Create_shreg(SMI_SHM_SMP, &int_region, &_smi_int_smp_shreg_id, &addr);    
    ASSERT_X(_smi_ll_all_true(error == SMI_SUCCESS),
	     "Could not allocate local shared memory segment",error);
    
    /* This is not a user-allocated region. */
    SMI_LOCK(&_smi_mis_lock);
    _smi_mis.nbr_user_regions--;
    SMI_UNLOCK(&_smi_mis_lock);

    error = SMI_Init_shregMMU(_smi_int_smp_shreg_id|INTERNAL);
    ASSERT_X(_smi_ll_all_true(error==SMI_SUCCESS),
	     "Could not init memory manager for local segment",error);
    
    DSECTLEAVE; 
    return SMI_SUCCESS;
}
#endif

static smi_error_t _smi_create_internal_region_smp(unsigned int int_segsize)
{
    void *addr;
    smi_error_t error;
    smi_region_info_t int_region;

    DSECTION("_smi_create_internal_region_smp");
    DSECTENTRYPOINT;

	
    DNOTICE("Creating internal shared memory segment");
    int_region.size   = int_segsize;
    int_region.owner  = _smi_first_proc_on_node(_smi_my_machine_rank);
    int_region.offset = 0;
    int_region.adapter= 0;
	
    error = SMI_Create_shreg(SMI_SHM_SMP|SMI_SHM_NONFIXED, &int_region, &_smi_int_shreg_id[0], &addr);
    ASSERT_X(_smi_ll_all_true(error == SMI_SUCCESS),
	     "Could not allocate internal shared memory segment",error);
    
    /* This is not a user-allocated region. */
    SMI_LOCK(&_smi_mis_lock);
    _smi_mis.nbr_user_regions--;
    SMI_UNLOCK(&_smi_mis_lock);

    error = SMI_Init_shregMMU(_smi_int_shreg_id[0]|INTERNAL);
    ASSERT_X(_smi_ll_all_true(error==SMI_SUCCESS),
	     "Could not init memory manager for internal segment",error);
    
#if TEST_LOCAL_SYNC
     /* additionally allocate an unix shared memory segment, for node-local operations */
    _smi_create_local_region(int_segsize);
#endif 

    DSECTLEAVE; return SMI_SUCCESS;
}

#ifndef NO_SISCI
/* shseg structure and sgmt_id to start trying with */
static smi_error_t _smi_create_local_segment_internal(shseg_t* shseg, int sgmt_id)
{
    DSECTION("_smi_create_local_segment_internal");
    sci_error_t sci_error; 

    DSECTENTRYPOINT;

    _smi_get_loc_scidesc(&shseg->smifd, &sci_error);
    EXIT_IF_FAIL("Could not get device descriptor for local SCI resource", sci_error, SMI_ERR_NODEVICE);
    shseg->fd = _smi_trans_scidesc(&(shseg->smifd));	
    
#ifdef DOLPHIN_SISCI
    do {
	shseg->id = sgmt_id++;
	rs_SCICreateSegment(shseg->fd, &shseg->localseg, shseg->id, 
			    shseg->size + SEGSIZE_ALIGNMENT, 0, 0, 0, &sci_error);
    } while (sci_error == SCI_ERR_SEGMENTID_USED && sgmt_id < INT_MAX);
    EXIT_IF_FAIL("Not enough SCI resources available: SCICreateSegment() failed ",
		 sci_error, 2000+sci_error);
    
    SCIPrepareSegment(shseg->localseg, _smi_DefAdapterNbr, 0,&sci_error);
    SCISetSegmentAvailable(shseg->localseg, _smi_DefAdapterNbr, 0,&sci_error);
#else
    /* Scali has diffent semantic for segment IDs */
    do {
	shseg->id = sgmt_id++;
	DNOTICEI("Trying to create segment with sgmt_id", shseg->id);
	rs_SCICreateSegment(shseg->fd, &(shseg->localseg), shseg->id, 
			    shseg->size + SEGSIZE_ALIGNMENT, 0, 0, 0, &sci_error);
	if (sci_error == SCI_ERR_OK) {
	    DNOTICEI("Preparing segment, binding to adapter", _smi_DefAdapterNbr);
	    SCIPrepareSegment(shseg->localseg, _smi_DefAdapterNbr, 0,&sci_error);
	    if (sci_error == SCI_ERR_OK) {
		DNOTICE("Setting segment available");
		SCISetSegmentAvailable(shseg->localseg, _smi_DefAdapterNbr, 0, &sci_error);
	    }
	    if (sci_error != SCI_ERR_OK) {
		DNOTICEP ("SetSegmentAvailable() failed (id not free?), SISCI error", sci_error);
		rs_SCIRemoveSegment(shseg->localseg, 0, &sci_error);
		sci_error = SCI_ERR_SEGMENTID_USED;
	    }
	}
    } while ((sci_error != SCI_ERR_OK) && (sgmt_id < INT_MAX));
    EXIT_IF_FAIL("Not enough SCI resources available: SCICreateSegment() failed",
		 sci_error,2000+sci_error);	
#endif
    shseg->address = (char *)rs_SCIMapLocalSegment(shseg->localseg, &(shseg->map), 0, 
						   shseg->size, 0, 0, &sci_error);
    DNOTICEP("Mapped Local Segment at",shseg->address);
    EXIT_IF_FAIL ("Insufficient address space available: SCIMapLocalSegment() failed ",
		  sci_error,2000+sci_error);
  
    DSECTLEAVE
    return SMI_SUCCESS;
}
#endif

static smi_error_t _smi_create_internal_regions_sci(unsigned int int_segsize)
{
    smi_error_t error = SMI_SUCCESS;
#ifndef NO_SISCI
    region_t **int_regions;
    shseg_t *shseg;
    sci_sequence_status_t seq_state;
    sci_error_t sci_error; 
    unsigned int *sgmt_sci_ids;
    int sgmt_id = 1, node, adpt, off;
    
    DSECTION("_smi_create_internal_regions_sci");
    DSECTENTRYPOINT;

    /* When using multiple adapters, we need to create a sequence on each of them
       to check transfer which have gone via either adapter. */
    ALLOCATE(sgmt_sci_ids, unsigned int *, sizeof(unsigned int) * _smi_nbr_procs);
    ALLOCATE(int_regions, region_t **, sizeof (region_t *) * _smi_nbr_machines * 
	     _smi_nbr_adapters[_smi_my_proc_rank]);
    for (adpt = 0; adpt < _smi_nbr_adapters[_smi_my_proc_rank]; adpt++) {
	off = adpt * _smi_nbr_machines;
	for (node = 0; node < _smi_nbr_machines; node++) {
	    _smi_prepare_for_new_region (&_smi_int_shreg_id[node+off], &int_regions[node+off]);
	    
	    ALLOCATE(int_regions[node+off]->addresses, char **, sizeof(char *));
	    int_regions[node+off]->addresses[0] = NULL;  
	    int_regions[node+off]->size         = int_segsize;
	    int_regions[node+off]->counterpart_id = -1;
	    int_regions[node+off]->replication    = false;
	    int_regions[node+off]->no_segments    = 1;
	    int_regions[node+off]->type           = SMI_SHM_FRAGMENTED;
	    int_regions[node+off]->collective     = 1;
	    
	    ALLOCATE (int_regions[node+off]->seg, shseg_t**, sizeof(shseg_t*));
	    ALLOCATE (int_regions[node+off]->seg[0], shseg_t*, sizeof(shseg_t));
	    /* Only the internal region mapped via the default adapter is used effectively;
	       the other mapping are only done to attach a sequence to them for transfer-checking. Therefore,
	       the size of these mappings is minimized to save ATT entries. */
	    int_regions[node+off]->seg[0]->size    = (adpt == _smi_DefAdapterNbr) ? int_segsize : _smi_page_size;
	    int_regions[node+off]->seg[0]->offset  = 0;
	    int_regions[node+off]->seg[0]->machine = node;
	    int_regions[node+off]->seg[0]->device  = 0;
	    int_regions[node+off]->seg[0]->adapter = adpt;
	    int_regions[node+off]->seg[0]->owner   = _smi_first_proc_on_node(node);
	    int_regions[node+off]->seg[0]->flags   = SHREG_NONFIXED;
	}
    }
    
    DNOTICE("Creating Remote/Internal segments");
    DTIMESTAMP("Creating Remote/Internal segments");
    
    for (node = 0; node < _smi_nbr_machines; node++) {
	/* create local segment if required */
	DTIMESTAMP("create local segment if required");
	if (_smi_first_proc_on_node(node) == _smi_my_proc_rank) {
	    DNOTICE ("Creating local internal segment");
	    shseg = int_regions[_smi_my_machine_rank]->seg[0];
	    _smi_create_local_segment_internal(shseg, sgmt_id);
	    int_regions[_smi_my_machine_rank]->addresses[0] = shseg->address;
	    sgmt_id = shseg->id;
	    _smi_ll_bcast(&sgmt_id, 1, _smi_first_proc_on_node(node),_smi_my_proc_rank);
	    /* this may be not fully EMT64 complient */
	    _smi_ll_bcast((int*)&shseg->address, sizeof (char*) / sizeof(int), _smi_first_proc_on_node(node),_smi_my_proc_rank);
	}
	else {
	    char* address_to_be_used_for_map;
	    _smi_ll_bcast(&sgmt_sci_ids[_smi_first_proc_on_node(node)], 1, _smi_first_proc_on_node(node),  _smi_my_proc_rank);
	    _smi_ll_bcast((int*)&address_to_be_used_for_map, 1, _smi_first_proc_on_node(node),  _smi_my_proc_rank);
	    for (adpt = 0; adpt < _smi_nbr_adapters[_smi_my_proc_rank]; adpt++) {
		DNOTICEI("checking availability of adapter:", adpt);
		if (_smi_adapter_available(adpt)) {
		    off = adpt * _smi_nbr_machines;
		    DNOTICEI("connecting via adapter:", adpt);	    
		    DNOTICEI("Processing Node",node);
		    shseg = int_regions[node+off]->seg[0];
		    shseg->id = sgmt_sci_ids[_smi_first_proc_on_node(node)];
		    
		    /* XXX workaround for problem with SCI driver: 
		       Do not connect to local internal region via the addtional adapters. */
		    if (adpt != _smi_DefAdapterNbr && node == _smi_my_machine_rank) {
			_smi_node_sequence[_smi_machine_rank[shseg->owner]+off] = NULL;
			_smi_error_seq_initialized[_smi_machine_rank[shseg->owner]+off] = FALSE;
			
			continue;
		    }
		    
		    _smi_get_rmt_scidesc(&shseg->smifd, &sci_error);
		    EXIT_IF_FAIL("Could not get device descriptor for remote SCI resource", 
				 sci_error, SMI_ERR_NODEVICE);
		    shseg->fd = _smi_trans_scidesc(&shseg->smifd);
		    
		    rs_SCIConnectSegment(shseg->fd, &shseg->segment,_smi_sci_rank[shseg->owner], shseg->id, 
					 adpt, CB_FCN, NULL, SCI_INFINITE_TIMEOUT, CB_FLAG, &sci_error);
		    while (sci_error != SCI_ERR_OK && sci_error != SCI_ERR_API_NOSPC) {
			/* we do a small delay here to avoid overload of the exporting node */
			usleep (500 + _smi_my_proc_rank*300);
			rs_SCIConnectSegment(shseg->fd, &shseg->segment,_smi_sci_rank[shseg->owner], shseg->id, 
					     adpt, CB_FCN, NULL, SCI_INFINITE_TIMEOUT, CB_FLAG, &sci_error);
		    }
		    EXIT_IF_FAIL ("Insufficient SCI resources available: SCIConnectSegment() failed",
				  sci_error, 2000+sci_error);
		    
		    shseg->address = (char *)rs_SCIMapRemoteSegment(shseg->segment, &shseg->map, 0, 
								    shseg->size, address_to_be_used_for_map, 0, &sci_error);
		    
		    DNOTICEP("Mapped Remote Segment at",shseg->address);
		    EXIT_IF_FAIL ("Insufficient address space available: SCIMapRemoteSegment() failed",
				  sci_error,2000+sci_error);
		    
		    if (shseg->address != address_to_be_used_for_map) {
			DWARNING("Address for segment is not equal in all processes");
		    }
		    
		    int_regions[node+off]->addresses[0] = shseg->address;
		    
		    /* bind an SCI sequences for the transfer checks */
		    rs_SCICreateMapSequence(shseg->map, &_smi_node_sequence[_smi_machine_rank[shseg->owner]+off], 
					    0, &sci_error);
		    EXIT_IF_FAIL ("Unknown reason: SCICreateMapSequence() failed",sci_error,2000+sci_error);
		    do {
			seq_state = SCIStartSequence(_smi_node_sequence[_smi_machine_rank[shseg->owner]+off],
						     0 , &sci_error);
		    } while (seq_state != SCI_SEQ_OK && seq_state != SCI_SEQ_NOT_RETRIABLE);
		    if (seq_state == SCI_SEQ_NOT_RETRIABLE) {
			/* something has gone wrong with this session! */
			DERROR("SCI session terminated - remote process crashed ?");
			SMI_Abort(-1);
		    }
		    _smi_error_seq_initialized[node+off] = TRUE;
		    SMI_Check_transfer_proc(shseg->owner, 0);
		}
	    }
	}
    }
	
    /* Init MMU for each internal region mapped via the default adapter. */
    DNOTICE("Initializing MMU for internal regions");
    DTIMESTAMP("Initializing MMU for internal regions");
    for (node = 0; node < _smi_nbr_machines; node++) {
	error = SMI_Init_shregMMU(_smi_int_shreg_id[node]|INTERNAL);
    }
    
#if TEST_LOCAL_SYNC    
    /* additionally allocate an unix shared memory segment, for node-local operations */
    _smi_create_local_region(int_segsize);
#endif     
    free(sgmt_sci_ids);
    free(int_regions);
    
    DSECTLEAVE;
#endif
    return error;
}


/* create the internal regions on startup (one for each node)

   This was done with normal SMI_Create_shreg(UNDIVIDED) calls, but these calls
   are very slow for a big number of nodes. For SCI, we use this custom function 
   instead. */
smi_error_t _smi_create_internal_regions(unsigned int int_segsize)
{
    smi_error_t ret;

    if (int_segsize % _smi_page_size != 0) 
	int_segsize = (int_segsize / _smi_page_size + 1) * _smi_page_size;

    ALLOCATE (_smi_int_shreg_id, int *, 
	      _smi_nbr_machines*sizeof(int)*_smi_nbr_adapters[_smi_my_proc_rank]);

    /* for pure SMP mode, do it the old style */
    ret = (_smi_all_on_one == TRUE) ?
	_smi_create_internal_region_smp(int_segsize) :
	_smi_create_internal_regions_sci(int_segsize);

    return ret;
}
	
/* Return the address of the internal memory region of a given node */
void *SMI_Nodemem_address (int node_rank)
{
    DSECTION ("SMI_Nodemem_address");
    void *addr;
    DSECTENTRYPOINT;
    
    if (!_smi_initialized)
	return NULL;
    if (node_rank < 0 || node_rank >= _smi_nbr_machines) {
	DPROBLEM ("Illegal node rank");
	return NULL;
    }	
    
    SMI_LOCK(&_smi_mis_lock);
    addr = _smi_mis.region[_smi_int_shreg_id[node_rank]]->addresses[0];
    SMI_UNLOCK(&_smi_mis_lock);

    DSECTLEAVE; return addr; 
}

/* Allocate/free globally accessible memory, located on this node. */
void *SMI_Nodemem_alloc (size_t bufsize)
{
    void *buf;
    smi_error_t smi_err;
    DSECTION ("SMI_Nodemem_alloc");
    DSECTENTRYPOINT;
    
    if (!_smi_initialized)
	return NULL;
    /* XXX: Limit the amount of memory that can be allocated? */
    
    smi_err = SMI_Imalloc (bufsize, _smi_int_shreg_id[_smi_my_machine_rank], &buf);
    
    DSECTLEAVE; return buf;
}

smi_error_t SMI_Nodemem_free (void *buf)
{
    smi_error_t smi_err;	
    DSECTION ("SMI_Nodemem_free");
    DSECTENTRYPOINT;
    
    smi_err = SMI_Ifree (buf);
    
    DSECTLEAVE; return smi_err;
}
