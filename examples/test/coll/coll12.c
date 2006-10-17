/*
 * $Id$
 */

#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"
#include "test.h"

int main(int argc, char **argv)
{
	int    rank, size;
	struct test_double_int { double a; int b; } *in, *out;
	int    i;
	int    errors = 0, toterrors;

	/* Initialize the environment and some variables */
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	in = (struct test_double_int*) malloc(size * sizeof(struct test_double_int));
	out = (struct test_double_int*) malloc(size * sizeof(struct test_double_int));

	/* Initialize the maxloc data */
	for (i = 0; i < size; i++) {
		in[i].a = (i >= rank)? (double)rank + 1.0 : 0;
		in[i].b = rank;
	}

	/* Reduce it! */
	MPI_Reduce(in, out, size, MPI_DOUBLE_INT, MPI_MAXLOC, 0, MPI_COMM_WORLD);
	MPI_Bcast(out, size, MPI_DOUBLE_INT, 0, MPI_COMM_WORLD);

	/* Check to see that we got the right answers */
	if (out[rank].b != rank) {
		printf("MAX (ranks[%d] = %d != %d\n", i, out[i].b, rank);
		errors++;
	}

	/* Initialize the minloc data */
	for (i = 0; i < size; i++)  {
		in[i].a = (i >= rank)? -(double)rank - 1.0 : 0;
		in[i].b = rank;
	}

	/* Reduce it! */
	MPI_Allreduce(in, out, size, MPI_DOUBLE_INT, MPI_MINLOC, MPI_COMM_WORLD);

	/* Check to see that we got the right answers */
	if (out[rank].b != rank) {
		printf("MIN (ranks[%d] = %d != %d\n", i, out[i].b, rank);
		errors++;
	}

	/* Finish up! */
	MPI_Allreduce(&errors, &toterrors, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	if (toterrors) {
		if (errors)
			printf("[%d] done with ERRORS(%d)!\n", rank, errors);
	}
	else {
		if (rank == 0) printf(" No Errors\n");
	}

	free(in);
	free(out);
	MPI_Finalize();
	return errors;
}
