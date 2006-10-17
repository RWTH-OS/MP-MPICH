/* $Id$ */

/* signal.c - test the usage of signals                                     */
/* The program sends a sinal from each process to all other processes.      */
/* Run this program with a single window for each process, so you can watch */
/* everything runs in the right order...                                    */

#include <stdio.h>
#include <unistd.h>
#include "smi.h"

int main (int argc, char *argv[]) 
{
    smi_signal_handle sig_h;
    double time;
    int rank, size, i, j;

    SMI_Init(&argc, &argv);

    SMI_Proc_rank(&rank);
    SMI_Proc_size(&size);

    if (size < 2) {
	fprintf(stderr, "this test may only be run with more than 1 process - aborting.\n");
	SMI_Finalize();
	return (0);
    }
    
    for (j=0; j<size; j++) {
	if (rank == j) {
	    for (i=0; i<size; i++) 
		if  (i != j) {
		    usleep(2000000);
		    printf("[%d] snding interrupt to P%d\n",rank,i);
		    fflush (stdout);
		    SMI_Signal_send(i|SMI_SIGNAL_ANY);
		}
	}
	else {
	    printf ("[%d] waiting for signal - \n", rank); fflush (stdout);
	    SMI_Signal_wait(SMI_SIGNAL_ANY);
	    printf ("[%d] signal received.\n",rank); fflush (stdout); 
	    usleep(1000000);
	}
	SMI_Barrier();
    }
    
    SMI_Finalize();
    return 0;
}
