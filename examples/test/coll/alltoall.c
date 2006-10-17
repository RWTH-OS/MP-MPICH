/*
 * $Id$
 *
 * This program tests MPI_Alltoallv by having processor i send different
 * amounts of data to each processor.
 *
 * Because there are separate send and receive types to alltoallv, there need
 * to be tests to rearrange data on the fly.  Not done yet.
 *
 * The first test sends i items to processor i from all processors.
 *
 * Currently, the test uses only MPI_INT; this is adequate for testing systems
 * that use point-to-point operations
 */

#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"
#include "test.h"

/* nbr of MPI_INTs send to each process */
#define DATASIZE 4

int main(int argc, char **argv)
{
	int *sbuf, *rbuf;
	int rank, size;
	int i, j, *p;
	char errmsg[200];

	MPI_Init(&argc, &argv);
	Test_Init_No_File();

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	/* Create the buffers */
	sbuf = (int *)malloc(size * DATASIZE * sizeof(int));
	rbuf = (int *)malloc(size * DATASIZE * sizeof(int));
	if (!sbuf || !rbuf) {
		fprintf(stderr, "Could not allocate buffers!\n");
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
	}

	/* Load up the buffers */
	for (i = 0; i < size * DATASIZE; i++) {
		sbuf[i] = i + 100 * rank;
		rbuf[i] = -(i + 1);
	}

	MPI_Alltoall(sbuf, DATASIZE, MPI_INT, rbuf, DATASIZE, MPI_INT, MPI_COMM_WORLD);

	/* Check rbuf */
	for (i = 0; i < size; i++) {
		p = rbuf + i * DATASIZE;
		for (j = 0; j < DATASIZE; j++) {
			if (p[j] != rank * DATASIZE + 100 * i + j) {
				sprintf(errmsg, "[%d] got %d expected %d for %dth\n",
						rank, p[j], rank * DATASIZE + 100 * i + j, i * DATASIZE + j);
				Test_Message( errmsg );
				Test_Failed( NULL );
			}
		}
	}

	free(rbuf);
	free(sbuf);

	Test_Waitforall();
	Test_Global_Summary();

	MPI_Finalize();
	exit( EXIT_SUCCESS );;
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
