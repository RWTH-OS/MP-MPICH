/* $Id$ */

/* test the performance of strided access to remote regions */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "smi.h"

#define REGION_SIZE (1024*1024)
#define ACCESS_SIZE 512
#define MAX_STRIDE  1024
#define REG_RANK    1
#define ALIGNMENT   32

/* test a region via a read-write access to each page of the region */
static void access_test (char *addr, int size, int access_size, int max_stride)
{
    int page_size, nbr_pages, offset = 0, access_cnt = 0, len = access_size, stride;
    char *buf;
    double t0;

    buf = (char *)malloc(size);
    
    printf ("# N_acc \t len \t stride \t B [MB/s]\n");
    for (stride = access_size; stride < max_stride; stride += 8) {
	access_cnt = 0;
	offset = 0;
	t0 = SMI_Wtime();
	while (offset + len <= size) {
	    SMI_Memcpy (addr + offset, buf + offset, len, 0);
	    offset += stride;
	    access_cnt++;
	}
	t0 = SMI_Wtime() - t0;
	printf ("%d \t\t %d \t %d \t %f \n", access_cnt, len, stride, (access_cnt*len)/(1024*1024*t0));
    }

    free (buf);
    return;
}

static void test_undivided (int reg_size, int rank, int access_size, int max_stride, int alignment) 
{
    smi_region_info_t reginfo;
    smi_error_t err;
    int shreg_id;
    char *shreg_addr;
    
    if (rank == 0) {
	printf ("# strided access to remote region, size %d kB\n", reg_size/1024);
	fflush (stdout);
    } 
    
    SMI_Init_reginfo (&reginfo, reg_size, 0, REG_RANK, SMI_ADPT_DEFAULT, 0, 0, NULL);
    err = SMI_Create_shreg(SMI_SHM_UNDIVIDED|SMI_SHM_NONFIXED, &reginfo, &shreg_id, (void **)&shreg_addr);
    if (err != SMI_SUCCESS) {
	if (rank == REG_RANK) {
	    printf ("###  creation failed for process %d with SMI error %d\n", rank, err);
	    fflush (stdout);
	} 
	shreg_id = -1;
    } else {
	shreg_addr += (alignment - (((unsigned long)shreg_addr) % alignment));
	/* test region by accessing it */
	if (REG_RANK != rank)
	    access_test(shreg_addr, reg_size - alignment, access_size, max_stride);
    }
    SMI_Barrier();
    
    if (shreg_id != -1) {
	SMI_Free_shreg(shreg_id);
    }

    return;
}    

int main (int argc, char *argv[]) 
{
    int rank, size, reg_size, access_size, max_stride, alignment;
    char c;

    SMI_Init(&argc, &argv);

    SMI_Proc_rank(&rank);
    SMI_Proc_size(&size);

    /* default values */
    reg_size = REGION_SIZE;
    max_stride = MAX_STRIDE;
    access_size = ACCESS_SIZE;
    alignment = ALIGNMENT;

    while((c = getopt(argc, argv, "s:a:l:r:?")) != EOF) {
	switch(c) {
	case 'r':
	    reg_size = atoi(optarg) * 1024 + alignment;
	    break;
	case 'l':
	    access_size = atoi(optarg);
	    break;
	case 'a':
	    alignment = atoi(optarg);
	    break;
	case 's':
	    max_stride = atoi(optarg);
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

    test_undivided(reg_size, rank, access_size, max_stride, alignment);

    SMI_Finalize();
    return 0;
}
