/* $Id$ */
/* sync.c - test the synchronization by mutex, barriers & progress counters */

#include <stdio.h>
#include <string.h>

#include "smi.h"

#define SHREG_SIZE (1*4096)
#define LOCK_LOOPS 1000


/* #define LOCKTYPE L_MUTEX    /* A Mutex-Algorithm from Leslie Lamport            */
#define LOCKTYPE BL_MUTEX   /* A Mutex-Algorithm from Leslie Lamport            */
/* #define LOCKTYPE SCH_MUTEX  /* A Mutex-Algorithm from Martin Schulz             */ 

int main (int argc, char *argv[]) 
{
    smi_region_info_t reginfo;
    int rank, size, lock_id, shreg_id, i, j, lock_errors;
    int *shreg_addr, *error_cnts, val;
    double avg_time, time, *timings;

    SMI_Init(&argc, &argv);

    SMI_Proc_rank(&rank);
    SMI_Proc_size(&size);

    SMI_Init_reginfo (&reginfo, SHREG_SIZE, 0, 0, SMI_ADPT_DEFAULT, 0, 0, NULL);
    SMI_Create_shreg (SMI_SHM_UNDIVIDED|SMI_SHM_NONFIXED, &reginfo, &shreg_id, (void **)&shreg_addr);

    SMI_MUTEX_INIT(&lock_id, LOCKTYPE, -1);
  
    /* test performance of locks: without locality */
    timings = (double *)shreg_addr;
    if (rank == 0) {
	printf ("*** testing lock performance (w/o locality)\n");
	fflush (stdout);
	memset ((void *)shreg_addr, 0, SHREG_SIZE);
    }
    
    SMI_Barrier();

    time = SMI_Wtime();
    for (i = 0; i < LOCK_LOOPS; i++) {
	SMI_Mutex_lock(lock_id);
	SMI_Mutex_unlock(lock_id);
    }
    time = SMI_Wtime() - time;

    timings[rank] = time;
    SMI_Barrier();

    if (rank == 0) {
	avg_time = 0;
	for (i = 0; i < size; i++)
	    avg_time += timings[i];
	avg_time /= (size*LOCK_LOOPS);

	printf (">>> avg. duration of lock/unlock operation is %6.3f us for %d processes \n", 
		avg_time*1e+6, size); 
	fflush(stdout);
    }

    /* test performance of locks: with locality */
    if (rank == 0) {
	printf ("\n*** testing lock performance (w/ locality)\n");
	fflush (stdout);
	memset ((void *)shreg_addr, 0, SHREG_SIZE);
    }
    SMI_Barrier();
    
    
    if (rank == 0) {
	printf (">>> avg. duration of local lock/unlock operation is %6.3f us\n"); 
	printf (">>> avg. duration of remote lock/unlock operation is %6.3f us\n"); 
	fflush(stdout);
    }


    /* test funcitonality of locks. It is hard to prove that locks really work; 
       instead, we try to see if they fail. If they do not fail, chances are
       be good the locks work alright. 
       We could also measure the time this testing needs.
    */
    lock_errors = 0;
    
    if (rank == 0) {
	printf ("\n*** testing lock functionality\n");
	fflush (stdout);
	memset ((void *)shreg_addr, 0, SHREG_SIZE);
    }
    SMI_Barrier();
    
    for (i = 0; i < LOCK_LOOPS; i++) {
	SMI_Mutex_lock(lock_id);
	
	/* check for inconsistencies and increment contents */
	val = shreg_addr[0];
	for (j = 0; j < SHREG_SIZE/sizeof(int); j++) {
	    if (shreg_addr[j] != val) {
		lock_errors++;
	    }
	    shreg_addr[j] = val + 1;
	}

	SMI_Flush_write(SMI_FLUSH_ALL);
	
	/* verify what has been written */
	for (j = 0; j < SHREG_SIZE/sizeof(int); j++) 
	    if (shreg_addr[j] != val + 1) {
		lock_errors++;
	    }
	
	SMI_Mutex_unlock(lock_id);
    }

    SMI_Barrier();
    /* all processes write their error counters into the shared memory */
    error_cnts = (int *)shreg_addr;
    error_cnts[rank] = lock_errors;
    SMI_Barrier();

    if (rank == 0) {	
	/* read out error counters */
	for (i = 0; i < size; i++)
	    lock_errors += error_cnts[i];

	if (lock_errors == 0) 
	    printf (">>> locks seem to work fine\n");
	else
	    printf ("### %d lock errors detected!\n", lock_errors);
	fflush (stdout);
    }
    

    SMI_Mutex_destroy(lock_id);
    SMI_Free_shreg(shreg_id);
    
    SMI_Finalize();
}
