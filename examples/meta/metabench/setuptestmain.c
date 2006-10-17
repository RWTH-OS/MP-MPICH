#include <stdlib.h>

#include "mpi.h"

#include "setuptest.h"

int main( int argc, char *argv[] )
{
    MPI_Init(&argc,&argv);

    setuptest();
    
    MPI_Finalize();

    exit( EXIT_SUCCESS );
}
