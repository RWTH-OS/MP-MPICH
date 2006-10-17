/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *   $Id: ad_pvfs2_close.c,v 1.1.1.1 2005/03/10 16:07:12 mhs Exp $
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_pvfs2.h"

void ADIOI_PVFS2_Close(ADIO_File fd, int *error_code)
{
    ADIOI_Free(fd->fs_ptr);
    /* pvfs2 doesn't have a 'close', but MPI-IO semantics dictate that we
     * ensure all data has been flushed  */
    /* XXX: reduce and sync? sync on all? */

    *error_code = MPI_SUCCESS;
}
/* 
 * vim: ts=8 sts=4 sw=4 noexpandtab 
 */
