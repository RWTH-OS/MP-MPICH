/* $Id$
 *
 * This file tests to see if short,eager,and rndv messages can all be 
 * successfully cancelled.  If they cannot be cancelled, then the 
 * program still must successfully complete.
 */

#include "mpi.h"
#include <stdio.h>

#if defined(NEEDS_STDLIB_PROTOTYPES)
#include "protofix.h"
#endif

#define SHORT_LEN  4
#define EAGER_LEN  1000
#define RNDV_LEN   50000


int main( argc, argv )
int argc;
char **argv;
{

    double       sbuf[RNDV_LEN], rbuf[RNDV_LEN];
    int          rank;
    int          n, flag, size;
    int          err = 0;
    MPI_Status   status;
    MPI_Request  req;

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );

    if (size < 2) {
	  printf( "Cancel test requires at least 2 processes\n" );
	  MPI_Abort( MPI_COMM_WORLD, 1 );
    }


    /* Short Message Test */
    n = SHORT_LEN;

    if (rank == 1) { 
	  MPI_Isend( &sbuf, n, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &req );
	  MPI_Cancel(&req); 
	  MPI_Wait(&req, &status);
	  MPI_Test_cancelled(&status, &flag);
	  if (!flag) {
	    err++;
	    printf( "Cancelling a short message failed where it should succeed.\n" );
	    MPI_Request_free( &req );
	  }
#ifdef _DEBUG
	  else
	  {
		  printf( "Cancelling a short message ok.\n" );
	  }
#endif
    }  
#ifdef _DEBUG
	printf("[%i] Barrier1\n",rank);fflush(stdout);
	Sleep(1);
#endif
    MPI_Barrier(MPI_COMM_WORLD); 
/*
//SI// receive leads to unpredictable results, if receiver is not fast enough
	   then the process hangs
    if (rank == 0) { 
	  MPI_Recv( &rbuf, n, MPI_DOUBLE, 1, 1, MPI_COMM_WORLD, &status);
	  printf( "received message\n" );
    } 

    else 
	*/
	if (rank == 1) { 
	  MPI_Isend( &sbuf, n, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &req );
	  MPI_Cancel(&req); 
	  MPI_Wait(&req, &status);
	  MPI_Test_cancelled(&status, &flag);
	  if (status.MPI_ERROR != MPI_SUCCESS) {
	    err++;
	    printf( "Cancel of a send returned an error in the status field.\n" );
	  }
#ifdef _DEBUG
	  else
	  {
		  printf( "Cancel of a send returned ok.\n" );
	  }
#endif
    } 
#ifdef _DEBUG
	printf("[%i] Barrier2\n",rank);fflush(stdout);
	Sleep(1);
#endif
    MPI_Barrier(MPI_COMM_WORLD);

    /* Eager Message Test */
    n = EAGER_LEN;

    if (rank == 1) { 
	  MPI_Isend( &sbuf, n, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &req );
	  MPI_Cancel(&req);
	  MPI_Wait(&req, &status);
	  MPI_Test_cancelled(&status, &flag);
	  if (!flag) {
	    err++;
	    printf( "Cancelling a eager message failed where it should succeed.\n" );
	}
    }  
#ifdef _DEBUG
	printf("[%i] Barrier3\n",rank);fflush(stdout);
	Sleep(1);
#endif
    MPI_Barrier(MPI_COMM_WORLD); 

    if (rank == 0) {
	  MPI_Irecv(&rbuf, n, MPI_DOUBLE, 1, 1, MPI_COMM_WORLD, &req );
/*
	  //SI// wait leads to unpredictable results, if receiver is not fast enough
	  then the process hangs
	  MPI_Wait( &req, &status);
*/
    }  
    else if (rank == 1) {
	  MPI_Isend( &sbuf, n, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &req );
	  MPI_Cancel(&req);
	  MPI_Wait(&req, &status);
	  MPI_Test_cancelled(&status, &flag);
	  if (status.MPI_ERROR != MPI_SUCCESS) {
	    err++;
	    printf( "Cancel of a send returned an error in the status field.\n" );
	}
    } 
#ifdef _DEBUG
	printf("[%i] Barrier4\n",rank);fflush(stdout);
	Sleep(1);
#endif
    MPI_Barrier(MPI_COMM_WORLD);

    /* Rndv Message Test */
    n = RNDV_LEN;

    if (rank == 1) { 
	  MPI_Isend( &sbuf, n, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &req );
	  MPI_Cancel(&req);
	  MPI_Wait(&req, &status);
	  MPI_Test_cancelled(&status, &flag);
	  if (!flag) {
	    err++;
	    printf( "Cancelling a eager message failed where it should succeed.\n" );
	}
    } 
#ifdef _DEBUG
	printf("[%i] Barrier5\n",rank);fflush(stdout);
	Sleep(1);
#endif
    MPI_Barrier(MPI_COMM_WORLD); 

    if (rank == 0) {
	  MPI_Irecv(rbuf, n, MPI_DOUBLE, 1, 1, MPI_COMM_WORLD, &req );
/*
	  //SI// wait leads to unpredictable results, if receiver is not fast enough
	  then the process hangs
	  MPI_Wait( &req, &status);
*/
    } 
    else if (rank == 1) {
	  MPI_Isend( sbuf, n, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &req );
	  MPI_Cancel(&req);
	  MPI_Wait(&req, &status);
	  MPI_Test_cancelled(&status, &flag);
	  if (status.MPI_ERROR != MPI_SUCCESS) {
	    err++;
	    printf( "Cancel of a send returned an error in the status field.\n" );
	}
    }
#ifdef _DEBUG
	printf("[%i] Barrier6\n",rank);fflush(stdout);
	Sleep(1);
#endif
    MPI_Barrier(MPI_COMM_WORLD); 

    if (rank == 1) { 
	  if (err) {
	    printf( "Test failed with %d errors.\n", err );
	  }
	  else {
	    printf( "Test passed\n" );
	  }
    }

    MPI_Finalize( );

    return 0;
}
