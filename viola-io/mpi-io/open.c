/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *   $Id: open.c,v 1.1.1.1 2005/03/10 16:07:13 mhs Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpioimpl.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_File_open = PMPI_File_open
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
    int error_code, file_system, flag, /* tmp_amode, */rank;
#if defined(MPICH2) || !defined(PRINT_ERR_MSG)
    static char myname[] = "MPI_FILE_OPEN";
#endif
    char *tmp;
    MPI_Comm dupcomm;
    ADIOI_Fns *fsops;

#ifdef MPI_hpux
    int fl_xmpi;

    HPMP_IO_OPEN_START(fl_xmpi, comm);
#endif /* MPI_hpux */

    /* --BEGIN ERROR HANDLING-- */
    if (comm == MPI_COMM_NULL)
    {
#ifdef MPICH2
	error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_COMM, "**comm", 0);
	return MPIR_Err_return_file(MPI_FILE_NULL, myname, error_code);
#elif defined(PRINT_ERR_MSG)
	FPRINTF(stderr, "MPI_File_open: Invalid communicator\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else /* MPICH-1 */
	error_code = MPIR_Err_setmsg(MPI_ERR_COMM, MPIR_ERR_COMM_NULL,
				     myname, (char *) 0, (char *) 0);
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    }
    /* --END ERROR HANDLING-- */

    MPI_Comm_test_inter(comm, &flag);
    /* --BEGIN ERROR HANDLING-- */
    if (flag)
    {
#ifdef MPICH2
	error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_COMM, 
	    "**commnotintra", 0);
	return MPIR_Err_return_file(MPI_FILE_NULL, myname, error_code);
#elif defined(PRINT_ERR_MSG)
	FPRINTF(stderr, "MPI_File_open: Intercommunicator cannot be passed to MPI_File_open\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else /* MPICH-1 */
	error_code = MPIR_Err_setmsg(MPI_ERR_COMM, MPIR_ERR_COMM_INTER,
				     myname, (char *) 0, (char *) 0);
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    }

    if ( ((amode&MPI_MODE_RDONLY)?1:0) + ((amode&MPI_MODE_RDWR)?1:0) +
	 ((amode&MPI_MODE_WRONLY)?1:0) != 1 )
    {
#ifdef MPICH2
	     error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_AMODE, 
		 "**fileamodeone", 0);
	     return MPIR_Err_return_file(MPI_FILE_NULL, myname, error_code);
#elif defined(PRINT_ERR_MSG)
	FPRINTF(stderr, "MPI_File_open: Exactly one of MPI_MODE_RDONLY, MPI_MODE_WRONLY, or MPI_MODE_RDWR must be specified\n");
	MPI_Abort(MPI_COMM_WORLD, 1); 
#else /* MPICH-1 */
	error_code = MPIR_Err_setmsg(MPI_ERR_AMODE, 3,
				     myname, (char *) 0, (char *) 0);
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    }

    if ((amode & MPI_MODE_RDONLY) && 
            ((amode & MPI_MODE_CREATE) || (amode & MPI_MODE_EXCL)))
    {
#ifdef MPICH2
		error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_AMODE, 
		    "**fileamoderead", 0);
		return MPIR_Err_return_file(MPI_FILE_NULL, myname, error_code);
#elif defined(PRINT_ERR_MSG)
	FPRINTF(stderr, "MPI_File_open: It is erroneous to specify MPI_MODE_CREATE or MPI_MODE_EXCL with MPI_MODE_RDONLY\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else /* MPICH-1 */
	error_code = MPIR_Err_setmsg(MPI_ERR_AMODE, 5,
				     myname, (char *) 0, (char *) 0);
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    }

    if ((amode & MPI_MODE_RDWR) && (amode & MPI_MODE_SEQUENTIAL))
    {
#ifdef MPICH2
	error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_AMODE, 
	    "**fileamodeseq", 0);
	return MPIR_Err_return_file(MPI_FILE_NULL, myname, error_code);
#elif defined(PRINT_ERR_MSG)
	FPRINTF(stderr, "MPI_File_open: It is erroneous to specify MPI_MODE_SEQUENTIAL with MPI_MODE_RDWR\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else /* MPICH-1 */
	error_code = MPIR_Err_setmsg(MPI_ERR_AMODE, 7,
				     myname, (char *) 0, (char *) 0);
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    }
    /* --END ERROR HANDLING-- */

/* check if amode is the same on all processes */
    MPI_Comm_dup(comm, &dupcomm);

/*  
    Removed this check because broadcast is too expensive. 
    tmp_amode = amode;
    MPI_Bcast(&tmp_amode, 1, MPI_INT, 0, dupcomm);
    if (amode != tmp_amode) {
	FPRINTF(stderr, "MPI_File_open: amode must be the same on all processes\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
    }
*/

/* check if ADIO has been initialized. If not, initialize it */
    if (ADIO_Init_keyval == MPI_KEYVAL_INVALID)
    {

/* check if MPI itself has been initialized. If not, flag an error.
   Can't initialize it here, because don't know argc, argv */
	MPI_Initialized(&flag);
	/* --BEGIN ERROR HANDLING-- */
	if (!flag) {
	    FPRINTF(stderr, "Error: MPI_Init() must be called before using MPI-IO\n");
	    MPI_Abort(MPI_COMM_WORLD, 1);
	}
	/* --END ERROR HANDLING-- */

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

    /* resolve file system type from file name; this is a collective call */
    ADIO_ResolveFileType(dupcomm, filename, &file_system, &fsops, &error_code);
    /* --BEGIN ERROR HANDLING-- */
    if (error_code != MPI_SUCCESS)
    {
	/* ADIO_ResolveFileType() will print as informative a message as it
	 * possibly can or call MPIR_Err_setmsg.  We just need to propagate 
	 * the error up.  In the PRINT_ERR_MSG case MPI_Abort has already
	 * been called as well, so we probably didn't even make it this far.
	 */
#ifdef MPICH2
	return MPIR_Err_return_file(MPI_FILE_NULL, myname, error_code);
#elif defined(PRINT_ERR_MSG)
	MPI_Abort(MPI_COMM_WORLD, 1); /* this is mostly here for clarity */
#else /* MPICH-1 */
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    }

    /* Test for invalid flags in amode.
     *
     * eventually we should allow the ADIO implementations to test for 
     * invalid flags through some functional interface rather than having
     *  these tests here. -- Rob, 06/06/2001
     */
    if (((file_system == ADIO_PIOFS) || (file_system == ADIO_PVFS) || (file_system == ADIO_PVFS2)) && 
        (amode & MPI_MODE_SEQUENTIAL))
    {
#ifdef MPICH2
	    error_code = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__, MPI_ERR_UNSUPPORTED_OPERATION, 
		"**iosequnsupported", 0);
	    return MPIR_Err_return_file(MPI_FILE_NULL, myname, error_code);
#elif defined(PRINT_ERR_MSG)
	FPRINTF(stderr, "MPI_File_open: MPI_MODE_SEQUENTIAL not supported on PIOFS and PVFS\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
#else /* MPICH-1 */
	error_code = MPIR_Err_setmsg(MPI_ERR_UNSUPPORTED_OPERATION, 
                    MPIR_ERR_NO_MODE_SEQ, myname, (char *) 0, (char *) 0);
	return ADIOI_Error(MPI_FILE_NULL, error_code, myname);
#endif
    }
    /* --END ERROR HANDLING-- */

    /* strip off prefix if there is one */
    tmp = strchr(filename, ':');
    /* Only skip prefixes greater than length one to allow for windows drive specification (c:\...)*/
    /*if (tmp) filename = tmp + 1;*/
    if (tmp > filename + 1)
	filename = tmp + 1;

/* use default values for disp, etype, filetype */    
/* set iomode=M_ASYNC. It is used to implement the Intel PFS interface
   on top of ADIO. Not relevant for MPI-IO implementation */    

    *fh = ADIO_Open(comm, dupcomm, filename, file_system, amode, 0, MPI_BYTE,
                    MPI_BYTE, M_ASYNC, info, ADIO_PERM_NULL, &error_code);

    if (error_code != MPI_SUCCESS)
        MPI_Comm_free(&dupcomm);

    /* determine name of file that will hold the shared file pointer */
    /* can't support shared file pointers on a file system that doesn't
       support file locking, e.g., PIOFS, PVFS, PVFS2 */
    if ((error_code == MPI_SUCCESS) && ((*fh)->file_system != ADIO_PIOFS)
          && ((*fh)->file_system != ADIO_PVFS) 
	  && ((*fh)->file_system != ADIO_PVFS2) ){
	MPI_Comm_rank(dupcomm, &rank);
	ADIOI_Shfp_fname(*fh, rank);

        /* if MPI_MODE_APPEND, set the shared file pointer to end of file.
           indiv. file pointer already set to end of file in ADIO_Open. 
           Here file view is just bytes. */
	if ((*fh)->access_mode & MPI_MODE_APPEND) {
	    if ((*fh)->io_worker)  /* only one person need set the sharedfp */
		    ADIO_Set_shared_fp(*fh, (*fh)->fp_ind, &error_code);
	    MPI_Barrier(dupcomm);
	}
    }

#ifdef MPI_hpux
    HPMP_IO_OPEN_END(fl_xmpi, *fh, comm);
#endif /* MPI_hpux */
    return error_code;
}
