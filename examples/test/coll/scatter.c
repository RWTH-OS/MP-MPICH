/* $Id$
 *
 * This example performs a simple test for MPI_Scatter()
 */

#include <stdlib.h>
#include <stdio.h>

#include "mpi.h"

#define NBR_ELMTS 1024 * 1024

int main(int argc, char **argv)
{
	MPI_Comm *comms;
	int max_n, min_n;
	int *sendbuf, *recvbuf, ivalue;
	int proc_nbr, j, n, err, errs, gerr = 0;
	int comm_world_rank, comm_world_size, comm_rank;

	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &comm_world_size);
	if (comm_world_size < 2) {
		fprintf(stderr, "Must be run with at least 2 processes -> Aborting!\n");
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
	}

	MPI_Comm_rank(MPI_COMM_WORLD, &comm_world_rank);

	min_n = 1;
	max_n = NBR_ELMTS;

	/* allocate buffers */
	sendbuf = (int *) malloc(max_n * comm_world_size * sizeof(int));
	recvbuf = (int *) malloc(max_n * sizeof(int));
	if ((sendbuf == 0) || (recvbuf == 0)) {
		fprintf(stderr, "Could not allocate buffers -> Aborting!\n");
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
	}

	/* initialize send buffer */
	for (j = 0; j < max_n * comm_world_size; j++)
		sendbuf[j] = j;

	/* array for communicators */
	comms = (MPI_Comm *) malloc((comm_world_size + 1) * sizeof(MPI_Comm));

	/* create communicators */
	for (proc_nbr = 2; proc_nbr < comm_world_size; proc_nbr++)
		MPI_Comm_split(MPI_COMM_WORLD, comm_world_rank < proc_nbr, 0,
				&(comms[proc_nbr]));
	comms[comm_world_size] = MPI_COMM_WORLD;


	/* loop over number of participating processes */
	for (proc_nbr = 2; proc_nbr <= comm_world_size; proc_nbr++) {

#ifdef VERBOSE
		if (comm_world_rank == 0) {
			printf("Testing MPI_Scatter() with %3d processes:",
					proc_nbr);
			fflush(stdout);
		}
#endif

		MPI_Barrier(MPI_COMM_WORLD);

		if (comm_world_rank < proc_nbr) {

			MPI_Comm_rank(comms[proc_nbr], &comm_rank);

			/* loop over number of elements to be sent to each receiving process */
			for (n = min_n; n <= max_n; n *= 2) {

#ifdef VERBOSE
				if (comm_world_rank == 0) {
					printf(" %d", n);
					fflush(stdout);
				}
#endif

				/* clean up receive buffer */
				for (j = 0; j < n; j++)
					recvbuf[j] = -1;

				MPI_Scatter(sendbuf, n, MPI_INT, recvbuf, n,
						MPI_INT, 0, comms[proc_nbr]);

				/* check receive buffer */
				err = 0;
				ivalue = comm_world_rank * n;
				for (j = 0; j < n; j++) {
					if (recvbuf[j] != ivalue) {
						err = 1;
					}
					ivalue++;
				}

				/* get sum of all errors found on all processes */
				MPI_Reduce(&err, &errs, 1, MPI_INT, MPI_SUM, 0,
						comms[proc_nbr]);
			}	/* end of loop over message size */

			/* report errors */
			if (comm_rank == 0) {
				gerr += errs;
#ifdef VERBOSE
				if (errs > 0) {
					printf(" Found errors!");
					fflush(stdout);
				}
#endif
			}

		}

		MPI_Barrier(MPI_COMM_WORLD);

#ifdef VERBOSE
		if (comm_world_rank == 0) {
			printf("\n");
			fflush(stdout);
		}
#endif

	}			/* end of loop over all communicators */

	if (comm_world_rank == 0) {
		if (gerr > 0)
			printf("Tests failed\n");
		else
			printf(" No Errors\n");
		fflush(stdout);
	}

	/* shutdown */

	free(comms);
	free(sendbuf);
	free(recvbuf);

	MPI_Finalize();

	exit(EXIT_SUCCESS);
}
