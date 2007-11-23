/* 
 *   $Id: ad_open.c 922 2001-05-31 14:10:48Z karsten $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "adio.h"
#include "adio_extern.h"
#ifdef MPISGI
#include "mpisgi2.h"
#endif

ADIO_File ADIO_Open(MPI_Comm comm, char *filename, int file_system,
		    int access_mode, ADIO_Offset disp, MPI_Datatype etype, 
		    MPI_Datatype filetype, int iomode,
                    MPI_Info info, int perm, int *error_code)
{
    ADIO_File fd;
    int orig_amode, err;

    fd = (ADIO_File) ADIOI_Malloc(sizeof(struct ADIOI_FileD));

    fd->cookie = ADIOI_FILE_COOKIE;
    fd->fp_ind = disp;
    fd->fp_sys_posn = 0;
    fd->comm = comm;       /* dup'ed in MPI_File_open */
    fd->filename = strdup(filename);
    fd->file_system = file_system;
    fd->disp = disp;
    fd->split_coll_count = 0;
    fd->shared_fp_fd = ADIO_FILE_NULL;
    fd->atomicity = 0;

    fd->etype = etype;          /* MPI_BYTE by default */
    fd->filetype = filetype;    /* MPI_BYTE by default */
    fd->etype_size = 1;  /* default etype is MPI_BYTE */

    fd->perm = perm;

    fd->iomode = iomode;
    fd->async_count = 0;

    fd->err_handler = ADIOI_DFLT_ERR_HANDLER;
#ifdef MPICH
    MPIR_Errhandler_mark(fd->err_handler,1);
#endif
/* set I/O function pointers */
    ADIOI_SetFunctions(fd);

/* create and initialize info object */
    fd->info = NULL;
    ADIO_SetInfo(fd, info, &err);

/* For writing with data sieving, a read-modify-write is needed. If 
   the file is opened for write_only, the read will fail. Therefore,
   if write_only, open the file as read_write, but record it as write_only
   in fd, so that get_amode returns the right answer. */

    orig_amode = access_mode;
    if (access_mode & ADIO_WRONLY) {
	access_mode = access_mode ^ ADIO_WRONLY;
	access_mode = access_mode | ADIO_RDWR;
    }
    fd->access_mode = access_mode;

    (*(fd->fns->ADIOI_xxx_Open))(fd, error_code);

    fd->access_mode = orig_amode;

    /* if error, may be it was due to the change in amode above. 
       therefore, reopen with access mode provided by the user.*/ 
    if (*error_code != MPI_SUCCESS) 
        (*(fd->fns->ADIOI_xxx_Open))(fd, error_code);

    /* if error, free and set fd to NULL */
    if (*error_code != MPI_SUCCESS) {
	ADIOI_Free(fd->fns);
	MPI_Comm_free(&(fd->comm));
	free(fd->filename);
	MPI_Info_free(&(fd->info));
	ADIOI_Free(fd);
	fd = ADIO_FILE_NULL;
    }

    return fd;
}
