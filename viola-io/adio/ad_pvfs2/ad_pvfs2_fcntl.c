/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *   $Id: ad_pvfs2_fcntl.c,v 1.1.1.1 2005/03/10 16:07:12 mhs Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_pvfs2.h"
#include "adio_extern.h"
#include "ad_pvfs2_common.h"

void ADIOI_PVFS2_Fcntl(ADIO_File fd, int flag, ADIO_Fcntl_t *fcntl_struct, int *error_code)
{
    int ret;
    ADIOI_PVFS2_fs *pvfs_fs;
    PVFS_sysresp_getattr resp_getattr;

    pvfs_fs = (ADIOI_PVFS2_fs*)fd->fs_ptr;

    switch(flag) {

    case ADIO_FCNTL_GET_FSIZE:
	ret = PVFS_sys_getattr(pvfs_fs->object_ref, PVFS_ATTR_SYS_SIZE, 
		&(pvfs_fs->credentials), &resp_getattr);
	if (ret < 0 ) {
	    ADIOI_PVFS2_pvfs_error_convert(ret, error_code);
	} else {
	    *error_code = MPI_SUCCESS;
	}
	fcntl_struct->fsize = resp_getattr.attr.size;
	return;

    case ADIO_FCNTL_SET_DISKSPACE:
	*error_code = MPI_ERR_UNKNOWN;
	break;

    case ADIO_FCNTL_SET_IOMODE:
	/* a relic from PFS */
	if (fd->iomode != fcntl_struct->iomode) {
	    fd->iomode = fcntl_struct->iomode;
	    MPI_Barrier(MPI_COMM_WORLD);
	}
	*error_code = MPI_SUCCESS;
	break;

    case ADIO_FCNTL_SET_ATOMICITY:
	*error_code = MPI_ERR_UNKNOWN;
	break;
    default:
	FPRINTF(stderr, "Unknown flag passed to ADIOI_PVFS2_Fcntl\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
    }
}

/* 
 * vim: ts=8 sts=4 sw=4 noexpandtab 
 */
