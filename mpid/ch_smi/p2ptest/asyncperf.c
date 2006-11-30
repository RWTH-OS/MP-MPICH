/* $Id$ */

/* A small testcase for the performance of asychnronous send & recv
   between two processes. Use mhostperf for a more general test. */

#include <stdio.h>
#include "mpi.h"


int main( int argc, char *argv[] ) {
    double before_isend, after_isend, after_completion;
    double isend_time, wait_time;
    int myrank, numprocs, msg_size, flag, i, loops;
    char *buffer;
    MPI_Request request;
    MPI_Status status;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

    if(argc != 3) {
	fprintf(stderr, "Usage: asyncperf msg_size loops\n");
	MPI_Finalize();
	return(-1);
    }
    if(numprocs != 2) {
	fprintf(stderr, "asyncperf runs with exactly 2 processes\n");
	MPI_Finalize();
	return(-1);
    }

    msg_size = atoi(argv[1]);
    loops = atoi(argv[2]);

    buffer = (char *)malloc(msg_size);
    isend_time = wait_time = 0.0;

    if(myrank == 0) {
	for (i = 0; i < loops; i++) {
	    before_isend = MPI_Wtime();
	    MPI_Isend(buffer, msg_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &request);
	    after_isend = MPI_Wtime();
	    flag = 0;
	    while(!flag)
		MPI_Test(&request, &flag, &status);
	    after_completion = MPI_Wtime();
	    
	    isend_time += after_isend - before_isend;
	    wait_time  += after_completion - after_isend;
	}
	printf("MPI_Isend: %.1fus\n", 1000000*isend_time/(double)loops);
	printf("MPI_Wait : %.1fus\n", 1000000*wait_time/(double)loops);
	printf("Total    : %.1fus\n", 1000000*(isend_time + wait_time)/(double)loops);
	
    }
    else
	for (i = 0; i < loops; i++) 
	    MPI_Recv(buffer, msg_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);

    MPI_Finalize();
    return 0;
}
