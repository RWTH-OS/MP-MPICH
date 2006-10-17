/*
 * $Id$
 *
 * This program tests MPI_Alltoallv() by having processor i send different
 * amounts of data to each processor.
 *
 * Because there are separate send and receive types to alltoallv, there need
 * to be tests to rearrange data on the fly.  Not done yet.
 *
 * The first test sends i items to processor i from all processors.
 *
 * Currently, the test uses only MPI_INT; this is adequate for testing systems
 * that use point-to-point operations.
 */

#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"
#include "test.h"

int main(int argc, char **argv)
{
	int *sbuf, *rbuf;
	int rank, size;
	int *sendcounts, *recvcounts, *rdispls, *sdispls;
	int i, j, *p;
	char errmsg[200];

	DBM("Calling MPI_Init()\n");
	MPI_Init(&argc, &argv);
	DBM("Passed MPI_Init()\n");
	Test_Init_No_File();

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	/* Create the buffers */
	sbuf = (int *)malloc(size * size * sizeof(int));
	rbuf = (int *)malloc(size * size * sizeof(int));
	if (!sbuf || !rbuf) {
		fprintf(stderr, "Could not allocate buffers!\n");
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
	}
	/* Load up the buffers */
	for (i = 0; i < size * size; i++) {
		sbuf[i] = i + 100 * rank;
		rbuf[i] = -i;
	}

	/* Create and load the arguments to alltoallv */
	sendcounts = (int *)malloc(size * sizeof(int));
	recvcounts = (int *)malloc(size * sizeof(int));
	rdispls    = (int *)malloc(size * sizeof(int));
	sdispls    = (int *)malloc(size * sizeof(int));
	if (!sendcounts || !recvcounts || !rdispls || !sdispls) {
		fprintf( stderr, "Could not allocate memory!\n" );
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
	}
	for (i = 0; i < size; i++) {
		sendcounts[i] = i;
		recvcounts[i] = rank;
		rdispls[i]    = i * rank;
		sdispls[i]    = i * (i + 1) / 2;
	}

	DBM("Calling MPI_Alltoallv()\n");
	MPI_Alltoallv(sbuf, sendcounts, sdispls, MPI_INT,
				  rbuf, recvcounts, rdispls, MPI_INT, MPI_COMM_WORLD);
	DBM("Passed MPI_Alltoallv()\n");

	/* Check rbuf */
	for (i = 0; i < size; i++) {
		p = rbuf + rdispls[i];
		for (j = 0; j < rank; j++) {
			if (p[j] != i * 100 + rank * (rank + 1) / 2 + j) {
				sprintf(errmsg, "[%d] got %d expected %d for %dth\n",
						rank, p[j], i * (i + 1) / 2 + j, j);
				Test_Message( errmsg );
				Test_Failed( NULL );
			}
		}
	}

	free(sdispls);
	free(rdispls);
	free(recvcounts);
	free(sendcounts);
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
