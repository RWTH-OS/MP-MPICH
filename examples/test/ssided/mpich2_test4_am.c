/*
 * $Id$
 *
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "../util/test.h"

/* tests passive target RMA on 2 processes. tests the lock-single_op-unlock
   optimization. */

/* same as test4.c but uses alloc_mem */

#define SIZE1 100
#define SIZE2 200

int main(int argc, char *argv[])
{
	int rank, nprocs, *A, *B, i;
	MPI_Win win;

	MPI_Init(&argc,&argv);
	Test_Init_No_File();
	MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);

	if (nprocs != 2) {
		printf("Run this program with 2 processes\n");
		MPI_Abort(MPI_COMM_WORLD,1);
	}

	i = MPI_Alloc_mem(SIZE2 * sizeof(int), MPI_INFO_NULL, &A);
	if (i) {
		printf("Can't allocate memory in test program\n");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	i = MPI_Alloc_mem(SIZE2 * sizeof(int), MPI_INFO_NULL, &B);
	if (i) {
		printf("Can't allocate memory in test program\n");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	if (rank == 0) {
		for (i = 0; i < SIZE2; i++)
			A[i] = B[i] = i;
		MPI_Win_create(NULL, 0, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &win);

		for (i = 0; i<SIZE1; i++) {
			MPI_Win_lock(MPI_LOCK_SHARED, 1, 0, win);
			MPI_Put(A + i, 1, MPI_INT, 1, i, 1, MPI_INT, win);
			MPI_Win_unlock(1, win);
		}

		for (i = 0; i < SIZE1; i++) {
			MPI_Win_lock(MPI_LOCK_SHARED, 1, 0, win);
			MPI_Get(B + i, 1, MPI_INT, 1, SIZE1 + i, 1, MPI_INT, win);
			MPI_Win_unlock(1, win);
		}

		MPI_Win_free(&win);

		for (i = 0; i < SIZE1; i++)
			if (B[i] != (-4) * (i + SIZE1)) {
				printf("Get Error: B[%d] is %d, should be %d\n", i, B[i], (-4) * (i + SIZE1));
				Test_Failed(NULL);
			}
	}

	else {  /* rank=1 */
		for (i = 0; i < SIZE2; i++)
			B[i] = (-4) * i;
		MPI_Win_create(B, SIZE2 * sizeof(int), sizeof(int), MPI_INFO_NULL,
				MPI_COMM_WORLD, &win);

		MPI_Win_free(&win);

		for (i = 0; i < SIZE1; i++) {
			if (B[i] != i) {
				printf("Put Error: B[%d] is %d, should be %d\n", i, B[i], i);
				Test_Failed(NULL);
			}
		}
	}

	MPI_Free_mem(A);
	MPI_Free_mem(B);

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
