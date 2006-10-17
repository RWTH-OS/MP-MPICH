/* $Id$ */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi.h"
#include "mpptest.h"

/* memory allocation and deallocation */

int use_mpi_alloc = 0;

void *mpp_alloc (size_t len)
{
    MPI_Info info;
    void *buf = NULL;

#if HAVE_MPI_ALLOC_MEM    
    if (use_mpi_alloc) {
	MPI_Info_create (&info);
#if 0
	MPI_Info_set (info, "alignment", "4096");
	MPI_Info_set (info, "type", "private");
#endif
	MPI_Alloc_mem (len, info, &buf);
	MPI_Info_free (&info);
   } else 
#endif	
	buf = malloc(len);

    if (buf == NULL) {
	fprintf (stderr, "Could not allocate %d byte buffer\n", len);
	MPI_Abort (MPI_COMM_WORLD, -1);
    }
    
    return buf;
}

void mpp_free (void *buf)
{
#if HAVE_MPI_ALLOC_MEM
    if (use_mpi_alloc) 
	MPI_Free_mem (buf);
    else 
#endif	
	free (buf);
    return;
}
