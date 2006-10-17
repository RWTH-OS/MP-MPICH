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
	char			* base = NULL;
	char			helpbuf[STRSIZE];
	char			* rcvbuffer, * cmpbuffer;
	int				numruns = 1;
	MPI_Datatype	dtype, dtype_help;
	struct test_t {
		int		a;
		char	b;
		int		c[4];
	} 				* test_val;
	int				blocklens[3];
	MPI_Aint		disps[3];
	MPI_Datatype	types[3];
	MPI_Aint		dtype_extent;
	int				dtype_size;

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
		}
	}

	prnt (myrank, "MPI_Comm_rank\n");
	MPI_Comm_rank (MPI_COMM_WORLD, &myrank);
	prnt (myrank, "MPI_Comm_size\n");
	MPI_Comm_size (MPI_COMM_WORLD, &numprocs);

	mysize = 3*sizeof (struct test_t) * numprocs;
	prnt (myrank, "MPI_Alloc_mem\n");
	MPI_Alloc_mem (mysize, MPI_INFO_NULL, &base);
	if (!base) {
		fprintf (stderr, "No shared memory\n");
		return -1; 
	}

	prnt (myrank, "MPI_Win_create\n");
	MPI_Win_create (base, mysize, sizeof (struct test_t), 
					MPI_INFO_NULL, MPI_COMM_WORLD, &win);

	if (myrank == 0) {
		printf ("put_same_type test 2 (local shared; n=%d) ...", numruns);
		if (dodebug) printf ("\n");
		fflush (stdout);

		/* create compare buffer */
		cmpbuffer = malloc (STRSIZE * numprocs * numruns);
		rcvbuffer = malloc (STRSIZE * numprocs * numruns);
		if (!cmpbuffer || !rcvbuffer) {
			fprintf (stderr, "Out of memory\n");
			return -4;
		}
		cmpbuffer[0] = 0;
		rcvbuffer[0] = 0;
		for (i=0; i<numruns; i++) {
			for (j=0; j<numprocs; j++) {
				for (k=0; k<3; k++) {
					sprintf (helpbuf, "(113a:%d/%d/%d:113) ", k, i, j);
					strcat (cmpbuffer, helpbuf);
				}
			strcat (cmpbuffer, "\n");
			}
		}
	}

	prnt (myrank, "create dtype\n");
	blocklens[0] = 1;
	blocklens[1] = 1;
	blocklens[2] = 4;
	disps[0] = offsetof (struct test_t, a);
	disps[1] = offsetof (struct test_t, b);
	disps[2] = offsetof (struct test_t, c);
	types[0] = MPI_INT;
	types[1] = MPI_CHAR;
	types[2] = MPI_INT;
	MPI_Type_struct (3, blocklens, disps, types, &dtype);
	MPI_Type_commit (&dtype);
	MPI_Type_extent (dtype, &dtype_extent);
	MPI_Type_size	(dtype, &dtype_size);

	prnt (myrank, "size = %d\nextent = %d\n",
					dtype_size, dtype_extent);
	prnt (myrank, "size/extent = %2.2f\n", (float)dtype_size / 
											(float)dtype_extent);
	for (j=0; j<numruns; j++) {
		MPI_Win_fence (0, win);
		test_val = (struct test_t *) base;
		for (i=0; i<3; i++) {
			test_val[i].a = 113;
			test_val[i].b = 'a';
			test_val[i].c[0] = i;
			test_val[i].c[1] = j;
			test_val[i].c[2] = myrank;
			test_val[i].c[3] = 113;
		}
		MPI_Win_fence (0, win);
		MPI_Win_lock (MPI_LOCK_EXCLUSIVE, 0, 0, win);
		MPI_Put (base, 3, dtype, 0, 3*myrank, 3, dtype, win);
		MPI_Win_unlock (0, win);
		MPI_Win_fence (0, win);

		if (myrank == 0) {
			for (i=0; i<numprocs; i++) {
				test_val = (struct test_t *) base + 3*i;
				for (k=0; k<3; k++) {
					sprintf (helpbuf, "(%d%c:%d/%d/%d:%d) ", 
							test_val[k].a, test_val[k].b,
							test_val[k].c[0], test_val[k].c[1],
							test_val[k].c[2], test_val[k].c[3]);
					strcat (rcvbuffer, helpbuf);
				}
				strcat (rcvbuffer, "\n");
			}
		}
	}
	
	prnt (myrank, "MPI_Win_fence\n");
	MPI_Win_fence (0, win);

	if (myrank == 0) {
		if (strcmp (rcvbuffer, cmpbuffer)) 
			printf (" ***NOT***");
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
