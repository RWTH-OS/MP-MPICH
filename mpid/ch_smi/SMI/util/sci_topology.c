/* $Id$

   Determine type and size of SCI topology, most notable multidimensional 
   torus topologies (the famous "n-ary k-cubes"). The result of this
   query is written to a configuration file (config.h) and will be used
   for this cluster - this means if the topology should change, be sure to
   re-configure and re-compile SMI!

   The core algorithm is taken from src/utility/query.c .
*/

#include <sys/types.h>
#include <stdio.h>

#include "sisci_api.h"
#include "config.h"

#define MAX_DIMS          3       /* max. number of phys. SCI dimensions */
#define MAX_NODES_PER_DIM 15      /* max. number of nodes per dimension. */
#ifdef DOLPHIN_SISCI
#define MIN_SCI_ID        4
#define SCI_ID_STRIDE     4
#else 
#define MIN_SCI_ID        0x0100
#define SCI_ID_STRIDE     (1 << 8)
#endif

#define TOPO_UNKNOWN 0
#define TOPO_TORUS   1
#define TOPO_SWITCH  2



int get_sci_topology (int *ndims, int *dim_extent)
{
    sci_desc_t SCIfd;
    sci_error_t SCIerror;
#if defined DOLPHIN_SISCI
    sci_query_adapter_t adapter_query;
#endif
    int dim, p, probe_id, node_avail;
    int min_pos[MAX_DIMS], max_pos[MAX_DIMS], pos;
    int pos_active[MAX_NODES_PER_DIM+1];
    int cnct_to_switch;
    int i;
    
    *ndims = 1;
    for (i = 0; i < MAX_DIMS; i++) {
	dim_extent[i] = 0;
    }
    
    /* XXX The underlying SISCI query deliver "switch" even for torus! Needs to be fixed? */
#if defined DOLPHIN_SISCI && 0
    /* If the cluster uses (a) central switch(es), we set up a "dummy topology" */
    adapter_query.localAdapterNo = 0;
    adapter_query.subcommand     = SCI_Q_ADAPTER_CONNECTED_TO_SWITCH;
    adapter_query.data           = (void *)&cnct_to_switch;
    SCIQuery(SCI_Q_ADAPTER,(void *)&adapter_query, 0, &SCIerror);
    if (cnct_to_switch) {
	dim_extent[0] = _smi_nbr_machines;

	return TOPO_SWITCH;
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
    
    /* Get physical dimensions of the cluster by probing relevant nodes. */
#ifdef HAVE_SCIINITIALIZE
    SCIInitialize (0, &SCIerror);
    if (SCIerror != SCI_ERR_OK) {
	fprintf (stderr, "*** Problem: could not init SISCI (SCIInitialize() failed with error 0x%x)\n", 
		 SCIerror);
	/* SMI will probe dynamicalkly for the topology. */
	*ndims = -1;
	return TOPO_UNKNOWN;
    }
#endif
    
    SCIOpen(&SCIfd, 0, &SCIerror);    
    if (SCIerror != SCI_ERR_OK) {
	fprintf (stderr, "*** Problem: could not open SISCI device (SCIOpen() failed with error 0x%x)\n", 
		 SCIerror);
	/* SMI will probe dynamicalkly for the topology. */
	*ndims = -1;
	return TOPO_UNKNOWN;
    }
    
    /* X-direction (Dolphin: node-ids 4, 8, .. 64; Scali: 0x0100, 0x0200, ..., 0x0f00) */
    for (probe_id = MIN_SCI_ID; probe_id <= SCI_ID_STRIDE*MAX_NODES_PER_DIM; probe_id += SCI_ID_STRIDE) {
	node_avail = SCIProbeNode(SCIfd, 0, probe_id, 0, &SCIerror);
	if (!node_avail)
	    break;
    }
    dim_extent[0] = node_avail ? MAX_NODES_PER_DIM : (probe_id - MIN_SCI_ID)/SCI_ID_STRIDE;

    /* Y-direction (node-ids Dolpin: 4, 68,  .., 900; Scali: 0x0100, 0x1100, ..., 0xf100 ) */
    for (probe_id = MIN_SCI_ID; probe_id <= MAX_NODES_PER_DIM*MAX_NODES_PER_DIM*SCI_ID_STRIDE; 
	 probe_id += SCI_ID_STRIDE*(MAX_NODES_PER_DIM+1)) {
	node_avail = SCIProbeNode(SCIfd, 0, probe_id, 0, &SCIerror);
	if (!node_avail)
	    break;
    }
    dim_extent[1] = node_avail ? MAX_NODES_PER_DIM : 
      (probe_id - MIN_SCI_ID)/(SCI_ID_STRIDE*(MAX_NODES_PER_DIM+1));
    if (dim_extent[1] > 1) 
	(*ndims)++;

    /* Z-direction (node-ids Dolphin: 4, 968,  .., 13500; Scali: not yet implemented ) */
#ifdef DOLPHIN_SISCI
    for (probe_id = MIN_SCI_ID; 
	 probe_id <= MAX_NODES_PER_DIM*MAX_NODES_PER_DIM*MAX_NODES_PER_DIM*SCI_ID_STRIDE; 
	 probe_id += SCI_ID_STRIDE*(MAX_NODES_PER_DIM+1)*MAX_NODES_PER_DIM) {
	node_avail = SCIProbeNode(SCIfd, 0, probe_id, 0, &SCIerror);
	if (!node_avail)
	    break;
    }
#else
    probe_id = MIN_SCI_ID;
    node_avail = 0;
#endif
    dim_extent[2] = node_avail ? MAX_NODES_PER_DIM : 
      (probe_id - MIN_SCI_ID)/(SCI_ID_STRIDE*(MAX_NODES_PER_DIM+1)*MAX_NODES_PER_DIM);
    if (dim_extent[2] > 1) 
	(*ndims)++;

    SCIClose(SCIfd, 0, &SCIerror);
    
    /* Sanity check for cases where i.e. no node-id 4 was assigned: */
    if (dim_extent[0] == 0) {
	fprintf (stderr, "*** Problem: detected invalid SCI node-id assignment - please validate!\n", 
		 SCIerror);
	/* SMI will probe dynamicalkly for the topology. */
	*ndims = -1;
	return TOPO_UNKNOWN;
    }

    return TOPO_TORUS;
}


int main (int argc, char *argv[])
{
    int topology;
    int ndims, dim_extent[MAX_DIMS];
    FILE *config_file;

    if (argc < 2) 
	return 1;

    /* open output file */
    config_file = fopen (argv[1], "a");
    if (config_file == NULL)
	return 1;
    fprintf (config_file, "\n/* SCI topology as determined during configure - adapt manually, if required. */\n");

    /* probe topology and write to file */
    topology = get_sci_topology(&ndims, dim_extent);
    switch (topology) {
    case TOPO_UNKNOWN:	
	fprintf (config_file, "#define CONFIGURE_TOPOLOGY_UNKNOWN 1\n");
	break;
    case TOPO_TORUS:
	fprintf (config_file, "#define CONFIGURE_TOPOLOGY_TORUS 1\n");
	fprintf (config_file, "#define CONFIGURE_TOPOLOGY_NDIMS %d\n", ndims);
	fprintf (config_file, "#define CONFIGURE_TOPOLOGY_EXTENT_X %d\n", dim_extent[0]);
	fprintf (config_file, "#define CONFIGURE_TOPOLOGY_EXTENT_Y %d\n", dim_extent[1]);
	fprintf (config_file, "#define CONFIGURE_TOPOLOGY_EXTENT_Z %d\n", dim_extent[2]);
	break;
    case TOPO_SWITCH:
	fprintf (config_file, "#define CONFIGURE_TOPOLOGY_SWITCH 1\n");
	break;
    }
    
    fclose(config_file);
    return (topology == TOPO_UNKNOWN);
}
