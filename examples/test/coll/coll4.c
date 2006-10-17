/*
 * $Id$
 */

#include <stdio.h>
#include "mpi.h"
#include "test.h"

#define MAX_PROCESSES 16

int main(int argc, char **argv)
{
	int rank, size, i, j;
	int table[MAX_PROCESSES][MAX_PROCESSES];
	int row[MAX_PROCESSES];
	int errors = 0;

	int send_count = MAX_PROCESSES;
	int recv_count = MAX_PROCESSES;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	/* A maximum of MAX_PROCESSES processes can participate */
	if (size > MAX_PROCESSES) {
		fprintf(stderr, "Maximum number of processors is %d\n",
				MAX_PROCESSES);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	

	/* If I'm the root (process 0), then fill out the big table */
	if (rank == 0)
		for (i = 0; i < size; i++)
			for (j = 0; j < MAX_PROCESSES; j++)
				table[i][j] = i + j;

	/* Scatter the big table to everybody's little table */
	MPI_Scatter(&table[0][0], send_count, MPI_INT, &row[0], recv_count,
			MPI_INT, 0, MPI_COMM_WORLD);

	/* Now see if our row looks right */
	for (i = 0; i < MAX_PROCESSES; i++)
		if (row[i] != i + rank)
			Test_Failed(NULL);

	Test_Waitforall();
	errors = Test_Global_Summary();
	MPI_Finalize();
	return errors;
}
