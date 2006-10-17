/* test the syncing of two processes via the pidsync-mechanism */

#include <stdio.h>
#include <unistd.h>

#include "pidsync.h"

#define SYNC_ID_1 1000
#define SYNC_ID_2 1001

int main (int argc, char **argv)
{
   pid_t my_pid, new_pid;
   int qid, rank;
   int nbr_procs, nbr_forks, i;

   nbr_procs = atoi (argv[1]);
   nbr_forks = atoi (argv[2]);
   my_pid = getpid();
   
   qid = pidopen (SYNC_ID_1);
   printf ("[%d] Syncing with %d procs on SYNC_ID_1, qid = %d\n", my_pid, nbr_procs, qid);
   fflush (stdout);
   rank = pidsync (nbr_procs, qid);
   pidclose (qid);
   printf ("[%d] OK Synced with %d procs on SYNC_ID_1, got rank %d\n",
	   my_pid, nbr_procs, rank); fflush (stdout);

   for (i = 0; i < nbr_forks; i++) {
     new_pid = fork();
     if (new_pid == 0)
       break;
   }
   my_pid = getpid();
  
   qid = pidopen (SYNC_ID_2);
   printf ("[%d] Syncing with %d procs on SYNC_ID_2, qid = %d\n", 
	   my_pid, nbr_procs*(nbr_forks + 1), qid); 
   fflush (stdout);
   rank = pidsync (nbr_procs*(nbr_forks + 1), qid);
   pidclose (qid);
   printf ("[%d] OK Synced with %d procs on SYNC_ID_2, got rank %d\n", 
	   my_pid, nbr_procs*(nbr_forks + 1), rank); fflush (stdout);    

   return 0;
}
   
