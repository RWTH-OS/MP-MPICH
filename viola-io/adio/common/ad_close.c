/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *   $Id: ad_close.c,v 1.1.1.1 2005/03/10 16:07:12 mhs Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "adio.h"
/* #ifdef MPISGI
#include "mpisgi2.h"
#endif */

void ADIO_Close(ADIO_File fd, int *error_code)
{
    int i, j, k, combiner, myrank, err, is_contig;
#if defined(MPICH2) || !defined(PRINT_ERR_MSG)
    static char myname[] = "ADIO_CLOSE";
#endif

    if (fd->async_count) {
#ifdef MPICH2
	*error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_IO, "**io",
	    "**io %s", strerror(errno));
	return;
#elif defined(PRINT_ERR_MSG)
	FPRINTF(stderr, "ADIO_Close: Error! There are outstanding nonblocking I/O operations on this file.\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else /* MPICH-1 */
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ERR_ASYNC_OUTSTANDING,
				     myname, (char *) 0, (char *) 0);
	ADIOI_Error(fd, *error_code, myname);
#endif
        return;
    }

    /* because of deferred open, this warants a bit of explaining.  First, if
     * we've done aggregation (fd->agg_comm has a non-nulll communicator ),
     * then close the file.  Then, if any process left has done independent
     * i/o, close the file.  Otherwise, we'll skip the fs-specific close and
     * just say everything is a-ok.
     *
     * XXX: is it ok for those processes with a "real" communicator and those
     * with "MPI_COMM_SELF" to both call ADIOI_xxx_Close at the same time ?
     * everyone who ever opened the file will close it. Is order important? Is
     * timing important?
     */
    if (fd->agg_comm != MPI_COMM_NULL) {
	    (*(fd->fns->ADIOI_xxx_Close))(fd, error_code);
    } else {
	    if(fd->is_open)  {
		    (*(fd->fns->ADIOI_xxx_Close))(fd, error_code);
	    } else {
		    *error_code = MPI_SUCCESS;
	    }
	    
    }

    if (fd->access_mode & ADIO_DELETE_ON_CLOSE) {
	/* if we are doing aggregation and deferred open, then it's possible
	 * that rank 0 does not have access to the file. make sure only an
	 * aggregator deletes the file.*/
	if (fd->agg_comm != MPI_COMM_NULL ) {
		MPI_Comm_rank(fd->agg_comm, &myrank);
		MPI_Barrier(fd->agg_comm);
                if (!myrank) ADIO_Delete(fd->filename, &err);
	} else {
		MPI_Comm_rank(fd->comm, &myrank);
		MPI_Barrier(fd->comm);
                if (!myrank) ADIO_Delete(fd->filename, &err);
	}
    }

    ADIOI_Free(fd->hints);
    ADIOI_Free(fd->fns);
    MPI_Comm_free(&(fd->comm));
    /* deferred open: if we created an aggregator communicator, free it */
    if (fd->agg_comm != MPI_COMM_NULL) {
	    MPI_Comm_free(&(fd->agg_comm));
    }
    free(fd->filename);  /* should not use ADIOI_Free here, because
                            it was strdup'ed */

    MPI_Type_get_envelope(fd->etype, &i, &j, &k, &combiner);
    if (combiner != MPI_COMBINER_NAMED) MPI_Type_free(&(fd->etype));

    ADIOI_Datatype_iscontig(fd->filetype, &is_contig);
    if (!is_contig) ADIOI_Delete_flattened(fd->filetype);

    MPI_Type_get_envelope(fd->filetype, &i, &j, &k, &combiner);
    if (combiner != MPI_COMBINER_NAMED) MPI_Type_free(&(fd->filetype));
    
    MPI_Info_free(&(fd->info));

    ADIOI_Free(fd);
}
