/* check for memory usage */

#include <stdio.h>

#ifndef WIN32
#include <unistd.h>
#endif
#include <stdlib.h>
#include <assert.h>

#include "mpi.h"
#ifdef WIN32
#include "memlog_win.h"
#endif
#include <iostream>

#define MAXPES 32
#define MYBUFSIZE 16*16*1024
static int sendbufs[MAXPES][MYBUFSIZE];
static int recvbufs[MAXPES][MYBUFSIZE];

#ifndef MSGSIZE
#define MSGSIZE 50000
#endif

#define RUNTIME 1200


int main ( int argc, char *argv[] )
{
  int i;
  int self, npes;
  double t0;
  int loopcount = 0;
#ifdef WIN32
  DWORD virtualmem1 = 0,virtualmem2 = 0;
#endif
  MPI_Request sendreqs[MAXPES];
  MPI_Request recvreqs[MAXPES];
  MPI_Status status[MAXPES];

std::cout<<"calling MPI_Init\n"<<std::flush;

  MPI_Init (&argc, &argv);
  MPI_Comm_rank (MPI_COMM_WORLD, &self);
  MPI_Comm_size (MPI_COMM_WORLD, &npes);

  assert (npes <= MAXPES);
  assert (MSGSIZE*sizeof(int) <= MYBUFSIZE);
  
  t0 = MPI_Wtime();
  std::cout<<"calling "<<RUNTIME<<" loops\n"<<std::flush;
  std::cout<<"message size "<<MSGSIZE<<" \n"<<std::flush;
  while (RUNTIME == 0 || loopcount < RUNTIME){
  //while (MPI_Wtime() - t0 < RUNTIME) {
      for (i = 0; i < npes; i++) {
#ifdef WIN32
		  virtualmem1 = GetProcessSize();
#endif
		  MPI_Irecv (&recvbufs[i], MSGSIZE, MPI_INT, i, 0, MPI_COMM_WORLD, &recvreqs[i]);
#ifdef WIN32
		  virtualmem2 = GetProcessSize();
		  if (virtualmem2>virtualmem1)
		  {
			  std::cout<<"mem increase: "<<(virtualmem2-virtualmem1)<<" at MPI_Irecv\n"<<std::flush;
		  }
#endif  
	  
      }

#ifdef EXPECTED
      MPI_Barrier(MPI_COMM_WORLD);
#endif

      for (i = 0; i < npes; i++) {
#ifdef WIN32
		  virtualmem1 = GetProcessSize();
#endif
		  MPI_Isend (&sendbufs[i], MSGSIZE, MPI_INT, i, 0, MPI_COMM_WORLD, &sendreqs[i]);
#ifdef WIN32
		  virtualmem2 = GetProcessSize();
		  if (virtualmem2>virtualmem1)
		  {
			  std::cout<<"mem increase: "<<(virtualmem2-virtualmem1)<<" at MPI_Isend\n"<<std::flush;
		  }
#endif  
	  
      }
#ifdef WIN32
	  virtualmem1 = GetProcessSize();
#endif
	  MPI_Waitall (npes, sendreqs, status);
#ifdef WIN32
	  virtualmem2 = GetProcessSize();
	  if (virtualmem2>virtualmem1)
	  {
		  std::cout<<"mem increase: "<<(virtualmem2-virtualmem1)<<" at MPI_Waitall sendreqs\n"<<std::flush;
	  }
#endif     
#ifdef WIN32
	  virtualmem1 = GetProcessSize();
#endif
	  MPI_Waitall (npes, recvreqs, status);
	  loopcount++;
#ifdef WIN32
	  virtualmem2 = GetProcessSize();
	  if (virtualmem2>virtualmem1)
	  {
		  std::cout<<"mem increase: "<<(virtualmem2-virtualmem1)<<" at MPI_Waitall recvreqs\n"<<std::flush;
	  }
#endif  
      
  }
  std::cout<<"calling MPI_Finalize\n"<<std::flush;
  MPI_Finalize();
  std::cout<<"exiting\n"<<std::flush;
  return (0);
}
