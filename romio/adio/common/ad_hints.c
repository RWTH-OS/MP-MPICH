/* 
 *   $Id: ad_hints.c 99 2000-05-04 15:55:51Z karsten $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "adio.h"

void ADIOI_GEN_SetInfo(ADIO_File fd, MPI_Info users_info, int *error_code)
{
/* if fd->info is null, create a new info object. 
   Initialize fd->info to default values.
   Examine the info object passed by the user. If it contains values that
   ROMIO understands, override the default. */

    MPI_Info info;
    char *value;
    int flag, intval, tmp_val, nprocs;

    if (!(fd->info)) MPI_Info_create(&(fd->info));
    info = fd->info;

    /* initialize to default values */

    value = (char *) ADIOI_Malloc((MPI_MAX_INFO_VAL+1)*sizeof(char));

    /* buffer size for collective I/O */
    MPI_Info_set(info, "cb_buffer_size", ADIOI_CB_BUFFER_SIZE_DFLT); 

    /* number of processes that perform I/O in collective I/O */
    MPI_Comm_size(fd->comm, &nprocs);
    sprintf(value, "%d", nprocs);
    MPI_Info_set(info, "cb_nodes", value);

    /* buffer size for data sieving in independent reads */
    MPI_Info_set(info, "ind_rd_buffer_size", ADIOI_IND_RD_BUFFER_SIZE_DFLT);
    /* buffer size for data sieving in independent writes */
    MPI_Info_set(info, "ind_wr_buffer_size", ADIOI_IND_WR_BUFFER_SIZE_DFLT);

    /* check users info */
    if (users_info != MPI_INFO_NULL) {
	MPI_Info_get(users_info, "cb_buffer_size", MPI_MAX_INFO_VAL, 
		     value, &flag);
	if (flag && ((intval=atoi(value)) > 0)) {
	    tmp_val = intval;
	    MPI_Bcast(&tmp_val, 1, MPI_INT, 0, fd->comm);
	    if (tmp_val != intval) {
		FPRINTF(stderr, "ADIOI_GEN_SetInfo: the value for key \"cb_buffer_size\" must be the same on all processes\n");
		MPI_Abort(MPI_COMM_WORLD, 1);
	    }
	    else MPI_Info_set(info, "cb_buffer_size", value);
	}

	MPI_Info_get(users_info, "cb_nodes", MPI_MAX_INFO_VAL, 
		     value, &flag);
	if (flag && ((intval=atoi(value)) > 0)) {
	    tmp_val = intval;
	    MPI_Bcast(&tmp_val, 1, MPI_INT, 0, fd->comm);
	    if (tmp_val != intval) {
		FPRINTF(stderr, "ADIOI_GEN_SetInfo: the value for key \"cb_nodes\" must be the same on all processes\n");
		MPI_Abort(MPI_COMM_WORLD, 1);
	    }
	    else {
		if (intval < nprocs)
		    MPI_Info_set(info, "cb_nodes", value);
	    }
	}

	MPI_Info_get(users_info, "ind_wr_buffer_size", MPI_MAX_INFO_VAL, 
		     value, &flag);
	if (flag && (atoi(value) > 0))
	    MPI_Info_set(info, "ind_wr_buffer_size", value);

	MPI_Info_get(users_info, "ind_rd_buffer_size", MPI_MAX_INFO_VAL, 
		     value, &flag);
	if (flag && (atoi(value) > 0))
	    MPI_Info_set(info, "ind_rd_buffer_size", value);
    }

    ADIOI_Free(value);

    if ((fd->file_system == ADIO_PIOFS) || (fd->file_system == ADIO_PVFS))
	MPI_Info_delete(info, "ind_wr_buffer_size");
    /* no data sieving for writes in PIOFS and PVFS, because it doesn't
       support file locking */

    *error_code = MPI_SUCCESS;
}
