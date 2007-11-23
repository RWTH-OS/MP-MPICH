/* $Id: rr_write.c 2884 2004-10-28 13:30:39Z  $ */

/* Synthetic benchmark 'rr_write' (round-robin write): 
   All processes create a common file, then each process N writes to the 
   part of the file which has previously written by N-1. */

#include "mpi.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DEFAULT_SIZE  (1048576*4)       /* default file size/per node in bytes */
#define DEFAULT_LOOPS 5

int main(int argc, char **argv)
{
    MPI_File fh;
    MPI_Status status;
    int *buf, i, j, mynod, nprocs, ntimes = DEFAULT_LOOPS, len, err, flag;
    int loop, proc_loop;
    double stim, read_tim, write_tim, new_read_tim, new_write_tim;
    double min_read_tim=10000000.0, min_write_tim=10000000.0, read_bw, write_bw;
    char *filename = NULL;
    unsigned int filelen;
    unsigned int frgmt_size = DEFAULT_SIZE;

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &mynod);

    /* process 0 parses the command-line arguments and broadcasts them to other processes */
    if (!mynod) {
	i = 1;
	while (i < argc) {
	    if (!strcmp("-fname", *argv)) {
		argv++; i++;
		len = strlen(*argv);
		filename = (char *) malloc(len+1);
		strcpy(filename, *argv);
	    }
	    if (!strcmp("-fsize", *argv)) {
		argv++; i++;
		frgmt_size = atoi(*argv)*1024*1024;
	    }
	    if (!strcmp("-loops", *argv)) {
		argv++; i++;
		ntimes = atoi(*argv);
	    }
	    i++;
	    argv++;
	}
	if (filename == NULL) {
	    printf("\n*#  Usage: %s -fname filename -fsize MB_per_proc -loops nbr_loops\n\n", argv[0]);
	    MPI_Abort(MPI_COMM_WORLD, 1);
	}
	MPI_Bcast(&frgmt_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(filename, len+1, MPI_CHAR, 0, MPI_COMM_WORLD);
	printf("Access size per process = %d bytes, ntimes = %d\n", frgmt_size, ntimes);
    } else {
	MPI_Bcast(&frgmt_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
	filename = (char *) malloc(len+1);
	MPI_Bcast(filename, len+1, MPI_CHAR, 0, MPI_COMM_WORLD);
    }

    filelen = frgmt_size * mynod;
    buf = (int *) malloc(frgmt_size);
    if (!buf) {
	printf("\n*#  not enough memory to allocate local buffer (%l bytes)\n", frgmt_size);
	MPI_Abort(MPI_COMM_WORLD, 1);
    }

    for (loop = 0; loop < ntimes; loop++) {
	MPI_File_open (MPI_COMM_WORLD, filename, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
	for (proc_loop = 0; proc_loop < nprocs; proc_loop++) {
	    MPI_File_seek(fh, proc_loop*frgmt_size, MPI_SEEK_SET);
	    MPI_Barrier(MPI_COMM_WORLD);

	    stim = MPI_Wtime();
	    MPI_File_write(fh, buf, frgmt_size, MPI_BYTE, &status);
	    write_tim = MPI_Wtime() - stim;
	    
	    MPI_Barrier(MPI_COMM_WORLD);
	    MPI_File_seek(fh, proc_loop*frgmt_size, MPI_SEEK_SET);
	    MPI_Barrier(MPI_COMM_WORLD);

	    stim = MPI_Wtime();
	    MPI_File_read(fh, buf, frgmt_size, MPI_BYTE, &status);
	    read_tim = MPI_Wtime() - stim;
	    
	    MPI_Allreduce(&write_tim, &new_write_tim, 1, MPI_DOUBLE, MPI_MAX,
			  MPI_COMM_WORLD);
	    MPI_Allreduce(&read_tim, &new_read_tim, 1, MPI_DOUBLE, MPI_MAX,
			  MPI_COMM_WORLD);
	    
	    min_read_tim = (new_read_tim < min_read_tim) ? 
		new_read_tim : min_read_tim;
	    min_write_tim = (new_write_tim < min_write_tim) ? 
		new_write_tim : min_write_tim;
	}
	MPI_File_close(&fh);
    }
    
    if (mynod == 0) {
	read_bw = filelen/(min_read_tim*1024.0*1024.0);
	write_bw = filelen/(min_write_tim*1024.0*1024.0);
	printf("Write bandwidth without file sync = %f Mbytes/sec\n", write_bw);
	printf("Read bandwidth without prior file sync = %f Mbytes/sec\n", read_bw);
    }

    min_write_tim = 10000000.0;
    min_read_tim = 10000000.0;

    flag = 0;
    for (loop = 0; loop < ntimes; loop++) {
	MPI_File_open (MPI_COMM_WORLD, filename, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
	for (proc_loop = 0; proc_loop < nprocs; proc_loop++) {
	    MPI_File_seek(fh, proc_loop*frgmt_size, MPI_SEEK_SET);
	    MPI_Barrier(MPI_COMM_WORLD);
	    
	    stim = MPI_Wtime();
	    MPI_File_write(fh, buf, frgmt_size, MPI_BYTE, &status);
	    err = MPI_File_sync(fh);
	    write_tim = MPI_Wtime() - stim;
	    if (err == MPI_ERR_UNKNOWN) {
		flag = 1;
		break;
	    }
	    
	    MPI_Barrier(MPI_COMM_WORLD);
	    MPI_File_seek(fh, proc_loop*frgmt_size, MPI_SEEK_SET);
	    MPI_Barrier(MPI_COMM_WORLD);

	    stim = MPI_Wtime();
	    MPI_File_read(fh, buf, frgmt_size, MPI_BYTE, &status);
	    read_tim = MPI_Wtime() - stim;
  
	    MPI_Allreduce(&write_tim, &new_write_tim, 1, MPI_DOUBLE, MPI_MAX,
			  MPI_COMM_WORLD);
	    MPI_Allreduce(&read_tim, &new_read_tim, 1, MPI_DOUBLE, MPI_MAX,
			  MPI_COMM_WORLD);
	    
	    min_read_tim = (new_read_tim < min_read_tim) ? 
		new_read_tim : min_read_tim;
	    min_write_tim = (new_write_tim < min_write_tim) ? 
		new_write_tim : min_write_tim;
	}
	MPI_File_close(&fh);
    }


    if (mynod == 0) {
	if (flag) 
	    printf("MPI_File_sync returns error.\n");
	else {
	    read_bw = filelen/(min_read_tim*1024.0*1024.0);
	    write_bw = filelen/(min_write_tim*1024.0*1024.0);
	    printf("Write bandwidth including file sync = %f Mbytes/sec\n", write_bw);
	    printf("Read bandwidth after file sync = %f Mbytes/sec\n", read_bw);
	}
    }
    
    free(buf);
    free(filename);
    MPI_Finalize();
    return 0;
}
