/*
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "test.h"

int main(int argc, char **argv)
{
	int   rank, size, i, j;
	int **table;
	int  *row;
	int   errors = 0;
	int  *displs;
	int  *send_counts;
	int   recv_count;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	table = (int**) malloc(size * sizeof(int*));
	table[0] = (int*) malloc(size * size * sizeof(int));
	for(i = 1; i < size; i++)
		table[i] = table[i - 1] + size;
	row = (int*) malloc(size * sizeof(int));
	displs = (int*) malloc(size * sizeof(int));
	send_counts = (int*) malloc(size * sizeof(int));
	recv_count = size;

	/* If I'm the root (process 0), then fill out the big table and setup
	 * send_counts and displs arrays */
	if (rank == 0) {
		for (i = 0; i < size; i++) {
			send_counts[i] = recv_count;
			displs[i] = i * size;
			for (j = 0; j < size; j++)
				table[i][j] = i + j;
		}
	}
	/* Scatter the big table to everybody's little table */
	MPI_Scatterv(&table[0][0], send_counts, displs, MPI_INT,
			&row[0]     , recv_count, MPI_INT, 0, MPI_COMM_WORLD);

	/* Now see if our row looks right */
	for (i = 0; i < size; i++) {
		if (row[i] != i + rank)
			Test_Failed(NULL);
	}
	Test_Waitforall();
	errors = Test_Global_Summary();
	MPI_Finalize();
	free(displs);
	free(row);
	free(table[0]);
	free(table);

	return errors;
}
