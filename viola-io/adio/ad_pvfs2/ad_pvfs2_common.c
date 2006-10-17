/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *   $Id: ad_pvfs2_common.c,v 1.1.1.1 2005/03/10 16:07:12 mhs Exp $
 *
 *   Copyright (C) 2003 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_pvfs2.h"
#include "ad_pvfs2_common.h"
#include <unistd.h>
#include <sys/types.h>

/* maybe give romio access to the globalconfig struct */
/* keyval hack to both tell us if we've already initialized pvfs2 and also
 * close it down when mpi exits */
int ADIOI_PVFS2_Initialized = MPI_KEYVAL_INVALID;

void ADIOI_PVFS2_End(int *error_code)
{
    int ret;
    ret = PVFS_sys_finalize();
    if (ret < 0 ) {
	ADIOI_PVFS2_pvfs_error_convert(ret, error_code);
    } else {
	*error_code = MPI_SUCCESS;
    }
}

int ADIOI_PVFS2_End_call(MPI_Comm comm, int keyval, 
	void *attribute_val, void *extra_state)
{
    int error_code;
    ADIOI_PVFS2_End(&error_code);
    return error_code;
}

void ADIOI_PVFS2_Init(int *error_code )
{
	int ret;

	/* do nothing if we've already fired up the pvfs2 interface */
	if (ADIOI_PVFS2_Initialized != MPI_KEYVAL_INVALID) {
		*error_code = MPI_SUCCESS;
		return;
	}

	ret = PVFS_util_init_defaults();
	if (ret < 0 ) {
	    /* XXX: better error handling */
	    PVFS_perror("PVFS_util_init_defaults", ret);
	    ADIOI_PVFS2_pvfs_error_convert(ret, error_code);
	    return;
	}

	MPI_Keyval_create(MPI_NULL_COPY_FN, ADIOI_PVFS2_End_call,
		&ADIOI_PVFS2_Initialized, (void *)0); 
	/* just like romio does, we make a dummy attribute so we 
	 * get cleaned up */
	MPI_Attr_put(MPI_COMM_WORLD, ADIOI_PVFS2_Initialized, (void *)0);
}

void ADIOI_PVFS2_makeattribs(PVFS_sys_attr * attribs)
{
    memset(attribs, 0, sizeof(PVFS_sys_attr));

    attribs->owner = geteuid();
    attribs->group = getegid();
    attribs->perms = 1877;
    attribs->mask =  PVFS_ATTR_SYS_ALL_SETABLE;
}


void ADIOI_PVFS2_makecredentials(PVFS_credentials * credentials)
{
    memset(credentials, 0, sizeof(PVFS_credentials));

    PVFS_util_gen_credentials(credentials);
}

/* pvfs_error_convert: given a pvfs error code, make it into the appropriate
 * mpi error code */ 
int ADIOI_PVFS2_pvfs_error_convert(int pvfs_error, int *mpi_error)
{
    *mpi_error = MPI_UNDEFINED;
    return 0;
}
/* 
 * vim: ts=8 sts=4 sw=4 noexpandtab 
 */
