/* $Id$ */

/* regalloc.c - test the repeated creation of shared memory regions with
   random-size to see if
   - resources (SMI, SCI, IRM, OS)  are lost
   - local physically contiguous memory or ATT entries to map remote memory
     are fragmented */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "smi.h"

#define MAX_SIZE     (16*1024*1024)
#define NBR_CREATES  512


int main (int argc, char *argv[]) 
{
    smi_region_info_t reginfo;
    smi_error_t err;
    int *shreg_id, p, n, max_size, do_map, local_only, owner, nbr_creates;
    int rank, size, reg_size, reg_type, reg_owner, iter, verbose;
    char **shreg_addr, c;

    SMI_Init(&argc, &argv);

    SMI_Proc_rank(&rank);
    SMI_Proc_size(&size);

    /* default values */
    max_size    = MAX_SIZE;
    nbr_creates = NBR_CREATES;
    owner       = -1;
    do_map      = 1;
    local_only  = 0;
    iter        = 0;
    verbose     = 0;

    while((c = getopt(argc, argv, "n:m:p:lv?")) != EOF) {
	switch(c) {
	case 'm':
	    max_size = atoi(optarg) * 1024;
	    break;
	case 'n':
	    nbr_creates = atoi(optarg);
	    break;
	case 'p':
	    owner = atoi(optarg);
	    break;
	case 'l':
	    local_only = 1;
	    break;
	case 'v':
	    verbose = 1;
	    break;
	case '?':
	    if (rank == 0) {
		printf("'regalloc' tests the repeated creation of undivided SMI regions.\n");
		printf(" options: \n");
		printf(" -n N   nbr of times to create a region (0 means endlessly)\n");
		printf(" -m N   max size of a region (kB)\n");
		printf(" -l     create only locally (do not map remote memory) (kB)\n");
		printf(" -p N   process which owns the region\n");
		printf(" -v     verbose output\n");
		printf(" -?     this help\n");
	    }
	    SMI_Finalize();
	    return 0;
	    break;
	}
    }

    shreg_id   = (int *)malloc(size*sizeof(int));
    shreg_addr = (char **)malloc(size*sizeof(char *));
    reg_type = local_only ? SMI_SHM_LOCAL : SMI_SHM_UNDIVIDED|SMI_SHM_NONFIXED;

    srand48((long int)getpid());

    if (verbose) {
	int node_size, nodes;
	char nodename[1024];
	size_t namelen = 1024;
	
	SMI_Local_proc_size(&node_size);
	SMI_Node_size(&nodes);
	SMI_Get_node_name(nodename, &namelen);
	
	printf("[%d] process %d (of %d), %d node(s), %d process(es) on local node %s\n",
	       rank, rank, size, nodes, node_size, nodename);
    }

    for (n = 1; nbr_creates == 0 || n <= nbr_creates; n++) {
	for (p = 0; p < size; p++) {
	    iter++;

	    reg_size =  (p == rank || p == owner || local_only) ? 
		(int)(1 + ((double)max_size)*drand48()) : 0;
	    reg_owner = local_only ? rank : 
		owner >= 0 ? owner : p;

	    if (verbose) {
		switch (reg_type) {
		case SMI_SHM_LOCAL:		    
		    printf("[%d] iteration %d: creating LOCAL region: size = %d \n", 
			   rank, iter, reg_size);
		    break; 
		case SMI_SHM_UNDIVIDED|SMI_SHM_NONFIXED:
		    if (rank == reg_owner) {
			printf("[%d] iteration %d: creating UNDIVIDED region: size = %d, owner = %d \n", 
			       rank, iter, reg_size, reg_owner);
		    }
		    break; 
		}
	    }
		    
	    SMI_Init_reginfo (&reginfo, reg_size, 0, reg_owner, SMI_ADPT_DEFAULT, 0, 0, NULL);
	    err = SMI_Create_shreg(reg_type, &reginfo, &shreg_id[p], (void **)&shreg_addr[p]);
	    
	    if (err != SMI_SUCCESS) {
		printf ("###  creation failed for process %d with SMI error %d\n", rank, err);
		printf ("     region owner is %d, region size is %d kB\n", p, reg_size/1024);
		fflush (stdout);
		
		shreg_id[p] = -1;
	    }
	    
	    if (local_only || owner >= 0)
		break;
	}
	for (p = 0; p < size; p++) {
	    if (shreg_id[p] != -1) {
		SMI_Free_shreg(shreg_id[p]);
	    }
	    if (local_only || owner >= 0)
		break;
	}
    } 

    free(shreg_id); free(shreg_addr);
    SMI_Finalize();
    return 0;
}
