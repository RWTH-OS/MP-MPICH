/*
 * $Id$
 *
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include "mpi.h"
#include "../util/test.h"

/* tests put and get with post/start/complete/wait on 2 processes */

#define SIZE1 100
#define SIZE2 200

int main(int argc, char *argv[])
{
	int rank, destrank, nprocs, A[SIZE2], B[SIZE2], i;
	MPI_Group comm_group, group;
	MPI_Win win;

	MPI_Init(&argc,&argv);
	Test_Init_No_File();
	MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);

	if (nprocs != 2) {
		printf("Run this program with 2 processes\n");
		MPI_Abort(MPI_COMM_WORLD,1);
	}

	MPI_Comm_group(MPI_COMM_WORLD, &comm_group);

	if (rank == 0) {
		for (i=0; i<SIZE2; i++) A[i] = B[i] = i;
		MPI_Win_create(NULL, 0, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &win);
		destrank = 1;
		MPI_Group_incl(comm_group, 1, &destrank, &group);
		MPI_Win_start(group, 0, win);
		for (i=0; i<SIZE1; i++)
			MPI_Put(A+i, 1, MPI_INT, 1, i, 1, MPI_INT, win);
		for (i=0; i<SIZE1; i++)
			MPI_Get(B+i, 1, MPI_INT, 1, SIZE1+i, 1, MPI_INT, win);

		MPI_Win_complete(win);

		for (i=0; i<SIZE1; i++)
			if (B[i] != (-4)*(i+SIZE1)) {
				printf("Get Error: B[i] is %d, should be %d\n", B[i], (-4)*(i+SIZE1));
				Test_Failed(NULL);
			}
	}

	else {  /* rank=1 */
		for (i=0; i<SIZE2; i++) B[i] = (-4)*i;
		MPI_Win_create(B, SIZE2*sizeof(int), sizeof(int), MPI_INFO_NULL,
				MPI_COMM_WORLD, &win);
		destrank = 0;
		MPI_Group_incl(comm_group, 1, &destrank, &group);
		MPI_Win_post(group, 0, win);
		MPI_Win_wait(win);

		for (i=0; i<SIZE1; i++) {
			if (B[i] != i) {
				printf("Put Error: B[i] is %d, should be %d\n", B[i], i);
				Test_Failed(NULL);
			}
		}
	}

	MPI_Group_free(&group);
	MPI_Group_free(&comm_group);
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
