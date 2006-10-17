/* $Id: simplesend.c,v 1.3 2004/04/06 09:12:48 boris Exp $ */

/* minimal send/receive for testing purposes */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "mpi.h"

#define VERIFY

#define REALLOC_CYCLE -1
#define OFFSET        0
#define MSG_SIZE      1024*1024
#define NBR_MSGS      100

int main(int argc, char **argv) {
    MPI_Status status;
    int myrank, mysize;
    int send_rank, recv_rank, nbr_msgs, msg_size, offset, realloc_cycle;
    unsigned char *buffer;
    int i, j, c;
    double start, end, bandwidth;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &mysize);    
    
    msg_size = MSG_SIZE;
    send_rank = 0;
    recv_rank = 1;
    nbr_msgs  = NBR_MSGS;
    offset    = OFFSET;
    realloc_cycle = REALLOC_CYCLE;

    while((c = getopt(argc, argv, "m:r:s:n:c:o:?")) != EOF) {
	switch(c) {
	case 'm':
	    msg_size = atoi (optarg);
	    break;
	case 'r':
	    recv_rank = atoi (optarg);
	    break;
	case 's':
	    send_rank = atoi (optarg);
	    break;
	case 'n':
	    nbr_msgs = atoi (optarg);
	    break;
	case 'c':
	    realloc_cycle = atoi (optarg);
	    break;
	case 'o':
	    offset = atoi (optarg);
	    break;
	case '?':
	    break;
	}
    }


    buffer = (unsigned char *)malloc(msg_size + offset);
    
    start = MPI_Wtime();
    for (i = 1; i <= nbr_msgs; i++) {
	if (myrank == send_rank) {
#ifdef VERIFY
	    for (j = 0; j < msg_size; j++)
		buffer[offset + j] = j % 253;
#endif
	    MPI_Send (buffer + offset, msg_size , MPI_BYTE, recv_rank, 1, MPI_COMM_WORLD);
	} else 
	    if (myrank == recv_rank) {
		/* test for transfers into registered user memory */
		if (realloc_cycle > 0 && i % realloc_cycle == 0) {
		    free (buffer);
		    buffer = (unsigned char *)malloc(msg_size+ offset);
		}
#ifdef VERIFY
		for (j = 0; j < msg_size; j++)
		    buffer[offset + j] = 255;
#endif
		MPI_Recv( buffer + offset, msg_size, MPI_BYTE, send_rank, 1, MPI_COMM_WORLD, &status);
#ifdef VERIFY
		for (j = 0; j < msg_size; j++)
		    if (buffer[offset + j] != j % 253) 
			printf ("[%d] recv error in msg %d at pos %d: buffer[%d] is %d, should be %d\n", 
				myrank, i, j + offset, j, buffer[j], j % 253);
#endif
	    }
    }
    end = MPI_Wtime();
    bandwidth = (nbr_msgs * msg_size) / ((end - start)*1024*1024);
    
#ifdef VERIFY
    if (myrank == send_rank) {
	printf  ("[%d] %d msg of %d bytes sent at %6.3f MB/s, time %9.6f s, latency %6.3f ms\n",
		 myrank, nbr_msgs, msg_size, bandwidth, end-start, (1000*(end-start))/nbr_msgs);
    } else 
	if (myrank == recv_rank) {
	    printf  ("[%d] %d msg of %d bytes received at %6.3f MB/s, time %9.6f s, latency %6.3f ms\n",
		     myrank, nbr_msgs, msg_size, bandwidth, end-start, (1000*(end-start))/nbr_msgs);
	}
#endif
    
    free (buffer);
    MPI_Finalize();
    return (0);
}

