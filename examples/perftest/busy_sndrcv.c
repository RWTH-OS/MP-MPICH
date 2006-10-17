/* $Id: busy_sndrcv.c,v 1.4 2004/03/05 13:57:06 joachim Exp $ */

/* busy_sndrcv - measure overlapping of computation and communication */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>

#include "mpi.h"

#define DEF_NBRTHREADS 1
#define DEF_MSGSIZE    (1024*1024)
#define DEF_REPEATS    100
#define DEF_BUSYCOUNT_DAXPY 10000
#define DEF_BUSYCOUNT_WAIT  2000
#define DEF_BUSYCOUNT_FIXED 10000

/* three ways of being "busy": 
   WAIT  fixed amount of ms in a tiny loop
   DAXPY perform a fixed number of DAXPY operation on an array of the size of the msgs to be send 
   FIXED perform a fixed number of x = A*sin(x) operations */

#define TAG_BUSY_SNDRCV 1

extern char *optarg;
char *buffer;
double *busy_buffer_1, *busy_buffer_2;
int busy_bufsize;
double threadstart;

void *wait (void *busy_delay);
void *fixed (void *busy_delay);
void *daxpy (void *busy_delay);
void *(*busy_fcn)(void *);

void busyrecv(int from, int msg_size, int nbr_msgs, int nbr_threads, int busy_delay, int do_persistent);
void busysend(int to, int msg_size, int nbr_msgs, int nbr_threads, int busy_delay, int do_persistent);

int main(int argc, char **argv) {
    MPI_Status status;
    int c, b;
    int msg_size, repeats, nbr_threads, busy_count, do_loop, loop_inc;
    int myrank, mysize, use_mpi_alloc, use_persistent;
   
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &mysize);
    
    if (mysize != 2) {
	printf ("%s must be used with 2 processes.\n", argv[0]);
	MPI_Finalize();	
	exit (1);
    }

    /* default values */
    nbr_threads = DEF_NBRTHREADS;
    busy_count  = -1;
    busy_fcn    = fixed;
    msg_size    = DEF_MSGSIZE;
    repeats     = DEF_REPEATS;
    do_loop     = 0;
    loop_inc    = 1;
    use_mpi_alloc  = 0;
    use_persistent = 0;
    /* args:
       -t nbr_threads
       -b busy [ms or ops]
       -s msgsize
       -r nbr msgs
       -l run a loop for BUSY from 0 to -b, increment by -i
       -i increment value
       -a use MPI_Alloc_mem
       -p persistent communication
       -d do DAXPY calculation over buffer as busy loop 
          (using memory bandwidth)
       -f do FIXED busy loop (no memory accesses)
    */

    while((c = getopt(argc, argv, "t:b:s:r:i:lapdf?")) != EOF) {
	switch(c) {
	case 't':
	  nbr_threads = atoi(optarg);
	  break;
	case 'b':
	  busy_count = atoi(optarg);
	  break;
	case 's':
	  msg_size = atoi(optarg);
	  break;
	case 'r':
	  repeats = atoi(optarg);
	  break;
	case 'i':
	  loop_inc = atoi(optarg);
	  break;
	case 'l':
	  do_loop = 1;
	  break;
	case 'a':
	  use_mpi_alloc = 1;
	  break;
	case 'p':
	  use_persistent = 1;
	  break;
	case 'd':
	  busy_fcn   = daxpy;
	  break;
	case 'f':
	  busy_fcn   = fixed;
	  break;
	case 'w':
	  busy_fcn   = wait;
	  break;
	case '?':
	  if (myrank == 0) {
	  printf ("busy_sndrcv - test overlapping of computation and communication.\n\
Arguments:\n\
 -t nbr_threads : number of threads polling on CPU during communication\n\
 -b busy [us or ops] : busy unit (us for FIXED, op's for DAXPY)\n\
 -s msgsize : message size to be sent and received (-1 for no messages at all)\n\
 -r nbr msgs : number of messages to be exchanged (iterations)\n\
 -l : run a loop for BUSY from 0 to -b, increment by -i\n\
 -i : increment value for loop (-l)\n\
 -a : use MPI_Alloc_mem() for buffer allocations\n\
 -p : use persistent communication\n\
 -d : do -b DAXPY calculations over buffer as busy loop\n\
      (using memory bandwidth)\n\
 -f : do FIXED number (-b) of x= A*sin(x) operations (no memory accesses)\n\
 -w : just busy-wait for a given time (wall-time) of -b ms\n");
	  }
	  MPI_Finalize();
	  exit (0);
	  break;
	}
    }

    if (busy_count == -1) {
      busy_count = (busy_fcn == daxpy) ? DEF_BUSYCOUNT_DAXPY : DEF_BUSYCOUNT_FIXED;
    }

    if (msg_size >= 0) 
      if (use_mpi_alloc) {
	MPI_Alloc_mem (msg_size, NULL, (void **)&buffer);
      } else {
	buffer = (char *)malloc (msg_size);
      }
    if (msg_size >= 0)
      busy_bufsize = msg_size/sizeof(double);
    else
      busy_bufsize = 16*1024;
    busy_buffer_1 = (double *)malloc (busy_bufsize * sizeof(double));
    busy_buffer_2 = (double *)malloc (busy_bufsize * sizeof(double));
    
    /* find send and recv processes */
    if (myrank == 0) {
      printf("# busy_sndrcv: %d threads\n", nbr_threads, busy_count);
      if (use_mpi_alloc)
	printf("# using MPI_Alloc_mem for communication buffers\n");
      if (use_persistent)
	printf("# using persistent communication\n");
      if (busy_fcn == daxpy) 
	  printf("# busy function: DAXPY\n");
      else
	  if (busy_fcn == wait) 
	       printf("# busy function: WAIT\n");
	  else
	      printf("# busy function: FIXED\n");
      printf("# msgsize\trepeats\tbusy_cnt\tlatency[ms]\n");
      fflush(stdout);
      if (!do_loop)
	busysend(  myrank + mysize/2, msg_size, repeats, nbr_threads, busy_count, use_persistent);
      else
	for (b = 0; b <= busy_count; b += loop_inc)
	  busysend(1, msg_size, repeats, nbr_threads, b, use_persistent);
    } else {
      if (!do_loop)
	busyrecv(0, msg_size, repeats, nbr_threads, busy_count, use_persistent);
      else
	for (b = 0; b <= busy_count; b += loop_inc)
	  busyrecv(0, msg_size, repeats, nbr_threads, b, use_persistent);
    }
    
    free (busy_buffer_1);
    free (busy_buffer_2);
    if (use_mpi_alloc)
	MPI_Free_mem (buffer);
    else
	free (buffer);

    MPI_Finalize();
}

double fixed_dummy;

void *wait(void *busy_delay)
{
    int busy_us = *(int *)busy_delay;

    fixed_dummy = 1.0;
    while ((double) busy_us >= ((MPI_Wtime() - threadstart)*1000000));
    
    return (void *)&fixed_dummy;
}

void *fixed(void *busy_delay)
{
    int ops = *(int *)busy_delay;

    fixed_dummy = 1.0;
    do {
	fixed_dummy = 11*sin(fixed_dummy);
    } while (--ops);
    
    return (void *)&fixed_dummy;
}

double dummy_return;

void *daxpy(void *busy_delay)
{
    int ops = *(int *)busy_delay;
    int i;
    double y = 3515.1516, dummy;

    while (ops > 0)
	for (i = 0; i < busy_bufsize; i++, ops--) {
	    dummy = busy_buffer_1[i] + y*busy_buffer_2[i];
	    if (ops == 0)
		break;
	}
    
    dummy_return = dummy;
    return (void *)&dummy_return;
}

void busysend(int to, int msg_size, int repeats, int nbr_threads, int busy_count, int do_persistent) {
    MPI_Request request;
    MPI_Status status;
    double starttime, totaltime;
    int t, j;
    pthread_t *thread_ids;
        
    thread_ids = (pthread_t *)malloc((nbr_threads-1)*sizeof(pthread_t));

    MPI_Barrier(MPI_COMM_WORLD);
    if (do_persistent && (msg_size >= 0))
	MPI_Send_init (buffer, msg_size, MPI_CHAR, to, TAG_BUSY_SNDRCV, MPI_COMM_WORLD, &request);

    starttime = MPI_Wtime();
    for( j = 0; j < repeats; j++) {
      if (msg_size >= 0) {
	if (do_persistent)
	    MPI_Start (&request);
	else
	    MPI_Isend( buffer, msg_size, MPI_CHAR, to, TAG_BUSY_SNDRCV, MPI_COMM_WORLD, &request);
      }

	/* busy delay */
	threadstart = MPI_Wtime();
	for (t = 0; t < nbr_threads-1; t++) 
	    pthread_create (&thread_ids[t], NULL, busy_fcn, &busy_count);
	busy_fcn (&busy_count);
	for (t = 0; t < nbr_threads-1; t++) 
	    pthread_join (thread_ids[t], NULL);
	
	if (msg_size >= 0) 
	  MPI_Wait (&request, &status);
    }
    totaltime = MPI_Wtime() - starttime;
    
    printf("%7d\t\t%7d\t%d\t\t%7.3f\n", msg_size, repeats, busy_count, (totaltime*1e+3)/repeats);
    fflush(stdout);

    if (do_persistent)
	MPI_Request_free(&request);
    free (thread_ids);
} 


void busyrecv(int from, int msg_size, int repeats, int nbr_threads, int busy_count, int do_persistent) {
    MPI_Request request;
    MPI_Status status;
    int t, j;
    pthread_t *thread_ids;
        
    thread_ids = (pthread_t *)malloc((nbr_threads-1)*sizeof(pthread_t));
        
    MPI_Barrier(MPI_COMM_WORLD);
    if (do_persistent && (msg_size >= 0))
	MPI_Recv_init (buffer, msg_size, MPI_CHAR, from, TAG_BUSY_SNDRCV, MPI_COMM_WORLD, &request);
    
    for (j = 0; j < repeats; j++) {
	if (msg_size >= 0) 
	  if (do_persistent)
	    MPI_Start (&request);
	  else
	    MPI_Irecv( buffer, msg_size, MPI_CHAR, from, TAG_BUSY_SNDRCV, MPI_COMM_WORLD, &request);

	/* busy delay */
	threadstart = MPI_Wtime();
	for (t = 0; t < nbr_threads-1; t++) 
	    pthread_create (&thread_ids[t], NULL, busy_fcn, &busy_count);
	busy_fcn (&busy_count);
	for (t = 0; t < nbr_threads-1; t++) 
	    pthread_join (thread_ids[t], NULL);
	
	if (msg_size >= 0) 
	  MPI_Wait (&request, &status);
    }
    
    if (do_persistent && (msg_size >= 0))
	MPI_Request_free(&request);
    free (thread_ids);
}
