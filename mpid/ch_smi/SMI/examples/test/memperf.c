/* $Id: memperf.c,v 1.1 2004/03/19 22:14:46 joachim Exp $ */
/* memperf.c - determine the basic remote memory performance values:
   - integer latency read/write
   - pingpong latency write
   - PIO write bandwidth
   - PIO read bandwidth
   - DMA write bandwidth (chained/unchained)
*/

#include <stdio.h>
#include <unistd.h>

#include "smi.h"

/* these values may be modified, but should be multiples of 4k, and
   SHREG_SIZE must be a multiple of MEM_BLOCK_WRITE. A bigger MEM_LOOPS
   may increase accuracy, but the default value of 100 should do fine. */
#define SHREG_SIZE (1*1024*1024)
#define MEM_LOOPS 100
#define MEM_BLOCK_WRITE (32*4096)
#define MEM_BLOCK_READ  (1*4096)

int main (int argc, char *argv[]) 
{
    smi_region_info_t reginfo;
    smi_memcpy_handle mcpy_h = NULL;
    int rank, size, lock_id, shreg_id[2], i, j, lock_errors;
    int *shreg_addr[2], *error_cnts, val;
    double avg_time, time, *timings;

    SMI_Init(&argc, &argv);

    SMI_Proc_rank(&rank);
    SMI_Proc_size(&size);

    if (size != 2) {
	if (rank == 0) {
	    printf ("please run %s with 2 processes\n", argv[0]);
	}
	SMI_Abort(-1);
    }

    for (i = 0; i < size; i++) {
	SMI_Init_reginfo (&reginfo, SHREG_SIZE, 0, i, SMI_ADPT_DEFAULT, 0, 0, NULL);
	if (SMI_Create_shreg (SMI_SHM_UNDIVIDED|SMI_SHM_NONFIXED, &reginfo, &shreg_id[i],
			      (void **)&shreg_addr[i]) != SMI_SUCCESS) {
	    j = SHREG_SIZE >> 10;
	    printf ("[%d] Could not allocate %d kB of shared memory.\n", rank, j);
	    SMI_Abort(-1);
	}
    }
    timings = (double *)shreg_addr[0];

    /*  write latency for integer write, no flushing */
    if (rank == 0) {
	printf ("*** testing latency for integer write, no flushing\n");
	fflush (stdout);
    } 
    val = SHREG_SIZE/sizeof(int);
    shreg_addr[rank][0] = 0;

    SMI_Barrier();
    time = SMI_Wtime();
    for (j = 0; j < 10; j++) {
	if (rank == 0) {
	    /* copy data to remote memory */
	    for (i = 0; i < val; i++) {
		shreg_addr[1-rank][0] = i;
	    }
	    
	    /* poll for new value */
	    while (shreg_addr[rank][0] < val-1)
		;
	    shreg_addr[rank][0] = 0;
	} else {
	    /* poll for new value */
	    while (shreg_addr[rank][0] < val-1)
		;
	    shreg_addr[rank][0] = 0;
	    
	    /* copy data to remote memory */
	    for (i = 0; i < val; i++) {
		shreg_addr[1-rank][0] = i;
	    }
	}
    }
    time = SMI_Wtime() - time;
    SMI_Barrier();

    timings[rank] = time;
    SMI_Barrier();
    if (rank == 0) {
	avg_time = 0;
	for (i = 0; i < size; i++) {
    	    avg_time += timings[i];
	}
	avg_time /= (size*2*val*10);

	printf (">>> avg. duration of remote integer write is %6.3f us\n", avg_time*1e+6);
	fflush(stdout);
    }

    /*  write latency for integer read, no flushing */
    if (rank == 0) {
	printf ("\n*** testing latency for integer read\n");
	fflush (stdout);
    } 
    shreg_addr[rank][0] = 0;
    avg_time = 0;
    /* set the number of remote reads to 0 */
    i = 0;

    SMI_Barrier();
    for (j = 0; j < 5; j++) {
	if (rank == 0) {
	    /* poll on data from remote memory */
	    val = 0;
	    time = SMI_Wtime();
	    while (!val) {
		val = shreg_addr[1-rank][0];
		i++;
	    }
	    time = SMI_Wtime() - time;
	    avg_time += time;
	    shreg_addr[1-rank][0] = 0;
	    
	    /* let the remote process poll for a while, then set the flag */
	    sleep(1);
	    shreg_addr[rank][0] = 1;
	} else {
	    sleep(1);
	    shreg_addr[rank][0] = 1;
	    
	    val = 0;
	    time = SMI_Wtime();
	    while (!val) {
		val = shreg_addr[1-rank][0];
		i++;
	    }
	    time = SMI_Wtime() - time;
	    avg_time += time;
	    shreg_addr[1-rank][0] = 0;
	}
    }
    SMI_Barrier();

    timings[rank] = avg_time/i;
    SMI_Barrier();
    if (rank == 0) {
	avg_time = 0;
	for (i = 0; i < size; i++) {
    	    avg_time += timings[i];
	}
	avg_time /= size;

	printf (">>> avg. duration of remote integer read is %6.3f us\n", avg_time*1e+6);
	fflush(stdout);
    }

    /*  ping-pong latency for integer write, no flushing */
    if (rank == 0) {
	printf ("\n*** testing ping-pong latency for integer write, no flushing\n");
	fflush (stdout);
	shreg_addr[rank][0] = rank;
    } else {
	shreg_addr[rank][0] = 1-rank;
    }
    val = 1-rank;

    SMI_Barrier();
    time = SMI_Wtime();
    for (i = 0; i < MEM_LOOPS; i++) {
	/* poll for new value */
	while (shreg_addr[rank][0] != rank)
	    ;
	shreg_addr[rank][0] = val;
	shreg_addr[val][0]  = val;
    }
    time = SMI_Wtime() - time;
    SMI_Barrier();

    timings[rank] = time;
    SMI_Barrier();
    if (rank == 0) {
	avg_time = 0;
	for (i = 0; i < size; i++)
	    avg_time += timings[i];
	avg_time /= (size*2*MEM_LOOPS);

	printf (">>> avg. duration of remote integer write (no flushing) is %6.3f us\n", avg_time*1e+6);
	fflush(stdout);
    }

    /*  ping-pong latency for integer write, with flushing by non-contig writet*/
    if (rank == 0) {
	printf ("\n*** testing ping-pong latency for integer write, flushing by non-contig write\n");
	fflush (stdout);
	shreg_addr[rank][0] = rank;
    } else {
	shreg_addr[rank][0] = 1-rank;
    }
    val = 1-rank;

    SMI_Barrier();
    time = SMI_Wtime();
    for (i = 0; i < MEM_LOOPS; i++) {
	/* poll for new value */
	while (shreg_addr[rank][0] != rank)
	    ;
	shreg_addr[rank][0] = val;
	shreg_addr[val][0]  = val;
	/* try to flush by this non-contignous write */
	shreg_addr[val][2]  = val;
    }
    time = SMI_Wtime() - time;
    SMI_Barrier();

    timings[rank] = time;
    SMI_Barrier();
    if (rank == 0) {
	avg_time = 0;
	for (i = 0; i < size; i++)
	    avg_time += timings[i];
	avg_time /= (size*2*MEM_LOOPS);

	printf (">>> avg. duration of remote integer write (with flushing) is %6.3f us\n", avg_time*1e+6);
	fflush(stdout);
    }

    /* test PIO write bandwidth */
    if (rank == 0) {
	printf ("\n*** testing peak PIO write bandwidth\n");
	fflush (stdout);
    }

    SMI_Barrier();
    time = SMI_Wtime();
    for (i = 0; i < MEM_LOOPS; i++) {
	if (rank == 1)
	    SMI_Barrier();

	SMI_Memcpy(shreg_addr[1-rank], shreg_addr[rank], MEM_BLOCK_WRITE, SMI_MEMCPY_LP_RS|SMI_MEMCPY_FAST);

	if (rank == 0) 
	    SMI_Barrier();

	SMI_Barrier();
    }
    time = SMI_Wtime() - time;
    SMI_Barrier();
    
    timings[rank] = time;
    SMI_Barrier();
    if (rank == 0) {
	avg_time = 0;
	for (i = 0; i < size; i++)
	    avg_time += timings[i];
	avg_time /= size;
	
	printf (">>> peak bandwidth for PIO writes is %4.1f MB/s (%d kB blocks)\n", 
		(MEM_BLOCK_WRITE*MEM_LOOPS*2)/(avg_time*1024*1024), MEM_BLOCK_WRITE >> 10);
	fflush(stdout);
    }
    SMI_Barrier();

    /* test PIO read bandwidth */
    if (rank == 0) {
	printf ("\n*** testing peak PIO read bandwidth\n");
	fflush (stdout);
    }

    SMI_Barrier();
    time = SMI_Wtime();
    for (i = 0; i < MEM_LOOPS; i++) {
	if (rank == 1)
	    SMI_Barrier();

	SMI_Memcpy(shreg_addr[rank], shreg_addr[1-rank], MEM_BLOCK_READ, SMI_MEMCPY_LS_RS|SMI_MEMCPY_FAST);

	if (rank == 0) 
	    SMI_Barrier();

	SMI_Barrier();
    }
    time = SMI_Wtime() - time;
    SMI_Barrier();
    
    timings[rank] = time;
    SMI_Barrier();
    if (rank == 0) {
	avg_time = 0;
	for (i = 0; i < size; i++)
	    avg_time += timings[i];
	avg_time /= size;
	
	printf (">>> peak bandwidth for reading is %4.1f MB/s (%d kB blocks)/s\n", 
		(MEM_BLOCK_READ*MEM_LOOPS*2)/(avg_time*1024*1024), MEM_BLOCK_READ >> 10);
	fflush(stdout);
    }
    SMI_Barrier();

    /* test DMA write bandwidth */
    if (rank == 0) {
	printf ("\n*** testing peak DMA write bandwidth\n");
	fflush (stdout);
    }

    SMI_Barrier();
    time = SMI_Wtime();
    for (i = 0; i < MEM_LOOPS; i++) {
	if (rank == 1)
	    SMI_Barrier();

	SMI_Imemcpy(shreg_addr[1-rank], shreg_addr[rank], MEM_BLOCK_WRITE, 
		    SMI_MEMCPY_LS_RS|SMI_MEMCPY_FAST, &mcpy_h);
	SMI_Memwait(mcpy_h);

	if (rank == 0) 
	    SMI_Barrier();

	SMI_Barrier();
    }
    time = SMI_Wtime() - time;
    SMI_Barrier();
    
    timings[rank] = time;
    SMI_Barrier();
    if (rank == 0) {
	avg_time = 0;
	for (i = 0; i < size; i++)
	    avg_time += timings[i];
	avg_time /= size;
	
	printf (">>> avg. bandwidth for DMA write is %4.1f MB/s (%d kB blocks)\n",  
		(MEM_BLOCK_WRITE*MEM_LOOPS*2)/(avg_time*1024*1024), MEM_BLOCK_WRITE>>10);
	fflush(stdout);
    }
    SMI_Barrier();

    /* test DMA write bandwidth (enqueued) */
    if (rank == 0) {
	printf ("\n*** testing peak DMA write bandwidth (enqueued transfers)\n");
	fflush (stdout);
    }

    SMI_Barrier();
    time = SMI_Wtime();
    for (i = 0; i < MEM_LOOPS; i++) {
	if (rank == 1)
	    SMI_Barrier();
	
	for (j = 0; j < SHREG_SIZE/MEM_BLOCK_WRITE - 1; j++) {
	    SMI_Imemcpy(&shreg_addr[1-rank][j*MEM_BLOCK_WRITE/sizeof(int)], 
			&shreg_addr[rank][j*MEM_BLOCK_WRITE/sizeof(int)], MEM_BLOCK_WRITE, 
			SMI_MEMCPY_LS_RS|SMI_MEMCPY_FAST|SMI_MEMCPY_ENQUEUE, &mcpy_h);
	}
	SMI_Imemcpy(&shreg_addr[1-rank][j*MEM_BLOCK_WRITE/sizeof(int)], 
		    &shreg_addr[rank][j*MEM_BLOCK_WRITE/sizeof(int)], MEM_BLOCK_WRITE, 
		    SMI_MEMCPY_LS_RS|SMI_MEMCPY_FAST, &mcpy_h);
	SMI_Memwait(mcpy_h);

	if (rank == 0) 
	    SMI_Barrier();

	SMI_Barrier();
    }
    time = SMI_Wtime() - time;
    SMI_Barrier();
    
    timings[rank] = time;
    SMI_Barrier();
    if (rank == 0) {
	avg_time = 0;
	for (i = 0; i < size; i++)
	    avg_time += timings[i];
	avg_time /= size;
	
	printf (">>> avg. bandwidth for enqueued DMA write is %4.1f MB/s (%d kB blocks)\n",  
		(SHREG_SIZE*MEM_LOOPS*2)/(avg_time*1024*1024), MEM_BLOCK_WRITE>>10);
	fflush(stdout);
    }
    SMI_Barrier();


    SMI_Finalize();
    return 0;
}
