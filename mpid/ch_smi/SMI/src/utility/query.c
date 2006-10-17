/* $Id$ */

#ifdef SOLARIS
#include <sys/processor.h>
#include <sys/procset.h>
#endif
#include <unistd.h>
#include <stdlib.h>

#ifdef WIN32
#include <wtypes.h>
#include <winbase.h>
#endif

#include "env/general_definitions.h"
#include "regions/region_layout.h"
#include "regions/address_to_region.h"

#include "query.h"
#if (defined LINUX) || (defined DARWIN)
#include "getus.h"
#endif
#include "cpuid.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#define FRQ_BASE CLOCK
#define FRQ_LOOPS 1000000

/* exports */
int _smi_sci_errcnt = 0;

/* internal */
static int nbr_adapters = 0;
static int  streambufsizes[MAX_ADAPTER]; /* size of streambuffer on adapter */
static int* nbr_streambufs = 0;
static int  adaptertypes[MAX_ADAPTER];
static int* sci_ids = 0;
static int local_page_size = 0;
static int wc_enabled = 0;
static int signals_avail = 0;
static int dma_avail = 0;
static int dma_minsize = 0;
static int dma_maxsize = 0;
static int dma_size_alignment = 0;
static int dma_offset_alignment = 0;
static int sci_ta_size = 0;        /* size of biggest SCI packet type supported by adapter */
static int cpu_frq;                /* CPU frequency in MHz*/
static int cpu_cachelinelen;
static int ps_per_tick;            /* duration of CPU tick in ps (derived fromcpu_frq) */
static int nbr_cpus;               /* nbr of CPUs in node */
static int has_valid_id;
static int sci_system_type;
static int phys_sci_dims;
static int phys_sci_dim_extent[MAX_DIMS];
static int app_sci_dims;
static int app_sci_dim_extent[MAX_DIMS];
static int app_sci_dim_offset[MAX_DIMS];
static int sci_topo_type = SMI_SCI_TOPOLOGY_UNKNOWN;

static char api_version[256]="012345678901234567890012345678901234567890";

static int adapter_available[MAX_ADAPTER];

#ifdef NO_SISCI
static unsigned int _smi_APIVersionString(char str[]) 
{
  char SMP_API[] = "SMP, SYS-V shared memory";
  strcpy (str, SMP_API);
  return 1;
}
#else
static unsigned int _smi_APIVersionString(char str[])
{
  
  sci_query_string_t queryString;
  sci_error_t  error;
  
  if (!_smi_all_on_one) {
    queryString.length = (unsigned int)strlen(str);
    queryString.str = str; 
    
    SCIQuery(SCI_Q_API,&queryString,0,&error);
    if (error != SCI_ERR_OK) {
      str = "";
      return 0;
    } 
  } else {
    char SMP_API[] = "SMP, SYS-V shared memory";
    strcpy (str, SMP_API);
  }
  
  return 1;
}
#endif

static int _smi_region_connected (int shreg_id) {
    region_t *reg;

    reg = _smi_get_region(shreg_id);
    if (reg != NULL)
	return (reg->addresses[0] != NULL);
    else
	return (0);
}

void _smi_set_adapter_available(int adapter_nbr)
{
  adapter_available[adapter_nbr] = TRUE;
  return;
}

void _smi_set_adapter_unavailable(int adapter_nbr)
{
  adapter_available[adapter_nbr] = FALSE;
  return;
}

int _smi_adapter_available(int adapter_nbr)
{
    if ((adapter_nbr < 0) || (adapter_nbr >= MAX_ADAPTER))
	return FALSE;

    return (adapter_available[adapter_nbr]);
}

static int _smi_region_sgmt_id (int shreg_id) 
{
    region_t *reg;
    shseg_t *seg;
    int seg_nbr;

    reg = _smi_get_region(shreg_id);
    if (reg != NULL) {
      if (reg->no_segments == 1)
	return (reg->seg[0]->id);
      else
	for (seg_nbr = 0; seg_nbr < reg->no_segments; seg_nbr++) {
	    seg = reg->seg[seg_nbr];
	    if (seg->owner == _smi_my_proc_rank)
		return (seg->id);
	}
    }
    
    return (-1);
}

static int _smi_region_owner (int shreg_id) 
{
    region_t *reg;

    reg = _smi_get_region(shreg_id);
    if (reg != NULL && reg->no_segments == 1)
      return (reg->seg[0]->owner);

    return (-1);
}

static size_t _smi_region_offset (int shreg_id) 
{
    region_t *reg;

    reg = _smi_get_region(shreg_id);
    if (reg != NULL && reg->no_segments == 1 && reg->addresses[0] != NULL)
	return (reg->seg[0]->offset);

    return (-1);
}

static int _smi_get_adptnbr (int shreg_id) 
{
    region_t *reg;
    shseg_t *seg;
    int seg_nbr;

    reg = _smi_get_region(shreg_id);
    if (reg != NULL) {
	for (seg_nbr = 0; seg_nbr < reg->no_segments; seg_nbr++) {
	    seg = reg->seg[seg_nbr];
	    if (seg->owner == _smi_my_proc_rank)
		return (seg->adapter);
	}
    }
    
    return (-1);
}

smi_error_t SMI_Query (smi_query_t cmd, int arg, void *result)
{ 
    DSECTION ("SMI_Query");
#ifndef NO_SISCI
    /*sci_query_adapter_t adapter_query;*/
#endif
    DSECTENTRYPOINT;

    DNOTICEI("cmd =",cmd);
    DNOTICEI("arg =",arg);

    if (!_smi_initialized && (cmd != SMI_Q_SMI_INITIALIZED)) {
	*(int *)result = 0;
	return SMI_ERR_NOINIT;
    }

    switch (cmd) {
	/* SCI queries */
    case SMI_Q_SCI_STREAMBUFSIZE:
	if (arg >= nbr_adapters || arg < 0) {
	    DWARNING ("illegal cmd: non-exitant adapter number");
	    DSECTLEAVE; return (SMI_ERR_PARAM);
	}
	*(int *) result = streambufsizes[arg];
	break;

    case SMI_Q_SCI_NBRSTREAMBUFS:
	if (arg >= nbr_adapters || arg < 0) {
	    DWARNING ("illegal cmd: non-exitant adapter number");
	    DSECTLEAVE; return (SMI_ERR_PARAM);
	}
	*(int *) result = nbr_streambufs[arg];
	break;

    case SMI_Q_SCI_PACKETSIZE:
	*(int *) result = sci_ta_size;
	break;

    case SMI_Q_SCI_NBRADAPTERS:	
	*(int *) result = _smi_nbr_adapters[arg];
	break;

    case SMI_Q_SCI_ADAPTERTYPE:
	if (arg >= nbr_adapters || arg < 0) {
	    DWARNING ("illegal cmd: non-exitant adapter number");
	    DSECTLEAVE; return (SMI_ERR_PARAM);
	}
	*(int *) result = adaptertypes[arg];
	break;

    case SMI_Q_SCI_DEFADAPTER:
      *(int*) result = _smi_DefAdapterNbr;
      break;

    case SMI_Q_SCI_VALIDADAPTER:
	*(int *)result =_smi_adapter_available(arg);
	break;

    case SMI_Q_SCI_ID:
	if (arg >= nbr_adapters || arg < 0) {
	    DWARNING ("illegal cmd: non-exitant adapter number");
	    DSECTLEAVE; return (SMI_ERR_PARAM);
	}
	*(int *) result = sci_ids[arg];
	break;

    case SMI_Q_SCI_PROC_ID:
	if (arg >= _smi_nbr_procs || arg < 0) {
	    DWARNING ("illegal cmd: non-exitant process rank");
	    DSECTLEAVE; return (SMI_ERR_PARAM);
	}
	/* XXX: _smi_sci_rank does only consider the default adatper in each node,
	   which is not necessarily the adapter used by the process for communication! */
	*(int *) result = _smi_sci_rank[arg];
	break;

    case SMI_Q_SCI_CONNECTION_STATE:
	*(int *) result = _smi_all_on_one ? SMI_SUCCESS : _smi_get_connection_state();
	break;

    case SMI_Q_SCI_ERRORS:
	*(int *) result = _smi_sci_errcnt;
	break;

    case SMI_Q_SCI_API_VERSION:
	if (arg < 1) {
	    DPROBLEM("specified length of string is 0 or less");
	    DSECTLEAVE; return(SMI_ERR_PARAM);
	}
	strncpy(result,api_version,arg-1);
	break;
    case SMI_Q_SCI_DMA_SIZE_ALIGN:
	*(int *) result = dma_size_alignment;
	break;	
    case SMI_Q_SCI_DMA_OFFSET_ALIGN:
	*(int *) result = dma_offset_alignment;
	break;	
    case SMI_Q_SCI_DMA_MINSIZE:
	*(int *) result = dma_minsize;
	break;	
    case SMI_Q_SCI_DMA_MAXSIZE:
	*(int *) result = dma_maxsize;
	break;	
    case SMI_Q_SCI_NBR_PHYS_DIMS:
	*(int*)result = phys_sci_dims;
	break;
    case SMI_Q_SCI_PHYS_DIM_EXTENT:
	if (arg >= 0 && arg < MAX_DIMS)
	    *(int*)result = phys_sci_dim_extent[arg];
	break;	
    case SMI_Q_SCI_SCI_TOPOLOGY_TYPE:
	*(int*)result = sci_topo_type;
	break;
    case SMI_Q_SCI_SCIIDS_VALID:
	*(int*)result = has_valid_id;
	break;
    case SMI_Q_SCI_SYSTEM_TYPE:
	*(int*)result = sci_system_type;
	break;

	/* SMI queries */
      
    case SMI_Q_SMI_REGION_CONNECTED:
	*(int *)result = _smi_region_connected(arg);
	break;
    case SMI_Q_SMI_REGION_SGMT_ID:
	*(int *)result = _smi_region_sgmt_id(arg);
	break;
    case SMI_Q_SMI_INITIALIZED:
	*(int *)result = _smi_initialized ? 1 : 0;
	break;
    case SMI_Q_SMI_SIGNALS_AVAILABLE:
	*(int *)result = signals_avail;
	break;
    case SMI_Q_SMI_DMA_AVAILABLE:
	*(int *)result = dma_avail;
	break;
    case SMI_Q_SMI_REGION_ADPTNBR:
	*(int *)result = _smi_get_adptnbr(arg);
	break;
    case SMI_Q_SMI_REGION_ADDRESS:
	*(long **)result = _smi_get_region_address(arg);
	break;
    case SMI_Q_SMI_REGION_SIZE:
      *(size_t*)result = _smi_get_region_size(arg);
      break;
    case SMI_Q_SMI_REGION_OWNER:
      *(int*)result = _smi_region_owner(arg);
      break;
    case SMI_Q_SMI_REGION_OFFSET:
      *(size_t *)result = _smi_region_offset(arg);
      break;
    case SMI_Q_SMI_NBR_APP_DIMS:
	*(int*)result = app_sci_dims;
	break;
    case SMI_Q_SMI_APP_DIM_EXTENT:
	if (arg >= 0 && arg < MAX_DIMS)
	    *(int*)result = app_sci_dim_extent[arg];
	break;
    case SMI_Q_SMI_APP_DIM_OFFSET:
	if (arg >= 0 && arg < MAX_DIMS)
	    *(int*)result = app_sci_dim_offset[arg];
	break;
	
	/* SYS queries */

    case SMI_Q_SYS_NBRCPUS:
	*(int *) result = nbr_cpus;
	break;
    case SMI_Q_SYS_CPUFREQ:
	*(int *) result = cpu_frq;
	break;
    case SMI_Q_SYS_PAGESIZE:
	*(int *) result = local_page_size;
	break;
    case SMI_Q_SYS_WRITECOMBINING:
	*(int *) result = wc_enabled;
	break;
    case SMI_Q_SYS_PID:
        *(int *) result = _smi_pids[arg];
	break;
    case SMI_Q_SYS_TICK_DURATION:
        *(int *) result = ps_per_tick;
	break;
    case SMI_Q_SYS_CPU_CACHELINELEN:
        *(int *) result = cpu_cachelinelen;
	break;

    default:
	DWARNING ("unknown or unimplemented query");
	DSECTLEAVE; return (SMI_ERR_PARAM);
    }

    DSECTLEAVE; return (SMI_SUCCESS);
}

/* Determine type and size of SCI topology, most notable multidimensional 
   torus topologies (the famous "n-ary k-cubes") */
smi_error_t _smi_topology_init()
{
    DSECTION ("_smi_topology_init");
#ifndef NO_SISCI
    /*sci_query_adapter_t adapter_query;*/
	int dim; /*, x_dim, y_dim, z_dim;*/
#if !defined(CONFIGURE_TOPOLOGY_TORUS) && !defined(CONFIGURE_TOPOLOGY_SWITCH)
	sci_desc_t SCIfd;
	sci_error_t SCIerror;
	int probe_id, node_avail;
#endif
    int p/*, cnct_to_switch*/;
    int min_pos[MAX_DIMS], max_pos[MAX_DIMS], pos;
    boolean pos_active[MAX_NODES_PER_DIM+1];
#endif
    int i;

    DSECTENTRYPOINT;
    
    phys_sci_dims = 1;
    app_sci_dims  = 0;
    for (i = 0; i < MAX_DIMS; i++) {
	phys_sci_dim_extent[i] = 0;
	app_sci_dim_extent[i]  = 0;
	app_sci_dim_offset[i]  = 0;
    }

    if (_smi_all_on_one) {
	/* for pure SMP, it's quite simple */
	phys_sci_dim_extent[0] = 1;
	app_sci_dims           = 1;
	app_sci_dim_extent[0]  = 1;
	app_sci_dim_offset[0]  = 0;

	has_valid_id = 1; /* although it may only be a dummy id */
	sci_topo_type = SMI_SCI_TOPOLOGY_SMP;
	DSECTLEAVE; return SMI_SUCCESS;
    }

#ifndef NO_SISCI
    /* First, we check if the physical SCI topology was determined during the configure run. 
       If not, we probe right here. */
#ifdef CONFIGURE_TOPOLOGY_TORUS
    phys_sci_dims = CONFIGURE_TOPOLOGY_NDIMS;
    phys_sci_dim_extent[0] = CONFIGURE_TOPOLOGY_EXTENT_X;
    phys_sci_dim_extent[1] = CONFIGURE_TOPOLOGY_EXTENT_Y;
    phys_sci_dim_extent[2] = CONFIGURE_TOPOLOGY_EXTENT_Z;
    sci_topo_type = SMI_SCI_TOPOLOGY_TORUS;
    DNOTICE ("Using torus topology definition from config.h");
#elif defined CONFIGURE_TOPOLOGY_SWITCH
    DNOTICE ("Using switch topology definition from config.h");
    phys_sci_dims = 1;  
    /* XXX Determine real number of nodes connected to switch - but this is not
       critical for performance. */
    phys_sci_dim_extent[0] = _smi_nbr_machines;
    phys_sci_dim_extent[1] = 0;
    phys_sci_dim_extent[2] = 0;
    sci_topo_type = SMI_SCI_TOPOLOGY_SWITCH;
#else
#if defined DOLPHIN_SISCI && 0
    /* XXX The underlying SISCI query deliver "switch" even for torus! Needs to be fixed? */
    /* If the cluster uses (a) central switch(es), we set up a "dummy topology" */
    adapter_query.localAdapterNo = _smi_DefAdapterNbr;
    adapter_query.subcommand     = SCI_Q_ADAPTER_CONNECTED_TO_SWITCH;
    adapter_query.data           = (void *)&cnct_to_switch;
    SCIQuery(SCI_Q_ADAPTER,(void *)&adapter_query, 0, &SCIerror);
    if (cnct_to_switch) {
	DNOTICE ("Determined switch topology.");
	phys_sci_dim_extent[0] = _smi_nbr_machines;
	app_sci_dim_extent[0]  = _smi_nbr_machines;
	app_sci_dims = 1;

	has_valid_id = 1; /* but only valid for switch-topology, NOT for torus! */
	sci_topo_type = SMI_SCI_TOPOLOGY_SWITCH;
	DSECTLEAVE; return SMI_SUCCESS;
    }
#endif
    /* same code for Dolphin and Scali; only some scaling for Scali node ids */

    /* Not connected to a switch. Check if it a regular torus topology (different conditions
       for Scali and Dolpin due to different routing techniques). Only if the system has
       a regular torus topology, the gathered information on dimensions, extents etc. makes 
       sense. For non-regular topologies, we again set up a dummy topology.
       
       Of course, this test is prone to be fooled by a user who sets up his SCI topology
       in a weird way. But this is the problem of the user... The worst thing that can
       happen for erraenous setups is reduced performance. */
    
    DNOTICE ("Probing nodes for torus topology.");
    /* Get physical dimensions of the cluster by probing relevant nodes. */
    rs_SCIOpen(&SCIfd, 0, &SCIerror);    
    
    /* X-direction (Dolphin: node-ids 4, 8, .. 64; Scali: 0x0100, 0x0200, ..., 0x0f00) */
    for (probe_id = MIN_SCI_ID; probe_id <= SCI_ID_STRIDE*MAX_NODES_PER_DIM; probe_id += SCI_ID_STRIDE) {
	node_avail = SCIProbeNode(SCIfd, _smi_DefAdapterNbr, probe_id, 0, &SCIerror);
	if (!node_avail)
	    break;
    }
    phys_sci_dim_extent[0] = node_avail ? MAX_NODES_PER_DIM : (probe_id - MIN_SCI_ID)/SCI_ID_STRIDE;

    /* Y-direction (node-ids Dolpin: 4, 68,  .., 900; Scali: 0x0100, 0x1100, ..., 0xf100 ) */
    for (probe_id = MIN_SCI_ID; probe_id <= MAX_NODES_PER_DIM*MAX_NODES_PER_DIM*SCI_ID_STRIDE; 
	 probe_id += SCI_ID_STRIDE*(MAX_NODES_PER_DIM+1)) {
	node_avail = SCIProbeNode(SCIfd, _smi_DefAdapterNbr, probe_id, 0, &SCIerror);
	if (!node_avail)
	    break;
    }
    phys_sci_dim_extent[1] = node_avail ? MAX_NODES_PER_DIM : 
      (probe_id - MIN_SCI_ID)/(SCI_ID_STRIDE*(MAX_NODES_PER_DIM+1));
    if (phys_sci_dim_extent[1] > 1) 
	phys_sci_dims++;

    /* Z-direction (node-ids Dolphin: 4, 968,  .., 13500; Scali: not yet implemented ) */
#ifdef DOLPHIN_SISCI
    for (probe_id = MIN_SCI_ID; 
	 probe_id <= MAX_NODES_PER_DIM*MAX_NODES_PER_DIM*MAX_NODES_PER_DIM*SCI_ID_STRIDE; 
	 probe_id += SCI_ID_STRIDE*(MAX_NODES_PER_DIM+1)*MAX_NODES_PER_DIM) {
	node_avail = SCIProbeNode(SCIfd, _smi_DefAdapterNbr, probe_id, 0, &SCIerror);
	if (!node_avail)
	    break;
    }
#else
    probe_id = MIN_SCI_ID;
    node_avail = false;
#endif
    phys_sci_dim_extent[2] = node_avail ? MAX_NODES_PER_DIM : 
      (probe_id - MIN_SCI_ID)/(SCI_ID_STRIDE*(MAX_NODES_PER_DIM+1)*MAX_NODES_PER_DIM);
    if (phys_sci_dim_extent[2] > 1) 
	phys_sci_dims++;

    rs_SCIClose(SCIfd, 0, &SCIerror);
    
    /* Sanity check for cases where i.e. no node-id 4 was assigned: */
    if (phys_sci_dim_extent[0] == 0) {
      phys_sci_dims = app_sci_dims = 1;
      phys_sci_dim_extent[0] = _smi_nbr_machines;
      app_sci_dim_extent[0]  = _smi_nbr_machines;
      app_sci_dim_offset[0]  = 0;

      has_valid_id = 0;
      DSECTLEAVE; return SMI_SUCCESS;
    }
#endif
    DNOTICEI ("Determined phsyical torus of dimension", phys_sci_dims);
    DNOTICEI (" X extent =", phys_sci_dim_extent[0]);
    DNOTICEI (" Y extent =", phys_sci_dim_extent[1]);
    DNOTICEI (" Z extent =", phys_sci_dim_extent[2]);

    /* Now for the dimensions of the grid/torus that the nodes with processes 
       of this application represent. We need to find the minimum and maximum
       position of an "active" node within each dimension of the grid which gives 
       us postion and  size of the application sub-grid. 

       First, do a sanity check if the id's are o.k. */
    for (p = 0; p < _smi_nbr_procs; p++) {
	int x_dim, y_dim, z_dim;
	/* Walk through all nodes as specified by the topology setup, and see
	   if each process has an SCI id of one of the nodes. */
	has_valid_id = 0;
	for (z_dim = 0; z_dim < phys_sci_dim_extent[2]; z_dim++) {
	    for (y_dim = 0; y_dim < phys_sci_dim_extent[1]; y_dim++) {
		for (x_dim = 0; x_dim < phys_sci_dim_extent[0]; x_dim++) {
		    if (_smi_sci_rank[p] == z_dim*MAX_NODES_PER_DIM*MAX_NODES_PER_DIM*SCI_ID_STRIDE +
			y_dim*MAX_NODES_PER_DIM*SCI_ID_STRIDE + x_dim*SCI_ID_STRIDE + MIN_SCI_ID)
			has_valid_id = 1;
		}
	    }
	}
	if (!has_valid_id) {
	    DWARNING ("SCI node id's do not match the specified topology - setting up dummy topology.");
	    break;
	}
    }

    /* We can use the SCI ids to determine process placement in the cluster
       if valid id assignment was found. If not, we virtually place the processes on the
       nodes by filling up the specifed physical grid. */
    for (dim = 0; dim < phys_sci_dims; dim++) {
	memset (pos_active, 0, sizeof(int)*(MAX_NODES_PER_DIM+1));
	app_sci_dim_extent[dim] = 0;
	min_pos[dim] = 16;
	max_pos[dim] = 0;
	for (p = 0; p < _smi_nbr_procs; p++) {
	    switch (dim) {
	    case 0: /* X-dim for 1-D torus (always existant) */
		pos = has_valid_id ? 
		    (_smi_sci_rank[p] % (SCI_ID_STRIDE*(MAX_NODES_PER_DIM+1)))/SCI_ID_STRIDE - 1 :
			p % phys_sci_dim_extent[0];
		if (!pos_active[pos]) {
		    pos_active[pos] = true;
		    app_sci_dim_extent[dim]++;
		}
		break;
	    case 1: /* Y-dim for 2-D torus */
		pos = has_valid_id ? 
		    _smi_sci_rank[p]/(SCI_ID_STRIDE*(MAX_NODES_PER_DIM+1)) :
			(p/phys_sci_dim_extent[0]) % phys_sci_dim_extent[1];
		if (!pos_active[pos]) {
		    pos_active[pos] = true;
		    app_sci_dim_extent[dim]++;
		}
		break;
	    case 2: /* Z-dim for 3-D torus */
		pos = has_valid_id ? 
		    _smi_sci_rank[p]/(SCI_ID_STRIDE*(MAX_NODES_PER_DIM+1)*MAX_NODES_PER_DIM + MIN_SCI_ID) :
			p/(phys_sci_dim_extent[0]*phys_sci_dim_extent[1]);
		if (!pos_active[pos]) {
		    pos_active[pos] = true;
		    app_sci_dim_extent[dim]++;
		}
		break;
	    }
	    min_pos[dim] = (pos < min_pos[dim]) ? pos : min_pos[dim];
	}
	app_sci_dim_offset[dim] = min_pos[dim];
    }
    sci_topo_type = SMI_SCI_TOPOLOGY_TORUS;
    
    /* Each dimension with an extent of only one node is not relevant => set it to 0. */
    for (dim = 0, app_sci_dims = 0; dim < phys_sci_dims; dim++) {
	if (app_sci_dim_extent[dim] == 1)
	    app_sci_dim_extent[dim] = 0;
	if (app_sci_dim_extent[dim] > 0)
	    app_sci_dims++;
    }
    /* Now transpose the dimensions (fill them up from x to z) to avoid confusion with 0-extent
       x-dimension. Kind of "normalization of dimension to X".  */
    for (dim = 0; dim < phys_sci_dims - 1; dim++) {
	if (app_sci_dim_extent[dim] == 0) {
	    app_sci_dim_extent[dim] = app_sci_dim_extent[dim+1];
	    app_sci_dim_extent[dim+1] = 0;
	}
    }
    
    /* Special case for 1-node torus */
    if (app_sci_dim_extent[0] + app_sci_dim_extent[1] + app_sci_dim_extent[2] == 0) {
	app_sci_dims = 1;
	app_sci_dim_extent[0] = 1;
    }
#endif
    DNOTICEI ("Determined logical torus of dimension", app_sci_dims);
    
    DSECTLEAVE; return SMI_SUCCESS;
}

smi_error_t _smi_init_query (int nbr_procs) 
{
    DSECTION ("_smi_init_query");
    int i, query_id[MAX_ADAPTER], last_id, nbr_tries;
#ifndef NO_SISCI
    sci_desc_t SCIFd;
    sci_error_t SCIerror;
    sci_query_adapter_t adapter_query;
#ifdef DOLPHIN_SISCI
    sci_query_system_t system_query;
#endif
#endif

    DSECTENTRYPOINT;

    /* for pure SMP mode, nbr_procs is negative */
    nbr_procs = abs(nbr_procs);

    for (i=0; i<MAX_ADAPTER; i++)
      adapter_available[i] = FALSE;

    sci_system_type = 
#ifndef NO_SISCI
#ifdef DOLPHIN_SISCI
	SMI_SCI_DOLPHIN;
#elif defined SCALI_SISCI
        SMI_SCI_SCALI;
#else
        SMI_SCI_UNKNOWN;
#endif
#else
        SMI_SCI_NONE;
#endif
	
#ifndef NO_SISCI
    if (!_smi_all_on_one) {
	/* prepare for SCI related queries */
#ifdef HAVE_SCIINITIALIZE
	DNOTICE("calling SCIInitialize");
    	/* this will be required in future SISCI versions */
    	SCIInitialize (0, &SCIerror);
#ifdef DOLPHIN_SISCI
    	if (SCIerror == SCI_ERR_INCONSISTENT_VERSIONS 
	    || SCIerror == SCI_ERR_ILLEGAL_PARAMETER) {
	    DERROR ("Version mismatch between SISCI API library and SISCI kernel driver");
    	}
#endif
    	ABORT_IF_FAIL("SCIInitialize() failed", SCIerror, -1);
#endif
	DNOTICE("opening SCI-descriptor");
	rs_SCIOpen(&SCIFd, 0, &SCIerror);
    	ABORT_IF_FAIL("SCIOpen() failed", SCIerror, -1);
	if (SCIerror == SCI_ERR_OK) {
	    int nbr_adpts_ok = 0;
	    int use_adapter = -1;
	    char *env_str;

	    /* check for explicit adapter specification by the user */
	    if ((env_str = getenv("SMI_USE_ADAPTER")) != NULL) {
	      use_adapter = atoi(env_str);
	    }

	    /* determine number of configured PCI-SCI adapters and their IDs */
	    ALLOCATE (_smi_nbr_adapters, int *, nbr_procs*sizeof(int));
	    for( i = 0; i < MAX_ADAPTER; i++ ) {
	        if (use_adapter >= 0 && i != use_adapter) 
		  continue;

		adapter_query.localAdapterNo = i;
		adapter_query.subcommand     = SCI_Q_ADAPTER_NODEID;
		adapter_query.data           = (void *)&query_id[i];
		SCIQuery(SCI_Q_ADAPTER,(void *)&adapter_query, 0, &SCIerror);
		if (SCIerror == SCI_ERR_OK) {
		    DNOTICEI("local adapter",i);
		    DNOTICEI("  SCI node_id =",query_id[i]);
		    nbr_adapters++;

#ifdef DOLPHIN_SISCI
		    /* Test if adapter is operational. We do this in a loop, because due to
		       the asynchronous nature of the IRM, the link may not yet be operartional
		       although the driver is running. This leads to wrong error message if
		       the driver has just been loaded by the SCIOpen above. */
		    adapter_query.localAdapterNo = i;
		    adapter_query.subcommand     = SCI_Q_ADAPTER_LINK_OPERATIONAL;
		    adapter_query.data           = (void *)&adapter_available[i];
		    SCIQuery(SCI_Q_ADAPTER,(void *)&adapter_query, 0, &SCIerror);			
		    for (nbr_tries = 0; 
			 !adapter_available[i] && (nbr_adpts_ok == 0) && (nbr_tries < SCI_PROBE_TRIES); 
			 nbr_tries++) {
		      sleep (SCI_PROBE_DELAY);
		      SCIQuery(SCI_Q_ADAPTER,(void *)&adapter_query, 0, &SCIerror);
		      if (!adapter_available[i]) {
			DPROBLEMP("  SCI link not operational for this adapter, SISCI error", SCIerror);
		      }
		    }
		    /* If one of multiple adapters is not operational, we do not want to 
		       wait for it to become operational, but will use one of the adapters
		       which are operational. */
		    if (adapter_available[i])
			nbr_adpts_ok++;
		    
		    adapter_query.localAdapterNo = i;
		    adapter_query.subcommand     = SCI_Q_ADAPTER_CARD_TYPE;
		    adapter_query.data           = (void *)&adaptertypes[i];
		    SCIQuery(SCI_Q_ADAPTER,(void *)&adapter_query, 0, &SCIerror);
		    
		    switch (adaptertypes[i]) {
		    /* are these supported bye the drivers any longer? */
		    case 304:
		    case 307:
			streambufsizes[i] = 0;
			sci_ta_size = 0;
			break;
		    /* all LC2 - PSB32 equipped adapters */
		    case 308:
		    case 310:
		    case 312:
			streambufsizes[i] = 64;
			sci_ta_size = 64;
			break;
		    /* all LC2 - PSB64 equipped adapters */
		    case 320:
		    case 321:
			streambufsizes[i] = 128;
			sci_ta_size = 64;
			break;
		    /* all LC3 - PSB66 equipped adapters */
		    case 330:
		    case 331:
		    case 332:
		    case 333:
		    case 334:
		    case 335:
		    case 336:
		    case 337:
		    case 339:
		    case 350:
		    case 351:
		    case 352:
		    case 353:
			streambufsizes[i] = 128;
			sci_ta_size = 128;
			break;
		    default: 
			/* unknown adapter found - for now, just mention it in
			   the logs and use some sensible default values. */
			DPROBLEMI("Found unknown PCI-SCI adapter of type ", adaptertypes[i]);
			DPROBLEM("Using default values.")
			streambufsizes[i] = 128;
			sci_ta_size = 128;
		    }
#if HAVE_STREAM_QUERIES
		    {
			int sbuf_sz;

			adapter_query.localAdapterNo = nbr_adapters - 1;
			adapter_query.subcommand     = SCI_Q_ADAPTER_STREAM_BUFFER_SIZE;
			adapter_query.data           = (void *)&sbuf_sz;
			SCIQuery(SCI_Q_ADAPTER,(void *)&adapter_query, 0, &SCIerror);
			if (SCIerror != SCI_ERR_OK) {
			    DPROBLEMP ("SCIQuery(SCI_Q_ADAPTER_STREAM_BUFFER_SIZE) failed, SISCI error", SCIerror);
			} else {
			    streambufsizes[i] = sbuf_sz;
			}
		    }
#endif
#else
		    /* XXX: Scali's SISCI is missing these query commands, we decide upon the
		       settings provided by the configure script */
		    DNOTICE("SCALI_SISCI queries");
#ifdef D31x
		    adaptertypes[i] = 312;
		    streambufsizes[i] = 64;
		    sci_ta_size = 64;
#elif defined D32x
		    adaptertypes[i] = 322;
		    streambufsizes[i] = 128;
		    sci_ta_size = 64;
#elif defined D33x
		    adaptertypes[i] = 335;
		    streambufsizes[i] = 128;
		    sci_ta_size = 128;
#else
#error PCI-SCI adapter type not specfied! Suppy --with-pcisci option when configuring.
#endif
#endif
		    /* This query is also missing in Scali SISCI - we have to assume that
		       the adapter is o.k..*/
		    adapter_available[i] = 1;

		    DNOTICEI("  type is D", adaptertypes[i]);
		    DNOTICEI("  is available:", adapter_available[i]);
		} else
		    query_id[i] = -1;		
	    }

	    /* entries for other procs are filled up in _smi_init_sci_subsystem() 
	       via an Allgather() operation */
	    for (i = 0; i < nbr_procs; i++)
		_smi_nbr_adapters[i] = nbr_adapters;
	    
	    ALLOCATE (nbr_streambufs, int *, sizeof(int)*nbr_adapters);
	    ALLOCATE (sci_ids, int *, sizeof(int)*nbr_adapters);
	
	    last_id = 0;
	    for (i = 0; i < nbr_adapters; i++) {
		while (query_id[last_id] == -1)
		    last_id++;
		sci_ids[i] = query_id[last_id++];
	    }
	    	    
#ifdef DOLPHIN_SISCI
	    /* some general parameters */
	    for (last_id = 0; query_id[last_id] == -1; last_id++)
		;
	    /* XXX Part of these queries return ridicilous values - need to check back with Dolphin! 
	       For now, we only use some of the driver-supplied information. */
#if 0 	    
	    adapter_query.localAdapterNo = last_id;
	    adapter_query.subcommand     = SCI_Q_ADAPTER_DMA_SIZE_ALIGNMENT;
	    adapter_query.data           = (void *)&dma_size_alignment;
	    SCIQuery(SCI_Q_ADAPTER,(void *)&adapter_query, 0, &SCIerror);
	    if (SCIerror != SCI_ERR_OK) {
	      DPROBLEMP ("SCIQuery(SCI_Q_ADAPTER_DMA_SIZE_ALIGNMENT) failed, SISCI error", SCIerror);
	    }
#else
	    dma_size_alignment = 8;
#endif
	    DNOTICEI("DMA size alignment =", dma_size_alignment);
#if 0
	    adapter_query.localAdapterNo = last_id;
	    adapter_query.subcommand     = SCI_Q_ADAPTER_DMA_OFFSET_ALIGNMENT;
	    adapter_query.data           = (void *)&dma_offset_alignment;
	    SCIQuery(SCI_Q_ADAPTER,(void *)&adapter_query, 0, &SCIerror);
	    if (SCIerror != SCI_ERR_OK) {
	      DPROBLEMP ("SCIQuery(SCI_Q_ADAPTER_DMA_OFFSET_ALIGNMENT) failed, SISCI error", SCIerror);
	    }
#else
	    dma_offset_alignment = 8;
#endif
	    DNOTICEI("DMA offset alignment =", dma_size_alignment);
	    
#if 0
	    adapter_query.localAdapterNo = last_id;
	    adapter_query.subcommand     = SCI_Q_ADAPTER_DMA_MTU;
	    adapter_query.data           = (void *)&dma_maxsize;
	    SCIQuery(SCI_Q_ADAPTER,(void *)&adapter_query, 0, &SCIerror);
	    if (SCIerror != SCI_ERR_OK) {
	      DPROBLEMP ("SCIQuery(SCI_Q_ADAPTER_DMA_MTU) failed, SISCI error", SCIerror);
	    }
#else
	    dma_maxsize = 256*1024;
#endif
	    DNOTICEI("DMA max. transfer unit =", dma_maxsize);
	    
#if 0
	    adapter_query.localAdapterNo = last_id;
	    adapter_query.subcommand     = SCI_Q_ADAPTER_SUGGESTED_MIN_BLOCK_SIZE;
	    adapter_query.data           = (void *)&dma_minsize;
	    SCIQuery(SCI_Q_ADAPTER,(void *)&adapter_query, 0, &SCIerror);
	    if (SCIerror != SCI_ERR_OK) {
	      DPROBLEMP ("SCIQuery(SCI_Q_ADAPTER_SUGGESTED_MIN_BLOCK_SIZE) failed, SISCI error", SCIerror);
	    }
#else
	    dma_minsize = 8;
#endif
	    DNOTICEI("DMA suggested min. transfer unit =", dma_minsize);
#endif

	    /* determine their number of streams */
	    last_id = 0;
	    for (i = 0; i < nbr_adapters; i++) {
		while (query_id[last_id] == -1)
		    last_id++;
#ifdef DOLPHIN_SISCI
		adapter_query.localAdapterNo = last_id;
		adapter_query.subcommand     = SCI_Q_ADAPTER_NUMBER_OF_STREAMS;
		adapter_query.data           = (void *)&nbr_streambufs[i];
		SCIQuery(SCI_Q_ADAPTER,(void *)&adapter_query, 0, &SCIerror);
		
		/* XXX work-around for not-implemtented / bugs in the SISCI - to be eliminated */
		if ((SCIerror != SCI_ERR_OK) || (nbr_streambufs[i] > 64) || (nbr_streambufs[i] < 8)) {
#endif
		    DWARNING ("SCIQuery(NUMBER_OF_STREAMS) failed, using values by adapter type");
		    if (adaptertypes[i] < 320) {
			nbr_streambufs[i] = 8;
		    } else {
			if (adaptertypes[i] >= 320 && adaptertypes[i] < 340) 
			    nbr_streambufs[i] = 16;
		    }
#ifdef DOLPHIN_SISCI
		}
#endif
	    }

	    wc_enabled = SMI_WC_UNKNOWN;
#ifdef DOLPHIN_SISCI
	    /* XXX this query is not yet implemented by Dolphin */
	    /* is write combining enabled ? */
	    system_query.subcommand = SCI_Q_SYSTEM_WRITE_COMBINING_ENABLED;
	    system_query.data = (unsigned int *)&wc_enabled;
	    SCIQuery(SCI_Q_SYSTEM,(void *)&system_query, 0, &SCIerror);
	    if (SCIerror != SCI_ERR_OK) {
		DPROBLEMP ("SCIQuery(SCI_Q_SYSTEM_WRITE_COMBINING_ENABLED) failed, SISCI error", SCIerror);
		DNOTICE ("Setting WC type to UNKNOWN");
		wc_enabled = SMI_WC_UNKNOWN;
	    } else {
		wc_enabled = wc_enabled ? SMI_WC_ENABLED : SMI_WC_DISABLED;
	    }
#endif
#ifdef LINUX
	    if (wc_enabled == SMI_WC_UNKNOWN) {
		/* Determine the write-combining state via /proc/mtrr - if there appears
		   a write-combined region, we assume that write-combining is enabled for
		   SCI-mapped memory. */
		FILE *mtrr_fh;
		char mtrr_buf[255], *mtrr_line;
		
		if ((mtrr_fh = fopen("/proc/mtrr", "r")) != NULL) {
		    DNOTICE ("probing /proc/mtrr");
		    while ((mtrr_line = fgets(mtrr_buf, 254, mtrr_fh)) != NULL) {
			if (strstr(mtrr_line, "write-combining") != NULL) {
			    DNOTICE ("Setting WC type to ENABLED");
			    wc_enabled = SMI_WC_ENABLED;
			    break;
			}
		    }
		    if (wc_enabled == SMI_WC_UNKNOWN) {
			DNOTICE ("Setting WC type to DISABLED");
			wc_enabled = SMI_WC_DISABLED;
		    }
		    fclose (mtrr_fh);
		}
	    }
#endif
	    
	    /* XXX dynamically determine these settings (especially signals) */
#ifdef DOLPHIN_SISCI
	    signals_avail = 1;
	    dma_avail = 1;	    
#else
	    signals_avail = 0;
	    dma_avail = 0;
#endif
	    
	    rs_SCIClose(SCIFd, 0, &SCIerror);
         }
     } else {
#endif /* NO_SISCI */
	/* all procs are running on a single SMP, but we need nevertheless
	   provide senseful information for SCI related queries because 
	   programms might depend on them. We simulate a D330 PCI-SCI adapter. */

	nbr_adapters = 1;  /* 0 might confuse! */

	ALLOCATE (nbr_streambufs, int *, sizeof(int)*nbr_adapters);
	ALLOCATE (sci_ids, int *, sizeof(int)*nbr_adapters);
	ALLOCATE (_smi_nbr_adapters, int *, nbr_procs*sizeof(int));
	for (i = 0; i < nbr_procs; i++)
	    _smi_nbr_adapters[i] = 1;
	streambufsizes[0]    = 128;
	sci_ta_size          = 128;
	nbr_streambufs[0]    = 16;
	adaptertypes[0]      = 330;
	sci_ids[0]           = 0;
	adapter_available[0] = TRUE;

	signals_avail = 0;  /* XXX: signals in SMP mode are not yet complete */
	dma_avail = 0;
#ifndef NO_SISCI
    }
#endif 

    /* get CPU information */
    cpu_cachelinelen = (int )_smi_get_CPU_cachelinesize();

#if defined SOLARIS
    {
	processor_info_t cpu_info;
    
	for (i = 0; processor_info((processorid_t) i, &cpu_info) == 0; i++) {
	    if (cpu_info.pi_state == P_ONLINE) {
		nbr_cpus++;
		cpu_frq = cpu_info.pi_clock;
	    }
	}
    }
#elif defined WIN32
    {
	LARGE_INTEGER t0;
	SYSTEM_INFO SInfo;
	int mask, cpu_cnt;
	
	QueryPerformanceFrequency( (LARGE_INTEGER*)&t0 );
	cpu_frq = (int)fabs((double)t0.QuadPart/(double)1e6);

	GetSystemInfo(&SInfo);
	nbr_cpus = SInfo.dwNumberOfProcessors;
	mask = 1;
	for (cpu_cnt = 0; cpu_cnt < nbr_cpus; cpu_cnt++) {
	    if (!(mask & SInfo.dwActiveProcessorMask))
		nbr_cpus--;
	    mask <<= 1;
	}
    }
#else
    {
	FILE *cpuinfo_fh;
	char cpuinfo_buf[255], *cpuinfo_line;
	
	cpu_frq = 0;
	if ((cpuinfo_fh = fopen("/proc/cpuinfo", "r")) != NULL) {
	    float cpu_frq_MHz = 1;

	    DNOTICE ("probing /proc/cpuinfo for cpu_freq");
	    while ((cpuinfo_line = fgets(cpuinfo_buf, 254, cpuinfo_fh)) != NULL) {
		if (strncmp(cpuinfo_line, "cpu MHz\t\t: ", 11) == 0) {
		    sscanf(cpuinfo_line, "cpu MHz\t\t: %f", &cpu_frq_MHz);
		}
	    }
	    cpu_frq = (int)cpu_frq_MHz;
	    fclose (cpuinfo_fh);
	}
	else {
#if defined X86
	    struct timeval tv0, tv1;
	    longlong_t t0, t1; 
	    
	    int usecs;
	    
	    /* no OS function - guess it by hand */
	    gettimeofday (&tv0, NULL);
	    GETTICKS(&t0);
	    for (i = 0; i < FRQ_LOOPS; i++) {
		asm ("nop");
	    }
	    GETTICKS(&t1);
	    gettimeofday (&tv1, NULL);
	    usecs = tv1.tv_usec - tv0.tv_usec + 1e+6*(tv1.tv_sec - tv0.tv_sec);
	    
	    cpu_frq = (int)fabs((((t1 - t0)/FRQ_BASE)*FRQ_BASE)/usecs);
#else
	    /* XXX until GETTICKS() is o.k. for Alpha and IA-64 */
            DWARNING("CPU architecture not fully supported - using dummy frequency.");
	    cpu_frq = 666;
#endif
	}
	{
	    nbr_cpus = 0;
	    if ((cpuinfo_fh = fopen("/proc/cpuinfo", "r")) != NULL) {
		DNOTICE ("probing /proc/cpuinfo for nbr_cpus");
		/* XXX is this valid for non-IA-32 kernels, too? */
		while ((cpuinfo_line = fgets(cpuinfo_buf, 254, cpuinfo_fh)) != NULL) {
		    if (strstr(cpuinfo_line, "processor") != NULL) {
			nbr_cpus++;
		    }
		}
		fclose (cpuinfo_fh);
	    }
	}
    }
#endif /* Get OS specific CPU Info */
    ps_per_tick = 1000000/cpu_frq;

    /* other queries are prepared here */
    if ((local_page_size = (int)sysconf(_SC_PAGESIZE)) == -1)
	DPROBLEM("Could not get local pagesize");

    /* Get Version of SISCI-API */
    if (_smi_APIVersionString(api_version) == 0) {
        DSECTLEAVE; 
	return (SMI_ERR_OTHER);
    }
    
    DSECTLEAVE;
    return SMI_SUCCESS;
}

void _smi_finalize_query () 
{
    /* nothing for now */

    return;
}
