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
	char			buffer [2048];
	char			helpbuf[STRSIZE];
	char			* cmpbuffer, * rcvbuffer;
	int				numruns = 1;
	MPI_Datatype	dtype_s, dtype_r;
	struct send_t {
		int		round;
		int		proc;
		int		a;
	} *send_val;
	struct recv_t {
		int		round;
		int		proc;
		char	a[4];
	} *recv_val;
	int				blocklens[3];
	MPI_Aint		disps[3];
	MPI_Datatype	types[3];
	MPI_Aint		dtype_extent;
	int				dtype_size;
	union map_t {
		int		a;
		char	b[4];
	}				map;

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

	mysize = (sizeof (struct send_t) + sizeof (struct recv_t)) * 3 * numprocs;
	prnt (myrank, "MPI_Alloc_mem\n");
	MPI_Alloc_mem (mysize, MPI_INFO_NULL, &base);
	if (!base) {
		fprintf (stderr, "No shared memory\n");
		return -1; 
	}

	prnt (myrank, "MPI_Win_create\n");
	MPI_Win_create (base, mysize, sizeof (struct recv_t), 
					MPI_INFO_NULL, MPI_COMM_WORLD, &win);

	prnt (myrank, "put_diff_type test\n");

	prnt (myrank, "create dtype\n");
	blocklens[0] = 1;
	blocklens[1] = 1;
	blocklens[2] = 1;
	disps[0] = offsetof (struct send_t, round);
	disps[1] = offsetof (struct send_t, proc);
	disps[2] = offsetof (struct send_t, a);
	types[0] = MPI_INT;
	types[1] = MPI_INT;
	types[2] = MPI_INT;
	MPI_Type_struct (3, blocklens, disps, types, &dtype_s);
	MPI_Type_commit (&dtype_s);

	blocklens[0] = 1;
	blocklens[1] = 1;
	blocklens[2] = 4;
	disps[0] = offsetof (struct recv_t, round);
	disps[1] = offsetof (struct recv_t, proc);
	disps[2] = offsetof (struct recv_t, a);
	types[0] = MPI_INT;
	types[1] = MPI_INT;
	types[2] = MPI_CHAR;
	MPI_Type_struct (3, blocklens, disps, types, &dtype_r);
	MPI_Type_commit (&dtype_r);

	if (myrank == 0) {
		printf ("put different type (n=%d) ...", numruns);
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
					map.a = (k+3*j) + 0x41424341;
					sprintf (helpbuf, "(%d:%d:%c,%c,%c,%c) ", i, j,
										map.b[3], map.b[2], map.b[1],
										map.b[0]);
					strcat (cmpbuffer, helpbuf);
				}
				strcat (cmpbuffer, "\n");
			}
		}
	}



	prnt (myrank, "start loop (%d times)\n", numruns);
	for (j=0; j<numruns; j++) {
		MPI_Win_fence (0, win);
		send_val = (struct send_t *) base;
		for (i=0; i<3; i++) {
			send_val[i].round = j;
			send_val[i].proc = myrank;
			send_val[i].a = (i+3*myrank) + 0x41424341;
		}
		MPI_Win_fence (0, win);
		MPI_Win_lock (MPI_LOCK_EXCLUSIVE, 0, 0, win);
		MPI_Put (send_val, 3, dtype_s, 0, 3*(myrank+1), 3, dtype_r, win);
		MPI_Win_unlock (0, win);
		MPI_Win_fence (0, win);

		if (myrank == 0) {
			for (i=0; i<numprocs; i++) {
				recv_val = (struct recv_t *)base + 3*(i+1);
				for (k=0; k<3; k++) {
					sprintf (helpbuf, "(%d:%d:%c,%c,%c,%c) ", 
							recv_val[k].round, recv_val[k].proc,
							recv_val[k].a[3], recv_val[k].a[2],
							recv_val[k].a[1], recv_val[k].a[0]);
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
