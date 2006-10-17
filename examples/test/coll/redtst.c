/*
 * $Id$
 */

#include <stdio.h>
#include "mpi.h"
#include "test.h"

int main(int argc, char **argv)
{
	int rank, value, result, size;

	MPI_Init(&argc, &argv);
	Test_Init_No_File();

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	value = (rank == 0) ? 3 : 6;
	MPI_Allreduce(&value, &result, 1, MPI_INT, MPI_BOR, MPI_COMM_WORLD);
	if (result != ((size > 1)? 3 | 6 : 3))
		Test_Failed("3 BOR 6 equal 3 | 6");

	Test_Waitforall();
	Test_Global_Summary();
	MPI_Finalize();

	return 0;
}
