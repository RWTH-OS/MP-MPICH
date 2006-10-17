#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "papi_wrappers.h"

#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif

/* colors for groups of functions */
static int _app_col  = YELLOW; /* Application */
static int _coll_col = BLUE;   /* Collective ops */
static int _send_col = RED;    /* Send */
static int _recv_col = GREEN;  /* Receive */
static int _wait_col = ORANGE; /* wait, test, cancel */
static int _def_col  = WHITE;  /* Non communicating */

static int bITrace=FALSE;

 /* Collective ops */
{{fn fn_name 
MPI_Allgather MPI_Allgatherv MPI_Allreduce MPI_Alltoall
MPI_Alltoallv MPI_Barrier MPI_Bcast MPI_Gather MPI_Gatherv 
MPI_Reduce MPI_Reduce_scatter MPI_Scatter MPI_Scatterv 
MPI_Sendrecv MPI_Sendrecv_replace
}}  
  if (bITrace) mark_perfometer(_coll_col, "Collective ops");
  {{callfn}}
  if (bITrace) mark_perfometer(_app_col, "Application");
{{endfn}}


/* Send */
{{fn fn_name 
MPI_Bsend MPI_Ibsend MPI_Irsend MPI_Isend MPI_Issend MPI_Rsend 
MPI_Send MPI_Ssend
}}  
  if (bITrace) mark_perfometer(_send_col, "Send");
  {{callfn}}
  if (bITrace) mark_perfometer(_app_col, "Application");
{{endfn}}

/* Receive */
{{fn fn_name MPI_Irecv MPI_Recv}}  
  if (bITrace) mark_perfometer(_recv_col, "Receive");
  {{callfn}}
  if (bITrace) mark_perfometer(_app_col, "Application");
{{endfn}}


/* wait, test, cancel */
{{fn fn_name 
MPI_Cancel MPI_Iprobe MPI_Probe MPI_Scan MPI_Test MPI_Test_cancelled
MPI_Testall MPI_Testany MPI_Testsome MPI_Wait MPI_Waitall 
MPI_Waitany MPI_Waitsome
}}  
  if (bITrace) mark_perfometer(_wait_col, "wait, test, cancel");
  {{callfn}}
  if (bITrace) mark_perfometer(_app_col, "Application");
{{endfn}}

/* Init */
{{fn fn_name MPI_Init}}  
  char szFileName[1024];
  int iRank;
  char* proclist = getenv("MPIPAPI_PROCS");
  char* pos;
  char* lasts;  

  {{callfn}}

  MPI_Comm_rank(MPI_COMM_WORLD, &iRank);
  if ((getenv("MPIRUNPID") == NULL ) || (getenv("MPIRUNWD") == NULL)) {
      perror("usage of papi requieres mpirunskript to export MPIRUNWD and MPIRUNPID shell variable");
      MPI_Abort(MPI_COMM_WORLD, -1);
  }
  
  if (proclist == NULL ) {
      bITrace = TRUE;
  }
  else {
    for(pos = strtok_r(proclist,":",&lasts); pos != NULL; pos = strtok_r(NULL,":",&lasts))
      if ( atoi(pos) == iRank )
        bITrace = TRUE;
  }
  sprintf(szFileName,"%s/%s.%d", getenv("MPIRUNWD"), getenv("MPIRUNPID"), iRank);
  if (bITrace) fperfometer(szFileName);
  if (bITrace) mark_perfometer(_app_col, "Application");
  
  MPI_Barrier(MPI_COMM_WORLD);

{{endfn}}

/* Finalize */
{{fn fn_name MPI_Finalize}}  
  /* stop_perfometer(); */
  {{callfn}}
{{endfn}}

/* Others */
{{fnall this_fn_name 
MPI_Allgather MPI_Allgatherv MPI_Allreduce MPI_Alltoall
MPI_Alltoallv MPI_Barrier MPI_Bcast MPI_Gather MPI_Gatherv 
MPI_Reduce MPI_Reduce_scatter MPI_Scatter MPI_Scatterv 
MPI_Sendrecv MPI_Sendrecv_replace
MPI_Bsend MPI_Ibsend MPI_Irsend MPI_Isend MPI_Issend MPI_Rsend 
MPI_Send MPI_Ssend MPI_Irecv MPI_Recv
MPI_Cancel MPI_Iprobe MPI_Probe MPI_Scan MPI_Test MPI_Test_cancelled
MPI_Testall MPI_Testany MPI_Testsome MPI_Wait MPI_Waitall 
MPI_Waitany MPI_Waitsome MPI_Init MPI_Finalize
}}  
  if (bITrace) mark_perfometer(_def_col, "Non communicating");
  {{callfn}}
  if (bITrace) mark_perfometer(_app_col, "Application");
{{endfnall}}

