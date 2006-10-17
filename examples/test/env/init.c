#include "mpi.h"
#include <stdio.h>
#include "test.h"

int main( int argc, char **argv )
{
    int        flag;
    MPI_Status status;
    int        size, myrank, partner, i,participants;

    for (i=0; i<2; i++ ) {
	  MPI_Initialized(&flag);
	  if(flag == 0)
	    MPI_Init(&argc,&argv);
    }

    MPI_Comm_size( MPI_COMM_WORLD, &size );
	participants = size;
    if (size % 2) {
/* even number of processes */
	  participants = size -1;
    }
    MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
/*    partner = (rank + 1) % size; */
	if (myrank < participants)
	{	
/* even number of processes is communicating */
	  if (myrank % 2) {
		  partner = myrank-1;
	  }
	  else {
		  partner = myrank+1;
	  }
      MPI_Sendrecv( MPI_BOTTOM, 0, MPI_INT, partner, 0,
		  MPI_BOTTOM, 0, MPI_INT, partner, 0, 
		  MPI_COMM_WORLD, &status );
      if (myrank == 0) printf( "Test completed\n" );
	}
    MPI_Finalize();
    return 0;
}
