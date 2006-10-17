/*
 * $Id$
 *
 * This example should be run with 2 processes and tests the ability of the
 * implementation to handle a flood of one-way messages.
 */

#include <stdio.h>
#include "mpi.h"
#include "test.h"

int main (int argc, char **argv)
{
	double wscale = 10.0, scale,mysum =0;
	int numprocs, myid, i, namelen;
	char processor_name[MPI_MAX_PROCESSOR_NAME];

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
#ifdef VERBOSE
	if (myid == 0)
		printf("Performing test allredmany\n");
	fflush(stdout);
#endif
	MPI_Get_processor_name(processor_name, &namelen);

	/* fprintf(stderr,"Process %d on %s\n",
	   myid, processor_name); */
	for (i = 0; i < 10000; i++) {
#ifdef VERBOSE
		if ((myid == 0) && ((i == 1) || (!(i % 1000)))) {
			printf("Starting call %i of MPI_Allreduce\n", i);
			fflush(stdout);
		}
#endif
		MPI_Allreduce(&wscale, &scale, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
		mysum = mysum + scale;
	}

	if (myid == 0) {
#ifdef VERBOSE
		printf("Passed 10000 calls of MPI_Allreduce\n");
#endif
		if (!(mysum == ((numprocs * 10.0) * 10000)))
			printf("ERROR: MPI_Allreduce calculated wrong value!\n");
		else
			printf(" No Errors\n");
	}
	MPI_Finalize();
	return 0;
}
