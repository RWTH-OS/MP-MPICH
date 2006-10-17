/* $Id$ */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "mpi.h"

#include "star.h"

int main( int argc, char *argv[] )
{
    int center = -1, minsize = -1, maxsize = -1, repeats = -1;
    int c;

    MPI_Init( &argc, &argv );

    while( ( c = getopt( argc, argv, "c:i:a:r:" ) ) != -1 ) {
	switch( c ) {
	case 'c':
	    if( sscanf( optarg, "%d", &center ) != 1 )
		exit( EXIT_FAILURE );
	    break;
	case 'i':
	    if( sscanf( optarg, "%d", &minsize ) != 1 )
		exit( EXIT_FAILURE );
	    break;
	case 'a':
	    if( sscanf( optarg, "%d", &maxsize ) != 1 )
		exit( EXIT_FAILURE );
	    break;
	case 'r':
	    if( sscanf( optarg, "%d", &repeats ) != 1 )
		exit( EXIT_FAILURE );
	    break;
	}
    }

    star( center, minsize, maxsize, repeats );

    MPI_Finalize();
    
    exit( EXIT_SUCCESS );
}
