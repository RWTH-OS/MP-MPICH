/*
 * $Id$
 */

#include <stdio.h>
#include "mpi.h"
#include "test.h"

#define MAX_PROCESSES 20

int main(int argc, char **argv)
{
	int rank, size, i, j;
	int table[MAX_PROCESSES][MAX_PROCESSES];
	int errors = 0;
	int block_size, begin_row, end_row, send_count, recv_count;

	DBM("calling MPI_Init\n");
	MPI_Init(&argc, &argv);
	DBM("calling MPI_Comm_rank\n");
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	DBM("calling MPI_Comm_size\n");
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	for (i = 0; i < MAX_PROCESSES; i++)
		for (j = 0; j < MAX_PROCESSES; j++)
			table[i][j]=-1;

	/* Determine what rows are my responsibility */
	block_size = MAX_PROCESSES / size;
	begin_row  = rank * block_size;
	end_row    = (rank + 1) * block_size;
	send_count = block_size * MAX_PROCESSES;
	recv_count = send_count;

	/* A maximum of MAX_PROCESSES processes can participate */
	if (size > MAX_PROCESSES) {
		fprintf(stderr, "Number of processors is maximal %d\n",
				MAX_PROCESSES);
		DBM("calling MPI_Abort\n");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	

	/* Paint my rows my color */
	for (i = begin_row; i < end_row ;i++)
		for (j = 0; j < MAX_PROCESSES; j++)
			table[i][j] = rank + 10;

#ifdef PRINTTABLE
	/* print table */
	printf("\n");
	for (i = 0; i < MAX_PROCESSES; i++)
	{
		for (j = 0; j < MAX_PROCESSES; j++)
			printf("%4d", table[i][j]);
		printf("\n");
	}
#endif
	DBM("starting algather\n");
	/* Everybody gets the gathered table */
	MPI_Allgather(&table[begin_row][0], send_count, MPI_INT,
			&table[0][0],         recv_count, MPI_INT,
			MPI_COMM_WORLD);

	/* Everybody should have the same table now.
	 * This test does not in any way guarantee there are no errors.
	 * Print out a table or devise a smart test to make sure it's correct */
#ifdef PRINTTABLE
	/* print table */
	printf("\n");
	for (i = 0; i < MAX_PROCESSES; i++)
	{
		for (j = 0; j < MAX_PROCESSES; j++)
			printf("%4d", table[i][j]);
		printf("\n");
	}
#endif

	for (i = 0; i < MAX_PROCESSES; i++) {
		if (table[i][0] - table[i][MAX_PROCESSES - 1] !=0)
		{
			Test_Failed(NULL);
			printf("error at i=%d\n", i);
		}
	}

	Test_Waitforall();
	errors = Test_Global_Summary();
	MPI_Finalize();

	return errors;
}
