/* 
 *   $Id: read_all.c 2193 2003-06-05 11:50:38Z rainer $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpioimpl.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_File_read_all = PMPI_File_read_all
#elif defined(HAVE_ATTRIBUTE_WEAK)
int MPI_File_read_all(MPI_File fh, void *buf, int count, MPI_Datatype datatype,
	MPI_Status *status) __attribute__ ((weak, alias ("PMPI_File_read_all")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_File_read_all MPI_File_read_all
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_File_read_all as PMPI_File_read_all
/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#define MPIO_BUILD_PROFILING
#include "mpioprof.h"
#endif

/* status object not filled currently */

/*@
    MPI_File_read_all - Collective read using individual file pointer

Input Parameters:
. fh - file handle (handle)
. count - number of elements in buffer (nonnegative integer)
. datatype - datatype of each buffer element (handle)

Output Parameters:
. buf - initial address of buffer (choice)
. status - status object (Status)

.N fortran
@*/
int MPI_File_read_all(MPI_File fh, void *buf, int count, 
                      MPI_Datatype datatype, MPI_Status *status)
{
    int error_code, datatype_size;
#ifndef PRINT_ERR_MSG
    static char myname[] = "MPI_FILE_READ_ALL";
#endif
#ifdef MPI_hpux
    int fl_xmpi;

    HPMP_IO_START(fl_xmpi, BLKMPIFILEREADALL, TRDTBLOCK, fh, datatype, count);
#endif /* MPI_hpux */

#ifdef PRINT_ERR_MSG
    if ((fh <= (MPI_File) 0) || (fh->cookie != ADIOI_FILE_COOKIE)) {
	FPRINTF(stderr, "MPI_File_read_all: Invalid file handle\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
    }
#else
    ADIOI_TEST_FILE_HANDLE(fh, myname);
#endif

    if (count < 0) {
#ifdef PRINT_ERR_MSG
	FPRINTF(stderr, "MPI_File_read_all: Invalid count argument\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else
	error_code = MPIR_Err_setmsg(MPI_ERR_ARG, MPIR_ERR_COUNT_ARG,
				     myname, (char *) 0, (char *) 0);
	return ADIOI_Error(fh, error_code, myname);
#endif
    }

    if (datatype == MPI_DATATYPE_NULL) {
#ifdef PRINT_ERR_MSG
        FPRINTF(stderr, "MPI_File_read_all: Invalid datatype\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
#else
	error_code = MPIR_Err_setmsg(MPI_ERR_TYPE, MPIR_ERR_TYPE_NULL,
				     myname, (char *) 0, (char *) 0);
	return ADIOI_Error(fh, error_code, myname);	    
#endif
    }

    MPI_Type_size(datatype, &datatype_size);
    if ((count*datatype_size) % fh->etype_size != 0) {
#ifdef PRINT_ERR_MSG
        FPRINTF(stderr, "MPI_File_read_all: Only an integral number of etypes can be accessed\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
#else
	error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ERR_ETYPE_FRACTIONAL,
				     myname, (char *) 0, (char *) 0);
	return ADIOI_Error(fh, error_code, myname);	    
#endif
    }

    if (fh->access_mode & MPI_MODE_WRONLY) {
#ifdef PRINT_ERR_MSG
	FPRINTF(stderr, "MPI_File_read_all: Can't read from a file opened with MPI_MODE_WRONLY\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else
	error_code = MPIR_Err_setmsg(MPI_ERR_UNSUPPORTED_OPERATION, 
 		MPIR_ERR_MODE_WRONLY, myname, (char *) 0, (char *) 0);
	return ADIOI_Error(fh, error_code, myname);	    
#endif
    }

    if (fh->access_mode & MPI_MODE_SEQUENTIAL) {
#ifdef PRINT_ERR_MSG
	FPRINTF(stderr, "MPI_File_read_all: Can't use this function because file was opened with MPI_MODE_SEQUENTIAL\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else
	error_code = MPIR_Err_setmsg(MPI_ERR_UNSUPPORTED_OPERATION, 
                        MPIR_ERR_AMODE_SEQ, myname, (char *) 0, (char *) 0);
	return ADIOI_Error(fh, error_code, myname);
#endif
    }

    ADIO_ReadStridedColl(fh, buf, count, datatype, ADIO_INDIVIDUAL,
			  0, status, &error_code);
#ifdef MPI_hpux
    HPMP_IO_END(fl_xmpi, fh, datatype, count);
#endif /* MPI_hpux */
    return error_code;
}
