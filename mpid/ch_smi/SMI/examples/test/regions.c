/* $Id$ */
/* regions.c - test the creation of different kinds of shared memory regions */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "smi.h"

#define REGION_SIZE (1024*1024)

/* test a region via a read-write access to each page of the region */
static void access_test (char *addr, int size)
{
    int page_size, nbr_pages, i;

    SMI_Query(SMI_Q_SYS_PAGESIZE, 0, &page_size);
    nbr_pages = size/page_size;

    for (i = 0; i < size; i += page_size) {
	addr[i] = addr[i+1];
    }

    return;
}

static void test_undivided (int reg_size, int size, int rank) 
{
    smi_region_info_t reginfo;
    smi_error_t err;
    int *shreg_id, p;
    char **shreg_addr;
    int all_ok = 1;

    shreg_id   = (int *)malloc(size*sizeof(int));
    shreg_addr = (char **)malloc(size*sizeof(char *));

    /* one UNDIVIDED region for each process */
    if (rank == 0) {
	printf ("*** testing creation of non-fixed UNDIVIDED regions (one per process), size %dkB\n",
		reg_size/1024);
	fflush (stdout);
    } 
    for (p = 0; p < size; p++) {
	SMI_Init_reginfo (&reginfo, reg_size, 0, p, SMI_ADPT_DEFAULT, 0, 0, NULL);
	err = SMI_Create_shreg(SMI_SHM_UNDIVIDED|SMI_SHM_NONFIXED, &reginfo, &shreg_id[p], 
			       (void **)&shreg_addr[p]);
	if (err != SMI_SUCCESS) {
	    if (rank == p) {
		printf ("###  creation failed for process %d with SMI error %d\n", rank, err);
		fflush (stdout);
	    } 
	    shreg_id[p] = -1;
	    all_ok = 0;
	} else {
	    /* test region by accessing it */
	    access_test(shreg_addr[p], reg_size);
	}
    }

    for (p = 0; p < size; p++) {
	if (shreg_id[p] != -1) {
	    SMI_Free_shreg(shreg_id[p]);
	}
    }

    if (rank == 0) {
	if (all_ok == 1)
	    printf (">>> non-fixed UNDIVIDED regions are o.k.\n");
	else
	    printf ("### some regions could not be created - try a smaller size\n");
	fflush (stdout);
    } 
    
    return;
}    

int main (int argc, char *argv[]) 
{
    int rank, size, reg_size;
    char c;

    SMI_Init(&argc, &argv);

    SMI_Proc_rank(&rank);
    SMI_Proc_size(&size);

    /* default values */
    reg_size = REGION_SIZE;

    while((c = getopt(argc, argv, "s:?")) != EOF) {
	switch(c) {
	case 's':
	    reg_size = atoi(optarg) * 1024;
	    break;
	case '?':
	    if (rank == 0) {
		printf("'regions' tests the creation of different SMI region types.\n");
		printf("usage: regions [-s region_size in kB] [-?]\n");
	    }
	    SMI_Finalize();
	    return 0;
	    break;
	}
    }

    test_undivided(reg_size, size, rank);

    SMI_Finalize();
    return 0;
}
