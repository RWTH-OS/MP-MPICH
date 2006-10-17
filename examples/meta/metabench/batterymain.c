/* $Id$ */
#include <stdlib.h>

#include "mpi.h"

#include "battery.h"

int main( int argc, char *argv[] )
{
    MPI_Init( &argc, &argv );

    battery();

    MPI_Finalize();

    exit( EXIT_SUCCESS );
}
