/*
 * $Id$
 *
 * This program performs some simple tests of the MPI_Bcast broadcast
 * functionality.
 */

#include <stdlib.h>
#include "mpi.h"
#include "test.h"

int main(int argc, char **argv)
{
	int rank, size, ret, passed, i, *test_array, allerr;

	/* Set up MPI */
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	/* Setup the tests */
	Test_Init("bcast");
	test_array = (int *)malloc(size*sizeof(int));

	/* Perform the test - this operation should really be done
	   with an allgather, but it makes a good test... */
	passed = 1;
	for (i = 0; i < size; i++) {
		if (i == rank)
			test_array[i] = i;
		MPI_Bcast(test_array, size, MPI_INT, i, MPI_COMM_WORLD);
		if (test_array[i] != i)
			passed = 0;
	}
	if (!passed)
		Test_Failed("Simple Broadcast test");
	else
		Test_Passed("Simple Broadcast test");

	/* Close down the tests */
	Test_Waitforall();
	free(test_array);
	ret = Summarize_Test_Results();
	Test_Finalize();

	allerr = Test_Global_Summary();
	if (rank == 0 && allerr > 0) {
		for (i = 0; i < size; i++) {
			Print_Filecontent("bcast", i);
		}
	}
	/* Close down MPI */
	MPI_Finalize();
	return ret;
}
