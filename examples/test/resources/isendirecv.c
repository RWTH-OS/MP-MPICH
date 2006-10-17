/* Check for memory usage of non-blocking send and recv
 * operations, either expected or potentially unexpected
 * messages.
 *
 * The resource usage checking is still very basic and does
 * only work on Linux. More work is needed to integrate this
 * into automatic testing.
 *
 * Joachim Worringen <joachim@maxperf.de>*/

#include <stdio.h>
#include <assert.h>

#ifndef WIN32
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif
#include "mpi.h"

#define WARMUP_LOOPS 50

#define MAXPES 32
#define MYBUFSIZE 200*1024
static int sendbufs[MAXPES][MYBUFSIZE];
static int recvbufs[MAXPES][MYBUFSIZE];

#define MSGSIZE 100
#define LOOPS   5000

#ifndef WIN32
#define RSSDIFF(_rank, _l, _pos, _rss, _prev, _print) _rss = get_VmRSS(); \
      if (_rss != _prev) { \
      if (_print) fprintf(stderr, "[%d] loop %d @ pos %d: diff_RSS = %d\n", _rank, _l, _pos, _rss - _prev); \
      _prev = _rss; }


FILE *procinfo_fh;


int get_VmRSS()
{
    char procinfo_buf[255];
    char *l;
    int rss = -1;

    rewind(procinfo_fh);

    while ((l = fgets(procinfo_buf, 254, procinfo_fh)) != NULL) {
	if (strstr(l, "VmRSS:") != NULL) {
	    l[16] = '\0';
	    l += 8;
	    rss = atoi(l);
	}
    }

    return rss;
}
#endif

int main ( int argc, char *argv[] )
{
  int i, arg_idx;
  int self, npes;

#ifndef WIN32
  int prev_rss, rss;
  char pidbuf[255];
#endif

  int msgsize;
  int be_verbose;
  int loop, n_loops, w_loops;
  int expected_msgs, unexpected_msgs;

  MPI_Request sendreqs[MAXPES];
  MPI_Request recvreqs[MAXPES];
  MPI_Status status[MAXPES];

  MPI_Init (&argc, &argv);
  MPI_Comm_rank (MPI_COMM_WORLD, &self);
  MPI_Comm_size (MPI_COMM_WORLD, &npes);

  msgsize = MSGSIZE;
  n_loops = LOOPS;
  expected_msgs = 0;
  unexpected_msgs = 0;
  w_loops = WARMUP_LOOPS;
  be_verbose = 0;

  arg_idx = 1;
  while (arg_idx < argc) {
      if (!strcmp("-l", argv[arg_idx])) {
	  n_loops = atoi(argv[++arg_idx]);
      } else if (!strcmp("-s", argv[arg_idx])) {
	  msgsize = atoi(argv[++arg_idx])/sizeof(int);
      } else if (!strcmp("-w", argv[arg_idx])) {
	  w_loops = atoi(argv[++arg_idx]);
      } else if (!strcmp("-e", argv[arg_idx])) {
	  expected_msgs = 1;
      } else if (!strcmp("-u", argv[arg_idx])) {
	  unexpected_msgs = 1;
      } else if (!strcmp("-v", argv[arg_idx])) {
	  be_verbose = 1;
      }
      arg_idx++;
  }
#ifndef WIN32
  sprintf(pidbuf, "/proc/%d/status\0", getpid());
  procinfo_fh = fopen(pidbuf, "r");
  assert (procinfo_fh != NULL);
#endif

  assert (npes <= MAXPES);
  assert (msgsize*sizeof(int) <= MYBUFSIZE);  

  if (be_verbose && self == 0) {
      printf("memory leak test for irecv/isend/waitall\n");
      printf(" %d loops, msgsize = %d byte\n", n_loops, msgsize*sizeof(int));
      if (expected_msgs)
	  printf (" only expected messages\n");
      else if (unexpected_msgs)
	  printf (" only unexpected messages\n");
      else
	  printf (" random mix of expected and unexpected messages\n");
  }

  loop = 0;
#ifndef WIN32
  prev_rss = get_VmRSS();
#endif
  while (n_loops == 0 || loop < n_loops) {

      for (i = 0; i < npes; i++) {
	  if (unexpected_msgs)
	      MPI_Isend (&sendbufs[i], msgsize, MPI_INT, i, 0, MPI_COMM_WORLD, &sendreqs[i]);
	  else
	      MPI_Irecv (&recvbufs[i], msgsize, MPI_INT, i, 0, MPI_COMM_WORLD, &recvreqs[i]);
      }
#ifndef WIN32
      RSSDIFF(self, loop, 0, rss, prev_rss, loop >= w_loops);
#endif
      if (expected_msgs || unexpected_msgs)
	  MPI_Barrier(MPI_COMM_WORLD);

      for (i = 0; i < npes; i++) {
	  if (unexpected_msgs)
	      MPI_Irecv (&recvbufs[i], msgsize, MPI_INT, i, 0, MPI_COMM_WORLD, &recvreqs[i]);
	  else
	      MPI_Isend (&sendbufs[i], msgsize, MPI_INT, i, 0, MPI_COMM_WORLD, &sendreqs[i]);
      }
#ifndef WIN32
      RSSDIFF(self, loop, 1, rss, prev_rss, loop >= w_loops);
#endif
      MPI_Waitall (npes, sendreqs, status);
#ifndef WIN32
      RSSDIFF(self, loop, 2, rss, prev_rss, loop >= w_loops);
#endif
      MPI_Waitall (npes, recvreqs, status);
#ifndef WIN32
      RSSDIFF(self, loop, 3, rss, prev_rss, loop >= w_loops);
#endif

      loop++;
  }

  MPI_Finalize();

#ifndef WIN32
  fclose(procinfo_fh);
#endif

  if (self == 0)
      printf("Done.\n");

  return (0);
}
