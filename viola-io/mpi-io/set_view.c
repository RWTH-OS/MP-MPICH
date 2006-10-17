/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *   $Id: set_view.c,v 1.1.1.1 2005/03/10 16:07:13 mhs Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpioimpl.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_File_set_view = PMPI_File_set_view
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_File_set_view MPI_File_set_view
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_File_set_view as PMPI_File_set_view
/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#define MPIO_BUILD_PROFILING
#include "mpioprof.h"
#endif

/*@
    MPI_File_set_view - Sets the file view

Input Parameters:
. fh - file handle (handle)
. disp - displacement (nonnegative integer)
. etype - elementary datatype (handle)
. filetype - filetype (handle)
. datarep - data representation (string)
. info - info object (handle)

.N fortran
@*/
int MPI_File_set_view(MPI_File fh, MPI_Offset disp, MPI_Datatype etype,
		 MPI_Datatype filetype, char *datarep, MPI_Info info)
{
    int filetype_size, etype_size, error_code;
#if defined(MPICH2) || !defined(PRINT_ERR_MSG)
    static char myname[] = "MPI_FILE_SET_VIEW";
#endif
    ADIO_Offset shared_fp, byte_off;

#ifdef PRINT_ERR_MSG
    if ((fh <= (MPI_File) 0) || (fh->cookie != ADIOI_FILE_COOKIE)) {
	FPRINTF(stderr, "MPI_File_set_view: Invalid file handle\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
    }
#else
    ADIOI_TEST_FILE_HANDLE(fh, myname);
#endif

    if ((disp < 0) && (disp != MPI_DISPLACEMENT_CURRENT)) {
#ifdef MPICH2
	error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_ARG, 
	    "**iobaddisp", 0);
	return MPIR_Err_return_file(fh, myname, error_code);
#elif defined(PRINT_ERR_MSG)
	FPRINTF(stderr, "MPI_File_set_view: Invalid disp argument\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else /* MPICH-1 */
	error_code = MPIR_Err_setmsg(MPI_ERR_ARG, MPIR_ERR_DISP_ARG,
				     myname, (char *) 0, (char *) 0);
	return ADIOI_Error(fh, error_code, myname);	    
#endif
    }

    /* rudimentary checks for incorrect etype/filetype.*/
    if (etype == MPI_DATATYPE_NULL) {
#ifdef MPICH2
	error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_ARG, "**ioetype", 0);
	return MPIR_Err_return_file(fh, myname, error_code);
#elif defined(PRINT_ERR_MSG)
	FPRINTF(stderr, "MPI_File_set_view: Invalid etype\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else /* MPICH-1 */
	error_code = MPIR_Err_setmsg(MPI_ERR_ARG, MPIR_ERR_ETYPE_ARG,
				     myname, (char *) 0, (char *) 0);
	return ADIOI_Error(fh, error_code, myname);	    
#endif
    }

    if (filetype == MPI_DATATYPE_NULL) {
#ifdef MPICH2
	error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_ARG, "**iofiletype", 0);
	return MPIR_Err_return_file(fh, myname, error_code);
#elif defined(PRINT_ERR_MSG)
	FPRINTF(stderr, "MPI_File_set_view: Invalid filetype\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else /* MPICH-1 */
	error_code = MPIR_Err_setmsg(MPI_ERR_ARG, MPIR_ERR_ETYPE_ARG,
				     myname, (char *) 0, (char *) 0);
	return ADIOI_Error(fh, error_code, myname);	    
#endif
    }

    if ((fh->access_mode & MPI_MODE_SEQUENTIAL) && (disp != MPI_DISPLACEMENT_CURRENT)) {
#ifdef MPICH2
	error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_ARG, 
			"**iodispifseq", 0);
	return MPIR_Err_return_file(fh, myname, error_code);
#elif defined(PRINT_ERR_MSG)
        FPRINTF(stderr, "MPI_File_set_view: disp must be set to MPI_DISPLACEMENT_CURRENT since file was opened with MPI_MODE_SEQUENTIAL\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
#else /* MPICH-1 */
	error_code = MPIR_Err_setmsg(MPI_ERR_ARG, 1,
				     myname, (char *) 0, "%s", "displacement must be set to MPI_DISPLACEMENT_CURRENT since file was opened with MPI_MODE_SEQUENTIAL");
	return ADIOI_Error(fh, error_code, myname);	    
#endif
    }

    if ((disp == MPI_DISPLACEMENT_CURRENT) && !(fh->access_mode & MPI_MODE_SEQUENTIAL)) {
#ifdef MPICH2
	error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_ARG, 
			"**iodispifseq", 0);
	return MPIR_Err_return_file(fh, myname, error_code);
#elif defined(PRINT_ERR_MSG)
        FPRINTF(stderr, "MPI_File_set_view: disp can be set to MPI_DISPLACEMENT_CURRENT only if file was opened with MPI_MODE_SEQUENTIAL\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
#else /* MPICH-1 */
	error_code = MPIR_Err_setmsg(MPI_ERR_ARG, 1,
				     myname, (char *) 0, "%s", "displacement can be set to MPI_DISPLACEMENT_CURRENT only if file was opened with MPI_MODE_SEQUENTIAL");
	return ADIOI_Error(fh, error_code, myname);	    
#endif
    }

    MPI_Type_size(filetype, &filetype_size);
    MPI_Type_size(etype, &etype_size);
    if (filetype_size % etype_size != 0) {
#ifdef MPICH2
	error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_ARG, "**iofiletype", 0);
	return MPIR_Err_return_file(fh, myname, error_code);
#elif defined(PRINT_ERR_MSG)
	FPRINTF(stderr, "MPI_File_set_view: Filetype must be constructed out of one or more etypes\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else /* MPICH-1 */
	error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ERR_FILETYPE,
				     myname, (char *) 0, (char *) 0);
	return ADIOI_Error(fh, error_code, myname);	    
#endif
    }

    if (strcmp(datarep, "native") && strcmp(datarep, "NATIVE")) {
#ifdef MPICH2
	error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_UNSUPPORTED_DATAREP, 
	    "**unsupporteddatarep",0);
	return MPIR_Err_return_file(fh, myname, error_code);
#elif defined(PRINT_ERR_MSG)
	FPRINTF(stderr, "MPI_File_set_view: Only \"native\" data representation currently supported\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else /* MPICH-1 */
	error_code = MPIR_Err_setmsg(MPI_ERR_UNSUPPORTED_DATAREP, 
               MPIR_ERR_NOT_NATIVE_DATAREP, myname, (char *) 0, (char *) 0);
	return ADIOI_Error(fh, error_code, myname);	    
#endif
    }


    if ((disp == MPI_DISPLACEMENT_CURRENT) && 
        (fh->file_system != ADIO_TUNNELFS)) {
	MPI_Barrier(fh->comm);
	ADIO_Get_shared_fp(fh, 0, &shared_fp, &error_code);
	MPI_Barrier(fh->comm); 
	ADIOI_Get_byte_offset(fh, shared_fp, &byte_off);
	disp = byte_off;
    }

    ADIO_Set_view(fh, disp, etype, filetype, info, &error_code);

    /* reset shared file pointer to zero */

    if ((fh->file_system != ADIO_PIOFS) && (fh->file_system != ADIO_PVFS) &&
		    (fh->file_system != ADIO_PVFS2) && 
            (fh->file_system != ADIO_TUNNELFS) &&
        (fh->shared_fp_fd != ADIO_FILE_NULL)) 
	ADIO_Set_shared_fp(fh, 0, &error_code);
    /* only one process needs to set it to zero, but I don't want to 
       create the shared-file-pointer file if shared file pointers have 
       not been used so far. Therefore, every process that has already 
       opened the shared-file-pointer file sets the shared file pointer 
       to zero. If the file was not opened, the value is automatically 
       zero. Note that shared file pointer is stored as no. of etypes
       relative to the current view, whereas indiv. file pointer is
       stored in bytes. */

    if ((fh->file_system != ADIO_PIOFS) && (fh->file_system != ADIO_PVFS) &&
		    (fh->file_system != ADIO_PVFS2 ) && 
            (fh->file_system != ADIO_TUNNELFS))
	MPI_Barrier(fh->comm); /* for above to work correctly */

    return error_code;
}
