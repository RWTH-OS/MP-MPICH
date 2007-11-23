/* 
 *   $Id: open.c 2193 2003-06-05 11:50:38Z rainer $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpioimpl.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_File_open = PMPI_File_open
#elif defined(HAVE_ATTRIBUTE_WEAK)
int MPI_File_open(MPI_Comm comm, char *filename, int amode, MPI_Info info,
	MPI_File *fh) __attribute__ ((weak, alias ("PMPI_File_open")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_File_open MPI_File_open
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_File_open as PMPI_File_open
/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#define MPIO_BUILD_PROFILING
#include "mpioprof.h"
#endif

extern int ADIO_Init_keyval;

/*@
    MPI_File_open - Opens a file

Input Parameters:
. comm - communicator (handle)
. filename - name of file to open (string)
. amode - file access mode (integer)
. info - info object (handle)

Output Parameters:
. fh - file handle (handle)

.N fortran
@*/
int MPI_File_open(MPI_Comm comm, char *filename, int amode, 
                  MPI_Info info, MPI_File *fh)
{
    int error_code, file_system, flag, tmp_amode, rank, orig_amode;
#ifndef PRINT_ERR_MSG
    static char myname[] = "MPI_FILE_OPEN";
#endif
    int err, min_code;
    char *tmp;
    MPI_Comm dupcomm, dupcommself;
#ifdef MPI_hpux
    int fl_xmpi;

    HPMP_IO_OPEN_START(fl_xmpi, comm);
#endif /* MPI_hpux */

	error_code = MPI_SUCCESS;

    if (comm == MPI_COMM_NULL) {
#ifdef PRINT_ERR_MSG
	FPRINTF(stderr, "MPI_File_open: Invalid communicator\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else
	error_code = MPIR_Err_setmsg(MPI_ERR_COMM, MPIR_ERR_COMM_NULL,
				     myname, (char *) 0, (char *) 0);
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    }

    MPI_Comm_test_inter(comm, &flag);
    if (flag) {
#ifdef PRINT_ERR_MSG
	FPRINTF(stderr, "MPI_File_open: Intercommunicator cannot be passed to MPI_File_open\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else
	error_code = MPIR_Err_setmsg(MPI_ERR_COMM, MPIR_ERR_COMM_INTER,
				     myname, (char *) 0, (char *) 0);
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    }

    if ( ((amode&MPI_MODE_RDONLY)?1:0) + ((amode&MPI_MODE_RDWR)?1:0) +
	 ((amode&MPI_MODE_WRONLY)?1:0) != 1 ) {
#ifdef PRINT_ERR_MSG
	FPRINTF(stderr, "MPI_File_open: Exactly one of MPI_MODE_RDONLY, MPI_MODE_WRONLY, or MPI_MODE_RDWR must be specified\n");
	MPI_Abort(MPI_COMM_WORLD, 1); 
#else
	error_code = MPIR_Err_setmsg(MPI_ERR_AMODE, 3,
				     myname, (char *) 0, (char *) 0);
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    }

    if ((amode & MPI_MODE_RDONLY) && 
            ((amode & MPI_MODE_CREATE) || (amode & MPI_MODE_EXCL))) {
#ifdef PRINT_ERR_MSG
	FPRINTF(stderr, "MPI_File_open: It is erroneous to specify MPI_MODE_CREATE or MPI_MODE_EXCL with MPI_MODE_RDONLY\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else
	error_code = MPIR_Err_setmsg(MPI_ERR_AMODE, 5,
				     myname, (char *) 0, (char *) 0);
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    }

    if ((amode & MPI_MODE_RDWR) && (amode & MPI_MODE_SEQUENTIAL)) {
#ifdef PRINT_ERR_MSG
	FPRINTF(stderr, "MPI_File_open: It is erroneous to specify MPI_MODE_SEQUENTIAL with MPI_MODE_RDWR\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else
	error_code = MPIR_Err_setmsg(MPI_ERR_AMODE, 7,
				     myname, (char *) 0, (char *) 0);
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    }

/* check if amode is the same on all processes */
    MPI_Comm_dup(comm, &dupcomm);
    tmp_amode = amode;

/*  
    Removed this check because broadcast is too expensive. 
    MPI_Bcast(&tmp_amode, 1, MPI_INT, 0, dupcomm);
    if (amode != tmp_amode) {
	FPRINTF(stderr, "MPI_File_open: amode must be the same on all processes\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
    }
*/

/* check if ADIO has been initialized. If not, initialize it */
    if (ADIO_Init_keyval == MPI_KEYVAL_INVALID) {

/* check if MPI itself has been initialized. If not, flag an error.
   Can't initialize it here, because don't know argc, argv */
	MPI_Initialized(&flag);
	if (!flag) {
	    FPRINTF(stderr, "Error: MPI_Init() must be called before using MPI-IO\n");
	    MPI_Abort(MPI_COMM_WORLD, 1);
	}

	MPI_Keyval_create(MPI_NULL_COPY_FN, ADIOI_End_call, &ADIO_Init_keyval,
			  (void *) 0);  

/* put a dummy attribute on MPI_COMM_WORLD, because we want the delete
   function to be called when MPI_COMM_WORLD is freed. Hopefully the
   MPI library frees MPI_COMM_WORLD when MPI_Finalize is called,
   though the standard does not mandate this. */

	MPI_Attr_put(MPI_COMM_WORLD, ADIO_Init_keyval, (void *) 0);

/* initialize ADIO */

	ADIO_Init( (int *)0, (char ***)0, &error_code);
    }


    file_system = -1;
    tmp = strchr(filename, ':');
#ifdef WIN32
    // Unfortunately Windows uses ':' behind the drive letter.
    // So we check if there is only one letter before the ':'
    // Please do not use a single letter filesystem name!
    if(tmp && ((tmp-filename) == 1)) tmp = 0;
#endif
    if (!tmp) {
	ADIO_FileSysType(filename, &file_system, &err);
	if (err != MPI_SUCCESS) {
#ifdef PRINT_ERR_MSG
	    FPRINTF(stderr, "MPI_File_open: Can't determine the file-system type. Check the filename/path you provided and try again. Otherwise, prefix the filename with a string to indicate the type of file sytem (piofs:, pfs:, nfs:, ufs:, hfs:, xfs:, sfs:, pvfs:, svm:).\n");
	    MPI_Abort(MPI_COMM_WORLD, 1);
#else
	error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ERR_NO_FSTYPE,
				     myname, (char *) 0, (char *) 0);
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
	}
	MPI_Allreduce(&file_system, &min_code, 1, MPI_INT, MPI_MIN, dupcomm);
	if (min_code == ADIO_NFS) file_system = ADIO_NFS;
    }


#ifndef PFS
    if (!strncmp(filename, "pfs:", 4) || !strncmp(filename, "PFS:", 4) || (file_system == ADIO_PFS)) {
#ifdef PRINT_ERR_MSG
	FPRINTF(stderr, "MPI_File_open: ROMIO has not been configured to use the PFS file system\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else
	error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ERR_NO_PFS,
				     myname, (char *) 0, (char *) 0,"PFS");
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    }
#endif
#ifndef PIOFS
    if (!strncmp(filename, "piofs:", 6) || !strncmp(filename, "PIOFS:", 6) || (file_system == ADIO_PIOFS)) {
#ifdef PRINT_ERR_MSG
	FPRINTF(stderr, "MPI_File_open: ROMIO has not been configured to use the PIOFS file system\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else
	error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ERR_NO_PIOFS,
				     myname, (char *) 0, (char *) 0,"PIOFS");
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    }
#endif
#ifndef UFS
    if (!strncmp(filename, "ufs:", 4) || !strncmp(filename, "UFS:", 4) || (file_system == ADIO_UFS)) {
#ifdef PRINT_ERR_MSG
	FPRINTF(stderr, "MPI_File_open: ROMIO has not been configured to use the UFS file system\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else
	error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ERR_NO_UFS,
				     myname, (char *) 0, (char *) 0,"UFS");
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    }
#endif
#ifndef NFS
    if (!strncmp(filename, "nfs:", 4) || !strncmp(filename, "NFS:", 4) || (file_system == ADIO_NFS)) {
#ifdef PRINT_ERR_MSG
	FPRINTF(stderr, "MPI_File_open: ROMIO has not been configured to use the NFS file system\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else
	error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ERR_NO_NFS,
				     myname, (char *) 0, (char *) 0,"NFS");
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    }
#endif
#ifndef HFS
    if (!strncmp(filename, "hfs:", 4) || !strncmp(filename, "HFS:", 4) || (file_system == ADIO_HFS)) {
#ifdef PRINT_ERR_MSG
	FPRINTF(stderr, "MPI_File_open: ROMIO has not been configured to use the HFS file system\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else
	error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ERR_NO_HFS,
				     myname, (char *) 0, (char *) 0,"HFS");
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    }
#endif
#ifndef XFS
    if (!strncmp(filename, "xfs:", 4) || !strncmp(filename, "XFS:", 4) || (file_system == ADIO_XFS)) {
#ifdef PRINT_ERR_MSG
	FPRINTF(stderr, "MPI_File_open: ROMIO has not been configured to use the XFS file system\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else
	error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ERR_NO_XFS,
				     myname, (char *) 0, (char *) 0,"XFS");
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    }
#endif
#ifndef SFS
    if (!strncmp(filename, "sfs:", 4) || !strncmp(filename, "SFS:", 4) || (file_system == ADIO_SFS)) {
#ifdef PRINT_ERR_MSG
	FPRINTF(stderr, "MPI_File_open: ROMIO has not been configured to use the SFS file system\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else
	error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ERR_NO_SFS,
				     myname, (char *) 0, (char *) 0,"SFS");
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    }
#endif
#ifndef PVFS
    if (!strncmp(filename, "pvfs:", 5) || !strncmp(filename, "PVFS:", 5) || (file_system == ADIO_PVFS)) {
#ifdef PRINT_ERR_MSG
	FPRINTF(stderr, "MPI_File_open: ROMIO has not been configured to use the PVFS file system\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else
	error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ERR_NO_PVFS,
				     myname, (char *) 0, (char *) 0,"PVFS");
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    }
#endif

#ifndef SVM
    if (!strncmp(filename, "svm:", 4) || !strncmp(filename, "SVM:", 4) || (file_system == ADIO_SVM)) {
#ifdef PRINT_ERR_MSG
	FPRINTF(stderr, "MPI_File_open: ROMIO has not been configured to use the SVM file system\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else
	error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ERR_NO_SVM,
				     myname, (char *) 0, (char *) 0,"SVM");
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    }
#endif

#ifndef NTFS
    if (!strncmp(filename, "svm:", 4) || !strncmp(filename, "NTFS:", 4) || (file_system == ADIO_SVM)) {
#ifdef PRINT_ERR_MSG
	FPRINTF(stderr, "MPI_File_open: ROMIO has not been configured to use the SVM file system\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else
	error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ERR_NO_NTFS,
				     myname, (char *) 0, (char *) 0,"NTFS");
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    }
#endif

    if (!strncmp(filename, "pfs:", 4) || !strncmp(filename, "PFS:", 4)) {
	file_system = ADIO_PFS;
	filename += 4;
    }
    else if (!strncmp(filename, "piofs:", 6) || !strncmp(filename, "PIOFS:", 6)) {
	file_system = ADIO_PIOFS;
	filename += 6;
    }
    else if (!strncmp(filename, "ufs:", 4) || !strncmp(filename, "UFS:", 4)) {
	file_system = ADIO_UFS;
	filename += 4;
    }
    else if (!strncmp(filename, "nfs:", 4) || !strncmp(filename, "NFS:", 4)) {
	file_system = ADIO_NFS;
	filename += 4;
    }
    else if (!strncmp(filename, "hfs:", 4) || !strncmp(filename, "HFS:", 4)) {
	file_system = ADIO_HFS;
	filename += 4;
    }
    else if (!strncmp(filename, "xfs:", 4) || !strncmp(filename, "XFS:", 4)) {
	file_system = ADIO_XFS;
	filename += 4;
    }
    else if (!strncmp(filename, "sfs:", 4) || !strncmp(filename, "SFS:", 4)) {
	file_system = ADIO_SFS;
	filename += 4;
    }
    else if (!strncmp(filename, "pvfs:", 5) || !strncmp(filename, "PVFS:", 5)) {
	file_system = ADIO_PVFS;
	filename += 5;
    }
    else if (!strncmp(filename, "svm:", 4) || !strncmp(filename, "SVM:", 4)) {
	file_system = ADIO_SVM;
	filename += 4;
    }
    else if (!strncmp(filename, "ntfs:", 4) || !strncmp(filename, "NTFS:", 4)) {
	file_system = ADIO_NTFS;
	filename += 5;
    }
    if (((file_system == ADIO_PIOFS) || (file_system == ADIO_PVFS)) && 
        (amode & MPI_MODE_SEQUENTIAL)) {
#ifdef PRINT_ERR_MSG
	FPRINTF(stderr, "MPI_File_open: MPI_MODE_SEQUENTIAL not supported on PIOFS and PVFS\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else
	error_code = MPIR_Err_setmsg(MPI_ERR_UNSUPPORTED_OPERATION, 
                    MPIR_ERR_NO_MODE_SEQ, myname, (char *) 0, (char *) 0);
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    }

    orig_amode = amode;
    MPI_Comm_rank(dupcomm, &rank);

    if ((amode & MPI_MODE_CREATE) && (amode & MPI_MODE_EXCL)) {
	/* the open should fail if the file exists. Only process 0 should
           check this. Otherwise, if all processes try to check and the file 
           does not exist, one process will create the file and others who 
           reach later will return error. */

	if (!rank) {
	    MPI_Comm_dup(MPI_COMM_SELF, &dupcommself);
	    /* this dup is freed either in ADIO_Open if the open fails,
               or in ADIO_Close */
	    *fh = ADIO_Open(dupcommself, filename, file_system, amode, 0, 
               MPI_BYTE, MPI_BYTE, M_ASYNC, info, ADIO_PERM_NULL, &error_code);
	    /* broadcast the error code to other processes */
	    MPI_Bcast(&error_code, 1, MPI_INT, 0, dupcomm);
	    /* if no error, close the file. It will be reopened normally 
               below. */
	    if (error_code == MPI_SUCCESS) ADIO_Close(*fh, &error_code);
	}
	else MPI_Bcast(&error_code, 1, MPI_INT, 0, dupcomm);

	if (error_code != MPI_SUCCESS) {
	    MPI_Comm_free(&dupcomm);
	    *fh = MPI_FILE_NULL;
#ifdef MPI_hpux
	    HPMP_IO_OPEN_END(fl_xmpi, *fh, comm);
#endif /* MPI_hpux */
	    return error_code;
	}
	else amode = amode ^ MPI_MODE_EXCL;  /* turn off MPI_MODE_EXCL */
    }

/* use default values for disp, etype, filetype */    
/* set iomode=M_ASYNC. It is used to implement the Intel PFS interface
   on top of ADIO. Not relevant for MPI-IO implementation */    

    *fh = ADIO_Open(dupcomm, filename, file_system, amode, 0, MPI_BYTE,
                     MPI_BYTE, M_ASYNC, info, ADIO_PERM_NULL, &error_code);

    /* if MPI_MODE_EXCL was removed, add it back */
    if ((error_code == MPI_SUCCESS) && (amode != orig_amode))
	(*fh)->access_mode = orig_amode;

    /* determine name of file that will hold the shared file pointer */
    /* can't support shared file pointers on a file system that doesn't
       support file locking, e.g., PIOFS, PVFS */
    if ((error_code == MPI_SUCCESS) && ((*fh)->file_system != ADIO_PIOFS)
          && ((*fh)->file_system != ADIO_PVFS)) {
	ADIOI_Shfp_fname(*fh, rank);

        /* if MPI_MODE_APPEND, set the shared file pointer to end of file.
           indiv. file pointer already set to end of file in ADIO_Open. 
           Here file view is just bytes. */
	if ((*fh)->access_mode & MPI_MODE_APPEND) {
	    if (!rank) ADIO_Set_shared_fp(*fh, (*fh)->fp_ind, &error_code);
	    MPI_Barrier(dupcomm);
	}
    }

#ifdef MPI_hpux
    HPMP_IO_OPEN_END(fl_xmpi, *fh, comm);
#endif /* MPI_hpux */
    return error_code;
}
