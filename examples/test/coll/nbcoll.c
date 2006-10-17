/*
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "test.h"

int main(int argc, char **argv)
{
	int rank, size, flag;
	MPI_Comm local_comm, group_comm;
	MPI_Request r;
	MPI_Status status;
	double t0;
	char errmsg[200];

	MPI_Init(&argc, &argv);
	Test_Init_No_File();

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	if (size < 3) {
		fprintf(stderr, "Need at least 3 processes\n");
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
	}

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_split(MPI_COMM_WORLD, rank / 3, rank, &group_comm);
	MPI_Comm_split(group_comm, (rank % 3) < 2, rank, &local_comm);

	MPI_Barrier(MPI_COMM_WORLD);
	/* Magic to let non-participating processes go to the default: branch */
	switch ((rank < size / 3 * 3)? rank % 3 : rank) {
	case 0:
		/* First, ensure ssend works */
		t0 = MPI_Wtime();
		MPI_Ssend(MPI_BOTTOM, 0, MPI_INT, 1, 1, group_comm);
		t0 = MPI_Wtime() - t0;
		if (t0 < 1.0) {
			sprintf(errmsg, "Ssend does not wait for recv!\n");
			Test_Message( errmsg );
			Test_Failed( NULL );
		}
		MPI_Barrier(group_comm);
		/* Start the ssend after process 1 is well into its barrier */
		t0 = MPI_Wtime();
		while (MPI_Wtime() - t0 < 1.0) ;
		MPI_Ssend(MPI_BOTTOM, 0, MPI_INT, 1, 0, group_comm);
		MPI_Barrier(local_comm);
		/* Send process 2 an alls well */
		MPI_Send(MPI_BOTTOM, 0, MPI_INT, 2, 0, group_comm);
		break;
	case 1:
		t0 = MPI_Wtime();
		while (MPI_Wtime() - t0 < 2.0) ;
		MPI_Recv(MPI_BOTTOM, 0, MPI_INT, 0, 1, group_comm, &status);
		MPI_Barrier(group_comm);
		MPI_Irecv(MPI_BOTTOM, 0, MPI_INT, 0, 0, group_comm, &r);
		MPI_Barrier(local_comm);
		MPI_Wait(&r, &status);
		break;

	case 2:
		MPI_Barrier(group_comm);
		MPI_Irecv(MPI_BOTTOM, 0, MPI_INT, 0, 0, group_comm, &r);
		t0 = MPI_Wtime();
		while (MPI_Wtime() - t0 < 3.0) ;
		MPI_Test(&r, &flag, &status);
		if (!flag) {
			Test_Failed( NULL );
		}
		break;
	default:
		fprintf(stderr, "WARNING: Non-ideal configuration. Process %i is idle.\n", rank);
		fflush( stderr );
	}
	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Comm_free(&local_comm);
	MPI_Comm_free(&group_comm);

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
