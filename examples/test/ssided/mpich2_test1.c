/*
 * $Id$
 *
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include "mpi.h"
#include "../util/test.h"

/* tests a series of puts, gets, and accumulate on 2 processes using fence */

#define SIZE 100

int main(int argc, char *argv[])
{
	int rank, nprocs, A[SIZE], B[SIZE], i;
	MPI_Win win;

	MPI_Init(&argc,&argv);
	Test_Init_No_File();
	MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);

	if (nprocs != 2) {
		printf("Run this program with 2 processes\n");
		MPI_Abort(MPI_COMM_WORLD,1);
	}

	if (rank == 0) {
		for (i=0; i<SIZE; i++)
			A[i] = B[i] = i;
	}
	else {
		for (i=0; i<SIZE; i++) {
			A[i] = (-3)*i;
			B[i] = (-4)*i;
		}
	}

	MPI_Win_create(B, SIZE*sizeof(int), sizeof(int), MPI_INFO_NULL,
			MPI_COMM_WORLD, &win);

	MPI_Win_fence(0, win);

	if (rank == 0) {
		for (i=0; i<SIZE-1; i++)
			MPI_Put(A+i, 1, MPI_INT, 1, i, 1, MPI_INT, win);
	}
	else {
		for (i=0; i<SIZE-1; i++)
			MPI_Get(A+i, 1, MPI_INT, 0, i, 1, MPI_INT, win);

		MPI_Accumulate(A+i, 1, MPI_INT, 0, i, 1, MPI_INT, MPI_SUM, win);
	}
	MPI_Win_fence(0, win);

	if (rank == 1) {
		for (i=0; i<SIZE-1; i++) {
			if (A[i] != B[i]) {
				printf("Put/Get Error: A[i]=%d, B[i]=%d\n", A[i], B[i]);
				Test_Failed(NULL);
			}
		}
	}
	else {
		if (B[SIZE-1] != SIZE - 1 - 3*(SIZE-1)) {
			printf("Accumulate Error: B[SIZE-1] is %d, should be %d\n", B[SIZE-1], SIZE - 1 - 3*(SIZE-1));
			Test_Failed(NULL);
		}
	}

	MPI_Win_free(&win);

	Test_Waitforall();
	Test_Global_Summary();

	MPI_Finalize();
	return 0;
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
 * vim:tw=0:ts=4:wm=0:sw=4:
 */
