/* $Id$ */

/* test SMI_Put and other DMA operations  */

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "smi.h"

#define REGION_SIZE (1*1024*1024+4)
#define LOOPS       100


int sender(int, int, int);
int receiver(int, int, int);

extern char *optarg;
int verbose = 0;

int main (int argc, char *argv[]) 
{
    int my_rank, size, nodes, c, reg_size, loops;

    SMI_Init(&argc, &argv);
    SMI_Proc_rank (&my_rank);
    SMI_Proc_size (&size);
    SMI_Node_size (&nodes);

    if (size != 2 || nodes != 2) {
	fprintf (stderr, "Must use two processes on two different nodes.\n");
	SMI_Finalize();
    }

    loops = LOOPS;
    reg_size = REGION_SIZE;
    while((c = getopt(argc, argv, "s:l:v?")) != EOF) {
	switch(c) {
	case 's':
	    reg_size = atoi(optarg);
	    break;
	case 'l':
	    loops = atoi(optarg);
	    break;
	case 'v':
	    verbose = 1;
	    break;
	case '?':
	    if (my_rank == 0) {
		printf("'%s' tests the registering of user buffers and DMA from and to them.\n", argv[0]);
		printf("usage: %s [-s region_size] [-l loops] [-?]\n", argv[0]);
	    }
	    SMI_Finalize();
	    return 0;
	    break;
	}
    }


    if (my_rank == 0) 
	sender(my_rank, reg_size, loops);
    else 
	receiver(my_rank, reg_size, loops);

    SMI_Finalize();
    return 0;
}

int sender (int my_rank, int reg_size, int loops)
{
    smi_region_info_t reg_info;
    smi_error_t smi_error;
    int *local_buffer, *rmt_buffer;
    int i, l, reg_id, rmt_reg_id, rmt_sgmt_id;
    double cpy_time;
    smi_memcpy_handle mc_handle = NULL;

    SMI_Init_reginfo (&reg_info, reg_size, 0, my_rank, SMI_ADPT_DEFAULT, 0, 0, NULL);
    smi_error = SMI_Create_shreg(SMI_SHM_LOCAL, &reg_info, &reg_id, (void **)&local_buffer);
    if (smi_error != SMI_SUCCESS) {
	printf ("[%d] local buffer could not be created (SMI errror %d)\n", my_rank, smi_error);
	SMI_Abort(-1);
    } 
    for (i = 0; i < reg_size/sizeof(int); i++)
	local_buffer[i] = 17*i + 13;
    
    SMI_Recv (&rmt_sgmt_id, sizeof(int), 1-my_rank);
    SMI_Init_reginfo (&reg_info, reg_size, 0, 1-my_rank, SMI_ADPT_DEFAULT, 0, rmt_sgmt_id, NULL);
    smi_error = SMI_Create_shreg (SMI_SHM_REMOTE, &reg_info, &rmt_reg_id, (void **)&rmt_buffer);
    if (smi_error != SMI_SUCCESS) {
	printf ("[%d] REMOTE region could not be created (SMI errror %d)\n", my_rank, smi_error);
	SMI_Abort(-1);
    } 

    cpy_time = SMI_Wtime();
    for (l = 0; l < loops; l++) {
	/* tranfer the data of this region to the remote region */
	if (verbose)
	    printf ("[%d] transfering data using DMA.\n", my_rank); fflush (stdout);
	SMI_Imemcpy (rmt_buffer, local_buffer, reg_size, SMI_MEMCPY_LS_RS, &mc_handle);
	SMI_Memwait (mc_handle);
	SMI_Signal_send ((1-my_rank)|SMI_SIGNAL_ANY);
	SMI_Signal_wait(SMI_SIGNAL_ANY);
    }
    cpy_time = SMI_Wtime() -  cpy_time;
    printf ("[%d] transfering %d blocks of %d kB (%d bytes) using SMI_Imemcpy() took %f sec (%f MB/s).\n", 
	    my_rank, loops, reg_size/1024, reg_size, cpy_time, 
	    (loops*reg_size)/(1024*1024*cpy_time)); fflush (stdout);
    SMI_Free_shreg (rmt_reg_id);

    /* now do the same, but use the remote region in RDMA mode */
    SMI_Init_reginfo (&reg_info, reg_size, 0, 1-my_rank, SMI_ADPT_DEFAULT, 0, rmt_sgmt_id, NULL);
    smi_error = SMI_Create_shreg (SMI_SHM_RDMA, &reg_info, &rmt_reg_id, (void **)&rmt_buffer);
    if (smi_error != SMI_SUCCESS) {
	printf ("[%d] RDMA region could not be created (SMI errror %d)\n", my_rank, smi_error);
	SMI_Abort(-1);
    } 
    
    cpy_time = SMI_Wtime();
    for (l = 0; l < loops; l++) {
	/* tranfer the data of this region to the remote region */
	if (verbose)
	    printf ("[%d] transfering data using RDMA.\n", my_rank); fflush (stdout);
	SMI_Put (rmt_reg_id, 0, local_buffer, reg_size);
	SMI_Signal_send ((1-my_rank)|SMI_SIGNAL_ANY);
	SMI_Signal_wait(SMI_SIGNAL_ANY);
    }
    cpy_time = SMI_Wtime() -  cpy_time;
    SMI_Free_shreg (rmt_reg_id);
    printf ("[%d] transfering %d blocks of %d kB (%d bytes) using SMI_Put() took %f sec (%f MB/s).\n", 
	    my_rank, loops, reg_size/1024, reg_size, cpy_time, 
	    (loops*reg_size)/(1024*1024*cpy_time)); fflush (stdout);
    
    SMI_Free_shreg (reg_id);
    SMI_Barrier();

    return 0;
}

int receiver (int my_rank, int reg_size, int loops)
{
    smi_region_info_t reg_info;
    smi_error_t smi_error;
    int *local_buffer, reg_id, sgmt_id, i, l;

    SMI_Init_reginfo (&reg_info, reg_size, 0, my_rank, SMI_ADPT_DEFAULT, 0, 0, NULL);
    smi_error = SMI_Create_shreg(SMI_SHM_LOCAL, &reg_info, &reg_id, (void **)&local_buffer);
    if (smi_error != SMI_SUCCESS) {
	printf ("[%d] local region could not be created (SMI errror %d)\n", my_rank, smi_error);
	SMI_Abort(-1);
    } 
    for (i = 0; i < reg_size/sizeof(int); i++)
	local_buffer[i] = 0;
    
    SMI_Query (SMI_Q_SMI_REGION_SGMT_ID, reg_id, &sgmt_id);
    SMI_Send (&sgmt_id, sizeof(int), 1-my_rank);
    
    for (l = 0; l < 2* loops; l++) {
	/* wait for transmission of data, then check data */
	SMI_Signal_wait(SMI_SIGNAL_ANY);
	for (i = 0; i < reg_size/sizeof(int); i++)
	    if (local_buffer[i] != 17*i + 13)
		fprintf (stderr, "[%d] remote: error at ((int *)buffer)[%d]: expected %d, got %d\n", 
			 my_rank, i, 17*i + 13, local_buffer[i]);
	if (verbose)
	    printf ("[%d] remote verification (%d bytes) finished.\n", my_rank, reg_size); fflush (stdout);
	memset (local_buffer, 0, reg_size);
	SMI_Signal_send(1-my_rank|SMI_SIGNAL_ANY);
    }

    SMI_Free_shreg (reg_id);
    SMI_Barrier();
    return 0;
}
