/* $Id$ */

#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include "getopt.h"
#else
#include "unistd.h"
#endif


#include "mpi.h"
#include "test.h"

#define MAX_REQ 16
#define DEF_MAX_MSG 1000000
#define DEF_BASE_MSG 128

#ifdef USE_MPI_ALLOC
#define ALLOC(buf, type, size) MPI_Alloc_mem (size, NULL, (void *)&(buf))
#define FREE(buf) MPI_Free_mem (buf)
#else
#define ALLOC(buf, type, size) buf = (type *)malloc(size)
#define FREE(buf) free(buf)
#endif 


/* This program tests a flood of data for both unexpected and expected messages   
   to test any internal message fragmentation or protocol shifts.

   Optional arguments
   - maximum message size in ints: -m msg_size  
   - root process (the one who recvs all messages): -r root_rank
*/

void SetupData ( int *, int, int );
void SetupRdata ( int *, int );
int  CheckData ( int *, int, int, MPI_Status * );

static int verbose = 1;

int main( int argc, char **argv )
{
    MPI_Comm comm;
    MPI_Request r[MAX_REQ];
    MPI_Status  s[MAX_REQ];
    int msgsize, maxmsg, root = 0, i, size, rank, err = 0, toterr, c;
    int max_msg_size = DEF_MAX_MSG;
    int base_msgsize = DEF_BASE_MSG;
    int *sbuf, *rbuf;

    MPI_Init( &argc, &argv );

    comm = MPI_COMM_WORLD;

    MPI_Comm_size( comm, &size );
    MPI_Comm_rank( comm, &rank );

    if (size < 2) {
	printf( "This test requires at least 2 processors\n" );
	fprintf( stderr, "[%i] Aborting\n",rank );fflush(stderr);
	MPI_Abort( comm, 1 );
    }

    if (rank == 0) {
	while((c = getopt(argc, argv, "m:r:b:?")) != EOF) {
	    switch(c) {
	    case 'm':
		max_msg_size = atoi (optarg);
		break;
	    case 'r':
		root = atoi (optarg);
		break;
	    case 'b':
		base_msgsize = atoi (optarg);
		break;
	    case '?':
		if ( rank == 0) {
		    printf("Usage: %s [options]\n", argv[0]);
		    printf("  -m MAXSIZE     maximal message size (nbr of ints)\n");
		    printf("  -r ROOT        process ROOT receives messages (default 0)\n");
		    printf("  -b BASESIZE    start with BASESIZE ints for messages\n");
		}
		exit(0);
		break;
	    }
	}
    }
    MPI_Bcast( &max_msg_size, 1, MPI_INT, 0, MPI_COMM_WORLD );
    MPI_Bcast( &base_msgsize, 1, MPI_INT, 0, MPI_COMM_WORLD );
    MPI_Bcast( &root, 1, MPI_INT, 0, MPI_COMM_WORLD );

    /* First, try large blocking sends to root */
    msgsize = base_msgsize;
    maxmsg  = max_msg_size;
    if (rank == root && verbose) {
	printf( "Blocking sends: " ); fflush (stdout); }
    while (msgsize < maxmsg) {
	if (rank == root) {
	    if (verbose) { printf( "%d ", msgsize ); fflush( stdout ); }
	    ALLOC (rbuf, int, msgsize * sizeof(int));
	    if (!rbuf) {
		printf( "Could not allocate %d words\n", msgsize );
		fprintf( stderr, "[%i] Aborting\n",rank );fflush(stderr);
		MPI_Abort( comm, 1 );
	    }
	    for (i=0; i<size; i++) {
		if (i == rank) continue;
		SetupRdata( rbuf, msgsize );
		MPI_Recv( rbuf, msgsize, MPI_INT, i, 2*i, comm, s );
		err += CheckData( rbuf, msgsize, 2*i, s );
	    }
	    FREE( rbuf );
	}
	else {
	    ALLOC (sbuf, int, msgsize * sizeof(int));
	    if (!sbuf) {
		printf( "Could not allocate %d words\n", msgsize );
		fprintf( stderr, "[%i] Aborting\n",rank );fflush(stderr);
		MPI_Abort( comm, 1 );
	    }
	    SetupData( sbuf, msgsize, 2*rank );
	    MPI_Send( sbuf, msgsize, MPI_INT, root, 2*rank, comm );
	    FREE( sbuf );
	}
	msgsize *= 4;
    }
    if (rank == root && verbose) { printf( "\n" ); fflush( stdout ); }

    /* Next, try unexpected messages with Isends */
    msgsize = base_msgsize;
    maxmsg  = max_msg_size;
    if (rank == root && verbose) {
	printf( "Unexpected recvs: " ); fflush (stdout); }
    while (msgsize < maxmsg) {
	if (rank == root) {
	    if (verbose) { printf( "%d ", msgsize ); fflush( stdout ); }
	    ALLOC (rbuf, int, msgsize * sizeof(int));
	    if (!rbuf) {
		printf( "Could not allocate %d words\n", msgsize );
		fprintf( stderr, "[%i] Aborting\n",rank );fflush(stderr);
		MPI_Abort( comm, 1 );
	    }
	    MPI_Barrier( comm );
	    for (i=0; i<size; i++) {
		if (i == rank) continue;
		SetupRdata( rbuf, msgsize );
		MPI_Recv( rbuf, msgsize, MPI_INT, i, 2*i, comm, s );
		err += CheckData( rbuf, msgsize, 2*i, s );
	    }
	    FREE(rbuf);
	}
	else {
	    ALLOC(sbuf, int, msgsize * sizeof(int));
	    if (!sbuf) {
		printf( "Could not allocate %d words\n", msgsize );
		fprintf( stderr, "[%i] Aborting\n",rank );fflush(stderr);
		MPI_Abort( comm, 1 );
	    }
	    SetupData( sbuf, msgsize, 2*rank );
	    MPI_Isend( sbuf, msgsize, MPI_INT, root, 2*rank, comm, r );
	    MPI_Barrier( comm );
	    MPI_Wait( r, s );
	    FREE(sbuf);
	}
	msgsize *= 4;
    }
    if (rank == root && verbose) { printf( "\n" ); fflush( stdout ); }

    /* Try large synchronous blocking sends to root */
    msgsize = base_msgsize;
    maxmsg  = max_msg_size;
    if (rank == root && verbose) {
	printf( "Synchronous sends: " ); ; fflush (stdout); }
    while (msgsize < maxmsg) {
	if (rank == root) {
	    if (verbose) { printf( "%d ", msgsize ); fflush( stdout ); }
	    ALLOC (rbuf, int, msgsize * sizeof(int));
	    if (!rbuf) {
		printf( "Could not allocate %d words\n", msgsize );
		fprintf( stderr, "[%i] Aborting\n",rank );fflush(stderr);
		MPI_Abort( comm, 1 );
	    }
	    for (i=0; i<size; i++) {
		if (i == rank) continue;
		SetupRdata( rbuf, msgsize );
		MPI_Recv( rbuf, msgsize, MPI_INT, i, 2*i, comm, s );
		err += CheckData( rbuf, msgsize, 2*i, s );
	    }
	    FREE (rbuf );
	}
	else {
	    ALLOC (sbuf, int, msgsize * sizeof(int));
	    if (!sbuf) {
		printf( "Could not allocate %d words\n", msgsize );
		fprintf( stderr, "[%i] Aborting\n",rank );fflush(stderr);
		MPI_Abort( comm, 1 );
	    }
	    SetupData( sbuf, msgsize, 2*rank );
	    MPI_Ssend( sbuf, msgsize, MPI_INT, root, 2*rank, comm );
	    FREE( sbuf );
	}
	msgsize *= 4;
    }
    if (rank == root && verbose) { printf( "\n" ); fflush( stdout ); }

    /* Next, try expected messages with Rsend */
    msgsize = base_msgsize;
    maxmsg  = max_msg_size;
    if (rank == root && verbose) {
	printf( "Expected recvs and Rsend: " ); fflush (stdout); }
	
    while (msgsize < maxmsg) {
	if (rank == root) {
	    if (verbose) { printf( "%d ", msgsize ); fflush( stdout ); }
	    ALLOC (rbuf, int, msgsize * sizeof(int));
	    if (!rbuf) {
		printf( "Could not allocate %d words\n", msgsize );
		fprintf( stderr, "[%i] Aborting\n",rank );fflush(stderr);
		MPI_Abort( comm, 1 );
	    }
	    for (i=0; i<size; i++) {
		if (i == rank) continue;
		SetupRdata( rbuf, msgsize );
		MPI_Irecv( rbuf, msgsize, MPI_INT, i, 2*i, comm, r );
		MPI_Send( MPI_BOTTOM, 0, MPI_INT, i, 2*i+1, comm );
		MPI_Wait( r, s );
		err += CheckData( rbuf, msgsize, 2*i, s );
	    }
	    FREE( rbuf );
	}
	else {
	    ALLOC (sbuf, int, msgsize * sizeof(int));
	    if (!sbuf) {
		printf( "Could not allocate %d words\n", msgsize );
		fprintf( stderr, "[%i] Aborting\n",rank );fflush(stderr);
		MPI_Abort( comm, 1 );
	    }
	    SetupData( sbuf, msgsize, 2*rank );
	    MPI_Recv( MPI_BOTTOM, 0, MPI_INT, root, 2*rank+1, comm, s );
	    MPI_Rsend( sbuf, msgsize, MPI_INT, root, 2*rank, comm );
	    FREE( sbuf );
	}
	msgsize *= 4;
    }
    if (rank == root && verbose) { printf( "\n" ); fflush( stdout ); }

    MPI_Allreduce( &err, &toterr, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
    
    if (rank == root) {
	if (toterr == 0) printf( "No Errors\n" );
	else printf( "!! found %d errors\n", toterr );
    }
    if (toterr) {
	printf( "!! found %d errors on processor %d\n", err, rank );
    }

    MPI_Finalize( );
    return 0;
}

void SetupData( sbuf, n, tag )
int *sbuf, n, tag;
{
    int i;

    for (i=0; i<n; i++) 
	sbuf[i] = i;
}

int CheckData( rbuf, n, tag, s )
int *rbuf, n, tag;
MPI_Status *s;
{
    int act_n, i, from = s->MPI_SOURCE;

    MPI_Get_count( s, MPI_INT, &act_n );
    if (act_n != n) {
	fprintf(stderr, "Received %d instead of %d ints from (%d)\n", act_n, n, from);
	return 1;
    }
    for (i=0; i<n; i++) {
	if (rbuf[i] != i) {
	    fprintf(stderr, "rbuf[%d] (of %d) from (%d) is %d, should be %d\n", i, n, from, rbuf[i], i );
	    return 1;
	}
    }
    return 0;
}

void SetupRdata( rbuf, n )
int *rbuf, n;
{
    int i;
    
    for (i=0; i<n; i++) rbuf[i] = -(i+1);
}
