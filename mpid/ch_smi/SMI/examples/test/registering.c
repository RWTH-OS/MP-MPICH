/* $Id$ */
/* registering.c - register user-allocated buffer with SCI */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "smi.h"

#define REGION_SIZE (1*1024*1024)
#define LOOPS       1


int sender(int, int, int, int);
int receiver(int, int, int, int);

extern char *optarg;
int verbose = 0;

int main (int argc, char *argv[]) 
{
    int my_rank, size, nodes, c, reg_size, loops, offset;

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
    offset = 0;
    while((c = getopt(argc, argv, "s:l:o:v?")) != EOF) {
	switch(c) {
	case 's':
	    reg_size = atoi(optarg);
	    break;
	case 'l':
	    loops = atoi(optarg);
	    break;
	case 'o':
	    offset = atoi(optarg);
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
	sender(my_rank, reg_size, offset, loops);
    else 
	receiver(my_rank, reg_size, offset, loops);

    SMI_Finalize();
    return 0;
}

int sender (int my_rank, int reg_size, int offset, int loops)
{
    smi_region_info_t reg_info;
    smi_error_t smi_error;
    int *user_buffer, *rmt_buffer;
    int i, l, reg_id, rmt_reg_id, rmt_sgmt_id;
    double cpy_time;
    smi_memcpy_handle mc_handle = NULL;

    user_buffer = (int *)malloc(reg_size+offset);
    if (verbose) {
	printf ("[%d] user buffer = 0x%x, size = 0x%x, cpy_size = 0x%x, offset = 0x%x\n", my_rank, 
		user_buffer, reg_size+offset, reg_size, offset);
	fflush (stdout);
    }

    for (i = 0; i < (reg_size+offset)/sizeof(int); i++)
	user_buffer[i] = 17*i + 13;
    
    SMI_Init_reginfo (&reg_info, reg_size+offset, 0, my_rank, SMI_ADPT_DEFAULT, 0, 0, NULL);
    smi_error = SMI_Create_shreg(SMI_SHM_LOCAL|SMI_SHM_REGISTER|SMI_SHM_PRIVATE, &reg_info, &reg_id, 
				 (void **)&user_buffer);
    if (smi_error != SMI_SUCCESS) {
	printf ("[%d] registered region could not be created (SMI errror %d)\n", my_rank, smi_error);
	SMI_Abort(-1);
    } 
    if (verbose)
	printf ("[%d] registered region = 0x%x\n", my_rank, user_buffer); fflush (stdout);
    for (i = 0; i < (reg_size+offset)/sizeof(int); i++)
	if (user_buffer[i] != 17*i + 13)
	    fprintf (stderr, "[%d] local: error at ((int *)buffer)[%d]: expected %d, got %d\n", my_rank, 
		     i, 17*i + 13, user_buffer[i]);
    if (verbose)
	printf ("[%d] local verification finished.\n", my_rank); fflush (stdout);
   
    SMI_Recv (&rmt_sgmt_id, sizeof(int), 1-my_rank);
    cpy_time = SMI_Wtime();
    for (l = 0; l < loops; l++) {
	SMI_Init_reginfo (&reg_info, 0, 0, 1-my_rank, SMI_ADPT_DEFAULT, 0, rmt_sgmt_id, NULL);
	smi_error = SMI_Create_shreg (SMI_SHM_REMOTE, &reg_info, &rmt_reg_id, (void **)&rmt_buffer);
	if (smi_error != SMI_SUCCESS) {
	    printf ("[%d] REMOTE region could not be created (SMI errror %d)\n", my_rank, smi_error);
	    SMI_Abort(-1);
	} 
	/* tranfer the data of this region to the remote region */
	if (verbose)
	    printf ("[%d] transfering data from local registered to rmt. contiguous by mapped DMA.\n", 
		    my_rank); fflush (stdout);
	mc_handle = NULL;
	SMI_Imemcpy ((char *)rmt_buffer + offset, (char *)user_buffer + offset, reg_size, 
		     SMI_MEMCPY_LS_RS, &mc_handle);
	SMI_Memwait (mc_handle);
	SMI_Signal_send ((1-my_rank)|SMI_SIGNAL_ANY);
	SMI_Free_shreg (rmt_reg_id);
	SMI_Signal_wait(SMI_SIGNAL_ANY);
    }
    cpy_time = SMI_Wtime() -  cpy_time;
    printf ("[%d] transfering %d blocks of %d kB using SMI_Imemcpy() was successful.\n", 
	    my_rank, loops, reg_size/1024); fflush (stdout);

    /* now do the same, but use the remote region in RDMA mode */
    cpy_time = SMI_Wtime();
    for (l = 0; l < loops; l++) {
	SMI_Init_reginfo (&reg_info, 0, 0, 1-my_rank, SMI_ADPT_DEFAULT, 0, rmt_sgmt_id, NULL);
	smi_error = SMI_Create_shreg (SMI_SHM_RDMA, &reg_info, &rmt_reg_id, (void **)&rmt_buffer);
	if (smi_error != SMI_SUCCESS) {
	    printf ("[%d] RDMA region could not be created (SMI errror %d)\n", my_rank, smi_error);
	    SMI_Abort(-1);
	} 
	/* tranfer the data of this region to the remote region */
	if (verbose)
	    printf ("[%d] transfering data from local registered to rmt. contiguous by RDMA.\n", 
		    my_rank); fflush (stdout);
	SMI_Put (rmt_reg_id, offset, (char *)user_buffer +  offset, reg_size);
	SMI_Signal_send ((1-my_rank)|SMI_SIGNAL_ANY);
	SMI_Free_shreg (rmt_reg_id);
	SMI_Signal_wait(SMI_SIGNAL_ANY);
    }
    cpy_time = SMI_Wtime() -  cpy_time;
    printf ("[%d] transfering %d blocks of %d kB using SMI_Put() was successful).\n", 
	    my_rank, loops, reg_size/1024); fflush (stdout);
    
    /* now do the same, but use the remote region, which is a user-allcocated, SCI-registered
       buffer, in RDMA mode */
    SMI_Recv (&rmt_sgmt_id, sizeof(int), 1-my_rank);
    cpy_time = SMI_Wtime();
    for (l = 0; l < loops; l++) {
	SMI_Init_reginfo (&reg_info, 0, 0, 1-my_rank, SMI_ADPT_DEFAULT, 0, rmt_sgmt_id, NULL);
	smi_error = SMI_Create_shreg (SMI_SHM_RDMA, &reg_info, &rmt_reg_id, (void **)&rmt_buffer);
	if (smi_error != SMI_SUCCESS) {
	    printf ("[%d] RDMA region could not be created (SMI errror %d)\n", my_rank, smi_error);
	    SMI_Abort(-1);
	} 
	/* tranfer the data of this region to the remote region */
	if (verbose)
	    printf ("[%d] transfering data from local registered to rmt. registered by RDMA.\n", 
		    my_rank); fflush (stdout);
	SMI_Put (rmt_reg_id, offset, (char *)user_buffer + offset, reg_size);
	SMI_Signal_send ((1-my_rank)|SMI_SIGNAL_ANY);
	SMI_Free_shreg (rmt_reg_id);
	SMI_Signal_wait(SMI_SIGNAL_ANY);
    }
    cpy_time = SMI_Wtime() -  cpy_time;
    printf ("[%d] transfering %d blocks of %d kB using SMI_Put() was successful.\n", 
	    my_rank, loops, reg_size/1024); fflush (stdout);

    SMI_Free_shreg (reg_id);
    free (user_buffer);

    SMI_Barrier();
    return 0;
}

int receiver (int my_rank, int reg_size, int offset, int loops)
{
    smi_region_info_t reg_info;
    smi_error_t smi_error;
    int *user_buffer, reg_id, sgmt_id, i, l;

    /* create a normal SCI segment and let the remote process write data into it*/
    SMI_Init_reginfo (&reg_info, reg_size + offset, 0, my_rank, SMI_ADPT_DEFAULT, 0, 0, NULL);
    smi_error = SMI_Create_shreg(SMI_SHM_LOCAL, &reg_info, &reg_id, (void **)&user_buffer);
    if (smi_error != SMI_SUCCESS) {
	printf ("[%d] local region could not be created (SMI errror %d)\n", my_rank, smi_error);
	SMI_Abort(-1);
    } 
    memset (user_buffer, 0, reg_size+offset);
    
    SMI_Query (SMI_Q_SMI_REGION_SGMT_ID, reg_id, &sgmt_id);
    SMI_Send (&sgmt_id, sizeof(int), 1-my_rank);
    
    for (l = 0; l < 2*loops; l++) {
	/* wait for transmission of data, then check data */
	SMI_Signal_wait(SMI_SIGNAL_ANY);
	for (i = offset/sizeof(int); i < (reg_size + offset)/sizeof(int); i++)
	    if (user_buffer[i] != 17*i + 13)
		fprintf (stderr, "[%d] remote access: error at ((int *)buffer)[%d]: expected %d, got %d\n", my_rank, 
			 i, 17*i + 13, user_buffer[i]);
	if (verbose)
	    printf ("[%d] remote access verification finished.\n", my_rank); fflush (stdout);
	memset (user_buffer, 0, reg_size+offset);
	SMI_Signal_send(1-my_rank|SMI_SIGNAL_ANY);
    }

    SMI_Free_shreg (reg_id);

    /* now allocate a user buffer, register & export it to let the remote process
       write data in it */
    user_buffer = (int *)malloc(reg_size+offset);
    if (verbose) {
	printf ("[%d] user buffer = 0x%x, size = 0x%x\n", my_rank, user_buffer, reg_size+offset); 
	fflush (stdout);
    }

    for (i = 0; i < (reg_size + offset)/sizeof(int); i++)
	user_buffer[i] = 17*i + 13;
    
    SMI_Init_reginfo (&reg_info, reg_size+offset, 0, my_rank, SMI_ADPT_DEFAULT, 0, 0, NULL);
    smi_error = SMI_Create_shreg(SMI_SHM_LOCAL|SMI_SHM_REGISTER, &reg_info, &reg_id, 
				 (void **)&user_buffer);
    if (smi_error != SMI_SUCCESS) {
	printf ("[%d] registered non-private region could not be created (SMI errror %d)\n", my_rank, smi_error);
	SMI_Abort(-1);
    } 
    if (verbose)
	printf ("[%d] registered region = 0x%x\n", my_rank, user_buffer); fflush (stdout);
    for (i = 0; i < (reg_size + offset)/sizeof(int); i++)
	if (user_buffer[i] != 17*i + 13)
	    fprintf (stderr, "[%d] local: error at ((int *)buffer)[%d]: expected %d, got %d\n", my_rank, 
		     i, 17*i + 13, user_buffer[i]);
    if (verbose)
	printf ("[%d] local access verification finished.\n", my_rank); fflush (stdout);
    memset (user_buffer, 0, reg_size+offset);
    
    SMI_Query (SMI_Q_SMI_REGION_SGMT_ID, reg_id, &sgmt_id);
    SMI_Send (&sgmt_id, sizeof(int), 1-my_rank);
    
    for (l = 0; l < loops; l++) {
	/* wait for transmission of data, then check data */
	SMI_Signal_wait(SMI_SIGNAL_ANY);
	for (i = offset/sizeof(int); i < (reg_size + offset)/sizeof(int); i++)
	    if (user_buffer[i] != 17*i + 13)
		fprintf (stderr, "[%d] remote: error at ((int *)buffer)[%d]: expected %d, got %d\n", my_rank, 
			 i, 17*i + 13, user_buffer[i]);
	if (verbose)
	    printf ("[%d] remote access verification finished.\n", my_rank); fflush (stdout);
	memset (user_buffer, 0, reg_size + offset);
	SMI_Signal_send(1-my_rank|SMI_SIGNAL_ANY);
    }

    SMI_Free_shreg (reg_id);
    
    SMI_Barrier();
    return 0;
}
