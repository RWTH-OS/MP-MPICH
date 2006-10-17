/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *   $Id: ad_pvfs2_delete.c,v 1.1.1.1 2005/03/10 16:07:12 mhs Exp $    
 *
 *   Copyright (C) 2003 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_pvfs2.h"
#include "adio.h"

#include "ad_pvfs2_common.h"

void ADIOI_PVFS2_Delete(char *filename, int *error_code)
{
    PVFS_credentials credentials;
    PVFS_sysresp_getparent resp_getparent;
    int ret;
    PVFS_fs_id cur_fs;
    char pvfs_path[PVFS_NAME_MAX] = {0};

    ADIOI_PVFS2_Init(error_code);
    if (*error_code != MPI_SUCCESS) 
    {
	ADIOI_PVFS2_pvfs_error_convert(0, error_code);
	return;
    }

    /* in most cases we'll store the credentials in the fs struct, but we don't
     * have one of those in Delete  */
    ADIOI_PVFS2_makecredentials(&credentials);

    /* given the filename, figure out which pvfs filesystem it is on */
    ret = PVFS_util_resolve(filename, &cur_fs, pvfs_path, PVFS_NAME_MAX);
    if (ret < 0) {
	PVFS_perror("PVFS_util_resolve", ret);
	/* TODO: pick a good error for this */
	ret = -1;
	goto resolve_error;
    }
    ret = PVFS_sys_getparent(cur_fs, pvfs_path, &credentials, &resp_getparent);

    ret = PVFS_sys_remove(resp_getparent.basename, 
	    resp_getparent.parent_ref, &credentials);
    if (ret < 0) {
	/* XXX: better error handling */
	ADIOI_PVFS2_pvfs_error_convert(ret, error_code);
	return;
    }
    *error_code = MPI_SUCCESS;
    return;

resolve_error:
    ADIOI_PVFS2_pvfs_error_convert(ret, error_code);
}

/* 
 * vim: ts=8 sts=4 sw=4 noexpandtab 
 */
