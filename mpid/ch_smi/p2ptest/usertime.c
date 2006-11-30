/* $Id$

   Test the system load (CPU usage) while waiting for a message:
   process 0 is busy for a certain period, while process 1 waits
   for a message from process 0. 

   Process 1 will report the system load that was caused by waiting 
   for the message. This will show if the process polls for 
   incoming messages (which is faster, but blocks a CPU). 
*/
#include <sys/times.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

#include "mpi.h"

/* nbr of seconds that process 0 is busy */
#define DELAY 15

int main(int argc, char **argv)
{
    MPI_Status status;
    struct tms timing;
    clock_t elapsed;
    int myrank, mysize;
    double start, total_time, usr_time, sys_time, child_time;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &mysize);
    
    if (mysize != 2) {
	if (!myrank)
	    printf ("%s can only be run with exactly 2 processes\n", argv[0]);
	MPI_Finalize();
	return 1;
    }
		
    
    if (myrank == 0) {
	printf ("Delaying for %d seconds\n", DELAY); fflush (stdout);
	start = MPI_Wtime();
	while (MPI_Wtime() - start < DELAY)
	    ;

	MPI_Send( &start, 1 , MPI_DOUBLE, 1, 1, MPI_COMM_WORLD);
    } else {
	elapsed = times (&timing);

	MPI_Recv( &start, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);

	elapsed = times (&timing) - elapsed;
	total_time = (elapsed * CLK_TCK)/10000;
	usr_time   = (timing.tms_utime * CLK_TCK)/10000;
	sys_time   = (timing.tms_stime * CLK_TCK)/10000;
	child_time = ((timing.tms_cstime+timing.tms_cutime) * CLK_TCK)/10000;

	printf ("\nTime consumed while waiting for a message for %d seconds:\n", DELAY);
	printf ("\ttotal time : %6.3f s\n", total_time);
	printf ("Consisting of (approximate timings):\n");
	printf ("\tuser time  : %6.3f s\n", usr_time);
	printf ("\tsystem time: %6.3f s\n", sys_time);
	printf ("\tchild time : %6.3f s\n", child_time);

	if ((sys_time < 0.001) && (total_time - usr_time < 0.001)) {
	    printf ("\n => This MPI implementation polls for new messages\n");
	} else {
	    printf ("\n => This MPI implementation blocks while waiting for new messages\n");
	}
    }
    
    MPI_Finalize();
    return 0;
}

