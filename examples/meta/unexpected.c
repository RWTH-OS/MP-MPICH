#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
/*#include "test.h"*/

#define MAX_REQ 16
#define DEF_MAX_MSG 2000000
/* 
   This program tests a flood of data for both unexpected and expected messages   to test any internal message fragmentation or protocol shifts

   An optional argument can change the maximum message size.  For example, use
      flood 9000000
   to stress the memory system (the size is the number of ints, not bytes)
 */

void SetupData ( char *, unsigned int, int );
void SetupRdata ( char *, unsigned int );
int  CheckData ( char *, unsigned int, int, MPI_Status * );

static int verbose = 1;

int main( int argc, char **argv )
{
    MPI_Comm comm;
    MPI_Request r[MAX_REQ];
    MPI_Status  s[MAX_REQ];
    int msgsize, maxmsg, root, i, size, rank, err = 0, toterr;
    int max_msg_size = DEF_MAX_MSG;
    char *sbuf, *rbuf;

    MPI_Init( &argc, &argv );

    comm = MPI_COMM_WORLD;

    MPI_Comm_size( comm, &size );
    MPI_Comm_rank( comm, &rank );

    if (size < 2) {
	printf( "This test requires at least 2 processors\n" );
	MPI_Abort( comm, 1 );
    }

    /* Check for a max message argument */
    if (rank == 0) {
	if (argc > 1) {
	    max_msg_size = atoi( argv[1] );
	    /* Correct if unrecognized argument */
	    if (max_msg_size <= 0) max_msg_size = DEF_MAX_MSG;
	}
    }
    /*    MPI_Bcast( &max_msg_size, 1, MPI_INT, 0, MPI_COMM_WORLD );*/

    /* First, try large blocking sends to root */
    root = 0;

    /*    msgsize = 16349;*/
    msgsize=32*1024;

    /* expected receive */
 
    if (rank == 0) {
	rbuf = (char *)malloc( msgsize * sizeof(char) );
	MPI_Recv( rbuf, msgsize, MPI_CHAR, 1, 2, comm, s );
	MPI_Barrier(comm);
    } else {
	sbuf = (char *)malloc( msgsize * sizeof(char) );
	MPI_Barrier( comm);
	MPI_Isend( sbuf, msgsize, MPI_CHAR, root, 2, comm, r );
    }
   
#ifdef bla 
   maxmsg  = msgsize+1;/*max_msg_size*/;
    if (rank == root && verbose) printf( "Unexpected recvs: " );
    while (msgsize < maxmsg) {
	if (rank == root) {
	    if (verbose) { printf( "%d ", msgsize ); fflush( stdout ); }
	    rbuf = (char *)malloc( msgsize * sizeof(char) );
	    if (!rbuf) {
		printf( "Could not allocate %d words\n", msgsize );
		MPI_Abort( comm, 1 );
	    }
	    MPI_Barrier( comm );
	    for (i=0; i<size; i++) {
		if (i == rank) continue;
		SetupRdata( rbuf, msgsize );
		MPI_Recv( rbuf, msgsize, MPI_CHAR, i, 2*i, comm, s );
		err += CheckData( rbuf, msgsize, 2*i, s );
	    }
	    free( rbuf );
	}
	else {
	    sbuf = (char *)malloc( msgsize * sizeof(char) );
	    if (!sbuf) {
		printf( "Could not allocate %d words\n", msgsize );
		MPI_Abort( comm, 1 );
	    }
	    SetupData( sbuf, msgsize, 2*rank );
	    MPI_Isend( sbuf, msgsize, MPI_CHAR, root, 2*rank, comm, r );
	    MPI_Barrier( comm );
	    MPI_Wait( r, s );
	    free( sbuf );
	}
	msgsize += 1;
    }
    if (rank == 0 && verbose) { printf( "\n" ); fflush( stdout ); }
#endif 
    MPI_Finalize( );
    return 0;
}

void SetupData( sbuf, n, tag )
char *sbuf;
unsigned int  n;
int tag;
{
    int i;

    for (i=0; i<n; i++) 
	sbuf[i] = i % 256;
}

int CheckData( rbuf, n, tag, s )
char *rbuf;
unsigned int  n;
int tag;
MPI_Status *s;
{
    int act_n, i;

    MPI_Get_count( s, MPI_CHAR, &act_n );
    if (act_n != n) {
	printf( "Received %d instead of %d char\n", act_n, n );
	return 1;
    }
    for (i=0; i<n; i++) {
	if (rbuf[i] != i % 256) {
	    printf( "rbuf[%d] is %d, should be %d\n", i, rbuf[i], i );
	    return 1;
	}
    }
    return 0;
}

void SetupRdata( rbuf, n )
char *rbuf;
unsigned int n;
{
    int i;
    
    for (i=0; i<n; i++) rbuf[i] = -(i+1);
}
