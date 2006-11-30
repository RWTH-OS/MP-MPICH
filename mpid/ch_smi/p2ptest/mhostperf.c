/* $Id$ */

/* This program tests the performance of asynchronous ("immediate")
   send and receive operations between a number of hosts. It was 
   designed to show the benefit (or disadvantage) of real 
   asynchronous transfers as they are realized in SCI-MPICH using
   threads or DMA. 

   Define VERIFY to enable message contents verification. */

#include <stdio.h>

#include "mpi.h"

int main( int argc, char *argv[] ) {
    int process, message, myid, numprocs, msg_size, nbr_msgs, loops, l;
    char **sendbuffer, **recvbuffer;
    double start_wtime, end_wtime, bandwidth;
    MPI_Request *request, **sendrequest, **recvrequest;
    MPI_Status *status;
#ifdef VERIFY
    char *content;
    int value, slot, false = 0;
#endif

    MPI_Init( &argc, &argv );

    if( argc != 4 ) {
	fprintf( stderr, "Usage: mhostperf msg_size nbr_msgs loops\n");
	MPI_Finalize();
	return( -1 );
    }

    MPI_Comm_rank( MPI_COMM_WORLD, &myid );
    MPI_Comm_size( MPI_COMM_WORLD, &numprocs );

    msg_size = atoi( argv[1] );
    nbr_msgs = atoi( argv[2] );
    loops    = atoi( argv[3] );

    /* allocate array for sendbuffers and initialize pointers */
    if(!(sendbuffer = (char **) malloc( numprocs * sizeof( char *) ) ) ) {
	fprintf( stderr, "Process %d, out of memory error!\n", myid );
	MPI_Finalize();
	return( -1 );
    }
	
    if(!(sendbuffer[0] = (char *) malloc( numprocs * nbr_msgs * msg_size * sizeof( char ) ) ) ) {
	fprintf( stderr, "Process %d, out of memory error!\n", myid );
	MPI_Finalize();
	return( -1 );
    }
	
    for( process = 1; process < numprocs; process++)
	sendbuffer[process] = sendbuffer[process - 1] + (nbr_msgs * msg_size);

#ifdef VERIFY
    /* initialize sendbuffers */
    content = sendbuffer[0];
    for( process = 0; process < numprocs; process++) {
      for( message = 0; message < nbr_msgs; message++) {
	value = ( message + myid + process ) % 256;
	for( slot = 0; slot < msg_size; slot++) {
	  *content = (char) value;
	  content++;
	}
      }
    }
#endif

    /* allocate array for receivebuffers and initialize pointers */
    if(!(recvbuffer = (char **) malloc( numprocs * sizeof( char * ) ) ) ) {
	fprintf( stderr, "Process %d, out of memory error!\n", myid );
	MPI_Finalize();
	return( -1 );
    }

    if(!(recvbuffer[0] = (char *) malloc( numprocs * nbr_msgs * msg_size * sizeof( char ) ) ) ) {
	fprintf( stderr, "Process %d, out of memory error!\n", myid );
	MPI_Finalize();
	return( -1 );
    }

    for( process = 1; process < numprocs; process++)
	recvbuffer[process] = recvbuffer[process - 1] + (nbr_msgs * msg_size);

    /* allocate array for request handles */
    if(!(request = (MPI_Request *) malloc( 2 * (numprocs - 1) * nbr_msgs * sizeof( MPI_Request *) ) ) ) {
	fprintf( stderr, "Process %d, out of memory error!\n", myid);
	MPI_Finalize();
	return(-1);
    }

    /* initialize send request handles */
    if(!(sendrequest = (MPI_Request **) malloc( (numprocs - 1) * sizeof( MPI_Request *) ) ) ) {
	fprintf( stderr, "Process %d, out of memory error!\n", myid);
	MPI_Finalize();
	return(-1);
    }
    
    sendrequest[0] = request;

    for( process = 1; process < numprocs - 1; process++)
	sendrequest[process] = sendrequest[process - 1] + nbr_msgs;

    /* initialize receive request handles */
    if(!(recvrequest = (MPI_Request **) malloc( (numprocs - 1)* sizeof( MPI_Request *) ) ) ) {
	fprintf( stderr, "Process %d, out of memory error!\n", myid);
	MPI_Finalize();
	return(-1);
    }

    recvrequest[0] = sendrequest[numprocs - 2] + nbr_msgs;

    for( process = 1; process < numprocs - 1; process++)
	recvrequest[process] = recvrequest[process - 1] + nbr_msgs;

    /* allocate array for status objects */
    status = (MPI_Status *) malloc( 2 * (numprocs - 1) * nbr_msgs * sizeof(MPI_Status) );
    
    start_wtime = MPI_Wtime();
    
    for (l = 0; l < loops; l++) {
	/* receive messages from processes with process ID smaller than mine */
	for( process = 0; process < myid; process++ ) {
	    for( message = 0; message < nbr_msgs; message++) {
		MPI_Irecv(&(recvbuffer[process][message * msg_size]), msg_size, MPI_CHAR, 
			  process, message, MPI_COMM_WORLD, &(recvrequest[process][message]) );
	    }
	}
	
	/* receive messages from processes with process ID bigger than mine */
	for( process = myid + 1; process < numprocs; process++) {
	    for( message = 0; message < nbr_msgs; message++) {
		MPI_Irecv(&(recvbuffer[process][message * msg_size]), msg_size, MPI_CHAR, 
			  process, message, MPI_COMM_WORLD, &(recvrequest[process - 1][message]) );
	    }
	}
	
	/* send messages to processes with process ID smaller than mine */
	for( process = 0; process < myid; process++ ) {
	    for( message = 0; message < nbr_msgs; message++ ) {
		MPI_Isend(&(sendbuffer[process][message * msg_size]), msg_size, MPI_CHAR, 
			  process, message, MPI_COMM_WORLD, &(sendrequest[process][message]));
	    }
	}
	
	/* send messages to processes with process ID bigger than mine */
	for( process = myid + 1; process < numprocs; process++) {
	    for( message = 0; message < nbr_msgs; message++ ) {
		MPI_Isend(&(sendbuffer[process][message * msg_size]), msg_size, MPI_CHAR, 
			  process, message, MPI_COMM_WORLD, &(sendrequest[process - 1][message]));
	    }
	}
	
	/* Wait for completion of all pending sends and receives */
	MPI_Waitall( 2 * (numprocs - 1) * nbr_msgs, request, status);
    }
    end_wtime = MPI_Wtime();
    
#ifdef VERIFY
    /* test correctness of transmission for messages from processes with 
       process ID smaller than mine */
    content = recvbuffer[0];
    for( process = 0; process < myid; process++) {
      for( message = 0; message < nbr_msgs; message++) {
	value = ( message + process + myid ) % 256;
	for( slot = 0; slot < msg_size; slot++) {
	  if( *content != (char) value )
	    false = 1;
	  content++;
	}
      }
    }

    /* test correctness of transmission for messages from processes with 
       process ID bigger than mine */
    if( myid < numprocs - 1 ) {
      content = recvbuffer[myid + 1];
      for( process = myid + 1; process < numprocs - 1; process++) {
	for( message = 0; message < nbr_msgs; message++) {
	  value = ( message + process + myid ) % 256;
	  for( slot = 0; slot < msg_size; slot++) {
	    if( *content != (char) value )
	      false = 1;
	    content++;
	  }
	}
      }
    }

    if(false)
      fprintf( stderr, "Process %d, Transmission error!\n", myid);
#endif

    bandwidth = 2 * (numprocs - 1) * nbr_msgs * msg_size * loops * sizeof(char) 
	        / ((end_wtime - start_wtime)*1024*1024);
    
    printf("Process %d, elapsed time: %.9fs, bandwidth[MB/s]: %.9f\n", myid, 
	   end_wtime - start_wtime, bandwidth);
    fflush(stdout);

    /* clean up */
    free(status);
    free(request);
    free(recvrequest);
    free(sendrequest);
    free(sendbuffer);
    free(recvbuffer);

    MPI_Finalize();

    return( 0 );
}


