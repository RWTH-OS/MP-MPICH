/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define MSGSIZE (128*1024)
#define MAX_REQUESTS 8

int main( int argc, char *argv[] )
{
	int rank, size, i, nbr_reqs;
	int buffer[MSGSIZE], ack;
	MPI_Request req[MAX_REQUESTS];
	MPI_Status status[MAX_REQUESTS];

	MPI_Init( &argc, &argv );

	MPI_Comm_size( MPI_COMM_WORLD, &size );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );

	if( rank == 0 ) {
		for( i = 0; i < MSGSIZE; i++ )
			buffer[i] = i;

		for( nbr_reqs = MAX_REQUESTS; nbr_reqs <= MAX_REQUESTS; nbr_reqs++ ) {
			printf( "[0] Testing %d requests: ", nbr_reqs ); fflush( stdout );
			for( i = 0; i < nbr_reqs; i++ ) {
				 MPI_Isend( buffer, MSGSIZE, MPI_INT, 1, 100+i, MPI_COMM_WORLD, &(req[i]) );
				/*				printf( "    MPI_Isend No %d issued\n", i ); fflush( stdout ); */
			}
			/* 			printf( "     Entering MPI_Waitall()\n" ); */
			MPI_Waitall( nbr_reqs, req, status );
			/* 			printf( "     Left MPI_Waitall()\n" ); */
			MPI_Recv( &ack, 1, MPI_INT, 1, 101, MPI_COMM_WORLD, &status[0] );
			printf( "O.k.!\n" ); fflush( stdout );
		}

	}
	if( rank == 1 ) {
		for( nbr_reqs = MAX_REQUESTS; nbr_reqs <= MAX_REQUESTS; nbr_reqs++ ) {
			for( i = 0; i < nbr_reqs; i++ ) {
				/* MPI_Recv( buffer, MSGSIZE, MPI_INT, 0, 100+i, MPI_COMM_WORLD, status );*/
				MPI_Irecv( buffer, MSGSIZE, MPI_INT, 0, 100+i, MPI_COMM_WORLD, &(req[i]) );
				/* 				printf( "    MPI_Irecv No %d issued\n", i ); fflush( stdout ); */
			}
			/*			printf( "     Entering MPI_Waitall()\n" );*/
			MPI_Waitall( nbr_reqs, req, status );
			/*			printf( "     Left MPI_Waitall()\n" );*/
			ack = 1;
			 MPI_Send( &ack, 1, MPI_INT, 0, 101, MPI_COMM_WORLD );
		}
	}

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
