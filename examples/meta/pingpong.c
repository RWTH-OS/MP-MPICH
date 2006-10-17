/* pingpong - measuring bandwidth and latency */

#include <mpi.h>
#include <stdio.h>

#define MAXSIZE (128*256*1024)
#define MINSIZE (1)
#define INCSIZE (2)
#define INCOP   *=
#define REPEAT  10

char buffer[MAXSIZE];
int min_size, max_size, inc_size, repeats;

void ping (int to, int from);
void pong (int from);


int main(int argc, char **argv) {
  MPI_Status status;
  int myrank, mysize;
  int first;
  
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &mysize);
  printf("I am %d / %d\n", myrank, mysize);
  if(mysize % 2 != 0) {
    printf ("pingpong must be used with an even number of processes.\n");
    exit (1);
  }

  if (argc != 5) {
      if ( argc != 1) {
	  printf ("usage: pingpong min_size max_size inc_size repeats\n");
	  printf ("using default values for this run\n");
      }

      min_size = MINSIZE;
      max_size = MAXSIZE;
      inc_size = INCSIZE;
      repeats  = REPEAT;
  } else {
      min_size = atoi( argv[1] );
      max_size = atoi( argv[2] );
      inc_size = atoi( argv[3] );
      repeats  = atoi( argv[4] );
  }

  if ( min_size < 0 ) min_size = 0;
  if ( inc_size < 2 ) inc_size = 2;
  if ( repeats < 1 ) repeats = 1;

  if ( myrank < mysize/2 ) {
      if (myrank % 2 == 0)
	  ping(  myrank + mysize/2, myrank);
      else
	  pong( myrank + mysize/2 );
  } else {
      first = (mysize/2) % 2;
      if (myrank % 2 == first)
	  pong( myrank - mysize/2 );
      else 
	  ping( myrank - mysize/2, myrank );
   }
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
}


void ping( int to, int from ) {
  MPI_Status status;
  double starttime, totaltime;
  int i, j;

  printf("pingpong from %d to %d\n=====================================\n\n", from, to);
  printf("    size [Byte]    oneway-latency [usec]      bandwidth [MByte/s]\n");
  printf("-------------------------------------------------------------------\n");
  fflush(stdout);

  for( i = min_size; i <= max_size; i INCOP inc_size) {
    MPI_Barrier(MPI_COMM_WORLD);

/*     printf ("sending %d bytes\n", i); */
    starttime = MPI_Wtime();

    for( j = 0; j < repeats; j++) {
	MPI_Send( buffer, i, MPI_CHAR, to, 1, MPI_COMM_WORLD);
	MPI_Recv( buffer, i, MPI_CHAR, to, 2, MPI_COMM_WORLD, &status);
    }   
    totaltime = MPI_Wtime() - starttime;

    printf("[%d]->[%d] %15d%25.4f%25.4f\n",from,to,i, totaltime*1000000/repeats/2, i*repeats*2/totaltime/1024/1024);
    fflush(stdout);
  }
}  


void pong( int from ) {
    MPI_Status status;
    int i, j;
    
    for( i = min_size; i <= max_size; i INCOP inc_size) {
	MPI_Barrier(MPI_COMM_WORLD);
	
	for(j = 0; j < repeats; j++) {
	    MPI_Recv( buffer, i, MPI_CHAR, from, 1, MPI_COMM_WORLD, &status);
	    MPI_Send( buffer, i, MPI_CHAR, from, 2, MPI_COMM_WORLD);
	}
    }  
}

