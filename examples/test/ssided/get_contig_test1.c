#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "mpi.h"
#include "stdarg.h"


static int dodebug = 0;

static
int
prnt (int myrank, const char *fmt, ...) 
{
	va_list	ap;
	char 	str[1024];
	char	*s;
	
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


#define STRSIZE 64


int
main (argc, argv)
	int		argc;
	char	** argv;
{
	int				myrank=-1, numprocs, mysize;
	int				i, j;
	MPI_Win			win;
	char			* base = NULL;
	char			buffer [STRSIZE];
	char			helpbuf[STRSIZE];
	char			* cmpbuffer, * rcvbuffer;
	int				numruns = 1;

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

	mysize = STRSIZE * numprocs;
	prnt (myrank, "MPI_Alloc_mem\n");
	MPI_Alloc_mem (mysize, MPI_INFO_NULL, &base);
	if (!base) {
		fprintf (stderr, "No shared memory\n");
		return -1; 
	}


	if (myrank == 0) {
		printf ("get_contig test 1 (local private; n=%d) ...", numruns);
		if (dodebug) printf ("\n");
		fflush (stdout);
		/* create compare buffer */
		cmpbuffer = malloc (mysize * numruns);
		rcvbuffer = malloc (mysize * numruns);
		if (!cmpbuffer || !rcvbuffer) {
			fprintf (stderr, "Out of memory\n");
			return -4;
		}
		cmpbuffer[0] = 0;
		rcvbuffer[0] = 0;
		for (i=0; i<numruns; i++) {
			for (j=0; j<numprocs; j++) {
				sprintf (helpbuf, "Hello, I'm Proc %d in round %d\n", j, i);
				strcat (cmpbuffer, helpbuf);
			}
		}
	}


	prnt (myrank, "MPI_Win_create\n");
	MPI_Win_create (base, mysize, STRSIZE, MPI_INFO_NULL, MPI_COMM_WORLD, &win);

	prnt (myrank, "get_contig test no. 1\n");
	for (j=0; j<numruns; j++) {
		MPI_Win_fence (0, win);
		sprintf (base, "Hello, I'm Proc %d in round %d\n", myrank, j);
		MPI_Win_fence (0, win);

		if (myrank == 0) {
			for (i=0; i<numprocs; i++) {
				prnt (myrank, "MPI_Get\n");
				MPI_Get (	buffer, 2048, MPI_CHAR, i, 0, STRSIZE, 
							MPI_CHAR, win);
				strcat (rcvbuffer, buffer);
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
