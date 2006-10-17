/* $Id */

/* simulate read-write access to a sparse matrix via single-sided communication */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <unistd.h>

#include "mpi.h"

/* default settings */
#define MPI_FUNC   0
#define MIN_COUNT  1
#define MAX_COUNT  1000
#define SHARED_MEM 0
#define RUNS       1
#define ZERO_ONLY  0
#define SYNCHRONISATION 0 /* 0 = Win_fence, 1 = exposure, 2 = Win_lock */
char* synchname[] = { "MPI_Win_fence", "Exposure", "MPI_Win_lock" };
char* mpi_funcname[] = { "MPI_Put", "MPI_Get", "MPI_Accumulate" };

#define BIGVALUE 12345678901234567890.1234567890123456789
#define ACCU_INC 5


int main (argc, argv)
	int		argc;
	char	** argv;
{
	MPI_Win		  win;
	MPI_Group     group;
	MPI_Datatype  dtype, dtype_help;
	MPI_Info      meminfo = MPI_INFO_NULL;

	double	*base = NULL, *buffer = NULL;
	double	secs, min_secs, max_secs, total_min, total_max;
	int		myrank, numprocs;
	int		i, j, k, c, offset;
	int		numruns, zero_only, testnum, window_memory_shared;
	int		cnt, mincnt, maxcnt, terminate_cnt = 0, synchronisation, mpi_func;
	char    pname[256];
	int     error = 0;
	int     target_rank, active_rank;
	double	*local = NULL, *remote = NULL;
	double  correct_value;


	numruns   = RUNS;
	zero_only = ZERO_ONLY;
	mincnt    = MIN_COUNT;
	maxcnt    = MAX_COUNT;	
	window_memory_shared = SHARED_MEM;
	mpi_func = MPI_FUNC;
	synchronisation = SYNCHRONISATION;

	MPI_Init (&argc, &argv);
	MPI_Get_processor_name (pname, &cnt);
	MPI_Comm_rank (MPI_COMM_WORLD, &myrank);
	MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
	printf ("# [%d] on %s\n", myrank, pname); fflush (stdout);

#ifndef CRAY
    while((c = getopt(argc, argv, "n:t:b:e:psgazfxlh?")) != EOF) {
		switch(c) {
		case 'n':
			numruns = atoi (optarg);
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
			mpi_func = 1;
			break;
		case 'a':
			mpi_func = 2;
			break;
		case 'z':
			zero_only = 1;
			break;
		case 'f':
			synchronisation = 0;
			break;
		case 'x':
			synchronisation = 1;
			break;
		case 'l':
			synchronisation = 2;
			break;
		case '?':
		case 'h':
			if ( myrank == 0) {
				printf("Usage: %s [options]\n", argv[0]);
				printf("  -g             use MPI_Get. [default: MPI_Put]\n");
				printf("  -a             use MPI_Accumulate. [default: MPI_Put]\n");
				printf("  -b MINCNT      start with MINCNT elements. [1]\n");
				printf("  -e MAXCNT      end with MAXCNT elements. [10000]\n");
				printf("  -n NUMRUNS     do each test NUMRUNS times. [1]\n");
				printf("  -p             force getting private memory.\n");
				printf("  -s             force getting shared memory.\n");
				printf("  -z             only process 0 is active.\n");
				printf("  -f             Synchronisation: MPI_Win_fence. [default]\n");
				printf("  -x             Synchronisation: Exposure.\n");
				printf("  -l             Synchronisation: MPI_Win_lock.\n");
			}
			exit(-1);
			break;
		}
	}
#endif /* CRAY */

	/* remote memory */		
	MPI_Alloc_mem (maxcnt*sizeof(double), meminfo, (void *)&base);
	if (!base) {
		fprintf (stderr, "No  memory\n");
		MPI_Abort (MPI_COMM_WORLD, -1); 
	}
	/* local memory */
	buffer = (double *)malloc (maxcnt*sizeof(double));
	if (!buffer) {
		fprintf (stderr, "Out of memory\n");
		return -2;
	}

	MPI_Win_create (base, maxcnt*sizeof(double), sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &win);

	MPI_Win_get_group (win, &group);

	switch (mpi_func) {
	case 0:
		local = base;
		remote = buffer;
		break;
	case 1:
		local = buffer;
		remote = base;
		break;
	case 2:
		local = base;
		remote = buffer;
		break;
	}		

	if (myrank == 0) {
		printf ("# checking for correctness of %s, strided access\n", mpi_funcname[mpi_func]);
		printf ("# numprocs = %d, window_size = %d, window_memory_shared = %d\n", 
				numprocs, maxcnt*sizeof(double), window_memory_shared);
		printf ("# mincnt = %d, maxcnt = %d, numruns = %d\n", mincnt, maxcnt, numruns);
		printf ("# synchronisation = %s\n", synchname[synchronisation]);
		fflush(stdout);
	}

	/* this loop increases the block size */
	for (cnt = mincnt; (cnt <= maxcnt) && ((terminate_cnt == 0) || (cnt < terminate_cnt));
										   cnt = cnt < 8 ?
										   cnt + 1 :cnt <= 256 ? 
										   cnt + 4 : cnt <= 1024 ? 
										   cnt + 16 : (cnt*3)/2) {
		/* all processes get the same target starting with rank0 till last one */
		for ( target_rank = 0; target_rank < numprocs; target_rank++ ) {
			/* only one process makes his jobs. Others only sync. */
		    for ( active_rank = 0; active_rank < numprocs; active_rank++ ) {
				offset = 0;
				
				/* remote memory get numbers checked for correctness.
				   local buffer is initialized with blanks for checking
				   if offset is untouched. */
				for (j=0; j<maxcnt; j++) {
					*(remote + j) = myrank + cnt + j;
					if ( mpi_func == 2 )
						*(local + j) = ACCU_INC;
					else
						*(local + j) = BIGVALUE;
				}
				
				/* START - choose synchronisation */
				switch (synchronisation) {
				case 0 :
					MPI_Win_fence (0, win);
					break;
				case 1 :
					MPI_Win_post (group, 0, win);
					MPI_Win_start(group, 0, win);
					break;
				case 2 :
					MPI_Win_lock(MPI_LOCK_SHARED, target_rank, 0, win);
					break;
				}

				if ( active_rank == myrank ) {
					/* do the MPI_Gets / MPI_Puts / MPI_Accumulate */
					if (!zero_only || (myrank == 0)) {
						while (offset + cnt <= maxcnt) {
							for (j = 0; j < numruns; j++) {
								switch ( mpi_func ) {
								case 0:
									MPI_Put (buffer + offset, cnt, MPI_DOUBLE, target_rank, offset, cnt, MPI_DOUBLE, win); 
									break;
								case 1:
									MPI_Get (buffer + offset, cnt, MPI_DOUBLE, target_rank, offset, cnt, MPI_DOUBLE, win);
									break;
								case 2:
									MPI_Accumulate (buffer + offset, cnt, MPI_DOUBLE, target_rank, offset, cnt, MPI_DOUBLE, MPI_SUM, win);
									break;
								}
							}
							offset += cnt + 1;
						}
					}
				}

				/* END - choose synchronisation */
				switch (synchronisation) {
				case 0 :
					MPI_Win_fence (0, win);
					break;
				case 1 :
					MPI_Win_complete(win);
					MPI_Win_wait(win);
					break;
				case 2 :
					MPI_Win_unlock(target_rank, win);
					break;
				}


				/* MPI_Put and locking synchronisation has to wait for
				   partner process complete writing in local memory. */
				if (synchronisation == 2 && mpi_func == 0)
					MPI_Win_fence (0, win);
				

				/* if you are target or you did a get you have to check for correct content */
				if ( (mpi_func != 1 && target_rank == myrank) || ( mpi_func == 1 && active_rank == myrank ) ) {
					for (j=0; j + cnt <= maxcnt; j++) {
						if ( (j+1) % (cnt+1) == 0 ) {
							if ( (*(local + j) != (double)BIGVALUE) && (mpi_func == 2 && *(local + j) != ACCU_INC) ) {
								printf("[%i] Error. Free place overwritten: cnt: %i  offset: %i  value: %f should be: %f\n", myrank, cnt, j, *(local + j), (double)BIGVALUE); fflush(stdout);
								error++;
							}
						}
						else {
							switch ( mpi_func ) {
							case 0:
								correct_value = (double)active_rank + cnt + j;
								break;
							case 1:
								correct_value = (double)target_rank + cnt + j;
								break;
							case 2:
								correct_value = numruns * ((double)active_rank + cnt + j) + ACCU_INC;
								break;
							}
							if ( *(local + j) != correct_value ) {
								printf("[%i] Error. Wrong value: active_rank: %i  target_rank: %i  cnt: %i  offset: %i  value: %f should be: %f\n", myrank, active_rank, target_rank, cnt, j, *(local + j), correct_value);  fflush(stdout);
								error++;
							}
						}
					}
				}
		
/* 		printf("rank: %i cnt: %i\n", myrank, cnt); */
/* 		for (j=0; j<maxcnt; j++) */
/* 			printf("%f - ", *(local + j)); */
/* 		printf("\n"); */
/* 		fflush(stdout); */
		
			}
		}
	}


	printf("[%d] errors: %d\n", myrank, error);
	fflush (stdout);

	
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
