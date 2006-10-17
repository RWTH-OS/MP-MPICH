/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*  $Id: adioi_error.h,v 1.1.1.1 2005/03/10 16:07:13 mhs Exp $
 *
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */
#ifdef MPICH2
#define ADIOI_TEST_FILE_HANDLE(fh, myname) \
{if (!(fh)) { \
	error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_FILE, "**iofilenull", 0);\
	return MPIR_Err_return_file(MPI_FILE_NULL, myname, error_code);}\
 else if ((fh)->cookie != ADIOI_FILE_COOKIE) { \
    error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_FILE, "**iofilecorrupt", 0);\
    return MPIR_Err_return_file(MPI_FILE_NULL, myname, error_code); } }
#else /* MPICH-1 */
#define ADIOI_TEST_FILE_HANDLE(fh, myname) \
{if (!(fh)) { \
    error_code = MPIR_Err_setmsg(MPI_ERR_FILE, MPIR_ERR_FILE_NULL, myname, (char *) 0, (char *) 0); \
    return ADIOI_Error(MPI_FILE_NULL, error_code, myname); } \
 else if ((fh)->cookie != ADIOI_FILE_COOKIE) { \
    error_code = MPIR_Err_setmsg(MPI_ERR_FILE, MPIR_ERR_FILE_CORRUPT, myname, (char *) 0, (char *) 0); \
    return ADIOI_Error(MPI_FILE_NULL, error_code, myname); } }
#endif


/* TODO: handle the independent io case more gracefully  */
#define ADIOI_TEST_DEFERRED(fh, myname, error_code)\
    if(! (fh)->is_open ) {\
	    ADIO_ImmediateOpen((fh), (error_code)); }
