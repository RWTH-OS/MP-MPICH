/* 
 *   $Id: ad_ntfs_fcntl.c 922 2001-05-31 14:10:48Z karsten $    
 *
 */

#include "ad_ntfs.h"
#include "adio_extern.h"

void ADIOI_NTFS_Fcntl(ADIO_File fd, int flag, ADIO_Fcntl_t *fcntl_struct, int *error_code)
{
    MPI_Datatype copy_etype, copy_filetype;
    int combiner, i, j, k, filetype_is_contig, ntimes, err,len;
    ADIOI_Flatlist_node *flat_file;
    ADIO_Offset curr_fsize, alloc_size, size, done;
	LARGE_INTEGER *LI;
    ADIO_Status status;
    char *buf;
#ifndef PRINT_ERR_MSG
    static char myname[] = "ADIOI_NTFS_FCNTL";
#endif


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
	LI = (LARGE_INTEGER *)&fcntl_struct->fsize;
	LI->LowPart=GetFileSize(fd->fd_sys,&LI->HighPart);
	*error_code = (LI->LowPart == 0xFFFFFFFF) ? MPI_ERR_UNKNOWN : MPI_SUCCESS;
	break;

    case ADIO_FCNTL_SET_DISKSPACE:
	/* will be called by one process only */
	/* On file systems with no preallocation function, I have to 
           explicitly write 
           to allocate space. Since there could be holes in the file, 
           I need to read up to the current file size, write it back, 
           and then write beyond that depending on how much 
           preallocation is needed.
           read/write in sizes of no more than ADIOI_PREALLOC_BUFSZ */


	LI = (LARGE_INTEGER *)&curr_fsize;
	LI->LowPart=GetFileSize(fd->fd_sys,&LI->HighPart);
	alloc_size = fcntl_struct->diskspace;
	if(LI->LowPart == 0xFFFFFFFF) {
		*error_code = MPI_ERR_UNKNOWN;
		break;
	}

	size = ADIOI_MIN(curr_fsize, alloc_size);
	
	ntimes = (size + ADIOI_PREALLOC_BUFSZ - 1)/ADIOI_PREALLOC_BUFSZ;
	buf = (char *) ADIOI_Malloc(ADIOI_PREALLOC_BUFSZ);
	done = 0;

	for (i=0; i<ntimes; i++) {
	    len = ADIOI_MIN(size-done, ADIOI_PREALLOC_BUFSZ);
	    ADIO_ReadContig(fd, buf, len,MPI_BYTE, ADIO_EXPLICIT_OFFSET, done,
			    &status, error_code);
		if (*error_code != MPI_SUCCESS) {
#ifdef PRINT_ERR_MSG
			FPRINTF(stderr, "ADIOI_NTFS_Fcntl: To preallocate disk space, ROMIO needs to read the file and write it back, but is unable to read the file. Please give the file read permission and open it with MPI_MODE_RDWR.\n");
			return;
#else
			*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_PREALLOC_PERM,
			      myname, (char *) 0, (char *) 0);
			ADIOI_Error(fd, *error_code, myname);
			return; 
#endif
	    }
	    ADIO_WriteContig(fd, buf, len,MPI_BYTE, ADIO_EXPLICIT_OFFSET, done,
			     &status, error_code);
	    if (*error_code != MPI_SUCCESS) return;
	    done += len;
	}

	if (alloc_size > curr_fsize) {
	    size = alloc_size - curr_fsize;
	    ntimes = (size + ADIOI_PREALLOC_BUFSZ - 1)/ADIOI_PREALLOC_BUFSZ;
	    for (i=0; i<ntimes; i++) {
			memset(buf, 0, ADIOI_PREALLOC_BUFSZ); 
			len = ADIOI_MIN(alloc_size-done, ADIOI_PREALLOC_BUFSZ);
			ADIO_WriteContig(fd, buf, len,MPI_BYTE, ADIO_EXPLICIT_OFFSET, 
					done, &status, error_code);
		if (*error_code != MPI_SUCCESS) return;
		done += len;  
	    }
	}
	ADIOI_Free(buf);
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
	printf("Unknown flag passed to ADIOI_UFS_Fcntl\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
    }
    if(*error_code = MPI_SUCCESS) {
#ifdef PRINT_ERR_MSG
	FPRINTF(stderr, "ADIOI_NTFS_Fcntl: %s\n", ad_ntfs_error(GetLastError()));
#else
    	*error_code = MPIR_Err_setmsg(MPI_ERR_IO, MPIR_ADIO_ERROR,
			      myname, "I/O Error", "I/O Error: %s", ad_ntfs_error(GetLastError()));
	ADIOI_Error(fd, *error_code, myname);	  
    
#endif
    }
}
