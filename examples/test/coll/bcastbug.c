/*
 * $Id$
 *
 * Guessed purpose: This test triggers a bug in some older MPICH, so if it
 * completes at all the test was successful.
 */

#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"
#include "test.h"

int main(int argc, char **argv)
{
	char *buf;
	int rank, size, i;
	MPI_Request *req;
	MPI_Status  *stat;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	MPI_Comm_size (MPI_COMM_WORLD, &size);

	buf = (char *)malloc(1024 * sizeof(char));
	req = (MPI_Request *)malloc(size * sizeof(MPI_Request));
	stat = (MPI_Status *)malloc(size * sizeof(MPI_Status));

	if (rank == 0) {
		for (i = 1; i < size; i++)
			MPI_Isend(buf, 1024, MPI_BYTE, i, 0, MPI_COMM_WORLD, &req[i]);
		MPI_Waitall(size - 1, &req[1], &stat[1]); /* Core dumps here! */
	}
	else
		MPI_Recv(buf, 1024, MPI_BYTE, 0, 0, MPI_COMM_WORLD, &stat[rank]);

	Test_Waitforall();

	if (rank == 0)
		printf(" No Errors\n");

	free(buf);
	free(req);
	free(stat);
	MPI_Finalize();
	return 0;
}

#if 0
int MPIND_Waitall(int count, MPI_Request array_of_requests[], MPI_Status array_of_statuses[])
{
	int i;
	MPIR_BOOL completed;

	for (i = 0; i < count; i++) {
		if (!array_of_requests[i]) continue;
		MPID_complete_send(&array_of_requests[i]->shandle,
				&(array_of_statuses[i]));

		MPIND_Request_free(&array_of_requests[i]); /* Core dumps here! */
		array_of_requests[i] = NULL;
	}
	return MPI_SUCCESS;
}

#define MPID_ND_free_send_handle(a)  if ((a)->buffer) { FREE((a)->buffer); }

int MPIND_Request_free(MPI_Request *request)
{
	int errno = MPI_SUCCESS;

	printf("Should be core dumping here (buffer = %d)...\n",
			(&((*request)->shandle.dev_shandle))->buffer);
	MPID_ND_free_send_handle(&((*request)->shandle.dev_shandle));
	printf("and not reaching here!\n");
	SBfree(MPIR_shandles, *request);

	return MPI_SUCCESS;
}
#endif
