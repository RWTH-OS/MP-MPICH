#include "mpi.h"
#include <stdio.h>
#include <string.h>


//#define _DEBUG

#ifndef DBG
#ifdef _DEBUG
#define DBG(m) printf("%s\n",m);
#else
#define DBG(m)
#endif
#endif

int main(int argc,char *argv[])
{
  int myid, numprocs;
  int  namelen;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  char* ping = (char*) malloc(5);
  char* pong = (char*) malloc(5);

  MPI_Status status;
  
  DBG("calling MPI_Init");
  MPI_Init(&argc,&argv);
  DBG("calling MPI_Comm_size");
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  DBG("calling MPI_Comm_rank");
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  DBG("calling MPI_Get_processor_name");
  MPI_Get_processor_name(processor_name, &namelen);

  /*   fprintf(stdout, "Executing %s: Process %d of %d on %s\n",argv[0], */
  /* 	  myid, numprocs, processor_name); */
  fflush (stdout); 

  if (myid == 0)
  {
    strcpy(ping, "PING");
    MPI_Send(ping, 5, MPI_CHARACTER, myid+1, 0,  MPI_COMM_WORLD); 

    printf("Proces %d recieving pong \n", myid);
    MPI_Recv(pong, 5, MPI_CHARACTER, myid+1, 0,  MPI_COMM_WORLD, &status);

    printf("%s\n", pong);
    fflush (stdout);
  }
  if (myid == 1)
  {
    printf("Proces %d receiving ping \n", myid);
    MPI_Recv(ping, 5, MPI_CHARACTER, myid-1, 0,  MPI_COMM_WORLD, &status); 

    printf("%s\n", ping);
    fflush (stdout);

    strcpy(pong, "PONG");
    MPI_Send(pong, 5, MPI_CHARACTER, myid-1, 0,  MPI_COMM_WORLD);
  }
  if(myid >= 2)
    printf("Process %d has nothing to do. Exiting.\n", myid);

  free(ping);
  free(pong);

  DBG("calling MPI_Finalize");
  MPI_Finalize();

  return 0;
}

            
