/*
 * $Id$
 */

#include <stdio.h>
#include "mpi.h"
#include "../util/test.h"

#define MAX_PROCESSES 20

int main(int argc, char **argv)
{
	int rank, size, i, j;
	int table[MAX_PROCESSES][MAX_PROCESSES];
	int errors = 0;
	int displs[MAX_PROCESSES];
	int recv_counts[MAX_PROCESSES];
	int block_size, begin_row, end_row, send_count;
	
	for(i = 1; i < MAX_PROCESSES; i++)
		for(j = 1; j < MAX_PROCESSES; j++)
			table[i][j] = 0;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	/* Determine what rows are my responsibility */
	block_size = MAX_PROCESSES / size;
	begin_row  = rank * block_size;
	end_row    = (rank + 1) * block_size;
	send_count = block_size * MAX_PROCESSES;

	/* A maximum of MAX_PROCESSES processes can participate */
	if (size > MAX_PROCESSES) {
		fprintf(stderr, "Number of processors is maximum %d\n",
				MAX_PROCESSES);
		fflush(stderr);
		DBM("calling MPI_Abort\n");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	

	/* Fill in the displacements and recv_counts */
	for (i = 0; i < size; i++) {
		displs[i]      = i * block_size * MAX_PROCESSES;
		recv_counts[i] = send_count;
	}

	/* Paint my rows my color */
	for (i = begin_row; i < end_row; i++)
		for (j = 0; j < MAX_PROCESSES; j++)
			table[i][j] = rank + 10;

	/* Everybody gets the gathered data */
	MPI_Allgatherv(&table[begin_row][0], send_count, MPI_INT,
			&table[0][0], recv_counts, displs, MPI_INT,
			MPI_COMM_WORLD);

	/* Everybody should have the same table now.
	 *
	 * The entries are:
	 *  Table[i][j] = i / block_size + 10;
	 */
	for (i = 0; i < size; i++)
		if (table[i][0] - table[i][MAX_PROCESSES - 1] != 0)
			Test_Failed(NULL);

	for (i = 0; i < size; i++)
		for (j = 0; j < MAX_PROCESSES; j++)
			if (table[i][j] != i / block_size + 10)
				Test_Failed(NULL);

	errors = Test_Global_Summary();
	if (errors) {
		/* Print out table if there are any errors */
		for (i = 0; i < size; i++) {
			printf("\n");
			for (j = 0; j < MAX_PROCESSES; j++)
				printf("  %d", table[i][j]);
		}
		printf("\n");
	}

	Test_Waitforall();
	MPI_Finalize();
	return errors;
}
