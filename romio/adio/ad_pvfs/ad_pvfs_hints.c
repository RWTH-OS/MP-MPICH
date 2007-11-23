/* 
 *   $Id: ad_pvfs_hints.c 993 2001-07-05 15:55:55Z stef $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_pvfs.h"


#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak ADIOI_PVFS_SetInfo = PADIOI_PVFS_SetInfo
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PADIOI_PVFS_SetInfo  ADIOI_PVFS_SetInfo
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate ADIOI_PVFS_SetInfo as PADIOI_PVFS_SetInfo
/* end of weak pragmas */
#endif

/* Include mapping from ADIOI_PVFS->PADIOI_PVFS */
#define PVFS_BUILD_PROFILING
#include "pvfsprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif



void ADIOI_PVFS_SetInfo(ADIO_File fd, MPI_Info users_info, int *error_code)
{
    char *value;
    int flag, tmp_val, str_factor=-1, str_unit=-1, start_iodev=-1;

    if (!(fd->info)) {
	/* This must be part of the open call. can set striping parameters 
           if necessary. */ 
	MPI_Info_create(&(fd->info));
	
	/* has user specified striping parameters 
           and do they have the same value on all processes? */
	if (users_info != MPI_INFO_NULL) {
	    value = (char *) ADIOI_Malloc((MPI_MAX_INFO_VAL+1)*sizeof(char));

	    MPI_Info_get(users_info, "striping_factor", MPI_MAX_INFO_VAL, 
			 value, &flag);
	    if (flag) {
		str_factor=atoi(value);
		tmp_val = str_factor;
		MPI_Bcast(&tmp_val, 1, MPI_INT, 0, fd->comm);
		if (tmp_val != str_factor) {
		    FPRINTF(stderr, "ADIOI_PVFS_SetInfo: the value for key \"striping_factor\" must be the same on all processes\n");
		    MPI_Abort(MPI_COMM_WORLD, 1);
		}
		else MPI_Info_set(fd->info, "striping_factor", value);
	    }

	    MPI_Info_get(users_info, "striping_unit", MPI_MAX_INFO_VAL, 
			 value, &flag);
	    if (flag) {
		str_unit=atoi(value);
		tmp_val = str_unit;
		MPI_Bcast(&tmp_val, 1, MPI_INT, 0, fd->comm);
		if (tmp_val != str_unit) {
		    FPRINTF(stderr, "ADIOI_PVFS_SetInfo: the value for key \"striping_unit\" must be the same on all processes\n");
		    MPI_Abort(MPI_COMM_WORLD, 1);
		}
		else MPI_Info_set(fd->info, "striping_unit", value);
	    }

	    MPI_Info_get(users_info, "start_iodevice", MPI_MAX_INFO_VAL, 
			 value, &flag);
	    if (flag) {
		start_iodev=atoi(value);
		tmp_val = start_iodev;
		MPI_Bcast(&tmp_val, 1, MPI_INT, 0, fd->comm);
		if (tmp_val != start_iodev) {
		    FPRINTF(stderr, "ADIOI_PVFS_SetInfo: the value for key \"start_iodevice\" must be the same on all processes\n");
		    MPI_Abort(MPI_COMM_WORLD, 1);
		}
		else MPI_Info_set(fd->info, "start_iodevice", value);
	    }

	    ADIOI_Free(value);
	}
    }	

    /* set the values for collective I/O and data sieving parameters */
    ADIOI_GEN_SetInfo(fd, users_info, error_code);

    *error_code = MPI_SUCCESS;
}
