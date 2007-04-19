#include "mpi.h"
#include <stdio.h>
#include "dtypes.h"
#include "gcomm.h"
    
#if defined(NEEDS_STDLIB_PROTOTYPES)
#include "protofix.h"
#endif
int main( int argc, char **argv ) {
int rank;
int size;
MPI_Status   status;

MPI_Comm     comms[20];
int          ncomm = 20;
MPI_Group group, newgroup;
int range[1][3];
int cnt = 0;

MPI_Init( &argc, &argv );
MPI_Comm_rank( MPI_COMM_WORLD, &rank );
MPI_Comm_size( MPI_COMM_WORLD, &size );

/*MakeComms( comms, 20, &ncomm, 0 );*/
comms[cnt++] = MPI_COMM_WORLD;

MPI_Comm_group(MPI_COMM_WORLD,&group);
range[0][0] = size-1;
range[0][1] = 0;
range[0][2] = -1;
MPI_Group_range_incl( group, 1, range, &newgroup );
MPI_Comm_create( MPI_COMM_WORLD, newgroup, &comms[cnt] );

printf("Finished without segfault...\n");


MPI_Finalize();

}


