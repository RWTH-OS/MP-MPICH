#include "mpi.h"
#include <stdio.h>
#include <unistd.h>
#include <math.h>

/* 
#define PRINT((A)) fprintf (stderr, "[%d] ", w_id); fprintf (stderr, (A)); fprintf (stderr, "\n");
*/

#define BUFFER_SIZE (1024*2048/sizeof(int))
#define START 200000
#define STRIDE 10000
#define OP += 

#define REPEAT 10
#define VERBOSE 1
#define VERIFY 1

/* simple test to determine the messaging bandwith between node 0 and 1 */

int main(argc,argv)
int argc;
char *argv[];
{
    int w_np, w_id, l_np, l_id;
    int namelen;
    int i, j;
    int split_size, nbr_channels, repeat, nbr_pkts;
    int buffer[BUFFER_SIZE];

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    FILE *results;

    double start_time, end_time;

    MPI_Status status;

    /* init */
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&w_np);
    MPI_Comm_rank(MPI_COMM_WORLD,&w_id);
    MPI_Comm_size(MPI_COMM_LOCAL,&l_np);
    MPI_Comm_rank(MPI_COMM_LOCAL,&l_id);

    MPI_Get_processor_name(processor_name,&namelen);
    printf("Process %d on %s\n w_np = %d w_id = %d l_np = %d l_id = %d\n", 
	    w_id, processor_name, w_np, w_id, l_np, l_id);    

    if (argc == 3) {
      split_size = atoi(argv[1]);
      nbr_channels = atoi(argv[2]);
    } else {
      printf ("Usage: mpi_speed split_size nbr_channels\n");
      printf ("Running test, but logfile wil be incomplete.\n");

      split_size = -1;
      nbr_channels = -1;
    }
    nbr_pkts = REPEAT;

    /*
     * test world communicator 
     */
    printf ("[%d] Testing COMM_WORLD\n", w_id);

    if (w_id == 0) {
      /* sender */
      
      /* init sendbuffer */
      for (i = 0; i < BUFFER_SIZE; i++)
	buffer[i] = i;

#ifdef VERBOSE
      printf ("[%d] Barrier synchronization.\n", w_id);    
#endif
      MPI_Barrier(MPI_COMM_WORLD);
      
      for (i = START; i <= BUFFER_SIZE; i OP STRIDE) {
#ifdef VERBOSE
	printf ("[%d] Sending %d MPI_INTs\n", w_id, i);
#endif
	for (repeat = 0; repeat < REPEAT; repeat++)
	  MPI_Send ((void *)buffer, i, MPI_INT, 1, 0, MPI_COMM_WORLD);
      }
    } else {
      /* receiver */
      results = fopen ("mpi_speed.txt", "w");
      fprintf (results, "# %d channels, splitsize = %d, nbr_pkts = %d\n", 
	       nbr_channels, split_size, nbr_pkts);

#ifdef VERBOSE
      printf ("[%d] Barrier synchronization.\n", w_id);    
#endif
      MPI_Barrier(MPI_COMM_WORLD);
      
      for (i = START; i <= BUFFER_SIZE; i OP STRIDE) {
#ifdef VERBOSE
	printf ("[%d] Receiving %d MPI_INTs\n", w_id, i);
#endif
	start_time = MPI_Wtime();
	for (repeat = 0; repeat < REPEAT; repeat++) {
	  MPI_Recv ((void *)buffer, i, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
#ifdef VERIFY
	  for (j = 0; j < i; j++) {
	    if (buffer[j] != j) {
	      printf ("[%d] *** MPI_recv from 0 failed: expected %d, got %d\n", 
		      w_id, j, buffer[j]); fflush (stdout);
	      exit (99);
	    }
	  }
#endif
	}
	end_time = MPI_Wtime();
	fprintf (results, "%7d %5.2f\n", i*sizeof(int),
		 i*sizeof(int)*REPEAT/(1024*(end_time - start_time)));
      }
    }
    printf ("[%d] COMM_WORLD test done.\n", w_id);
    
    MPI_Finalize();
    fclose (results);
    return (0);
}

            
