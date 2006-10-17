/* $Id$ 

   How many collective regions can be created? To test this,
   all procs create fragmented regions (which required an
   all-to-all connect) until a creation fails. */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "smi.h"

#define NAMELEN 256
#define MAX_NBR_REGS 1024
#define REG_SIZE (64*1024)

int main (int argc, char *argv[]) 
{
    smi_region_info_t reginfo;
    smi_error_t err;
    int proc_rank, proc_size, local_proc_size;
    int reg_size = REG_SIZE, nbr_regs = MAX_NBR_REGS, access = 0;
    int reg_cnt = 0, *shreg_id, c, proc;
    char nodename[NAMELEN], **shreg_addr;
    size_t len;

    SMI_Init(&argc, &argv);
    SMI_Proc_rank(&proc_rank);
    SMI_Proc_size(&proc_size);
    SMI_Local_proc_size(&local_proc_size);
    len = NAMELEN;
    SMI_Get_node_name(nodename, &len);

    while((c = getopt(argc, argv, "s:m:a?")) != EOF) {
	switch(c) {
	case 's':
	    reg_size = atoi(optarg)*1024;
	    break;
	case 'm':
	    nbr_regs = atoi(optarg);
	    break;
	case 'a':
	    access = 1;
	    break;
	case '?':
	    if (proc_rank == 0) {
		printf ("%s [-s <region_size>] [-m <max_nbr_regions>] [-a]\n\
  Test how many FRAGMENTED regions can be created.\n\
  '-a' accesses all regions completely for testing \n", argv[0]);
	    }
	    SMI_Finalize();
	    return 0;
	}
    }

    shreg_id = (int *)malloc (nbr_regs*sizeof(int));
    shreg_addr = (char **)malloc(proc_size*sizeof(char *));
    do {
	SMI_Init_reginfo(&reginfo, reg_size, 0, proc_rank, SMI_ADPT_DEFAULT, 0, 0, NULL);
	err = SMI_Create_shreg(SMI_SHM_FRAGMENTED, &reginfo, &shreg_id[reg_cnt], (void **)shreg_addr);

	/* access the regions for testing */
	if (err == SMI_SUCCESS && access) {
	    int *access_ptr;
	    for (proc = 0; proc < proc_size; proc++) {
		access_ptr = (int *)shreg_addr[proc];
		for (c = 0; c < reg_size/(proc_size*sizeof(int)); c++)
		    access_ptr[c] = proc_rank;
	    }
	}
    } while (err == SMI_SUCCESS && ++reg_cnt && reg_cnt < nbr_regs);
    
    for (c = 0; c < proc_size; c++) {
	if (c == proc_rank) {
	    printf ("[%d] of (%d): could create %d fragmented regions of size %d kb on node %s\n\
 per-proc sgmt-size   %d kB\t(%d MB)\n\
 per-proc total-size: %d kB\t(%d MB)\n\
 per-node size:  %d kB\t(%d MB)\n\
 total size:     %d kB\t(%d MB)\n\
 per-proc sgmts: %d\n\
 per-node sgmts: %d\n\
 per-proc cncts: %d\n\
 per-node cncts: %d\n",
 		    proc_rank, proc_size, reg_cnt, reg_size/1024, nodename,
		    reg_size/(1024*proc_size), reg_size/(1024*1024*proc_size),
		    reg_cnt*reg_size/(1024*proc_size), reg_cnt*reg_size/(1024*1024*proc_size),
		    reg_cnt*local_proc_size*(reg_size/(1024*proc_size)), reg_cnt*local_proc_size*(reg_size/(1024*1024*proc_size)),
		    reg_cnt*(reg_size/1024), reg_cnt*(reg_size/(1024*1024)),
		    proc_size*reg_cnt,
		    local_proc_size*reg_cnt,
		    (proc_size - 1)*reg_cnt,
		    local_proc_size*reg_cnt*(proc_size-local_proc_size)+(local_proc_size-1)*reg_cnt*local_proc_size);

	    fflush (stdout);
	}
	usleep (10000);
    }
    
    if (err != SMI_SUCCESS)
	SMI_Abort (0);

    while (reg_cnt > 0) {
	SMI_Free_shreg (shreg_id[--reg_cnt]);
    }
    free (shreg_addr);

    return 0;
}
    
	
