/* $Id$ */
#include <stdio.h>
#include <stdlib.h>

#include "mpi.h"

#include "star.h"

int comm_world_rank, comm_world_size;
char *buffer;

void ping( int minsize, int maxsize, int repeats )
{
    int to, msg, size;
    double time;
    MPI_Status status;

    /* communicate with any other process in MPI_COMM_WORLD */
    for( to = 0; to < comm_world_size; to++ ) {
	if( to != comm_world_rank ) {

	    printf( "Measuring pingpong performance from %d to %d; minsize = %d, maxsize = %d, repeats = %d\n\n",
		    comm_world_rank, to, minsize, maxsize, repeats );
	    printf( "Message Size [Byte]\t\tBandwidth [MB/s]\t\t\tLatency [us]\n" );
	    printf( "================================================================================================================\n" );
	    
	    for( size = minsize; size <= maxsize; size *= 2 ) {
		
		/* synchronize with communication partner */
		MPI_Sendrecv(buffer, size, MPI_CHAR, to, 3, buffer, size, MPI_CHAR, to, 3, MPI_COMM_WORLD, &status );
		
		time = MPI_Wtime();
		for( msg = 0; msg < repeats; msg++ ) {
		    MPI_Send( buffer, size, MPI_CHAR, to, 1, MPI_COMM_WORLD );
		    MPI_Recv( buffer, size, MPI_CHAR, to, 2, MPI_COMM_WORLD, &status );
		}
		time = MPI_Wtime() - time;

		printf( "%10d\t\t\t%10.3f\t\t\t\t%10.3f\n", size, (2*size*repeats)/(time*1024*1024), (time*1e+6)/(repeats*2) );
		fflush( stdout );
	    }
	    printf( "\n\n\n" );
	}
    }
    
}

void pong( int center, int minsize, int maxsize, int repeats )
{
    int msg, size;
    MPI_Status status;

    for( size = minsize; size <= maxsize; size *= 2 ) {
		
	/* synchronize with communication partner */
	MPI_Sendrecv( buffer, size, MPI_CHAR, center, 3, buffer, size, MPI_CHAR, center, 3, MPI_COMM_WORLD, &status );

	for( msg = 0; msg < repeats; msg++ ) {
	    MPI_Recv( buffer, size, MPI_CHAR, center, 1, MPI_COMM_WORLD, &status );
	    MPI_Send( buffer, size, MPI_CHAR, center, 2, MPI_COMM_WORLD );
	}
    }
}


void star( int center, int minsize, int maxsize, int repeats )
{
    int new_center, new_minsize, new_maxsize, new_repeats;

    MPI_Comm_size( MPI_COMM_WORLD, &comm_world_size );
    MPI_Comm_rank( MPI_COMM_WORLD, &comm_world_rank );

    new_minsize = (minsize == -1) ? STAR_MINSIZE_DEFAULT : minsize;
    new_maxsize = (maxsize == -1) ? STAR_MAXSIZE_DEFAULT : maxsize;
    new_repeats = (repeats == -1) ? STAR_REPEATS_DEFAULT : repeats;

    buffer = (char *)malloc( new_maxsize * sizeof(char) );

    if( center != -1 ) {
	if( comm_world_rank == center )
		ping( new_minsize, new_maxsize, new_repeats );
	    else
		pong( center, new_minsize, new_maxsize, new_repeats );
    }
    else {
	for( new_center = 0; new_center < comm_world_size; new_center++ ) {
	    if( comm_world_rank == new_center )
		ping( new_minsize, new_maxsize, new_repeats );
	    else
		pong( new_center, new_minsize, new_maxsize, new_repeats );
	}
    }

    free( buffer );
}
