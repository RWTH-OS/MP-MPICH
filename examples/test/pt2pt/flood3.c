/* $Id$ */

/* This program tests a flood of data for both unexpected and expected messages   
   to test any internal message fragmentation or protocol shifts.

   This is a variant of flood.c. The only difference is that the message buffers are
   allocated only *once* for all operations (in the size big enough for the maximum message).

   Optional arguments
    -m msg_size     maximum message size in ints:  
    -b msg_size     minimal message size in ints:  
    -r root_rank    root process (the one who recvs all messages)
    -s              allocate shared memory via MPI_Alloc_mem
    -p              allocate private memory via MPI_Alloc_mem
    -a              allocate memory separately for each message    
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mpi.h"
#include "test.h"

#define MAX_REQ 16
#define DEF_MAX_MSG 2000000
#define DEF_BASE_MSG 1

#define HAVE_MPI_ALLOC_MEM

/* different type of memory allocation */
typedef enum { MALLOC, MPI_ALLOC_PRIV, MPI_ALLOC_SHARED } alloc_style_t;

void SetupData ( int *, int, int );
void SetupRdata ( int *, int );
int  CheckData ( int *, int, int, MPI_Status * );

static int verbose = 1;

void *alloc_buf (size_t len, alloc_style_t type)
{
    MPI_Info info;
    void *buf = NULL;

#ifdef HAVE_MPI_ALLOC_MEM    
    if (type != MALLOC) {
	MPI_Info_create (&info);
	MPI_Info_set (info, "alignment", "8");

	switch (type) {
	MPI_ALLOC_PRIV: 
	    MPI_Info_set (info, "type", "private");
	    break;
	MPI_ALLOC_SHARED:
	    MPI_Info_set (info, "type", "shared");
	    break;
	}

	MPI_Alloc_mem (len, info, &buf);
	MPI_Info_free (&info);
   } else 
#endif	
	buf = malloc(len);

    if (buf == NULL) {
	fprintf (stderr, "Could not allocate %d byte buffer\n", len);
	MPI_Abort (MPI_COMM_WORLD, -1);
    }
    
    return buf;
}

void free_buf (void *buf, alloc_style_t type)
{
#ifdef HAVE_MPI_ALLOC_MEM    
    if (type != MALLOC) {
	MPI_Free_mem (buf);
    } else 
#endif	
	free (buf);

    return;
}

int main( int argc, char **argv )
{
    MPI_Comm comm;
    MPI_Request r[MAX_REQ];
    MPI_Status  s[MAX_REQ];
    int msgsize, maxmsg, root = 0, i, size, rank, err = 0, toterr, c;
    int alloc_once = 1;
    int max_msgsize = DEF_MAX_MSG;
    int base_msgsize = DEF_BASE_MSG;
    int *sbuf, *rbuf;
    alloc_style_t alloc_style = MALLOC;
    
    MPI_Init( &argc, &argv );

    comm = MPI_COMM_WORLD;

    MPI_Comm_size( comm, &size );
    MPI_Comm_rank( comm, &rank );

    if (size < 2) {
	printf( "This test requires at least 2 processors\n" );
	MPI_Abort( comm, 1 );
    }

    if (rank == 0) {
	while((c = getopt(argc, argv, "m:r:b:asp?")) != EOF) {
	    switch(c) {
	    case 'm':
		max_msgsize = atoi (optarg);
		break;
	    case 'r':
		root = atoi (optarg);
		break;
	    case 's':
		alloc_style = MPI_ALLOC_SHARED;
		break;
	    case 'p':
		alloc_style = MPI_ALLOC_PRIV;
		break;
	    case 'a':
		alloc_once = 0;
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
		    printf("  -s             allocate shared memory for message buffers\n");
		    printf("  -p             allocate private memory for message buffers\n");
		    printf("  -a             allocate message buffers only once for all transfers\n");
		}
		MPI_Abort(MPI_COMM_WORLD, 1);
		break;
	    }
	}
    }
    MPI_Bcast( &max_msgsize, 1, MPI_INT, 0, MPI_COMM_WORLD );
    MPI_Bcast( &base_msgsize, 1, MPI_INT, 0, MPI_COMM_WORLD );
    MPI_Bcast( &root, 1, MPI_INT, 0, MPI_COMM_WORLD );
    MPI_Bcast( &alloc_style, 1, MPI_INT, 0, MPI_COMM_WORLD );
    MPI_Bcast( &alloc_once, 1, MPI_INT, 0, MPI_COMM_WORLD );

    /* allocate buffers for all message transfers */
    if (alloc_once) {
	if (rank == root) {
	    rbuf = alloc_buf (max_msgsize * sizeof(int), alloc_style);
	} else {
	    sbuf = alloc_buf (max_msgsize * sizeof(int), alloc_style);
	}
    }

    /* First, try large blocking sends to root */
    msgsize = base_msgsize;
    maxmsg  = max_msgsize;
    if (rank == root && verbose) {
	printf( "Blocking sends: " ); fflush (stdout); }
    while (msgsize < maxmsg) {
	if (rank == root) {
	    if (verbose) { printf( "%d ", msgsize ); fflush( stdout ); }
	    for (i=0; i<size; i++) {
		if (i == rank) continue;

		if (!alloc_once)
		    rbuf = alloc_buf (msgsize * sizeof(int), alloc_style);
		SetupRdata( rbuf, msgsize );

		MPI_Recv( rbuf, msgsize, MPI_INT, i, 2*i, comm, s );

		err += CheckData( rbuf, msgsize, 2*i, s );
		if (!alloc_once)
		    free_buf (rbuf, alloc_style);
	    }
	}
	else {
	    if (!alloc_once)
		sbuf = alloc_buf (msgsize * sizeof(int), alloc_style);
	    SetupData( sbuf, msgsize, 2*rank );
	    MPI_Send( sbuf, msgsize, MPI_INT, root, 2*rank, comm );
	    if (!alloc_once)
		free_buf (sbuf, alloc_style);
	}
	msgsize *= 4;
    }
    if (rank == root && verbose) { printf( "\n" ); fflush( stdout ); }

    /* Next, try unexpected messages with Isends */
    msgsize = base_msgsize;
    maxmsg  = max_msgsize;
    if (rank == root && verbose) {
	printf( "Unexpected recvs: " ); fflush (stdout); }
    while (msgsize < maxmsg) {
	if (rank == root) {
	    if (verbose) { printf( "%d ", msgsize ); fflush( stdout ); }
	    MPI_Barrier( comm );
	    for (i=0; i<size; i++) {
		if (i == rank) continue;

		if (!alloc_once)
		    rbuf = alloc_buf (msgsize * sizeof(int), alloc_style);
		SetupRdata( rbuf, msgsize );

		MPI_Recv( rbuf, msgsize, MPI_INT, i, 2*i, comm, s );

		err += CheckData( rbuf, msgsize, 2*i, s );
		if (!alloc_once)
		    free_buf (rbuf, alloc_style);
	    }
	}
	else {
	    if (!alloc_once)
		sbuf = alloc_buf (msgsize * sizeof(int), alloc_style);
	    SetupData( sbuf, msgsize, 2*rank );
	    MPI_Isend( sbuf, msgsize, MPI_INT, root, 2*rank, comm, r );
	    MPI_Barrier( comm );
	    MPI_Wait( r, s );
	    if (!alloc_once)
		free_buf (sbuf, alloc_style);
	}
	msgsize *= 4;
    }
    if (rank == root && verbose) { printf( "\n" ); fflush( stdout ); }

    /* Try large synchronous blocking sends to root */
    msgsize = base_msgsize;
    maxmsg  = max_msgsize;
    if (rank == root && verbose) {
	printf( "Synchronous sends: " ); ; fflush (stdout); }
    while (msgsize < maxmsg) {
	if (rank == root) {
	    if (verbose) { printf( "%d ", msgsize ); fflush( stdout ); }
	    for (i=0; i<size; i++) {
		if (i == rank) continue;

		if (!alloc_once)
		    rbuf = alloc_buf (msgsize * sizeof(int), alloc_style);
		SetupRdata( rbuf, msgsize );

		MPI_Recv( rbuf, msgsize, MPI_INT, i, 2*i, comm, s );

		err += CheckData( rbuf, msgsize, 2*i, s );
		if (!alloc_once)
		    free_buf (rbuf, alloc_style);
	    }
	}
	else {
	    if (!alloc_once)
		sbuf = alloc_buf (msgsize * sizeof(int), alloc_style);
	    SetupData( sbuf, msgsize, 2*rank );
	    MPI_Ssend( sbuf, msgsize, MPI_INT, root, 2*rank, comm );
	    if (!alloc_once)
		free_buf (sbuf, alloc_style);
	}
	msgsize *= 4;
    }
    if (rank == root && verbose) { printf( "\n" ); fflush( stdout ); }

    /* Next, try expected messages with Rsend */
    msgsize = base_msgsize;
    maxmsg  = max_msgsize;
    if (rank == root && verbose) {
	printf( "Expected recvs and Rsend: " ); fflush (stdout); }
	
    while (msgsize < maxmsg) {
	if (rank == root) {
	    if (verbose) { printf( "%d ", msgsize ); fflush( stdout ); }
	    for (i=0; i<size; i++) {
		if (i == rank) continue;

		if (!alloc_once)
		    rbuf = alloc_buf (msgsize * sizeof(int), alloc_style);
		SetupRdata( rbuf, msgsize );

		MPI_Irecv( rbuf, msgsize, MPI_INT, i, 2*i, comm, r );
		MPI_Send( MPI_BOTTOM, 0, MPI_INT, i, 2*i+1, comm );
		MPI_Wait( r, s );

		err += CheckData( rbuf, msgsize, 2*i, s );
		if (!alloc_once)
		    free_buf (rbuf, alloc_style);
	    }
	}
	else {
	    if (!alloc_once)
		sbuf = alloc_buf (msgsize * sizeof(int), alloc_style);
	    SetupData( sbuf, msgsize, 2*rank );
	    MPI_Recv( MPI_BOTTOM, 0, MPI_INT, root, 2*rank+1, comm, s );
	    MPI_Rsend( sbuf, msgsize, MPI_INT, root, 2*rank, comm );
	    if (!alloc_once)
		free_buf (sbuf, alloc_style);
	}
	msgsize *= 4;
    }
    if (rank == root && verbose) { printf( "\n" ); fflush( stdout ); }

    MPI_Allreduce( &err, &toterr, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
    
    if (rank == root) {
	if (toterr == 0) printf( "No errors\n" );
	else printf( "!! found %d errors\n", toterr );
    }
    if (toterr) {
	printf( "!! found %d errors on processor %d\n", err, rank );
    }

    if (alloc_once) {
	if (rank == root) {
	    free_buf (rbuf, alloc_style);
	} else {
	    free_buf (sbuf, alloc_style);
	}
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
