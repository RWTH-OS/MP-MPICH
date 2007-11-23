/* 
 *   $Id: delete.c 2193 2003-06-05 11:50:38Z rainer $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpioimpl.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_File_delete = PMPI_File_delete
#elif defined(HAVE_ATTRIBUTE_WEAK)
int MPI_File_delete(char *filename,
	MPI_Info info) __attribute__ ((weak, alias ("PMPI_File_delete")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_File_delete MPI_File_delete
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_File_delete as PMPI_File_delete
/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#define MPIO_BUILD_PROFILING
#include "mpioprof.h"
#endif

extern int ADIO_Init_keyval;

/*@
    MPI_File_delete - Deletes a file

Input Parameters:
. filename - name of file to delete (string)
. info - info object (handle)

.N fortran
@*/
int MPI_File_delete(char *filename, MPI_Info info)
{
    int flag, error_code;
    char *tmp;
#ifdef MPI_hpux
    int fl_xmpi;
  
    HPMP_IO_START(fl_xmpi, BLKMPIFILEDELETE, TRDTBLOCK,
                MPI_FILE_NULL, MPI_DATATYPE_NULL, -1);
#endif /* MPI_hpux */

    /* first check if ADIO has been initialized. If not, initialize it */
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

    tmp = strchr(filename, ':');
#ifdef WIN32
    // Unfortunately Windows uses ':' behind the drive letter.
    // So we check if there is only one letter before the ':'
    // Please do not use a single letter filesystem name!
    if(tmp && ((tmp-filename) == 1)) tmp = 0;
#endif
    if (tmp) filename = tmp + 1;

    ADIO_Delete(filename, &error_code);
#ifdef MPI_hpux
    HPMP_IO_END(fl_xmpi, MPI_FILE_NULL, MPI_DATATYPE_NULL, -1);
#endif /* MPI_hpux */
    return error_code;
}
