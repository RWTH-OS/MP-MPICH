/* $Id$ */
/* alltoall.c - test the behaviour of the cluster under high communication load */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "smi.h"

/* parameters specifying the test size */
#define SEGSIZE     (1024*1024)
#define BLOCKSIZE   (128*1024)
#define LOOPS       200

#define NO_SYNC      0
#define LOCK_SYNC    1
#define SMP_SYNC     2
#define BARRIER_SYNC 3

char **shseg_addrs;
int *out_lock, *in_lock;
char *src_buf;
int rank, size, nrank, nsize, lsize;


static int get_lock(int dest)
{ 
    /* which locks do we need ? */ 
    int have_out_lock, have_in_lock, dest_node; 
    
    have_out_lock = (lsize < 2);  /* outgoing sync (local node) */ 
    have_in_lock  = 0;            /* ingoing sync (remote node) */ 
    SMI_Proc_to_node(dest, &dest_node);
    
    /* try to get the locks we need */ 
    if (!have_out_lock) 
	SMI_Mutex_trylock(out_lock[nrank], &have_out_lock); 
    if (!have_in_lock) 
	SMI_Mutex_trylock(in_lock[dest_node], &have_in_lock); 
    
    /* if we don't have all locks we need, we should do something useful */ 
    if (!(have_out_lock && have_in_lock)) { 
	if (have_out_lock && (lsize > 1)) { 
	    SMI_Mutex_unlock(out_lock[nrank]); 
	} 
	if (have_in_lock) { 
	    SMI_Mutex_unlock(in_lock[dest_node]); 
	} 
	return 0;
    }
    return 1;
}


static void return_lock(int dest)
{
    int dest_node;

    /* which locks do we own ? */ 
    if (lsize > 1) 
	SMI_Mutex_unlock(out_lock[nrank]); 

    SMI_Proc_to_node(dest, &dest_node);
    SMI_Mutex_unlock(in_lock[dest_node]);

    return;
}


static int alltoall(int do_sync)
{
    unsigned long offset, data;
    int dest, dest_node, p, l;

    data = 0;

    for (l = 0; l < LOOPS; l++) {	
	p = 0;
	dest = (rank + 1)%size;
	while (p < size) { 
	    SMI_Proc_to_node(dest, &dest_node);

	    if (dest_node != nrank) {
		switch (do_sync) {
		case BARRIER_SYNC:
		    break;
		case LOCK_SYNC:
		    SMI_Mutex_lock(in_lock[dest_node]);
		    break;
		case SMP_SYNC:
		    while (!get_lock(dest))
		    break;
		}
	    }

	    SMI_Memcpy(shseg_addrs[dest], src_buf, BLOCKSIZE, 
		       dest_node == nrank ? SMI_MEMCPY_LP_LP : SMI_MEMCPY_LP_RS);

	    if (dest_node != nrank) {
		switch (do_sync) {
		case BARRIER_SYNC:
		    SMI_Barrier();
		    break;
		case LOCK_SYNC:
		    SMI_Mutex_unlock(in_lock[dest_node]);
		    break;
		case SMP_SYNC:
		    return_lock(dest);
		    break;
		}
	    }

	    data += BLOCKSIZE;
	    dest = (dest + 1)%size;
	    p++;
	}
    }
 
    return (data);
}

int main (int argc, char *argv[]) 
{
    smi_region_info_t shreg_info;
    double t;
    unsigned long data;
    int shseg_id, i, e, errors, sync;

    SMI_Init(&argc, &argv);

    SMI_Proc_rank(&rank);
    SMI_Proc_size(&size);
    SMI_Node_size(&nsize);
    SMI_Node_rank(&nrank);
    SMI_Local_proc_size(&lsize);
    
    /* create sync locks */
    in_lock = (int *)malloc( nsize*sizeof(int) );
    out_lock = (int *)malloc( nsize*sizeof(int) );
    for (i = 0; i < nsize; i++) {
	SMI_Mutex_init(&in_lock[i]);
	SMI_Mutex_init(&out_lock[i]);
    }

    /* establish shared memory */
    shseg_addrs = (char **)malloc( size*sizeof(char *) );
    src_buf     = (char *)malloc( SEGSIZE );

    shreg_info.size    = size*SEGSIZE;
    shreg_info.offset  = 0;
    shreg_info.adapter = SMI_ADPT_DEFAULT;

    if (SMI_Create_shreg(SMI_SHM_FRAGMENTED, &shreg_info, &shseg_id, (void **)shseg_addrs) 
	!= SMI_SUCCESS) {
	fprintf (stderr, "could not establish shared memory\n");
	SMI_Abort (1);
    }

    /* perform tests */
    if (rank == 0) {
	printf("performing remote memory stress test ALLTOALL (3 parts)\n");
	fflush(stdout);
    }

    for (sync = 3; sync >= 0; sync--) {
	if (sync == 0 && rank == 0)
	    printf(" part 4: memcpy() w/o synchronization\n");
	if (sync == 1 && rank == 0)
	    printf(" part 3: memcpy() with simple synchronization\n");
	if (sync == 2 && rank == 0)
	    printf(" part 2: memcpy() with SMP synchronization\n");
	if (sync == 3 && rank == 0)
	    printf(" part 1: memcpy() with barrier synchronization\n");
	fflush(stdout);

	SMI_Query(SMI_Q_SCI_ERRORS, 0, (void *)&e);
	errors = e;
	SMI_Barrier();
	t = SMI_Wtime();
	
	data = alltoall(sync);
	
	t = SMI_Wtime() - t;
	SMI_Barrier();
	SMI_Query(SMI_Q_SCI_ERRORS, 0, (void *)&e);
	errors = e - errors;
	
	if (rank == 0) {
	    printf("   time:           %6.3f s\n", t);
	    printf("   acc. bandwidth: %6.3f MB/s\n", (size*data)/(1024*1024*t));
	    printf("   deadlocks:      %d\n", errors);
	    fflush(stdout);
	}
    }

    
    if (rank == 0) {
	printf("stress test passed.\n");
	fflush(stdout);
    }

    SMI_Finalize();
    return 0;
}
