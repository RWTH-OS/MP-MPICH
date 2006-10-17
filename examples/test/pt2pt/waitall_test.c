#include <mpi.h>
#include <stdio.h>

#define MAXLEN 204800
//#define MAXLEN 50000

int a_feld[MAXLEN];
int b_feld[MAXLEN];

/* This program has to be started with 2 processes! */
int main(int argc, char** argv) {

	int len = MAXLEN;
	int size, myid;
	int i;

	MPI_Status status;
	MPI_Request req[2];
	MPI_Status statusarray[2];

	/* Initialize the environment */
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	
	/*if (size!=2) {
		fprintf(stderr, "This program has to be started with exactly 2 processes! Aborting...\n");
		MPI_Finalize();
		exit(1);
	}
	*/

	fprintf(stderr, "MAXLEN = %d\n",len);
	/* Initialize the buffers */
	if (myid <= 1)
	{
		for (i=0; i<len; i++) {
			a_feld[i]= i;
			b_feld[1]=i;
		}
	}
	fprintf(stderr, "(%d) Buffers initialized\n", myid);

	/* create remote SMI Region */
	if (myid == 0) {
		MPI_Send(a_feld, len, MPI_INT, 1, 0, MPI_COMM_WORLD);
	} 
	if (myid == 1) {
		MPI_Recv(a_feld, len, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	if (myid <= 1)
	{
		fprintf(stderr, "(%d) First message sent/received\n", myid);
	}

	/*  */
	if (myid == 1) {
		MPI_Irecv(a_feld, len, MPI_INT, 0, 0, MPI_COMM_WORLD, &req[0]);
		MPI_Irecv(b_feld, len, MPI_INT, 0, 0, MPI_COMM_WORLD, &req[1]);
		fprintf(stderr, "(%d) Irecvs called\n", myid);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	if (myid == 0) {
		MPI_Isend(a_feld, len, MPI_INT, 1, 0, MPI_COMM_WORLD, &req[0]);
		MPI_Isend(b_feld, len, MPI_INT, 1, 0, MPI_COMM_WORLD, &req[1]);
		fprintf(stderr, "(%d) Isends called\n", myid);

	}
	MPI_Barrier(MPI_COMM_WORLD);

	if (myid <= 1)
	{
		fprintf(stderr, "(%d) Calling MPI_Waitall()\n", myid);
		MPI_Waitall(2, req, statusarray);
		fprintf(stderr, "(%d) Finished MPI_Waitall()\n", myid);
	}

	fprintf(stderr, "(%d) Calling MPI_Finalize()\n", myid);	
	MPI_Finalize();
	fprintf(stderr, "(%d) Finished MPI_Finalize()\n", myid);
	if (myid == 0) {
		fprintf(stdout, "No Errors\n", myid);
	}
	
	return 0;
}
