#include <stdio.h>
#include <unistd.h>

#include "mpi.h"


int main (int argc, char *argv[]) {
    MPI_Status status;
    MPI_Request request;
    double time;
    int i, repeats, size, minbufsize, maxbufsize, mysize;
    char *sendbuf, *recvbuf;

    MPI_Init (&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &mysize);

    if (mysize != 1) {
	printf ("sendself runs only with one process. Terminating.\n");
	MPI_Finalize();
	exit (-1);
    }
    if (argc != 4) {
	printf ("usage: sendself minbufsize maxbufsize nbriters. Terminating.\n");
	MPI_Finalize();
	exit (-1);
    }
    minbufsize = atoi(argv[1]);
    maxbufsize = atoi(argv[2]);
    repeats = atoi(argv[3]);

    for (size = minbufsize; size <= maxbufsize; size *=2 ) {
	sendbuf = (char *)malloc(size);
	recvbuf = (char *)malloc(size);

	time = MPI_Wtime();
	for( i = 0; i < repeats; i++) {
	    MPI_Isend( sendbuf, size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &request);
	    MPI_Recv( recvbuf, size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
	}
	time = MPI_Wtime() - time;

	printf("size = %8d  bandwidth = %7.3f MB/s    latency = %7.3f usec.,\n",
	       size, ((double)(2*size*repeats))/(time*1<024*1024), (time*1e+6)/(repeats*2));    
	free (sendbuf); 
	free (recvbuf);
    }

    return 0;
}
   
