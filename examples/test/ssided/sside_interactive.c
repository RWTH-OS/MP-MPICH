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



int
main (argc, argv)
	int		argc;
	char	** argv;
{
	int				myrank=-1, numprocs, mysize;
	int				i, j, k;
	MPI_Win			win;
	char			* base = NULL;
	int				*stop, *value;
	int				sendval=0, sendstop=0;
	int				sendproc;
	int				oldvalue;
	char			buffer[256];
	char			valstr[256], opstr[256];
	int				num;
	MPI_Op			op;
	char			*endptr;

	prnt (myrank, "MPI_Init\n");
	MPI_Init (&argc, &argv);

	for (i=1; i<argc; i++) {
		if (!strcmp (argv[i], "-v")) {
			dodebug = 1;
		}
	}

	prnt (myrank, "MPI_Comm_rank\n");
	MPI_Comm_rank (MPI_COMM_WORLD, &myrank);
	prnt (myrank, "MPI_Comm_size\n");
	MPI_Comm_size (MPI_COMM_WORLD, &numprocs);

	prnt (myrank, "MPI_Alloc_mem\n");
	if (myrank != 0) {
		MPI_Alloc_mem (2*sizeof(int), MPI_INFO_NULL, &base);
		if (!base) {
			fprintf (stderr, "No shared memory\n");
			return -1; 
		}

		prnt (myrank, "MPI_Win_create\n");
		MPI_Win_create (base, 2*sizeof (int), sizeof (int), 
						MPI_INFO_NULL, MPI_COMM_WORLD, &win);
		stop = &((int*)(void*)base)[0];
		value = &((int*)(void*)base)[1];
		*stop = 0;
		*value = 0;
		oldvalue = *value;
	} else {
		prnt (myrank, "MPI_Win_create\n");
		MPI_Win_create (NULL, 0, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &win);
	}
		
	prnt (myrank, "start test\n");

	if (myrank != 0) {
		printf ("value = %d\n", *value);
		fflush (stdout);
		while (!*stop) {
			if (oldvalue != *value) {
				printf ("value = %d\n", *value);
				fflush (stdout);
				oldvalue = *value;
			}
		}
	} else {
		while (1) {
			printf ("enter command: ");
			fflush (stdout);
			if (fgets (buffer, 256, stdin) == NULL || 
						buffer[0] == 'q' || buffer[0] == 'Q') {
				sendstop = 1;
				for (i=1; i<numprocs; i++) 
					MPI_Put (&sendstop, 1, MPI_INT, i, 0, 1, MPI_INT, win);
				break;
			}
			if ((num = sscanf (buffer, "%d %s %s", &sendproc, opstr, 
										valstr)) == 0) {
				fprintf (stderr, "usage: <proc> [[<op>]<value>]\n"
								 "   or: quit\n");
				fflush (stderr);
				continue;
			}
			if (sendproc < 1 || sendproc >= numprocs) {
				fprintf (stderr, "invalid process %d (must be in the range"
								 " 1-%d\n", sendproc, numprocs-1);
				fflush (stderr);
				continue;
			}
			if (num == 1) {
				MPI_Get (&sendval, 1, MPI_INT, sendproc, 1, 1, MPI_INT, win);
				printf ("value on proc %d is %d\n", sendproc, sendval);
				fflush (stdout);
				continue;
			}
			if (num == 3) {
				sendval = strtol (valstr, &endptr, 10);
				if (endptr == valstr) {
					fprintf (stderr, "usage: <proc> [[<op>]<value>]\n"
									 "   or: quit\n");
					fflush (stderr);
					continue;
				}
			} else if (num == 2) {
				if (isdigit (opstr[0])) {
					sendval = strtol (opstr, &endptr, 10);
					if (endptr == opstr) {
						fprintf (stderr, "usage: <proc> [[<op>]<value>]\n"
										 "   or: quit\n");
						fflush (stderr);
						continue;
					}
				} else {
					sendval = strtol (opstr+1, &endptr, 10);
					if (endptr == opstr+1) {
						fprintf (stderr, "usage: <proc> [[<op>]<value>]\n"
										 "   or: quit\n");
						fflush (stderr);
						continue;
					}
					opstr[1] = 0;
					num = 3;
				}
			}
			if (num == 2) {
				MPI_Put (&sendval, 1, MPI_INT, sendproc, 1, 1, MPI_INT, win);
			} else if (num == 3) {
				switch (opstr[0]) {
				case '+': op = MPI_SUM; break;
				case '=': op = MPI_REPLACE; break;
				case '-': op = MPI_SUM; sendval *= -1; break;
				case '*': op = MPI_PROD; break;
				case '/': op = MPI_DIV; break;
				case '%': op = MPI_MOD; break;
				case '>': op = MPI_MAX; break;
				case '<': op = MPI_MIN; break;
				case '&': op = MPI_BAND; break;
				case '|': op = MPI_BOR; break;
				case '^': op = MPI_BXOR; break;
				default:
					fprintf (stderr, "invalid operator %c\n", opstr[0]);
					fflush (stderr);
					continue;
				}
				MPI_Accumulate (&sendval, 1, MPI_INT, sendproc,
								1, 1, MPI_INT, op, win);
			}
		}
	}

	printf ("Good bye!\n");
	fflush (stdout);

	prnt (myrank, "MPI_Win_fence\n");
	MPI_Win_fence (0, win);
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
