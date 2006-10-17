/* $Id: sendself.c,v 1.1 2000/04/12 16:22:04 joachim Exp $ */

/* Test the performance of sending message to itself. For SCI-MPICH,
   use the SENDSELF configuration switch to measure the difference
   using or not using the shortcut. */

#include <stdio.h>
#include <unistd.h>

#include "mpi.h"


int main (int argc, char *argv[]) {
    MPI_Status *status;
    MPI_Request *request;
    double time;
    int i, repeats, size, minbufsize, maxbufsize, mysize;
    char **sendbufs, **recvbufs;

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

    status = (MPI_Status *)calloc (repeats*2 ,sizeof(MPI_Status));
    request = (MPI_Request *)calloc (repeats*2, sizeof(MPI_Request));
    sendbufs = (char **) malloc (repeats, sizeof(char *));
    recvbufs = (char **) malloc (repeats, sizeof(char *));

    for (size = minbufsize; size <= maxbufsize; size *=2 ) {
	for (i = 0; i < repeats; i++) {
	    sendbufs[i] = (char *)malloc(size);
	    recvbufs[i] = (char *)malloc(size);
	}

	time = MPI_Wtime();
	for( i = 0; i < repeats; i++) {
	    MPI_Irecv( recvbufs[i], size, MPI_CHAR, 0, i, MPI_COMM_WORLD, &request[i*2+1]);
	    MPI_Isend( sendbufs[i], size, MPI_CHAR, 0, i, MPI_COMM_WORLD, &request[i*2]);
	}
	MPI_Waitall (2*repeats, request, status);
	time = MPI_Wtime() - time;

	printf("size = %8d  bandwidth = %7.3f MB/s    latency = %7.3f usec.,\n",
	       size, ((double)(2*size*repeats))/(time*1024*1024), (time*1e+6)/(repeats*2));    
	for (i = 0; i < repeats; i++) {
	    free (sendbufs[i]); 
	    free (recvbufs[i]);
	}
    }

    MPI_Finalize();
    return 0;
}
   
