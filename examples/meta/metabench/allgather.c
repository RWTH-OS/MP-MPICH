/* $Id$ */

#include <stdio.h>
#include <stdlib.h>

#include "mpi.h"

#include "allgather.h"

void allgather( int minsize, int maxsize, int repeats )
{
    int comm_world_rank, comm_world_size, size, msg;
    char *sendbuffer, *recvbuffer;
    int new_minsize, new_maxsize, new_repeats;
    double time;

    new_minsize = (minsize == -1) ? ALLGATHER_MINSIZE_DEFAULT : minsize;
    new_maxsize = (maxsize == -1) ? ALLGATHER_MAXSIZE_DEFAULT : maxsize;
    new_repeats = (repeats == -1) ? ALLGATHER_REPEATS_DEFAULT : repeats;
    
    MPI_Comm_rank( MPI_COMM_WORLD, &comm_world_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &comm_world_size );

    sendbuffer = (char *)malloc( new_maxsize * sizeof(char) );
    recvbuffer = (char *)malloc( comm_world_size * new_maxsize * sizeof(char) );

    if( comm_world_rank == 0 ) {
	printf( "Measuring allgather performance; minsize = %d, maxsize = %d, repeats = %d\n",
		new_minsize, new_maxsize, new_repeats );
	printf( "Message Size [Byte]\t\tAverage time for one operation [us]\n" );
	printf( "================================================================================================================\n" );
    }

    for( size = new_minsize; size <= new_maxsize; size *= 2 ) {

	time = MPI_Wtime();
	for( msg = 0; msg < new_repeats; msg++ )
	    MPI_Allgather( sendbuffer, size, MPI_CHAR, recvbuffer, size, MPI_CHAR, MPI_COMM_WORLD );
	time = MPI_Wtime() - time;

	if( comm_world_rank == 0 ) {
	    printf( "%10d\t\t\t%10.3f\n", size, (time*1e+6)/new_repeats );
	    fflush( stdout );
	}
    }

    if( comm_world_rank == 0 )
	printf( "\n\n\n" );

    free( recvbuffer );
    free( sendbuffer );
}

 
