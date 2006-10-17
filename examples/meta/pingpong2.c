
#include <mpi.h>
#include <stdio.h>

#define MAXSIZE (4*1024*1024)
#define MINSIZE (128*1024)
#define REPEAT    2
char buffer[MAXSIZE];
void ping (int to, int from);
void pong(int from, int myr);

int main(int argc, char **argv) {

  int myrank,mysize;
  MPI_Status status;

  MPI_Init(&argc, &argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &mysize);
  if(mysize % 2 !=0) {
    printf("Es muss eine gerade Prozessanzahl gestartet werden.\n");
    exit(1);
  }
  /*
  if(myrank%2==0) {
     ping(myrank+1,myrank);
  } else {
     pong(myrank-1,myrank);
  }
  */

  if(myrank<mysize/2) {
      ping(mysize/2+myrank,myrank);
  } else {
     pong(myrank-mysize/2,myrank);
  }
  MPI_Finalize();
}

/* ================================================================ */
void ping(int to,int from) {
  int i, j, k;
  double starttime, totaltime;
  MPI_Status status;

  printf("Ping-Pong von %d nach %d\n================\n",from,to);fflush(stdout);
  for(i=MINSIZE; i<=MAXSIZE; i*=2) {
    starttime=MPI_Wtime();
    for(j=1; j<REPEAT; j++) {
      for(k=0; k<2; k++) {
	MPI_Barrier(MPI_COMM_WORLD);
	if(k==from) {
	  MPI_Send(buffer, i, MPI_CHAR, to, 1+100*from, MPI_COMM_WORLD);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	if(k==from) {
	  MPI_Recv(buffer, i, MPI_CHAR, to, 2+100*to  , MPI_COMM_WORLD, &status);
	}
      }   /* for k */
    }   /* for j =1..REPEAT */
    totaltime = MPI_Wtime()-starttime;
    printf("size = %7d, time = %7.4f sec., bandwidth = %10.3f MByte/s\n",
           i,totaltime,2*i*REPEAT/totaltime/1024/1024);fflush(stdout);
  }   /* for i=1..MAXSIZE */
}  

/* ================================================================ */

void pong(int from, int myr) {
  int i, j, k;
  MPI_Status status;

  for(i=MINSIZE; i<=MAXSIZE; i*=2) {
    for(j=1; j<REPEAT; j++) {
      for(k=2; k<4; k++) {
	MPI_Barrier(MPI_COMM_WORLD);
	if(k==myr) {
	  MPI_Recv(buffer, i, MPI_CHAR, from, 1+100*from, MPI_COMM_WORLD, 
		   &status);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	if(k==myr) {
	  MPI_Send(buffer, i, MPI_CHAR, from, 2+100*myr, MPI_COMM_WORLD);
	}   /* if */
      }   /* for k */
    }   /* for j =1..REPEAT */
  }   /* for i=1..MAXSIZE */
}
