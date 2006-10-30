/*
 * $Id$
 *
 * This file includes only the MPI_PROD testcase for MPI_Allreduce. See
 * allred.c for all other tests. The MPI_PROD test has problems with 10 or more
 * processes, since almost every type just overflows. We just exit when more
 * processes are requested.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"
#include "../util/test.h"
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

	MPI_Init(&argc, &argv);
	Test_Init_No_File();

	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (size > 9) {
		if (world_rank == 0)
			fprintf(stderr, "ERROR: Tests with more then 9 processes overflow and are therefore useless.\n");
		MPI_Abort( MPI_COMM_WORLD, EXIT_FAILURE );
	}

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

		/* Test product */
		check_info(world_rank, "PROD");
		CHECK_SIMPLE_ALL(PROD, i, (i > 0) ? (int) (pow((double) i,
													   (double) size) + 0.1) : 0);

	}

	Test_Waitforall();
	Test_Global_Summary();

	FreeComms(comms, ncomm);

	MPI_Finalize();
	exit( EXIT_SUCCESS );
}

/*
 * Overrides for XEmacs and vim so that we get a uniform tabbing style.
 * XEmacs/vim will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:tw=0:ts=4:wm=0:
 */
