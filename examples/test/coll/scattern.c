/*
 * $Id$
 *
 * In this example, the root process sends a vector containing doubles and
 * each process receives these doubles into a contiguous array.
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include "mpi.h"

#define NBR_BLOCKS 10
#define STRIDE     15

int main(int argc, char **argv)
{
	MPI_Datatype vec;
	double *sendbuf, *recvbuf, ivalue;
	int root, i, err = 0;
	int rank, size;

	MPI_Init(&argc, &argv);

	MPI_Comm_size( MPI_COMM_WORLD, &size );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );

	/* construct an MPI datatype with NBR_BLOCKS doubles and some space in between them */
	MPI_Type_vector( NBR_BLOCKS, 1, STRIDE, MPI_DOUBLE, &vec );
	MPI_Type_commit( &vec );

	/* buffer to hold a vec for each process */
	sendbuf = (double *) malloc( size * NBR_BLOCKS * STRIDE * sizeof(double) );

	/* buffer to receive NBR_BLOCKS doubles contiguously */
	recvbuf = (double *) malloc(NBR_BLOCKS * sizeof(double));

	for (i = 0; i < NBR_BLOCKS * STRIDE * size; i++)
		sendbuf[i] = (double) i;
	
	for (root = 0; root < size; root++) {
		for (i = 0; i < NBR_BLOCKS; i++)
			recvbuf[i] = -1.0;

		MPI_Scatter(sendbuf, 1, vec,
					recvbuf, NBR_BLOCKS, MPI_DOUBLE,
					root, MPI_COMM_WORLD);

		/* the extent of a vec doesn't include space after the last double, so the part of a vec sent to each process isn't
		   NBR_BLOCKS * STRIDE doubles, but NBR_BLOCKS * STRIDE - (STRIDE - 1) = (NBR_BLOCKS - 1) * STRIDE + 1 */
		ivalue = rank * ((NBR_BLOCKS - 1) * STRIDE + 1);
		for (i = 0; i < NBR_BLOCKS; i++) {
			if (recvbuf[i] != ivalue) {
				printf("Root = %2d, Rank = %2d: Expected %f at position %d, but found %f\n",
					   root, rank, ivalue, i, recvbuf[i]);
				err++;
			}
			ivalue += STRIDE;
		}
	}

	i = err;
	MPI_Allreduce(&i, &err, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	if (rank == 0) {
		if (err > 0)
			printf("Found %d errors!\n", err);
		else
			printf(" No Errors\n");
	}

	MPI_Type_free(&vec);

	free(sendbuf);
	free(recvbuf);

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
 * vim:tw=0:ts=4:wm=0:
 */
