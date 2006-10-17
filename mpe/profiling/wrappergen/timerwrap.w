#include <stdio.h>
#include "mpi.h"
#include "timerwrap.h"

#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif

static statistics_t statistics[fun_dummy + 1];

static char stat_name[][24] = {
	"Application           ",
	"MPI collectives       ",
	"MPI barrier           ",
	"MPI send (blocking)   ",
	"MPI send (nonblocking)",
	"MPI recv (blocking)   ",
	"MPI recv (nonblocking)",
	"MPI completion check  ",
	"MPI onesided put      ",
	"MPI onesided get      ",
	"MPI onesided accu     ",
	"MPI onesided sync     ",
	"MPI memory management ",
	"other MPI functions   ",
	"DUMMY                 "
};

static double app_time;
static double rec_depth;

#define STAT_UPDATE(functype, t) \
if (rec_depth == 1) { \
statistics[functype].number_calls++; \
statistics[functype].acc_time += t; \
if ( t < statistics[functype].min_time) statistics[functype].min_time = t; \
if ( t > statistics[functype].max_time) statistics[functype].max_time = t; \
}

#define STAT_IN \
rec_depth++; \
if (rec_depth == 1) { \
  app_time = PMPI_Wtime() - app_time; \
  STAT_UPDATE(application, app_time); \
}

#define STAT_OUT \
if (rec_depth == 1) { \
  app_time = PMPI_Wtime(); \
} \
rec_depth--;


/* Collective ops */
{{fn fn_name 
MPI_Allgather MPI_Allgatherv MPI_Allreduce MPI_Alltoall
MPI_Alltoallv MPI_Bcast MPI_Gather MPI_Gatherv 
MPI_Reduce MPI_Reduce_scatter MPI_Scatter MPI_Scatterv 
MPI_Sendrecv MPI_Sendrecv_replace
}}
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  {{callfn}}
  t = PMPI_Wtime() - t;
  STAT_UPDATE(collective_ops, t);

  STAT_OUT;
{{endfn}}

/* Barrier */
{{fn fn_name MPI_Barrier}}  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  {{callfn}}
  t = PMPI_Wtime() - t;
  STAT_UPDATE(barrier, t);

  STAT_OUT;
{{endfn}}

/* Send blocking */
{{fn fn_name MPI_Bsend MPI_Rsend MPI_Send MPI_Ssend}}  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  {{callfn}}
  t = PMPI_Wtime() - t;
  STAT_UPDATE(send_blck, t);

  STAT_OUT;
{{endfn}}

/* Receive blocking */
{{fn fn_name MPI_Recv}}  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  {{callfn}}
  t = PMPI_Wtime() - t;
  STAT_UPDATE(recv_blck, t);

  STAT_OUT;
{{endfn}}

/* Send nonblocking */
{{fn fn_name MPI_Ibsend MPI_Irsend MPI_Isend MPI_Issend}}  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  {{callfn}}
  t = PMPI_Wtime() - t;
  STAT_UPDATE(send_noblck, t);

  STAT_OUT;
{{endfn}}

/* Receive nonblocking */
{{fn fn_name MPI_Irecv}}  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  {{callfn}}
  t = PMPI_Wtime() - t;
  STAT_UPDATE(recv_noblck, t);

  STAT_OUT;
{{endfn}}


/* wait, test, cancel */
{{fn fn_name 
MPI_Cancel MPI_Iprobe MPI_Probe MPI_Scan MPI_Test MPI_Test_cancelled
MPI_Testall MPI_Testany MPI_Testsome MPI_Wait MPI_Waitall 
MPI_Waitany MPI_Waitsome
}}  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  {{callfn}}
  t = PMPI_Wtime() - t;
  STAT_UPDATE(wait, t);

  STAT_OUT;
{{endfn}}


/* Onesided put */
{{fn fn_name MPI_Put }}  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  {{callfn}}
  t = PMPI_Wtime() - t;
  STAT_UPDATE(onesided_put, t);

  STAT_OUT;
{{endfn}}

/* Onesided get */
{{fn fn_name MPI_Get }}  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  {{callfn}}
  t = PMPI_Wtime() - t;
  STAT_UPDATE(onesided_get, t);

  STAT_OUT;
{{endfn}}

/* Onesided accumulate */
{{fn fn_name MPI_Accumulate }}  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  {{callfn}}
  t = PMPI_Wtime() - t;
  STAT_UPDATE(onesided_accu, t);

  STAT_OUT;
{{endfn}}

/* Onesided synchronisation */
{{fn fn_name 
MPI_Win_fence MPI_Win_start MPI_Win_complete MPI_Win_post MPI_Win_wait MPI_Win_test 
MPI_Win_lock MPI_Win_unlock }}  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  {{callfn}}
  t = PMPI_Wtime() - t;
  STAT_UPDATE(onesided_sync, t);

  STAT_OUT;
{{endfn}}

/* Memory management */
{{fn fn_name 
MPI_Win_create MPI_Win_free MPI_Alloc_mem MPI_Free_mem 
}}  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  {{callfn}}
  t = PMPI_Wtime() - t;
  STAT_UPDATE(memory, t);

  STAT_OUT;
{{endfn}}


/* Init */
{{fn fn_name MPI_Init}}  
  int i;

  {{callfn}}

  for (i=0; i<fun_dummy; i++) {
    statistics[i].name = stat_name[i];   
    statistics[i].number_calls = 0;
    statistics[i].min_time = 1000000.0;
    statistics[i].max_time = 0.0;
    statistics[i].acc_time = 0.0;
  }  
  
  app_time = PMPI_Wtime();
  rec_depth = 0;
{{endfn}}

/* Finalize */
{{fn fn_name MPI_Finalize}}
  int i,p;
  int iRank;
  int iSize;
  double total_time = 0; 

  rec_depth = 1;

  app_time = PMPI_Wtime() - app_time;
  STAT_UPDATE(application, app_time);

  for (i = 0; i < fun_dummy; i++) {
	total_time  += statistics[i].acc_time;
  }

  PMPI_Comm_rank(MPI_COMM_WORLD, &iRank);
  PMPI_Comm_size(MPI_COMM_WORLD, &iSize);

  for (p=0; p<iSize; p++) {
    if(iRank == p) {
      printf("# time spent by process %d:\n", p);
      printf("# type                   calls  min [us]  max [us]  avg [us]   acc [s]  %% of total \n");

      for (i=0; i<fun_dummy; i++) {
        if (statistics[i].number_calls > 0) {
          printf("%s%8d%10.1f%10.1f%10.1f%10.3f%10.2f\n",
          statistics[i].name,
          statistics[i].number_calls,
          statistics[i].min_time * 1000000,
          statistics[i].max_time * 1000000,
          (statistics[i].acc_time / (double)statistics[i].number_calls) * 1000000,
          statistics[i].acc_time,
          statistics[i].acc_time/total_time*100);
        }
      }
      printf("\n");
      fflush(stdout);
    }
    PMPI_Barrier(MPI_COMM_WORLD);
  }
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
MPI_Waitany MPI_Waitsome 
MPI_Alloc_mem MPI_Free_mem MPI_Win_create MPI_Win_free
MPI_Put MPI_Get MPI_Accumulate MPI_Win_fence MPI_Win_start MPI_Win_complete MPI_Win_post 
MPI_Win_wait MPI_Win_test MPI_Win_lock MPI_Win_unlock 
MPI_Win_create_errhandler MPI_Win_set_errhandler MPI_Win_get_errhandler
MPI_Init MPI_Finalize
}}  
  double t;

  STAT_IN;

  t = PMPI_Wtime();
  {{callfn}}
  t = PMPI_Wtime() - t;
  STAT_UPDATE(def, t);

  STAT_OUT;
{{endfnall}}

