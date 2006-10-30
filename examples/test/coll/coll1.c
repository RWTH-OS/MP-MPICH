/*
 * $Id$
 */

#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"
#include "../util/test.h"

int main(int argc, char **argv)
{
	int              rank, size, i;
	int             *table;
	MPI_Aint         address;
	MPI_Datatype     type, newtype;
	int              lens, errors;

	MPI_Init(&argc, &argv);
	Test_Init_No_File();

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	/* Make data table */
	table = (int *) calloc(size, sizeof(int));
	table[rank] = rank + 1;

	MPI_Barrier(MPI_COMM_WORLD);
	DBM("Passed Barrier\n");
	/* Broadcast the data */
	for (i = 0; i < size; i++)
		MPI_Bcast(&table[i], 1, MPI_INT, i, MPI_COMM_WORLD);

	/* See if we have the correct answers */
	for (i = 0; i < size; i++)
		if (table[i] != i + 1) Test_Failed(NULL);

	MPI_Barrier(MPI_COMM_WORLD);

	/* Try the same thing, but with a derived datatype */
	for (i = 0; i < size; i++)
		table[i] = 0;
	table[rank] = rank + 1;

	for (i = 0; i < size; i++) {
		MPI_Address(&table[i], &address);
		type = MPI_INT;
		lens = 1;
		MPI_Type_struct(1, &lens, &address, &type, &newtype);
		DBM("Passed MPI_Type_struct\n");
		MPI_Type_commit(&newtype);
		MPI_Bcast(MPI_BOTTOM, 1, newtype, i, MPI_COMM_WORLD);
		MPI_Type_free(&newtype);
		DBM("Passed MPI_Type_free\n");
	}
	/* See if we have the correct answers */
	for (i = 0; i < size; i++)
		if (table[i] != i+1) Test_Failed(NULL);

	MPI_Barrier(MPI_COMM_WORLD);

	Test_Waitforall();
	DBM("Passed Test_Waitforall\n");
	errors = Test_Global_Summary();

	MPI_Finalize();
	return errors;
}

