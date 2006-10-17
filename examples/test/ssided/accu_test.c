#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "mpi.h"
#include "stdarg.h"

#ifdef offsetof
#undef offsetof
#endif
#define offsetof(s_type,field)	((size_t) &(((s_type *)(void *)0)->field))

static int dodebug = 0;

static
int
prnt (int myrank, const char *fmt, ...) 
{
	va_list	ap;
	char    str[1024];
	char    *s;

	if (!dodebug) return 1;
	va_start (ap, fmt);
	sprintf (str, "[%d]: ", myrank);
	s = index (str, ':') + 2;
	vsnprintf (s, 1000, fmt, ap);
	s[1001] = 0;
	fputs (str, stderr);
	fflush (stderr);
	va_end (ap);
	return 1;
}

#define STRSIZE 256


int
main (argc, argv)
	int		argc;
	char	** argv;
{
	int				myrank=-1, numprocs, mysize;
	int				i, j, k;
	MPI_Win			win;
	char			* buffer;	
	char			* base = NULL;
	char			helpbuf[STRSIZE];
	char			* cmpbuffer, * rcvbuffer;
	int				numruns = 1;
	int				numblocks = 3;
	MPI_Datatype	dtype;
	struct test_t {
		int		a;
		int		b;
	} *test_val;
	int				blocklens[2];
	MPI_Aint		disps[2];
	MPI_Datatype	types[2];
	MPI_Aint		dtype_extent;
	int				dtype_size;
	int				a, b;

	prnt (myrank, "MPI_Init\n");
	MPI_Init (&argc, &argv);

	for (i=1; i<argc; i++) {
		if (!strcmp (argv[i], "-n")) {
			if (++i < argc) 
				numruns = atoi (argv[i]);
		} else if (!strcmp (argv[i], "-q")) {
			dodebug = 0;
		} else if (!strcmp (argv[i], "-v")) {
			dodebug = 1;
		} else if (!strcmp (argv[i], "-b")) {
			if (++i <argc)
				numblocks = atoi (argv[i]);
		}
	}

	prnt (myrank, "MPI_Comm_rank\n");
	MPI_Comm_rank (MPI_COMM_WORLD, &myrank);
	prnt (myrank, "MPI_Comm_size\n");
	MPI_Comm_size (MPI_COMM_WORLD, &numprocs);

	mysize = sizeof (struct test_t) * numblocks;
	prnt (myrank, "MPI_Alloc_mem\n");
	MPI_Alloc_mem (mysize, MPI_INFO_NULL, &base);
	if (!base) {
		fprintf (stderr, "No shared memory\n");
		return -1; 
	}
	buffer = malloc (mysize);
	if (!buffer) {
		fprintf (stderr, "Out of memory\n");
		return -4;
	}

	if (myrank == 0) {
		printf ("accumulate test (n=%d; b=%d) ...", numruns, numblocks);
		if (dodebug) printf ("\n");
		fflush (stdout);
		/* create compare buffer */
		cmpbuffer = malloc (STRSIZE * numblocks * numruns * numprocs);
		rcvbuffer = malloc (STRSIZE * numblocks * numruns * numprocs);
		if (!cmpbuffer || !rcvbuffer) {
			fprintf (stderr, "Out of memory\n");
			return -4;
		}
		cmpbuffer[0] = 0;
		rcvbuffer[0] = 0;
		a = b = 0;
		for (i=0; i<numruns; i++) {
			for (j=0; j<numprocs; j++) {
				a += i;
				b += j + 1;
				for (k=0; k<numblocks; k++) {
					sprintf (helpbuf, "(%d,%d) ", a, b);
					strcat (cmpbuffer, helpbuf);
				}
				strcat (cmpbuffer, "\n");
			}
		}
	}


	prnt (myrank, "MPI_Win_create\n");
	MPI_Win_create (base, mysize, sizeof (struct test_t), 
					MPI_INFO_NULL, MPI_COMM_WORLD, &win);

	prnt (myrank, "accumulate test\n");

	prnt (myrank, "create dtype\n");
	blocklens[0] = 1;
	blocklens[1] = 1;
	disps[0] = offsetof (struct test_t, a);
	disps[1] = offsetof (struct test_t, b);
	types[0] = MPI_INT;
	types[1] = MPI_INT;
	MPI_Type_struct (2, blocklens, disps, types, &dtype);
	MPI_Type_commit (&dtype);


	if (myrank == 0) {	
		test_val = (struct test_t *) base;
		for (i=0; i<3; i++) {
			test_val[i].a = 0;
			test_val[i].b = 0;
		}
	}

	prnt (myrank, "start loop (%d times)\n", numruns);
	for (j=0; j<numruns; j++) {
		MPI_Win_fence (0, win);
		test_val = (struct test_t *) buffer;
		for (i=0; i<numblocks; i++) {
			test_val[i].a = j;
			test_val[i].b = myrank+1;
		}
		for (i=0; i< numprocs; i++) {
			MPI_Win_fence (0, win);
			if (i == myrank) 
				MPI_Accumulate (test_val, numblocks, dtype, 0, 0, numblocks, 
								dtype, MPI_SUM, win);
			MPI_Win_fence (0, win);

			if (myrank == 0) {
				test_val = (struct test_t *)base;
				for (k=0; k<numblocks; k++) {
					sprintf (helpbuf, "(%d,%d) ", 	test_val[k].a, 
													test_val[k].b);
					strcat (rcvbuffer, helpbuf);
				}
				strcat (rcvbuffer, "\n");
			}
		}
	}
	
	prnt (myrank, "MPI_Win_fence\n");
	MPI_Win_fence (0, win);

	if (myrank == 0) {
		if (strcmp (cmpbuffer, rcvbuffer))
			printf (" ****NOT****");
		printf (" successfull\n");
		fflush (stdout);
	}

	prnt (myrank, "MPI_Win_free\n");
	MPI_Win_free (&win);
	prnt (myrank, "MPI_Finalize\n");
	MPI_Finalize ();
	prnt (myrank, "Good bye!\n");

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
