/**************************************************************************
* MEMFS                                                                   *
***************************************************************************
*                                                                         *
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              *
*                                                                         *
* See COPYRIGHT notice in base directory for details                      *
**************************************************************************/
/**************************************************************************
* File:         ad_memfs_files.c                                          *
* Description:  This is the "memfs filesystem"                            *
* Author(s):    Jan Seidel <Jan.Seidel@fh-bonn-rhein-sieg.de>             *
* Last change:                                                            *
*                                                                         *
* Changelog:                                                              *
**************************************************************************/

/*==========================================================================================================*/
/* INCLUDES */

#include "ad_memfs_functions.h"

/*==========================================================================================================*/
/* DEFINES */

/*==========================================================================================================*/
/* TYPEDEFS */

/*==========================================================================================================*/
/* VARIABLES */

/*==========================================================================================================*/
/* STATIC FUNCTIONS */

/*==========================================================================================================*/
/* NON-STATIC FUNCTIONS */


/*
     Open a file via the memfs filesystem
*/
int open_file(char *filename, int access_mode, int *error, int64_t blocksize, int pos)
{
    int fd_sys;

#ifdef DEBUG_MEMFS
    int comm_rank;
    fprintf(stderr, "ADIOI_MEMFS_Open called\n");

    MPI_Comm_rank(MPI_COMM_META_REDUCED, &comm_rank);
    fprintf(stderr, "memfs_main: [%d] opening file: %s with blocksize: %lld, access_mode: %d\n", comm_rank, filename, blocksize, access_mode);
#endif

    fd_sys = memfs_open(filename, access_mode, blocksize, pos);

    if(sizeof(FDTYPE) != sizeof(int)) fprintf(stderr, "ERROR! Unrecognized FDTYPE in ADIOI_MEMFS_Open\n");

    if (fd_sys == -1) {
        LOCK_MPI();
        *error = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR, filename, "I/O Error", "%s", strerror(errno));
        ADIOI_Error(ADIO_FILE_NULL, *error, filename);
        UNLOCK_MPI();
    } else {
        *error = MPI_SUCCESS;
    }

#ifdef DEBUG_MEMFS
    fprintf(stderr, "ADIO_MEMFS_Open finished: fd->fd_sys: %d for file %s. Accessmode: %d\n", fd_sys, filename, access_mode);
#endif
    return fd_sys;
}

/*
    Close a file in the memfs filesystem
*/
void close_file(int fd_sys, int *error)
{
    int err;
#if defined(MPICH2) || !defined(PRINT_ERR_MSG)
    static char myname[] = "ADIOI_MEMFS_CLOSE";
#endif

#ifdef DEBUG_MEMFS
    fprintf(stderr, "ADIOI_MEMFS_Close called.\n");
#endif

    err = memfs_close(fd_sys);
/*
    fprintf(stderr, "Closing file: %s\n", fd->filename);
*/
#ifdef DEBUG_MEMFS
    fprintf(stderr, "ADIOI_MEMFS_Close finished\n");
#endif

    if (err == -1) {
#ifdef MPICH2
        LOCK_MPI();
        *error = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_IO, "**io",
            "**io %s", strerror(errno));
        UNLOCK_MPI();
#elif defined(PRINT_ERR_MSG)
                        *error = MPI_ERR_UNKNOWN;
#else
        LOCK_MPI();
        *error = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
                              myname, "I/O Error", "%s", strerror(errno));
/* TODO: ERROR HANDLING ! */
/*        ADIOI_Error(fd, *error, myname); */
        UNLOCK_MPI();

#endif
    }
    else *error = MPI_SUCCESS;

    return;
}

/*
    Read contiguous from a file in the memfs filesystem
*/
void read_file_contig(ADIO_File fd, void *buf, int count,
               MPI_Datatype datatype, int file_ptr_type,
               ADIO_Offset offset, ADIO_Status *status, int *error)
{
    int err= -1;
    int datatype_size, len;
    int mutex_error;
    double t1, t2;

#if defined(MPICH2) || !defined(PRINT_ERR_MSG)
    static char myname[] = "ADIOI_MEMFS_READCONTIG";
#endif



#ifdef DEBUG_MEMFS
    fprintf(stderr, "ADIOI_MEMFS_ReadContig called\n");
#endif

    LOCK_MPI();
    MPI_Type_size(datatype, &datatype_size);
    UNLOCK_MPI();
    len = datatype_size * count;

#if 0
    fprintf(stdout, "ADIOI_MEMFS_ReadContig: fd->fd_sys: %d. Reading %d Elements\n", fd->fd_sys, count);
    fprintf(stdout, "fd->fp_ind: %lld\n", fd->fp_ind);
    fprintf(stdout, "fd->fp_sys_posn: %lld\n", fd->fp_sys_posn);
    fprintf(stdout, "fd->disp: %lld\n", fd->disp);
    fprintf(stdout, "offset: %lld\n", offset);
#endif


    if (file_ptr_type == ADIO_EXPLICIT_OFFSET) {
        #ifdef MEMFS_TIME
        t1 = gettime();
        #endif
        mutex_error = pthread_mutex_lock(&filesystem_mutex);
        #ifdef MEMFS_TIME
        t2 = gettime();
        settime(FS_LOCK, t2-t1);
        #endif
        err = memfs_read(fd->fd_sys, offset, buf, len);
        pthread_mutex_unlock(&filesystem_mutex);
        fd->fp_sys_posn = offset + err;
    } else { /* read from current location of individual file pointer */
        fprintf(stderr, "\n\nReading from individual file pointer\n\n");
        offset = fd->fp_ind;
        err = memfs_read(fd->fd_sys, offset, buf, len);
        fd->fp_ind += err;
        fd->fp_sys_posn = fd->fp_ind;
   }


#ifdef DEBUG_MEMFS
    fprintf(stderr, "ADIOI_MEMFS_ReadContig finished\n");
#endif

    if (err == -1) {
#ifdef MPICH2
        LOCK_MPI();
        *error = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_IO, "**io",
            "**io %s", strerror(errno));
        UNLOCK_MPI();
#elif defined(PRINT_ERR_MSG)
                        *error = MPI_ERR_UNKNOWN;
#else /* MPICH-1 */
        LOCK_MPI();
        *error = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
                              myname, "I/O Error", "%s", strerror(errno));
        ADIOI_Error(fd, *error, myname);
        UNLOCK_MPI();
#endif
    }
    else *error = MPI_SUCCESS;

    return;
}

/*
    read strided from a file in the memfs filesystem
*/
void read_file_strided(ADIO_File fd, void *buf, int count,
               MPI_Datatype datatype, int file_ptr_type,
               ADIO_Offset offset, ADIO_Status *status, int *error)
{
    fprintf(stderr, "\n\n\nUsing read_file_strided\n\n\n\n");
    ADIOI_GEN_ReadStrided(fd, buf, count, datatype, file_ptr_type,
        offset, status, error);
}

/*
    Write contiguous to a file in the memfs filesystem
*/
void write_file_contig(ADIO_File fd, void *buf, int count,
               MPI_Datatype datatype, int file_ptr_type,
               ADIO_Offset offset, ADIO_Status *status, int *error)
{
    int err, datatype_size, len;
    int mutex_error;
    double t1,t2;
#if defined(MPICH2) || !defined(PRINT_ERR_MSG)
    static char myname[] = "ADIOI_MEMFS_WRITECONTIG";
#endif

    LOCK_MPI();
    MPI_Type_size(datatype, &datatype_size);
    len = datatype_size * count;
    UNLOCK_MPI();

#ifdef DEBUG_MEMFS
    fprintf(stderr, "ADIOI_MEMFS_WriteContig called\n");
#endif


#if 0
    fprintf(stdout, "ADIOI_MEMFS_WriteContig: fd->fd_sys: %d\n", fd->fd_sys);
    fprintf(stdout, "fd->fp_disp: %d\n", fd->disp);
    fprintf(stdout, "offset: %lld\n", offset);
    fprintf(stdout, "fd->fp_sys_posn: %lld\n", fd->fp_sys_posn);

    fprintf(stdout, "ADIOI_MEMFS_WRITE: count: %d\n", count);
#endif

    if (file_ptr_type == ADIO_EXPLICIT_OFFSET) {
        #ifdef MEMFS_TIME
        t1 = gettime();
        #endif
        mutex_error = pthread_mutex_lock(&filesystem_mutex);
        #ifdef MEMFS_TIME
        t2 = gettime();
        settime(FS_LOCK, t2-t1);
        #endif
        err = memfs_write(fd->fd_sys, offset, buf, len);
        pthread_mutex_unlock(&filesystem_mutex);
        fd->fp_sys_posn = offset + err;
    } else { /* write from current location of individual file pointer */
        fprintf(stderr, "\n\nUsing individual file pointer in write_file_contig\n\n");
        offset = fd->fp_ind;
        err = memfs_write(fd->fd_sys, offset, buf, len);
        fd->fp_ind += err;
        fd->fp_sys_posn = fd->fp_ind;
#ifdef DEBUG_MEMFS
        fprintf(stderr, "ADIOI_MEMFS_WriteContig: Writing without ADIO_EXPLICIT_OFFSET\n");
#endif
    }
    if (err == -1) {
#ifdef MPICH2
        LOCK_MPI();
        *error = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_IO, "**io",
            "**io %s", strerror(errno));
        UNLOCK_MPI();
#elif defined(PRINT_ERR_MSG)
                        *error = MPI_ERR_UNKNOWN;
#else
        LOCK_MPI();
        *error = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
                              myname, "I/O Error", "%s", strerror(errno));
        ADIOI_Error(fd, *error, myname);
        UNLOCK_MPI();
#endif
    }
    else *error = MPI_SUCCESS;

#ifdef DEBUG_MEMFS
    fprintf(stderr, "ADIOI_MEMFS_WriteContig finished\n");
#endif

    return;
}

/*
    Write strided to a file in the memfs filesystem
*/
void write_file_strided(ADIO_File fd, void *buf, int count,
               MPI_Datatype datatype, int file_ptr_type,
               ADIO_Offset offset, ADIO_Status *status, int *error)
{
    fprintf(stderr, "ADIOI_MEMFS_WriteStrided: Calling ADIOI_GEN_WriteStrided\n");
    ADIOI_GEN_WriteStrided(fd, buf, count, datatype, file_ptr_type,
                        offset, status, error);
}


/*
    resize a file in the memfs filesystem
*/
void resize_file(ADIO_File fd, ADIO_Offset size, int *error) {
    int err;
#if defined(MPICH2) || !defined(PRINT_ERR_MSG)
    static char myname[] = "ADIOI_MEMFS_RESIZE";
#endif

#ifdef DEBUG_MEMFS
    fprintf(stderr, "ADIOI_MEMFS_Resize called\n");
#endif

    err = memfs_resize(fd->fd_sys, size);

    if (err == -1) {
#ifdef MPICH2
        LOCK_MPI();
        *error = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_IO, "**io",
            "**io %s", strerror(errno));
        UNLOCK_MPI();
#elif defined(PRINT_ERR_MSG)
                        *error = MPI_ERR_UNKNOWN;
#else /* MPICH-1 */
        LOCK_MPI();
        *error = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
                              myname, "I/O Error", "%s", strerror(errno));
        ADIOI_Error(fd, *error, myname);
        UNLOCK_MPI();
#endif
    }
    else *error = MPI_SUCCESS;

    return;
}


/*
    delete a file in the memfs filesystem
*/
void delete_file(char *filename, int *error) {
    int err;

#ifdef DEBUG_MEMFS
    fprintf(stderr, "ADIOI_MEMFS_Delete called!\n");
#endif

    err = memfs_del(filename);

#ifdef DEBUG_MEMFS
    fprintf(stderr, "ADIOI_MEMFS_Delete finished\n");
#endif

    if (err == -1) {
      LOCK_MPI();
      *error = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR, filename, "I/O Error ADIOI_MEMFS_Delete", "%s", strerror(errno));
      ADIOI_Error(ADIO_FILE_NULL, *error, filename);
      UNLOCK_MPI();
    } else {
      *error = MPI_SUCCESS;
    }

    return;
}


/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *   $Id: ad_memfs_fcntl.c,v 1.15 2004/07/27 20:44:04 thakur Exp $
 *
 *   Copyright (C) 1997 University of Chicago.
 *   See COPYRIGHT notice in top-level directory.
 *
 *   Modified for memfs filesystem by Jan Seidel
 */

void fcntl_file(ADIO_File fd, int flag, ADIO_Fcntl_t *fcntl_struct, int *error_code)
{
    int i, ntimes;
    ADIO_Offset curr_fsize, alloc_size, size, len, done;
    ADIO_Status status;
    char *buf;
#if defined(MPICH2) || !defined(PRINT_ERR_MSG)
    static char myname[] = "ADIOI_MEMFS_FCNTL";
#endif

#ifdef DEBUG_MEMFS
    fprintf(stderr, "ADIOI_MEMFS_Fcntl called\n");
#endif

    switch(flag) {
    case ADIO_FCNTL_GET_FSIZE:
        fcntl_struct->fsize = memfs_getfilesize(fd->fd_sys);
        if (fcntl_struct->fsize == -1) {
#ifdef MPICH2
            LOCK_MPI();
            *error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_IO, "**io",
                "**io %s", strerror(errno));
            UNLOCK_MPI();
#elif defined(PRINT_ERR_MSG)
                        *error_code = MPI_ERR_UNKNOWN;
#else /* MPICH-1 */
            LOCK_MPI();
            *error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
                              myname, "I/O Error", "%s", strerror(errno));
            ADIOI_Error(fd, *error_code, myname);
            UNLOCK_MPI();
#endif
        }
        else *error_code = MPI_SUCCESS;
        break;

    case ADIO_FCNTL_SET_DISKSPACE:
        /* will be called by one process only */
        /* On file systems with no preallocation function, I have to
           explicitly write
           to allocate space. Since there could be holes in the file,
           I need to read up to the current file size, write it back,
           and then write beyond that depending on how much
           preallocation is needed.
           read/write in sizes of no more than ADIOI_PREALLOC_BUFSZ */

        curr_fsize = memfs_getfilesize(fd->fd_sys);
        alloc_size = fcntl_struct->diskspace;

        size = ADIOI_MIN(curr_fsize, alloc_size);

        ntimes = (size + ADIOI_PREALLOC_BUFSZ - 1)/ADIOI_PREALLOC_BUFSZ;
        buf = (char *) ADIOI_Malloc(ADIOI_PREALLOC_BUFSZ);
        done = 0;

        for (i=0; i<ntimes; i++) {
            len = ADIOI_MIN(size-done, ADIOI_PREALLOC_BUFSZ);
            ADIO_ReadContig(fd, buf, len, MPI_BYTE, ADIO_EXPLICIT_OFFSET, done,
                            &status, error_code);
            if (*error_code != MPI_SUCCESS) {
#ifdef MPICH2
                LOCK_MPI();
                *error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_IO, "**io",
                    "**io %s", strerror(errno));
                UNLOCK_MPI();
#elif defined(PRINT_ERR_MSG)
                FPRINTF(stderr, "ADIOI_MEMFS_Fcntl: To preallocate disk space, ROMIO needs to read the file and write it back, but is unable to read the file. Please give the file read permission and open it with MPI_MODE_RDWR.\n");
                LOCK_MPI();
                MPI_Abort(MPI_COMM_WORLD, 1);
                UNLOCK_MPI();
#else /* MPICH-1 */
                LOCK_MPI();
                *error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_PREALLOC_PERM,
                              myname, (char *) 0, (char *) 0);
                ADIOI_Error(fd, *error_code, myname);
                UNLOCK_MPI();
#endif
                return;
            }
            ADIO_WriteContig(fd, buf, len, MPI_BYTE, ADIO_EXPLICIT_OFFSET,
                             done, &status, error_code);
            if (*error_code != MPI_SUCCESS) return;
            done += len;
        }

        if (alloc_size > curr_fsize) {
            memset(buf, 0, ADIOI_PREALLOC_BUFSZ);
            size = alloc_size - curr_fsize;
            ntimes = (size + ADIOI_PREALLOC_BUFSZ - 1)/ADIOI_PREALLOC_BUFSZ;
            for (i=0; i<ntimes; i++) {
                len = ADIOI_MIN(alloc_size-done, ADIOI_PREALLOC_BUFSZ);
                ADIO_WriteContig(fd, buf, len, MPI_BYTE, ADIO_EXPLICIT_OFFSET,
                                 done, &status, error_code);
                if (*error_code != MPI_SUCCESS) return;
                done += len;
            }
        }
        ADIOI_Free(buf);
        *error_code = MPI_SUCCESS;
        break;

    case ADIO_FCNTL_SET_IOMODE:
        /* for implementing PFS I/O modes. will not occur in MPI-IO
           implementation.*/
        if (fd->iomode != fcntl_struct->iomode) {
            fd->iomode = fcntl_struct->iomode;
            LOCK_MPI();
            MPI_Barrier(MPI_COMM_WORLD);
            UNLOCK_MPI();
        }
        *error_code = MPI_SUCCESS;
        break;

    case ADIO_FCNTL_SET_ATOMICITY:
        fd->atomicity = (fcntl_struct->atomicity == 0) ? 0 : 1;
        *error_code = MPI_SUCCESS;
        break;

    default:
        FPRINTF(stderr, "Unknown flag passed to ADIOI_MEMFS_Fcntl\n");
        /*
        MPI_Abort(MPI_COMM_WORLD, 1);
        */
    }
}

/*
    seek the individual file pointer. Uses the ADIOI generic function
*/
ADIO_Offset seekind_file(ADIO_File fd, ADIO_Offset offset,
                      int whence, int *error_code)
{
#ifdef DEBUG_MEMFS
    printf("ADIOI_MEMFS_SeekIndividual: calling ADIOI_GEN_SeekIndividual\n");
#endif
    return ADIOI_GEN_SeekIndividual(fd, offset, whence, error_code);
}
/*
    Set the info object of filehandle fd. Supports MEMFS blocksize parameter
*/
void setinfo_file(ADIO_File fd, MPI_Info users_info, int *error_code)
{
    char *value;
    int flag;

#ifdef DEBUG_MEMFS
    fprintf(stdout, "ADIOI_MEMFS_SetInfo called\n");
#endif
    ADIOI_GEN_SetInfo(fd, users_info, error_code);


    if(users_info != MPI_INFO_NULL) {
        value = (char *)malloc(MPI_MAX_INFO_VAL+1 * sizeof(char));
        LOCK_MPI();
        MPI_Info_get(users_info, "blocksize", MPI_MAX_INFO_VAL, value, &flag);
        if(flag)
            MPI_Info_set(fd->info, "blocksize", value);
        UNLOCK_MPI();
        free(value);
    }

/* Test if fd->info is set correctly */
/*
        value = (char *)malloc(MPI_MAX_INFO_VAL+1 * sizeof(char));
        MPI_Info_get(fd->info, "blocksize", MPI_MAX_INFO_VAL, value, &flag);
        if(flag)
            blocksize = atoi(value);
        free(value);


    fprintf(stderr, "Blocksize is set to %d\n", blocksize);
*/

}


