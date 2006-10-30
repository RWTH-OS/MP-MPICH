/*
  $Id$

  This program tests MPI_Allgatherv.

  Currently, the test uses only MPI_INT; this is adequate for testing systems
  that use point-to-point operations
*/

#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"
#include "../util/test.h"

/* nbr of MPI_INTs sent to each process */
#define DATASIZE 4

int main(int argc, char **argv)
{
	int datasize;
	int *sbuf, *rbuf;
	int rank, size;
	int sendcount, *recvcounts, *displs;
	int i, j, k, *p;
	char errmsg[200];
	int bufsize;

	MPI_Init(&argc, &argv);
	Test_Init_No_File();

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	/* is the buffersize specified? */
	if (argc == 2) {
		datasize = atoi(argv[1]);
		if (datasize == 0) {
			fprintf(stderr, "Invalid data size!\n");
			MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
		}
	} else {
		datasize = DATASIZE;
	}

	/* Create the buffers */
	sbuf = (int *) malloc((rank + 1) * datasize * sizeof(int));

	rbuf = (int *) malloc( (size * (size+1) / 2 * datasize  + (size-1)) * sizeof(int) );
	recvcounts = (int *) malloc(size * sizeof(int));
	displs     = (int *) malloc(size * sizeof(int));
	if (!sbuf || !rbuf || !recvcounts || !displs) {
		fprintf(stderr, "Could not allocate buffers!\n");
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
	}

	/* Load up the buffers */
	for (i = 0; i < rank + 1; i++) {
		for (j = 0; j < datasize; j++)
			sbuf[i * datasize + j] = i + 100 * rank;
	}
	for (i = 0; i < size * (size + 1) / 2 * datasize + (size-1); i++) {
		rbuf[i] = -(i + 1);
	}

	/* Create the arguments to MPI_Allgatherv() */
	sendcount = (rank + 1) * datasize;
	j = 0;
	for (i = 0; i < size; i++) {
		recvcounts[i] = (i + 1) * datasize;
		displs[i] = j;
		j += (i + 1) * datasize + 1;
	}

	MPI_Allgatherv(sbuf, sendcount, MPI_INT,
				   rbuf, recvcounts, displs, MPI_INT, MPI_COMM_WORLD);

	/* Check rbuf */
	p = rbuf;
	for (i = 0; i < size; i++) {
		for (j = 0; j < i + 1; j++) {
			for (k = 0; k < datasize; k++) {
				if (p[j * datasize + k] != j + 100 * i) {
					sprintf(errmsg, "[%d] got %d expected %d for %dth\n", rank, p[j * datasize + k], j + 100 * i, j * datasize + k);
					Test_Message( errmsg );
					Test_Failed( NULL );
				}
			}
		}
		p += (i + 1) * datasize + 1;
	}

	free(rbuf);
	free(sbuf);
	free(recvcounts);
	free(displs);

	Test_Waitforall();
	Test_Global_Summary();

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
