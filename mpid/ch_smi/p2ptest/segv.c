/* $Id$ */

/* Provoke abnormal termination to test the resource management 
   of SCI-MPICH. It should be possible to let this programm loop
   forever without running out of SCI resources. */

#include <stdio.h>
#include <unistd.h>

#include "mpi.h"

#define BUFSIZE 1000

int main (int argc, char *argv[]) {
    int i, rank;
    char *buf, b;

    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);

    buf = (char *) malloc (BUFSIZE, sizeof(char));

    /* provoke SIGSEGV */
    if (rank == 0) {
	fprintf (stderr, "[%d] provoking SIGSEGV...\n", rank);
	for (i = 1; i > 0; i++)
	    b = buf[i];
    }
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();
    return 0;
}
   
