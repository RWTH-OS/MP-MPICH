/*
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "../util/test.h"

#define SIZE 4096

/*
 * The operation is inoutvec[i] = invec[i] op inoutvec[i] (see 4.9.4). The
 * order is important.
 *
 * Note that the computation is in process rank (in the communicator) order,
 * independant of the root.
 */
int assoc(int *invec, int *inoutvec, int *len, MPI_Datatype *dtype)
{
	char err[100];
	if (inoutvec[0] <= invec[0]) {
		sprintf(err, "assoc (inout = %d, in = %d)",
				inoutvec[0], invec[0]);
		Test_Failed(err);
	}
	else
		inoutvec[0] = invec[0];
	return (1);
}

int main(int argc, char **argv)
{
	int    rank, size;
	int   *data, *result;
	int    errors = 0;
	MPI_Op op;

	MPI_Init(&argc, &argv);
	Test_Init_No_File();
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	data = (int *)malloc(SIZE * sizeof(int));
	result = (int *)malloc(SIZE * sizeof(int));
	data[0] = rank;

	MPI_Op_create((MPI_User_function*)assoc, 0, &op);
	MPI_Reduce(data, result, SIZE, MPI_INT, op, size - 1, MPI_COMM_WORLD);
	MPI_Bcast(result, SIZE, MPI_INT, size - 1, MPI_COMM_WORLD);
	MPI_Op_free(&op);

	Test_Waitforall();
	errors = Test_Global_Summary();

	free(data);
	free(result);
	MPI_Finalize();

	return errors;
}
