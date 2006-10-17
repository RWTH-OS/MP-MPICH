/* $Id: progress.c,v 1.2.8.1 2004/11/24 17:17:15 rainer Exp $ */

/* benchmark to rate the effective performance of one-sided communication 
   for scenarios in which the target process does *not* frequently, or even 
   steadlily polls the incoming queue. 

   Joachim Worringen, 2002 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif

#include <stdarg.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>

#include <mpi.h>

/* default settings */
#define DISTANCE   1

#define MIN_COUNT  1
#define MAX_COUNT  10000
#define SHARED_MEM 0
#define RUNS       1
#define ZERO_ONLY  0

#define OP_PUT     0
#define OP_GET     1
#define OP_ACCU    2
#define DEF_OP     OP_PUT
char* mpi_funcname[] = { "MPI_Put", "MPI_Get", "MPI_Accumulate" };


/* 0 = Win_fence, 1 = exposure, 2 = Win_lock */
#define SYNC_FENCE  0
#define SYNC_GROUP  1
#define SYNC_LOCK   2
#define DEF_SYNC    SYNC_LOCK
char* synchname[] = { "MPI_Win_fence", "Exposure", "MPI_Win_lock" };

#define DEF_CNT        2
#define DEF_EPOCHS     100
#define DEF_CYCLES     100
#define DEF_VARIANCE   3  
#define DEF_MIN_ITER   1e2
#define DEF_MAX_ITER   1e4

typedef double(*busy_fcn_t)(int iterations);

double busy_cpu (int iterations) 
{
	int i;
	double f = 17;

	for (i = 0; i < iterations; i++)
		f = sin(f) + f*(double)i;
	
	return f;
}

#define ARRAY_SZ   (1024*1024)
double busy_mem (int iterations) 
{
	int i;
	double f, *a, b;

	f = 3.5; b = 21.52;
	a = (double *)malloc (sizeof(double)*ARRAY_SZ);

	for (i = 0; i < iterations; i++)
		f = f*a[i%ARRAY_SZ] + b;
	
	free (a);
	return f;
}


int main (argc, argv)
	int		argc;
	char	** argv;
{
	MPI_Win		  win;
	MPI_Group     exp_group, acc_group, group;
	MPI_Datatype  dtype, dtype_help;
	MPI_Info      meminfo = MPI_INFO_NULL;
	busy_fcn_t bfcn;
	double	*base = NULL, *buffer = NULL;
	double	acc_min, acc_max, sync_min, sync_max, total_min, total_max;
	double  t0, t_acc, t_sync, t_total;	
	int		myrank, numprocs;
	int		e, c, i, access_cnt, maxcnt, offset, partner, lpartner, one_on_one;
	int		numruns, window_memory_shared;
	int     epochs, variance, min_iter, max_iter, cycles, cnt;
	int		synchronisation, mpi_func, pname_len;
	char    pname[256];
	
	bfcn = busy_cpu;
	cnt = DEF_CNT;
	epochs = DEF_EPOCHS;
	cycles = DEF_CYCLES;
	min_iter = DEF_MIN_ITER;
	max_iter = DEF_MAX_ITER;
	variance = DEF_VARIANCE;
	one_on_one = 0;
	window_memory_shared = SHARED_MEM;
	mpi_func = DEF_OP;
	synchronisation = DEF_SYNC;

	MPI_Init (&argc, &argv);
	pname_len = 256;
	MPI_Get_processor_name (pname, &pname_len);
	MPI_Comm_rank (MPI_COMM_WORLD, &myrank);
	MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
	printf ("# [%d] on %s\n", myrank, pname); fflush (stdout);

#ifndef CRAY
    while((c = getopt(argc, argv, "b:psgafxoli:I:e:c:v:mh?")) != EOF) {
		switch(c) {
		case 'b':
			cnt =  atoi (optarg)/8;
			if (cnt == 0)
				cnt = 1;
			break;
		case 'p':
			MPI_Info_create (&meminfo);
			MPI_Info_set (meminfo, "type", "private");
			break;
		case 's':
			MPI_Info_create (&meminfo);
			MPI_Info_set (meminfo, "type", "shared");
			window_memory_shared = 1;
			break;
		case 'g':
			mpi_func = OP_GET;
			break;
		case 'a':
			mpi_func = OP_ACCU;
			break;
		case 'f':
			synchronisation = SYNC_FENCE;
			break;
		case 'x':
			synchronisation = SYNC_GROUP;
			break;
		case 'o':
			one_on_one = 1;
			break;
		case 'l':
			synchronisation = SYNC_LOCK;
			break;
		case 'i':
			min_iter = atoi (optarg);
			break;
		case 'I':
			max_iter = atoi (optarg);
			break;
		case 'e':
			epochs = atoi (optarg);
			break;
		case 'c':
			cycles = atoi (optarg);
			break;
		case 'v':
			variance = atoi (optarg);
			break;
		case 'm':
			bfcn = busy_mem;
			break;
		case '?':
		case 'h':
			if ( myrank == 0) {
				printf("Usage: %s [options]\n", argv[0]);
				printf("  -g             use MPI_Get. [default: MPI_Put]\n");
				printf("  -a             use MPI_Accumulate. [default: MPI_Put]\n");
				printf("  -l             Synchronisation: MPI_Win_lock.[default]\n");
				printf("  -f             Synchronisation: MPI_Win_fence.\n");
				printf("  -x             Synchronisation: Exposure.\n");
				printf("  -o             one-on-one exposure synchronization [default: all].\n");
				printf("  -p             force getting private memory.\n");
				printf("  -s             force getting shared memory.\n");
				printf("  -b BLOCKSIZE   access blocks sized BLOCKSIZE bytes (rounded to doubles) [1]\n");
				printf("  -e EPOCHS      number of access epochs\n");
				printf("  -c CYCLES      number of iteration/access phases in each epoch\n");
				printf("  -i MIN_ITER    min number of iterations for each calculating phase [1e2]\n");
				printf("  -I MIN_ITER    max number of iterations for each calculating phase [1e4]\n");
				printf("  -v VARIANCE    +/- variance for ITERATIONS value [3, which mean 1/3 of the iter value]\n");
				printf("  -m             use memory-intensive busy-loop [default: cache-local]\n");
			}
			MPI_Finalize();
			exit(1);
		}
	}
#endif /* CRAY */
	maxcnt = 1024*cnt;

	/* window memory */		
	MPI_Alloc_mem (maxcnt*sizeof(double), meminfo, (void *)&base);
	if (!base) {
		fprintf (stderr, "No  memory\n");
		MPI_Abort (MPI_COMM_WORLD, -1); 
	}
	/* origin memory */
#if 1
	MPI_Alloc_mem (maxcnt*sizeof(double), meminfo, (void *)&buffer);
#else
	buffer = (double *)malloc (maxcnt*sizeof(double));
#endif
	if (!buffer) {
		fprintf (stderr, "Out of memory\n");
		return -2;
	}

	MPI_Win_create (base, maxcnt*sizeof(double), sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &win);
	
	partner = (myrank + DISTANCE) % numprocs;
	lpartner = (myrank + numprocs - 1) % numprocs;

	MPI_Win_get_group (win, &group);
	MPI_Group_incl (group, 1, &lpartner, &exp_group);
	MPI_Group_incl (group, 1, &partner, &acc_group);

	if (myrank == 0) {
		printf ("# performance of %s with temporaly irregular access\n", mpi_funcname[mpi_func]);
		printf ("# numprocs = %d, window_size = %d, window_memory_shared = %d\n", 
				numprocs, maxcnt*sizeof(double), window_memory_shared);
		printf ("# blocksize = %d, (MPI_DOUBLE), epochs = %d , cycles = %d, iterations %d .. %d\n", 
				cnt, epochs, cycles, min_iter, max_iter);
		printf ("# synchronisation = %s\n", synchname[synchronisation]);
		if (bfcn == busy_mem)
			printf ("# busy loop causes memory stress\n");
		else 
			printf ("# busy loop only stresses the CPU\n");			
		printf ("# synchronisation = %s\n", synchname[synchronisation]);
		printf ("# all values for a complete iteration turnaround\n");
		printf ("# len\tn_iter\tn_acc \tacc_min[s] \tacc_max[s] \tsync_min[s] \tsync_max[s] \ttotal_min[s] \ttotal_max[s] \tB_min [MB/s] \tB_max [MB/s]\n");
		fflush(stdout);
	}

	/* loop over number of iterations */
	for (i = min_iter; i <= max_iter; i = i < 1e2?
			 i + 1e1 : i < 1e3 ? 
			 i + 1e2 : i < 1e4 ? 
			 i + 1e3 : i < 1e5 ? 
			 i + 1e4 : i < 1e6 ? 
			 i + 1e5 : i < 1e7 ? 
			 i + 1e6 : (i*3)/2) {
		access_cnt = 0;
		t_sync = 0;
		t_acc = 0;
		t_total = MPI_Wtime();
		/* loop over number of access epochs */
		for (e = 0; e < epochs; e++) {
			if (synchronisation != SYNC_LOCK) {
				t0 = MPI_Wtime();
				switch (synchronisation) {
				case SYNC_FENCE:
					MPI_Win_fence (0, win);
					break;
				case SYNC_GROUP:
					if (one_on_one) {
						MPI_Win_post (exp_group, 0, win);
						MPI_Win_start(acc_group, 0, win);
					} else {
						MPI_Win_post (group, 0, win);
						MPI_Win_start(group, 0, win);
					}
					break;
				}
				t_sync = t_sync + MPI_Wtime() - t0;
			}
		
			/* loop over number of access/calculation cycles */
			for (c = 0; c < cycles; c++) {
				offset = 0;
			
				if (c % 2 == myrank % 2) {
					/* do calculations */
					bfcn (i + (c%3 - 1)*(i/variance));
				}
			
				if (synchronisation == SYNC_LOCK) {
					t0 = MPI_Wtime();
					MPI_Win_lock(MPI_LOCK_SHARED, partner, 0, win);
					t_sync = t_sync + MPI_Wtime() - t0;
				}

				/* do the accesses and take time */
				t0 = MPI_Wtime();
				switch (mpi_func) {
				case OP_PUT:
					MPI_Put (buffer + offset, cnt, MPI_DOUBLE, partner, offset, cnt, 
							 MPI_DOUBLE, win); 
					break;
				case OP_GET:
					MPI_Get (buffer + offset, cnt, MPI_DOUBLE, partner, offset, cnt, 
							 MPI_DOUBLE, win);
					break;
				case OP_ACCU:
					MPI_Accumulate (buffer + offset, cnt, MPI_DOUBLE, partner, offset, cnt, 
									MPI_DOUBLE, MPI_SUM, win);
					break;
				}
				t_acc = t_acc + MPI_Wtime() - t0;
			
				if (synchronisation == SYNC_LOCK) {
					t0 = MPI_Wtime();
					MPI_Win_unlock(partner, win);
					t_sync = t_sync + MPI_Wtime() - t0;
				}

				access_cnt++;
				offset += cnt + 1;
				if (offset + cnt > maxcnt)
					offset = 0;
			
				if (c % 2 != myrank % 2) {
					/* do calculations */
					bfcn (i + (c%3 - 1)*(i/variance));
				}
			}
		
			if (synchronisation != SYNC_LOCK) {
				t0 = MPI_Wtime();
				switch (synchronisation) {
				case SYNC_FENCE:
					MPI_Win_fence (0, win);
					break;
				case SYNC_GROUP:
					MPI_Win_complete(win);
					MPI_Win_wait(win);
					break;
				}
				t_sync = t_sync + MPI_Wtime() - t0;
			}

			bfcn (i);
		}
		t_total = MPI_Wtime() - t_total;

	/* print out performance of connection */
#if 0
		MPI_Reduce (&t_acc, &acc_min, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
		MPI_Reduce (&t_acc, &acc_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
		MPI_Reduce (&t_sync, &sync_min, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
		MPI_Reduce (&t_sync, &sync_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
		MPI_Reduce (&t_total, &total_min, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
		MPI_Reduce (&t_total, &total_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
#else
		acc_min = acc_max = t_acc;
		sync_min = sync_max = t_sync;
		total_min = total_max = t_total;
#endif
	
		if (myrank == 0) {
			printf ("%d \t%d \t%d \t%8.6f \t%8.6f \t%8.6f \t%8.6f \t%8.6f \t%8.6f \t%8.6f \t%8.6f\n", 
					cnt*sizeof(double), i, access_cnt,
					acc_min, acc_max,sync_min, sync_max, 
					total_min, total_max, 
					(cycles*epochs*sizeof(double)*cnt)/(total_min*1024*1024), 
					(cycles*epochs*sizeof(double)*cnt)/(total_max*1024*1024));
			fflush (stdout);
		}
	}
	MPI_Win_free (&win);
	MPI_Finalize ();

	return 0;
}







/*
 * Overrides for XEmacs and vim so that we get a uniform tabbing style.
 * XEmacs/vim will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:tw=0:ts=4:wm=0:
 */
