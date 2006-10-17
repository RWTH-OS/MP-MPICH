/*
 * $Id$
 *
 * This file includes many testcases for MPI_Allreduce. When adding new test
 * cases please make sure that your check expressions are right even in corner
 * cases (e.g. running only one process, running with an odd number of
 * processes, etc.).
 *
 * The code in this file was very long, ugly and repitive. In C++ we would use
 * templates to make the code more readable, but in C we have no choice and use
 * macros...
 *
 * (Tested with 1-16 processes -- tobias, 2006-04-07)
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"
#include "test.h"
#include "../pt2pt/gcomm.h"
#include "allred.h"

#define COMMS 10

void check_info(int rank, char *type)
{
#ifdef VERBOSE
	if (rank == 0)
		printf("Testing MPI_%s...\n", type);
#endif
}

int main(int argc, char **argv)
{
	int count, errcnt = 0, size, rank;
	MPI_Comm comm;

	MPI_Comm comms[COMMS];
	int ncomm, i, world_rank;

#ifdef VERBOSE
		printf("Testing allred...\n");fflush(stdout);
#endif

	MPI_Init(&argc, &argv);
	Test_Init_No_File();
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	/* First tests */
	MakeComms(comms, COMMS, &ncomm, 0);
	for (i = 0; i < ncomm; i++) {
#ifdef VERBOSE
		if (world_rank == 0)
			printf("Testing with communicator %d\n", i);
#endif
		comm = comms[i];

		MPI_Comm_size(comm, &size);
		MPI_Comm_rank(comm, &rank);
		count = 10;

		/* Test sum */
		check_info(world_rank, "SUM");
		CHECK_SIMPLE_ALL(SUM, i, i * size);

		/* Test max */
		check_info(world_rank, "MAX");
		CHECK_SIMPLE_ALL(MAX, rank + i, size - 1 + i);

		/* Test min */
		check_info(world_rank, "MIN");
		CHECK_SIMPLE_ALL(MIN, rank + i, i);

		/* Test LOR */
		check_info(world_rank, "LOR");
		CHECK_SIMPLE_ALL_INT(LOR, rank & 0x1, size > 1);
		CHECK_SIMPLE_ALL_INT(LOR, 0, 0);

		/* Test LXOR */
		check_info(world_rank, "LXOR");
		CHECK_SIMPLE_ALL_INT(LXOR, rank == 1, size > 1);
		CHECK_SIMPLE_ALL_INT(LXOR, 0, 0);
		CHECK_SIMPLE_ALL_INT(LXOR, 1, size % 2);

		/* Test LAND */
		check_info(world_rank, "LAND");
		CHECK_SIMPLE_ALL_INT(LAND, rank & 0x1, 0);
		CHECK_SIMPLE_ALL_INT(LAND, 1, 1);

		/* Test BOR */
		check_info(world_rank, "BOR");
		CHECK_SIMPLE_ALL_INT_CHAR(BOR, rank & 0x3,
				(size < 3) ? size - 1 : 0x3);

		/* Test BAND */
		check_info(world_rank, "BAND");
		CHECK_SIMPLE_ALL_INT_CHAR(BAND, rank == size - 1 ? i : ~0, i);
		CHECK_SIMPLE_ALL_INT_CHAR(BAND,
				rank == size - 1 ? i : 0, (size > 1) ? 0 : i);

		/* Test BXOR */
		check_info(world_rank, "BXOR");
		CHECK_SIMPLE_ALL_INT_CHAR(BXOR, (rank == 1) * 0xf0,
				(size > 1) * 0xf0);
		CHECK_SIMPLE_ALL_INT_CHAR(BXOR, 0, 0);
		CHECK_SIMPLE_ALL_INT_CHAR(BXOR, ~0, (size % 2) ? ~0 : 0);

		/* Test Maxloc */
		check_info(world_rank, "MAXLOC");
		CHECK_STRUCT_ALL(MAXLOC, rank + i, rank, size - 1 + i,
				size - 1);

		/* Test minloc */
		check_info(world_rank, "MINLOC");
		CHECK_STRUCT_ALL(MINLOC, rank + i, rank, i, 0);

	}

	Test_Global_Summary();
	FreeComms(comms, ncomm);
	MPI_Finalize();
	return 0;
}
