/*$Id: metatest.c 3071 2005-01-21 22:52:31Z martin $*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi.h"

#include "setuptest.h"
#include "star.h"
#include "bcast.h"

int main(argc,argv)
int argc;
char *argv[];
{
    int i;
    int run_setuptest = 0, run_star = 0, run_bcast = 0, run_allgather = 0;

    MPI_Init(&argc,&argv);

    for( i = 1; i < argc; i++ ) {
	if( strcmp( argv[i], "all" ) == 0 )
	    run_setuptest = run_star = run_bcast = run_allgather = 1;
	else if( strcmp( argv[i], "setuptest" ) == 0 )
	    run_setuptest = 1;
	else if( strcmp( argv[i], "star" ) == 0 )
	    run_star = 1;
	else if( strcmp( argv[i], "bcast" ) == 0 )
	    run_bcast = 1;
	else if( strcmp( argv[i], "allgather" ) == 0 )
	    run_allgather = 1;
    }

    if( run_setuptest )
	setuptest();
    if( run_star )
	star( -1, -1, -1, -1 );
   if( run_bcast )
       bcast( -1, -1, -1 );
   if( run_allgather )
       allgather( -1, -1, -1 );

    MPI_Finalize();

    exit( EXIT_SUCCESS );
}
