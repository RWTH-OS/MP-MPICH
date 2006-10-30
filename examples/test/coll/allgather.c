/*
  $Id$
  
  This program tests MPI_Allgather.
  
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
	int i, j, *p;
	char errmsg[200];

	MPI_Init(&argc, &argv);
	Test_Init_No_File();

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	/* is the buffersize specified? */
	if (argc == 2) {
		datasize = atoi(argv[1]);
		if (datasize <= 0) {
			fprintf(stderr, "Invalid data size!\n");
			MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
		}
	} else {
		datasize = DATASIZE;
	}

	/* Create the buffers */
	sbuf = (int *) malloc(datasize * sizeof(int));
	rbuf = (int *) malloc(size * datasize * sizeof(int));
	if (!sbuf || !rbuf) {
		fprintf(stderr, "Could not allocate buffers!\n");
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
	}

	/* Load up the buffers */
	for (i = 0; i < datasize; i++) {
		sbuf[i] = i + 100 * rank;
	}
	for (i = 0; i < size * datasize; i++) {
		rbuf[i] = -(i + 1);
	}

	MPI_Allgather(sbuf, datasize, MPI_INT, rbuf, datasize, MPI_INT, MPI_COMM_WORLD);

	/* Check rbuf */
	for (i = 0; i < size; i++) {
		p = rbuf + i * datasize;
		for (j = 0; j < datasize; j++) {
			if (p[j] != j + 100 * i) {
				sprintf(errmsg, "[%d] got %d expected %d for %dth\n", rank, p[j], j + 100 * i, i * datasize + j);
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
