/* 
 *   $Id$    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 *
 */

#include "ad_svm_fsys.h"
#include "adio_extern.h"
#ifdef __MPISGI
#include "mpisgi2.h"
#endif

void ADIOI_SVM_Fcntl(ADIO_File fd, int flag, ADIO_Fcntl_t *fcntl_struct, int *error_code)
{
    MPI_Datatype copy_etype, copy_filetype;
    int combiner, i, j, k, filetype_is_contig, ntimes, err;
    ADIOI_Flatlist_node *flat_file;
    ADIO_Offset curr_fsize, alloc_size, size, len, done;
    ADIO_Status status;
    char *buf;
    /* added by RAY */
    FileTable  files_local;
    /* end RAY */
    

    /*printf("Entering ADIOI_SVM_Fcntl().\n");*/
    
    switch(flag) {
    case ADIO_FCNTL_SET_VIEW:
        /* free copies of old etypes and filetypes and delete flattened 
           version of filetype if necessary */

	MPI_Type_get_envelope(fd->etype, &i, &j, &k, &combiner);
	if (combiner != MPI_COMBINER_NAMED) MPI_Type_free(&(fd->etype));

	ADIOI_Datatype_iscontig(fd->filetype, &filetype_is_contig);
	if (!filetype_is_contig) ADIOI_Delete_flattened(fd->filetype);

	MPI_Type_get_envelope(fd->filetype, &i, &j, &k, &combiner);
	if (combiner != MPI_COMBINER_NAMED) MPI_Type_free(&(fd->filetype));

	/* set new info */
	ADIO_SetInfo(fd, fcntl_struct->info, &err);

        /* set new etypes and filetypes */

	MPI_Type_get_envelope(fcntl_struct->etype, &i, &j, &k, &combiner);
	if (combiner == MPI_COMBINER_NAMED) fd->etype = fcntl_struct->etype;
	else {
	    MPI_Type_contiguous(1, fcntl_struct->etype, &copy_etype);
	    MPI_Type_commit(&copy_etype);
	    fd->etype = copy_etype;
	}
	MPI_Type_get_envelope(fcntl_struct->filetype, &i, &j, &k, &combiner);
	if (combiner == MPI_COMBINER_NAMED) 
	    fd->filetype = fcntl_struct->filetype;
	else {
	    MPI_Type_contiguous(1, fcntl_struct->filetype, &copy_filetype);
	    MPI_Type_commit(&copy_filetype);
	    fd->filetype = copy_filetype;
	    ADIOI_Flatten_datatype(fd->filetype);
            /* this function will not flatten the filetype if it turns out
               to be all contiguous. */
	}

	MPI_Type_size(fd->etype, &(fd->etype_size));
	fd->disp = fcntl_struct->disp;

        /* reset MPI-IO file pointer to point to the first byte that can
           be accessed in this view. */

        ADIOI_Datatype_iscontig(fd->filetype, &filetype_is_contig);
	if (filetype_is_contig) fd->fp_ind = fcntl_struct->disp;
	else {
	    flat_file = ADIOI_Flatlist;
	    while (flat_file->type != fd->filetype) 
		flat_file = flat_file->next;
	    for (i=0; i<flat_file->count; i++) {
		if (flat_file->blocklens[i]) {
		    fd->fp_ind = fcntl_struct->disp + flat_file->indices[i];
		    break;
		}
	    }
	}
	*error_code = MPI_SUCCESS;
	break;

    case ADIO_FCNTL_GET_FSIZE:
    	/* changed by RAY */
	/*fcntl_struct->fsize = lseek(fd->fd_sys, 0, SEEK_END);*/
	files_local = (FileTable)NULL;
	if (ADIOI_SVM_Lookup_fd(fd->fd_sys,&files_local)==MPI_SUCCESS)
	   fcntl_struct->fsize = files_local->size;
	else
	   fcntl_struct->fsize = -1;
	/*if (fd->fp_sys_posn != -1) */
	     /*lseek(fd->fd_sys, fd->fp_sys_posn, SEEK_SET);*/
	free(files_local);
	/* end RAY */
	*error_code = (fcntl_struct->fsize == -1) ? MPI_ERR_UNKNOWN : MPI_SUCCESS;
	break;

    case ADIO_FCNTL_SET_DISKSPACE:
	/* will be called by one process only */
	/* added by RAY */
	/* !!! Not with AD_SVM - here every process calls it !!! */
	/* end RAY */
	/* On file systems with no preallocation function, I have to 
           explicitly write 
           to allocate space. Since there could be holes in the file, 
           I need to read up to the current file size, write it back, 
           and then write beyond that depending on how much 
           preallocation is needed.
           read/write in sizes of no more than ADIOI_PREALLOC_BUFSZ */

	/* changed by RAY */
	/*curr_fsize = lseek(fd->fd_sys, 0, SEEK_END);*/
	files_local = (FileTable)NULL;
	if (ADIOI_SVM_Lookup_fd(fd->fd_sys,&files_local)==MPI_SUCCESS)
	   curr_fsize = files_local->size;
	else
	   curr_fsize = -1;

	free(files_local);

	if (curr_fsize < fcntl_struct->diskspace)
	   ADIOI_SVM_Resize(fd,fcntl_struct->diskspace,error_code);
		
	if (fd->fp_sys_posn != -1)
	   /*lseek(fd->fd_sys, fd->fp_sys_posn, SEEK_SET);*/
	   ADIOI_SVM_Lseek(fd,fd->fp_sys_posn);
	/* end RAY */ 
	*error_code = MPI_SUCCESS;
	break;

    case ADIO_FCNTL_SET_IOMODE:
        /* for implementing PFS I/O modes. will not occur in MPI-IO
           implementation.*/
	if (fd->iomode != fcntl_struct->iomode) {
	    fd->iomode = fcntl_struct->iomode;
	    MPI_Barrier(MPI_COMM_WORLD);
	}
	*error_code = MPI_SUCCESS;
	break;

    case ADIO_FCNTL_SET_ATOMICITY:
	fd->atomicity = (fcntl_struct->atomicity == 0) ? 0 : 1;
	*error_code = MPI_SUCCESS;
	break;

    default:
	printf("Unknown flag passed to ADIOI_SVM_Fcntl\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
    }
}
