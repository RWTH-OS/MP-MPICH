/* test the syncing of two processes via the pidsync-mechanism */

#include <stdio.h>
#include <unistd.h>

#include "pidsync.h"

#define SYNC_ID_1 1000
#define SYNC_ID_2 1001

int main (int argc, char **argv)
{
   pid_t my_pid;
   int qid;
   int rank_1, rank_2;
   int nbr_sync_1, nbr_sync_2;

   nbr_sync_1 = atoi (argv[1]);
   nbr_sync_2 = atoi (argv[2]);
   my_pid = getpid();
  
   if (nbr_sync_1) {
     qid = pidopen (SYNC_ID_1);
     printf ("[%d] Syncing with %d procs on SYNC_ID_1, qid = %d\n", my_pid, nbr_sync_1, qid); 
     fflush (stdout);
     rank_1 = pidsync (nbr_sync_1, qid);
     pidclose (qid);
     printf ("[%d] OK Synced with %d procs on SYNC_ID_1, got rank %d\n", 
	     my_pid, nbr_sync_1, rank_1); fflush (stdout);    
   }
  
   if (nbr_sync_2) {
     qid = pidopen (SYNC_ID_2);
     printf ("[%d] Syncing with %d procs on SYNC_ID_2, qid = %d\n", my_pid, nbr_sync_2, qid); 
     fflush (stdout);
     rank_2 = pidsync (nbr_sync_2, qid);
     pidclose (qid);
     printf ("[%d] OK Synced with %d procs on SYNC_ID_2, got rank %d\n", 
	     my_pid, nbr_sync_2, rank_2); fflush (stdout);
   }

   return 0;
}
   
