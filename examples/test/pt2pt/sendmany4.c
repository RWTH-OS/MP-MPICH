/* $Id$

   Nonblocking send and recv from all other processes, starting with proc 0 */

#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <stdlib.h>
#include <assert.h>
#include <mpi.h>

#if defined(NEEDS_STDLIB_PROTOTYPES)
#include "protofix.h"
#endif

#define MAXPES 32
#define MYBUFSIZE 16*1024
static int buffer[2*MAXPES][MYBUFSIZE];

#define NUM_RUNS 10


int main ( int argc, char *argv[] )
{
  int i, j;
  int count, size;
  int self, npes;
  double secs;
  MPI_Request request[2*MAXPES];
  MPI_Status status[2*MAXPES];


  MPI_Init (&argc, &argv);
  MPI_Comm_rank (MPI_COMM_WORLD, &self);
  MPI_Comm_size (MPI_COMM_WORLD, &npes);

  assert (npes <= MAXPES);

  for (size = 1; size  <= MYBUFSIZE ; size += size) {
      
      secs = -MPI_Wtime ();
      for (count = 0; count < NUM_RUNS; count++) {
	  MPI_Barrier (MPI_COMM_WORLD);

	  for (j = 0, i = 0; i < npes; i++) {
	      if (i == self)
		continue;
	      MPI_Irecv (buffer[i], size, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &request[j]);
	      j++;
	    }

	  for (j = 0, i = 0; i < npes; i++) {
	      if (i == self)
		continue;
	      MPI_Isend (buffer[npes - 1 + i], size, MPI_INT, i, 0, MPI_COMM_WORLD, &request[npes - 1 + j]);
	      j++;
	    }

	  MPI_Waitall (2*(npes-1), request, status);

	}
      MPI_Barrier (MPI_COMM_WORLD);
      secs += MPI_Wtime ();

      if (self == 0)
	{
	  secs = secs / (double) NUM_RUNS;
	  printf ( "length = %d ints\n", size );
	  fflush(stdout);
/*
	  printf ("%f\n",
		  (double) (size * sizeof (int) * (npes - 1)) /
		    (secs * 1024.0 * 1024.0));
 */
	}
    }
  MPI_Finalize();
  return (0);
}
