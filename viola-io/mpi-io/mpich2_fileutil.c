/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*  
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */
#include "mpioimpl.h"
#include "adio_extern.h"

#ifdef MPICH2

/* Forward ref for the routine to extract and set the error handler
   in a ROMIO File structure.  FIXME: These should be imported from a common
   header file that is also used in errhan/file_set_errhandler.c
 */
int MPIR_ROMIO_Get_file_errhand( MPI_File, MPI_Errhandler * );
int MPIR_ROMIO_Set_file_errhand( MPI_File, MPI_Errhandler );
void MPIR_Get_file_error_routine( MPI_Errhandler, 
				  void (**)(MPI_File *, int *, ...), 
				  int * );
struct MPID_Comm;
int MPID_Abort( struct MPID_Comm *comm, int mpi_errno, int exit_code, char *error_msg );

int MPIR_Err_return_file( MPI_File file_ptr, const char fcname[], int errcode )
{
    MPI_Errhandler e;
    void (*c_errhandler)(MPI_File *, int *, ... );
    int  kind;   /* Error handler kind (see below) */
    char error_msg[4096];
    int len;

    /* First, check the nesting level */
    /*if (MPIR_Nest_value()) return errcode;*/ /* Is there any recursion in the MPI_File_... interface? */

    /* If the file pointer is not valid, we use the handler on
       MPI_FILE_NULL (MPI-2, section 9.7).  For now, this code assumes that 
       MPI_FILE_NULL has the default handler (return).  FIXME.  See
       below - the set error handler uses ADIOI_DFLT_ERR_HANDLER; 
    */

    /* First, get the handler and the corresponding function */
    
    if (file_ptr == MPI_FILE_NULL) e = ADIOI_DFLT_ERR_HANDLER;
    else                           e = file_ptr->err_handler;

    /* Actually, e is just the value provide by the MPICH2 routines
       file_set_errhandler.  This is actually a *pointer* to the
       errhandler structure.  We don't know that, so we ask
       the MPICH2 code to translate this object into an error handler.
       kind = 0: errors are fatal
       kind = 1: errors return
       kind = 2: errors call function
    */
    if (e == MPI_ERRORS_RETURN || !e) {
	/* FIXME: This is a hack in case no error handler was set */
	kind = 1;
	c_errhandler = 0;
    }
    else {
	MPIR_Get_file_error_routine( e, &c_errhandler, &kind );
    }

    /* --BEGIN ERROR HANDLING-- */
    if (MPIR_Err_is_fatal(errcode) || kind == 0) 
    {
	/* This file does not get the correct definitions for MPIU_Snprintf because it cannot include mpiimpl.h */
	/* MPIU_Snprintf(error_msg, 4096, "Fatal error in %s: ", fcname); */
	sprintf(error_msg, "Fatal error in %s: ", fcname);
	len = (int)strlen(error_msg);
	MPIR_Err_get_string(errcode, &error_msg[len], 4096-len, NULL);
	MPID_Abort(NULL, MPI_SUCCESS, errcode, error_msg);
    }
    /* --END ERROR HANDLING-- */
    else if (kind == 2) {
	(*c_errhandler)( &file_ptr, &errcode, 0 );
    }
    /* if kind == 1, just return */

    return errcode;
}

/* These next two routines are used to allow MPICH2 to access/set the
   error handers in the MPI_File structure until MPICH2 knows about the
   file structure, and to handle the errhandler structure, which 
   includes a reference count.  Not currently used. */
int MPIR_ROMIO_Set_file_errhand( MPI_File file_ptr, MPI_Errhandler e )
{
    if (file_ptr == MPI_FILE_NULL) ADIOI_DFLT_ERR_HANDLER = e;
    /* --BEGIN ERROR HANDLING-- */
    else if (file_ptr->cookie != ADIOI_FILE_COOKIE) {
	return MPI_ERR_FILE;
    }
    /* --END ERROR HANDLING-- */
    else 
	file_ptr->err_handler = e;
    return 0;
}
int MPIR_ROMIO_Get_file_errhand( MPI_File file_ptr, MPI_Errhandler *e )
{
    if (file_ptr == MPI_FILE_NULL) {
	if (ADIOI_DFLT_ERR_HANDLER == MPI_ERRORS_RETURN)
	    *e = 0;
	else {
	    *e = ADIOI_DFLT_ERR_HANDLER;
	}
    }
    /* --BEGIN ERROR HANDLING-- */
    else if (file_ptr->cookie != ADIOI_FILE_COOKIE) {
	return MPI_ERR_FILE;
    }
    /* --END ERROR HANDLING-- */
    else {
	if (file_ptr->err_handler == MPI_ERRORS_RETURN) 
	    *e = 0;
	else
	    *e = file_ptr->err_handler;
    }
    return 0;
}

#endif /* MPICH2 */
