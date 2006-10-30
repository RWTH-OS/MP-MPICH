/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "../util/test.h"

typedef struct {
	short a;
	int b;
} s1;

main( int argc, char *argv[] )
{
	s1 s[10], sout[10];
	int i, rank, size;
	MPI_Status status;
	int errors;

	MPI_Init( &argc, &argv );
	Test_Init_No_File();

	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &size );

	if( size < 2 ) {
		fprintf( stderr, "Must be run with at least 2 processes -> Aborting!\n" );
		MPI_Abort( MPI_COMM_WORLD, EXIT_FAILURE );
	}

	for( i = 0; i < 10; i++ ) {
		s[i].a = rank + i;
		s[i].b = rank;
		sout[i].a = -1;
		sout[i].b = -1;
	}

	MPI_Reduce( s, sout, 10, MPI_SHORT_INT, MPI_MINLOC, 1, MPI_COMM_WORLD );
	DBM( "Passed Reduce" );

	if (rank == 1) {
		for( i = 0; i < 10; i++ ) {
			if( (sout[i].a != i) || (sout[i].b != 0) )
				Test_Failed( NULL );
		}
	}

	Test_Waitforall();
	DBM( "Passed Test_Waitforall\n" );
	errors = Test_Global_Summary();

	MPI_Finalize();
	exit( EXIT_FAILURE );
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
