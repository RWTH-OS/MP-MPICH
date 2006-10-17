/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *   $Id: read_ord.c,v 1.1.1.1 2005/03/10 16:07:13 mhs Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpioimpl.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_File_read_ordered = PMPI_File_read_ordered
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_File_read_ordered MPI_File_read_ordered
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_File_read_ordered as PMPI_File_read_ordered
/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#define MPIO_BUILD_PROFILING
#include "mpioprof.h"
#endif

/* status object not filled currently */

/*@
    MPI_File_read_ordered - Collective read using shared file pointer

Input Parameters:
. fh - file handle (handle)
. count - number of elements in buffer (nonnegative integer)
. datatype - datatype of each buffer element (handle)

Output Parameters:
. buf - initial address of buffer (choice)
. status - status object (Status)

.N fortran
@*/
int MPI_File_read_ordered(MPI_File fh, void *buf, int count, 
                          MPI_Datatype datatype, MPI_Status *status)
{
    int error_code, datatype_size, nprocs, myrank, incr;
    int source, dest;
#if defined(MPICH2) || !defined(PRINT_ERR_MSG)
    static char myname[] = "MPI_FILE_READ_ORDERED";
#endif
    ADIO_Offset shared_fp;

#ifdef PRINT_ERR_MSG
    if ((fh <= (MPI_File) 0) || (fh->cookie != ADIOI_FILE_COOKIE)) {
	FPRINTF(stderr, "MPI_File_read_ordered: Invalid file handle\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
    }
#else
    ADIOI_TEST_FILE_HANDLE(fh, myname);
#endif

    if (count < 0) {
#ifdef MPICH2
	error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_ARG, 
	    "**iobadcount", 0);
	return MPIR_Err_return_file(fh, myname, error_code);
#elif defined(PRINT_ERR_MSG)
	FPRINTF(stderr, "MPI_File_read_ordered: Invalid count argument\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else /* MPICH-1 */
	error_code = MPIR_Err_setmsg(MPI_ERR_ARG, MPIR_ERR_COUNT_ARG,
				     myname, (char *) 0, (char *) 0);
	return ADIOI_Error(fh, error_code, myname);
#endif
    }

    if (datatype == MPI_DATATYPE_NULL) {
#ifdef MPICH2
	error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_TYPE, 
	    "**dtypenull", 0);
	return MPIR_Err_return_file(fh, myname, error_code);
#elif defined(PRINT_ERR_MSG)
        FPRINTF(stderr, "MPI_File_read_ordered: Invalid datatype\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
#else /* MPICH-1 */
	error_code = MPIR_Err_setmsg(MPI_ERR_TYPE, MPIR_ERR_TYPE_NULL,
				     myname, (char *) 0, (char *) 0);
	return ADIOI_Error(fh, error_code, myname);	    
#endif
    }

    MPI_Type_size(datatype, &datatype_size);
    if ((count*datatype_size) % fh->etype_size != 0) {
#ifdef MPICH2
	error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_IO, 
	    "**ioetype", 0);
	return MPIR_Err_return_file(fh, myname, error_code);
#elif defined(PRINT_ERR_MSG)
        FPRINTF(stderr, "MPI_File_read_ordered: Only an integral number of etypes can be accessed\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
#else /* MPICH-1 */
	error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ERR_ETYPE_FRACTIONAL,
				     myname, (char *) 0, (char *) 0);
	return ADIOI_Error(fh, error_code, myname);	    
#endif
    }

    if ((fh->file_system == ADIO_PIOFS) || (fh->file_system == ADIO_PVFS) || 
		    (fh->file_system == ADIO_PVFS2)) {
#ifdef MPICH2
	error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_UNSUPPORTED_OPERATION, "**iosharedunsupported", 0);
	return MPIR_Err_return_file(fh, myname, error_code);
#elif defined(PRINT_ERR_MSG)
	FPRINTF(stderr, "MPI_File_read_ordered: Shared file pointer not supported on PIOFS and PVFS\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else /* MPICH-1 */
	error_code = MPIR_Err_setmsg(MPI_ERR_UNSUPPORTED_OPERATION, 
                    MPIR_ERR_NO_SHARED_FP, myname, (char *) 0, (char *) 0);
	return ADIOI_Error(fh, error_code, myname);
#endif
    }

    ADIOI_TEST_DEFERRED(fh, "MPI_File_read_ordered", &error_code);

    MPI_Comm_size(fh->comm, &nprocs);
    MPI_Comm_rank(fh->comm, &myrank);

    if (fh->file_system != ADIO_TUNNELFS)
    {
        incr = (count*datatype_size)/fh->etype_size;
        
        /* Use a message as a 'token' to order the operations */
        source = myrank - 1;
        dest   = myrank + 1;
        if (source < 0) source = MPI_PROC_NULL;
        if (dest >= nprocs) dest = MPI_PROC_NULL;
        MPI_Recv( NULL, 0, MPI_BYTE, source, 0, fh->comm, MPI_STATUS_IGNORE );
        ADIO_Get_shared_fp(fh, incr, &shared_fp, &error_code);
        if (error_code != MPI_SUCCESS) {
#ifdef MPICH2
        error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_FATAL, myname, __LINE__, MPI_ERR_INTERN, 
                          "**iosharedfailed", 0);
        return MPIR_Err_return_file(fh, myname, error_code);
#else
        FPRINTF(stderr, "MPI_File_read_ordered: Error! Could not access shared file pointer.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
#endif
        }
        MPI_Send( NULL, 0, MPI_BYTE, dest, 0, fh->comm );

        ADIO_ReadStridedColl(fh, buf, count, datatype, ADIO_EXPLICIT_OFFSET,
                 shared_fp, status, &error_code);
    }
    else
    {
        shared_fp = 0;  /* shared fp and ordered access is handled on server side */
        ADIO_ReadStridedColl(fh, buf, count, datatype, ADIO_SHARED, shared_fp,
                             status, &error_code);
    }

    /* FIXME: Check for error code from ReadStridedColl? */
    return error_code;
}
