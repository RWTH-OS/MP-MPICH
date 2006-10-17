/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *   $Id: ad_pvfs2_hints.c,v 1.1.1.1 2005/03/10 16:07:12 mhs Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_pvfs2.h"

void ADIOI_PVFS2_SetInfo(ADIO_File fd, MPI_Info users_info, int *error_code)
{
    char *value;
    int flag, tmp_mask;
    if ((fd->info) == MPI_INFO_NULL) {
	/* part of the open call */
	MPI_Info_create(&(fd->info));
	MPI_Info_set(fd->info, "romio_pvfs2_debugmask", "0");
	fd->hints->fs_hints.pvfs2.debugmask = 0;
	
	/* any user-provided hints? */
	if (users_info != MPI_INFO_NULL) {
	    value = (char *) ADIOI_Malloc( (MPI_MAX_INFO_VAL+1)*sizeof(char));
	    MPI_Info_get(users_info, "romio_pvfs2_debugmask", 
		    MPI_MAX_INFO_VAL, value, &flag);
	    if (flag) {
		tmp_mask = fd->hints->fs_hints.pvfs2.debugmask = 
		    PVFS_debug_eventlog_to_mask(value);
		MPI_Bcast(&tmp_mask, 1, MPI_INT, 0, fd->comm);
		if (tmp_mask != fd->hints->fs_hints.pvfs2.debugmask) {
		    FPRINTF(stderr, "ADIOI_PVFS_SetInfo: the value for key \"romio_pvfs2_debugmask\" must be the same on all processes\n");
		    MPI_Abort(MPI_COMM_WORLD, 1);
		}
		else MPI_Info_set(fd->info, "romio_pvfs2_debugmask", value);
	    }
	}
    }
    /* set the values for collective I/O and data sieving parameters */
    ADIOI_GEN_SetInfo(fd, users_info, error_code);

    *error_code = MPI_SUCCESS;
}

/*
 * vim: ts=8 sts=4 sw=4 noexpandtab
 */
