#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "mpi.h"

#define MINSIZE 1
#define MAXSIZE 262144
#define REPEATS 500

int comm_world_rank, comm_world_size;
char *buffer;

double ping( int size, int to )
{
    int msg;
    double totaltime;
    MPI_Status status;

    MPI_Barrier( MPI_COMM_WORLD );
    totaltime = MPI_Wtime();

    /* send messages */ 
    for( msg = 0; msg < REPEATS; msg++ )
	MPI_Send( buffer, size, MPI_CHAR, to, msg, MPI_COMM_WORLD );

    /* wait for ACK */
    MPI_Recv( buffer, 1, MPI_CHAR, to, REPEATS, MPI_COMM_WORLD, &status );

    MPI_Barrier( MPI_COMM_WORLD );
    totaltime = MPI_Wtime() - totaltime;

    return( ((double)(size*REPEATS))/(totaltime*1024*1024) );
}

void pong( int size, int from )
{
    int msg;
    MPI_Status status;

    MPI_Barrier( MPI_COMM_WORLD );

    /* receive messages */
    for( msg = 0; msg < REPEATS; msg++ )
	MPI_Recv( buffer, size, MPI_CHAR, from, msg, MPI_COMM_WORLD, &status );

    /* send ACK */
    MPI_Send( buffer, 1, MPI_CHAR, from, REPEATS, MPI_COMM_WORLD );

    MPI_Barrier( MPI_COMM_WORLD );
}
 

void battery( void )
{
    int i, msg_size;
    char **hostnames, myhostname[255];
    int *partner, mypartner;
    double *bandwidth, mybandwidth, aggregate_bandwidth;
    MPI_Status status;

    MPI_Comm_size( MPI_COMM_WORLD, &comm_world_size );
    MPI_Comm_rank( MPI_COMM_WORLD, &comm_world_rank );

    if( comm_world_size % 2 != 0 ) {
	fprintf( stderr, "Must be run with an even number of application processes\n" );
	
	MPI_Abort( MPI_COMM_WORLD, EXIT_FAILURE );
    }

    /* PREPARATIONS */

    buffer = (char *)malloc( MAXSIZE * sizeof(char) );

    if( comm_world_rank == 0 ) {

	/* setup data structures for layout */
	hostnames = (char **)malloc( comm_world_size * sizeof(char *) );
	for( i = 0; i < comm_world_size; i++ )
	    hostnames[i] = (char *)malloc( 255 );
	partner = (int *)malloc( comm_world_size * sizeof(int) ); 

	/* get own information for data structures */
	if( gethostname( myhostname, 255 ) != 0 ) {
	    fprintf( stderr, "[%d] Could not get host name\n", comm_world_rank );

	    MPI_Abort( MPI_COMM_WORLD, EXIT_FAILURE );
	}
	mypartner = comm_world_size / 2;

	strncpy( hostnames[0], myhostname, 255 );
	partner[0] = mypartner;

	/* get layout information from other processes */
	for( i = 1; i < comm_world_size; i++ ) {
	    MPI_Recv( hostnames[i], 255, MPI_CHAR, i, i, MPI_COMM_WORLD, &status );
	    MPI_Recv( &(partner[i]), 1, MPI_INT, i, 2*i, MPI_COMM_WORLD, &status );
	}

	/* print out layout information */
	for( i = 0; i < comm_world_size; i++ ) {
	    printf( "Process %d is running on %s and communicates with process %d.\n", i, hostnames[i], partner[i] );
	}

	/* array for results */
	bandwidth = (double *)malloc( (comm_world_size/2) * sizeof(double) );
    }
    else {
	/* get own layout information */
	if( gethostname( myhostname, 255 ) != 0 ) {
	    fprintf( stderr, "[%d] Could not get host name\n", comm_world_rank );

	    MPI_Abort( MPI_COMM_WORLD, EXIT_FAILURE );
	}
	mypartner = (comm_world_rank >= (comm_world_size/2) ? (comm_world_rank-(comm_world_size/2)) : (comm_world_size/2)+comm_world_rank);

	/* send layout information to root process */
	MPI_Send( myhostname, 255, MPI_CHAR, 0, comm_world_rank, MPI_COMM_WORLD );
	MPI_Send( &mypartner, 1, MPI_INT, 0, 2*comm_world_rank, MPI_COMM_WORLD );
    }

    /* COMMUNICATION */


    for( msg_size = MINSIZE; msg_size <= MAXSIZE; msg_size *= 2 ) {
	if( comm_world_rank < (comm_world_size/2) ) {
	    /* lower half */
	    mybandwidth = ping( msg_size, mypartner );
	    if( comm_world_rank == 0 ) {

		/* fill array of results */
		bandwidth[0] = mybandwidth;
		for( i = 1; i < (comm_world_size/2); i++ )
		    MPI_Recv( &(bandwidth[i]), 1, MPI_DOUBLE, i, i, MPI_COMM_WORLD, &status );

		/* output of results */
		aggregate_bandwidth = 0.0;
		for( i = 0; i < (comm_world_size/2); i++ )
		    aggregate_bandwidth += bandwidth[i];

		printf( "size = %7d, bandwidth = %3.7f\n", msg_size, aggregate_bandwidth );

	    }
	    else {
		/* send result for this message size to root process */ 
		MPI_Send( &mybandwidth, 1, MPI_DOUBLE, 0, comm_world_rank, MPI_COMM_WORLD );
	    }
	}
	else {
	    /* upper half */
	    pong( msg_size, mypartner );
	}
    }

    /* CLEANUP */
    free( buffer );
    if( comm_world_rank == 0 ) {
	for( i = 0; i < comm_world_size; i++ )
	    free( hostnames[i] );
	free( hostnames );
	free( partner );
	free( bandwidth );
    }
}
