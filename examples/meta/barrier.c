/*
 * Program to test that the "no overtaking messages" semantics
 * of point to point communications in MPI is satisfied. 
 /* Simple barrier test
 */

#include <stdio.h>
/* Needed for malloc declaration */
#include <stdlib.h>
#include "mpi.h"



int main(argc, argv)
int argc;
char **argv;
{
    int rank; /* My Rank (0 or 1) */

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    printf("my rank is %d\n", rank);

    MPI_Barrier (MPI_COMM_WORLD);
    
    MPI_Finalize();
    return 0;
}



