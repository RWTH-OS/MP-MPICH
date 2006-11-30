/* $Id$ */
/* signal.c - test the usage of signals */

#include <stdio.h>

#include "smi.h"

#define ITERATIONS 1000
#define LOOPS 1000000

static void print_rank(void *arg) 
{
    int rank = *(int *)arg;
    
    printf ("  [%d] callback of process %d triggered\n", rank, rank);
    fflush(stdout);

    return;
}

static double busy(int loops)
{
    double a, b, c;
    int i;
    
    a = 15151.2; b = 141.5151; c = 961.81;
    
    for (i = 0; i < loops; i++) {
	a += (b - c*a);
    }

    return a;
}

int main (int argc, char *argv[]) 
{
    smi_signal_handle sig_h;
    double time;
    int rank, size, i;

    SMI_Init(&argc, &argv);

    SMI_Proc_rank(&rank);
    SMI_Proc_size(&size);

    if (size != 2) {
	fprintf(stderr, "this test may only be run with 2 processes - aborting.\n");
	SMI_Finalize();
	return (0);
    }

    /* test if signals are stored */
    if (rank == 0) {
	printf ("*** testing if signals are stored (programm may deadlock)\n");
	fflush (stdout);

	SMI_Barrier();
	printf ("  [%d] sending signal\n", rank); fflush (stdout);
	SMI_Signal_send(1|SMI_SIGNAL_ANY);
	printf ("  [%d] busy\n", rank); fflush (stdout);
	busy(3*LOOPS);
	printf ("  [%d] waiting for signal - ", rank); fflush (stdout);
	SMI_Signal_wait(SMI_SIGNAL_ANY);
	printf ("signal received.\n"); fflush (stdout);

	printf (">>> signals are stored\n");
	fflush (stdout);
    }
    if (rank == 1) {
	SMI_Barrier();
	busy(LOOPS);
	printf ("  [%d] busy\n", rank); fflush (stdout);
	busy(LOOPS);
	printf ("  [%d] waiting for signal - ", rank); fflush (stdout);
	SMI_Signal_wait(SMI_SIGNAL_ANY);
	printf ("signal received.\n"); fflush (stdout);
	printf ("  [%d] sending signal\n", rank); fflush (stdout);
	SMI_Signal_send(0|SMI_SIGNAL_ANY);
	busy(2*LOOPS);
    }
    SMI_Barrier();
    
    /* test the signal latency */
    if (rank == 0) {
	printf ("\n*** testing signal latency (roundtrip/2)\n");
	fflush (stdout);

	time = SMI_Wtime();
	for (i = 0; i < ITERATIONS; i++) {
	    SMI_Signal_send(1|SMI_SIGNAL_ANY);
	    SMI_Signal_wait(SMI_SIGNAL_ANY);
	}
	time = SMI_Wtime() - time;   
	
	printf (">>> average signal latency is %6.3f us\n", (time*1e+6)/(2*ITERATIONS));
	fflush (stdout);
    }
    if (rank == 1) {
	for (i = 0; i < ITERATIONS; i++) {
	    SMI_Signal_wait(SMI_SIGNAL_ANY);
	    SMI_Signal_send(0|SMI_SIGNAL_ANY);
	}
	busy(2*LOOPS);
    }
    SMI_Barrier();

    /* test callbacks */
    if (rank == 0) {
	printf ("\n*** testing signal callbacks\n"); fflush(stdout);
    }
    
    SMI_Signal_setCallBack (SMI_SIGNAL_ANY, print_rank, (void *)&rank, &sig_h);
    SMI_Signal_send (1-rank|SMI_SIGNAL_ANY);
    SMI_Signal_joinCallBack(&sig_h);
    SMI_Barrier();

    if (rank == 0) {
	busy(100000);
	printf (">>> two callback messages should have been printed\n"); fflush(stdout);
    }

    /* XXX more tests to be implemented:
       - signal "chain" with many processes
       - pt2pt signals / global signals 
    */

    SMI_Finalize();
    return 0;
}
