/* 
 *   $Id: ad_pvfs_seek.c 993 2001-07-05 15:55:55Z stef $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "ad_pvfs.h"
#include "adio_extern.h"
#ifdef PROFILE
#include "mpe.h"
#endif



#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak ADIOI_PVFS_SeekIndividual = PADIOI_PVFS_SeekIndividual
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PADIOI_PVFS_SeekIndividual  ADIOI_PVFS_SeekIndividual
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate ADIOI_PVFS_SeekIndividual as PADIOI_PVFS_SeekIndividual
/* end of weak pragmas */
#endif

/* Include mapping from ADIOI_PVFS->PADIOI_PVFS */
#define PVFS_BUILD_PROFILING
#include "pvfsprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif



ADIO_Offset ADIOI_PVFS_SeekIndividual(ADIO_File fd, ADIO_Offset offset, 
		      int whence, int *error_code)
{
/* implemented for whence=SEEK_SET only. SEEK_CUR and SEEK_END must
   be converted to the equivalent with SEEK_SET before calling this 
   routine. */
/* offset is in units of etype relative to the filetype */

#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_PVFS_SEEKINDIVIDUAL";
#endif
    ADIO_Offset off, err;
    ADIOI_Flatlist_node *flat_file;

    int i, n_etypes_in_filetype, n_filetypes, etype_in_filetype;
    ADIO_Offset abs_off_in_filetype=0;
    int size_in_filetype, sum;
    int filetype_size, etype_size, filetype_is_contig;
    MPI_Aint filetype_extent;

    ADIOI_Datatype_iscontig(fd->filetype, &filetype_is_contig);
    etype_size = fd->etype_size;

    if (filetype_is_contig) off = fd->disp + etype_size * offset;
    else {
        flat_file = ADIOI_Flatlist;
        while (flat_file->type != fd->filetype) flat_file = flat_file->next;

	MPI_Type_extent(fd->filetype, &filetype_extent);
	MPI_Type_size(fd->filetype, &filetype_size);
	if ( ! filetype_size ) {
	    *error_code = MPI_SUCCESS; 
	    return;
	}

	n_etypes_in_filetype = filetype_size/etype_size;
	n_filetypes = (int) (offset / n_etypes_in_filetype);
	etype_in_filetype = (int) (offset % n_etypes_in_filetype);
	size_in_filetype = etype_in_filetype * etype_size;
 
	sum = 0;
	for (i=0; i<flat_file->count; i++) {
	    sum += flat_file->blocklens[i];
	    if (sum > size_in_filetype) {
		abs_off_in_filetype = flat_file->indices[i] +
		    size_in_filetype - (sum - flat_file->blocklens[i]);
		break;
	    }
	}

	/* abs. offset in bytes in the file */
	off = fd->disp + (ADIO_Offset) n_filetypes * filetype_extent +
                abs_off_in_filetype;
    }

#ifdef PROFILE
    MPE_Log_event(11, 0, "start seek");
#endif
    err = pvfs_lseek(fd->fd_sys, off, SEEK_SET);
#ifdef PROFILE
    MPE_Log_event(12, 0, "end seek");
#endif
    fd->fp_ind = off;
    fd->fp_sys_posn = off;

#ifdef PRINT_ERR_MSG
    *error_code = (err == -1) ? MPI_ERR_UNKNOWN : MPI_SUCCESS;
#else
    if (err == -1) {
	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
			      myname, "I/O Error", "%s", strerror(errno));
	ADIOI_Error(fd, *error_code, myname);	    
    }
    else *error_code = MPI_SUCCESS;
#endif
    return off;
}
