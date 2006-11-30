/* $Id$ */

/* simulate read-write access to a sparse matrix via single-sided communication */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <stdarg.h>
#include <sys/time.h>
#include <unistd.h>

#include <mpi.h>

/* default settings */
#define DISTANCE      1
#define MPI_ALLOC_MEM 1

#define MIN_COUNT  1
#define MAX_COUNT  10000
#define SHARED_MEM 0
#define RUNS       1
#define EPOCHS     1
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
#define DEF_SYNC    SYNC_FENCE
char* synchname[] = { "MPI_Win_fence", "Exposure", "MPI_Win_lock" };



int main (argc, argv)
	int		argc;
	char	** argv;
{
	MPI_Win		  win;
	MPI_Group     exp_group, acc_group, group;
	MPI_Datatype  dtype, dtype_help;
	MPI_Info      meminfo = MPI_INFO_NULL;

	double	*base = NULL, *buffer = NULL;
	double	acc_min, acc_max, sync_min, sync_max, total_min, total_max;
	double  t0, t_acc, t_sync, t_sync_open, t_sync_close, total_acc, total_sync;	
	int		myrank, numprocs;
	int		i, j, k, c, e, a, w;
	int     access_cnt, offset, partner, lpartner, distance, one_on_one, warm_up;
	int		numacc, numepochs, zero_only, testnum, window_memory_shared;
	int     contiguous, variable_nacc;
	int		cnt, mincnt, maxcnt, terminate_cnt = 0, synchronisation, mpi_func;
	char    pname[256];
	
	one_on_one = 0;
	contiguous = 0;
	variable_nacc = 0;
	numacc    = RUNS;
	numepochs = EPOCHS;
	zero_only = ZERO_ONLY;
	mincnt    = MIN_COUNT;
	maxcnt    = MAX_COUNT;	
	window_memory_shared = SHARED_MEM;
	mpi_func = DEF_OP;
	warm_up  = 0;
	synchronisation = DEF_SYNC;
	distance = DISTANCE;

	MPI_Init (&argc, &argv);
	MPI_Get_processor_name (pname, &cnt);
	MPI_Comm_rank (MPI_COMM_WORLD, &myrank);
	MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
	printf ("# [%d] on %s\n", myrank, pname); fflush (stdout);

#ifndef CRAY
    while((c = getopt(argc, argv, "n:N:t:b:e:d:psgazZfxlhocvw?")) != EOF) {
		switch(c) {
		case 'n':
			numacc = atoi (optarg);
			break;
		case 'N':
			numepochs = atoi (optarg);
			break;
		case 'b':
			mincnt =  atoi (optarg);
			break;
		case 't':
			terminate_cnt =  atoi (optarg);
			break;
		case 'e':
			maxcnt = atoi (optarg);
			break;
		case 'd':
			distance = atoi (optarg);
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
		case 'z':
			zero_only = 1;
			break;
		case 'Z':
			zero_only = -1;
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
		case 'c':
			contiguous = 1;
			break;
		case 'v':
			variable_nacc = 1;
			break;
		case '?':
		case 'h':
			if ( myrank == 0) {
				printf("Usage: %s [options]\n", argv[0]);
				printf("  -g             use MPI_Get. [default: MPI_Put]\n");
				printf("  -a             use MPI_Accumulate. [default: MPI_Put]\n");
				printf("  -b MINCNT      min. access size MINCNT elements. [1]\n");
				printf("  -e MAXCNT      max. access size MAXCNT elements. [10000]\n");
				printf("  -N NUMEPOCHS   test over NUMEPOCHS access epochs for each access size. [1]\n");
				printf("  -n NUMACCESS   in each epoch, perform NUMACCESS accesses. [1]\n");
				printf("  -v             vary the number of accesses depending on access size. [1]\n");
				printf("  -w             do a warm-up run in advance. [1]\n");
				printf("  -c             perform contiguous accesses. [1]\n");
				printf("  -p             force getting private memory.\n");
				printf("  -s             force getting shared memory.\n");
				printf("  -z             only process 0 performs accesses.\n");
				printf("  -Z             all  processes access window at process 0.\n");
				printf("  -d DISTANCE    processs i sends to i+DISTANCE mod numprocs [default: 1]\n");
				printf("  -f             Synchronisation: MPI_Win_fence. [default]\n");
				printf("  -x             Synchronisation: Exposure.\n");
				printf("  -o             one-on-one exposure synchronization [default: all].\n");
				printf("  -l             Synchronisation: MPI_Win_lock.\n");
			}
			MPI_Finalize();
			exit(1);
		}
	}
#endif /* CRAY */

	/* window memory */		
#if MPI_ALLOC_MEM
	MPI_Alloc_mem (maxcnt*sizeof(double), meminfo, (void *)&base);
#else
	base = (void *)malloc(maxcnt*sizeof(double));
#endif
	if (!base) {
		fprintf (stderr, "No  memory\n");
		MPI_Abort (MPI_COMM_WORLD, -1); 
	}
	/* origin memory */
#if MPI_ALLOC_MEM
	MPI_Alloc_mem (maxcnt*sizeof(double), meminfo, (void *)&buffer);
#else
	buffer = (double *)malloc (maxcnt*sizeof(double));
#endif
	if (!buffer) {
		fprintf (stderr, "Out of memory\n");
		return -2;
	}

	MPI_Win_create (base, maxcnt*sizeof(double), sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &win);
	
	partner = (zero_only == -1) ? 0 : (myrank + distance) % numprocs;
	lpartner = (zero_only == -1) ? 0 : (myrank + numprocs - 1) % numprocs;

	MPI_Win_get_group (win, &group);
	MPI_Group_incl (group, 1, &lpartner, &exp_group);
	MPI_Group_incl (group, 1, &partner, &acc_group);

	if (myrank == 0) {
		if (!contiguous)
			printf ("# performance of %s, strided access\n", mpi_funcname[mpi_func]);
		else
			printf ("# performance of %s, contiguous access\n", mpi_funcname[mpi_func]);
		printf ("# numprocs = %d, window_size = %d, window_memory_shared = %d\n", 
				numprocs, maxcnt*sizeof(double), window_memory_shared);
		printf ("# mincnt = %d, maxcnt = %d (MPI_DOUBLE), %d epochs with %d accesses per epoch, distance = %d\n", 
				mincnt, maxcnt, numepochs, numacc, distance);
		if (zero_only == 1)
			printf ("# only process 0 performs accesses\n");
		else
			if (zero_only == -1)
				printf ("# all processes access data at process 0\n");		
		printf ("# synchronisation = %s\n", synchname[synchronisation]);
		printf ("# latencies for single access [us], effective bandwidth for epoch [MB/s]/s\n");
		printf ("# len (byte)  \tn_acc \tacc_min \tacc_max \tsync_min \tsync_max \ttotal_min \ttotal_max \tB_min [MB/s] \tB_max [MB/s]\n");
		fflush(stdout);
	}

	for (w = warm_up; w >= 0; w--) {
		for (cnt = mincnt; (cnt <= maxcnt) && ((terminate_cnt == 0) || (cnt < terminate_cnt));
										   cnt = cnt < 8 ?
										   cnt + 1 :cnt <= 256 ? 
										   cnt + 4 : cnt <= 1024 ? 
										   cnt + 16 : (cnt*3)/2) {
			MPI_Barrier(MPI_COMM_WORLD);
			
			access_cnt = 0; offset = 0;
			t_sync_open = 0.0; t_sync_close = 0.0; t_acc = 0.0;
			for (e = 0; e < numepochs; e++) {
				t0 = MPI_Wtime();
				/* START - choose synchronisation */
				switch (synchronisation) {
				case SYNC_FENCE:
					MPI_Win_fence (0, win);
					break;
				case SYNC_GROUP:
					if (zero_only == -1) {
						/* all processes access memory at process 0 */
						if (myrank == 0) {
							MPI_Win_post (group, 0, win);
							MPI_Win_start(group, 0, win);
						} else {
							MPI_Win_start(acc_group, 0, win);
						}
					} else 						
						if (one_on_one) {
							MPI_Win_post (exp_group, 0, win);
							MPI_Win_start(acc_group, 0, win);
						} else {
							MPI_Win_post (group, 0, win);
							MPI_Win_start(group, 0, win);
						}
					break;
				case SYNC_LOCK:
					MPI_Win_lock(MPI_LOCK_SHARED, partner, 0, win);
					break;
				}
				t_sync_open += (MPI_Wtime() - t0);
				
				/* do the MPI_Gets / MPI_Puts and take time */
				if ((zero_only <= 0 || myrank == 0) && cnt > 0) {
					for (a = 0; a < numacc; a++) {
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
						t_acc += (MPI_Wtime() - t0);
						access_cnt++;
						offset += cnt;
						if (!contiguous)
							offset++;
						if (offset + cnt > maxcnt)
							offset = 0;
					}
				}
				
				/* END - choose synchronisation */
				t0 = MPI_Wtime();
				switch (synchronisation) {
				case SYNC_FENCE:
					MPI_Win_fence (0, win);
					break;
				case SYNC_GROUP:
					MPI_Win_complete(win);
					MPI_Win_wait(win);
					break;
				case SYNC_LOCK:
					MPI_Win_unlock(partner, win);
					break;
				}
				t_sync_close += (MPI_Wtime() - t0);
			}
			/* print out performance of communication epoch */
			t_sync = t_sync_open +  t_sync_close;

			if (zero_only <= 0) {
				MPI_Reduce (&t_acc, &acc_min, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
				MPI_Reduce (&t_acc, &acc_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
				MPI_Reduce (&t_sync, &sync_min, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
				MPI_Reduce (&t_sync, &sync_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
				total_min = acc_min + sync_min;
				total_max = acc_max + sync_max;
			} else {
				total_min = t_acc + t_sync;
				total_max = t_acc + t_sync;
				acc_min = acc_max = t_acc;
				sync_min = sync_max = t_sync;
			}
			
			if (myrank == 0 && !warm_up) {
				printf ("%d \t\t%d \t%8.3f \t%8.3f \t%8.3f \t%8.3f \t%8.3f \t%8.3f \t%8.3f \t%8.3f\n", 
						cnt*sizeof(double), 
						access_cnt,
						(acc_min*1e+6)/(double)access_cnt, (acc_max*1e+6)/(double)access_cnt, 
						sync_min*1e+6/numepochs, sync_max*1e+6/numepochs, 
						total_min*1e+6, total_max*1e+6, 
						(cnt*sizeof(double)*access_cnt)/(total_min*1024*1024), 
						(cnt*sizeof(double)*access_cnt)/(total_max*1024*1024));
				fflush (stdout);
			}
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
