/* split.c - test how many new communicators can be generated */

#include <stdio.h>
#include "mpi.h"
#include "test.h"

#define N_COMMS  50

int main( int argc, char **argv )
{
  int size, rank, key, n, m, k;
  MPI_Comm split_comms[N_COMMS];
  int mpi_errno, errors = 0, sum_errors;
  MPI_Status status;
  
  MPI_Init ( &argc, &argv );
  MPI_Comm_rank ( MPI_COMM_WORLD, &rank);

  /* Generate membership key in the range [0,1] */
  key = rank % 2;   

  /* Try to create a large number of new communicators. Well, what is large? Set N_COMMS
     to any value you need, but 1000 is already a significant number. 

     We also could perform more complex and "parallel" communicator creation - this is left 
     for future versions of this test. */
  for (n = 0, mpi_errno = MPI_SUCCESS; n < N_COMMS && mpi_errno == MPI_SUCCESS; n++) {
      mpi_errno = MPI_Comm_split (MPI_COMM_WORLD, key, rank, &split_comms[n]);
  }
  if (!rank) {
      if (mpi_errno == MPI_SUCCESS) 
	  printf ("First run: successfully created %d communicators.\n", N_COMMS);
      else
	  printf ("First run: creation of %dth communicator failed with MPI error %d\n", mpi_errno);
  }

  for (m = 0; m < n; m++)
      MPI_Comm_free (&split_comms[m]);
  
  /* It should be possible to create the same number of communicators again! */
  for (m = 0, mpi_errno = MPI_SUCCESS; m < n && mpi_errno == MPI_SUCCESS; m++) {
      mpi_errno = MPI_Comm_split (MPI_COMM_WORLD, key, rank, &split_comms[m]);
  }
  if (!rank) {
      if (mpi_errno == MPI_SUCCESS) 
	  printf ("Second run: successfully created %d communicators (again).\n", N_COMMS);
      else
	  printf ("Second run: creation of %dth communicator failed with MPI error %d\n", mpi_errno);
  }

  for (k = 0; k < m; k++)
      MPI_Comm_free (&split_comms[k]);

  MPI_Finalize();
  return 0;
}










