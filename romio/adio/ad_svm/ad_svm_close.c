/* 
 *   $Id$    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 *
 *	added by RAY - 23.06.99
 *	last modification: 07.01.2000
 */

#include "ad_svm_fsys.h"


void ADIOI_SVM_Close(ADIO_File fd, int *error_code)
{
    int           files_fd, i=0, perm, old_mask, amode, mynod, nprocs, namelen, ret;
    char          *filename, *proc_name, *seg_name, *name_buf, *buf;
    FileTable     files_local;
    SegmentTable  segments_local;
    SegmentTable  segments_buf_local;
		
	
    /*printf("Entering ADIOI_SVM_Close().\n");*/
    
    MPI_Barrier(fd->comm);

    /* Get copy of data-structures */
    files_local = (FileTable)NULL;
    segments_local = (SegmentTable)NULL;
    segments_buf_local = (SegmentTable)NULL;
    ADIOI_SVM_STRUCT_Copy_files(&files_local);
    ADIOI_SVM_STRUCT_Copy_segments(&files_local,&segments_local);
		
    MPI_Comm_size(fd->comm,&nprocs);
    MPI_Comm_rank(fd->comm,&mynod);

    if (fd->perm == ADIO_PERM_NULL) {
	old_mask = umask(022);
	umask(old_mask);
	perm = old_mask ^ 0666;
    }
    else perm = fd->perm;
    
    /* search file in file-list */
    if (ADIOI_SVM_Lookup_file(fd->filename,&files_local) == MPI_ERR_UNKNOWN) {
       /* file not found => nothing to do... */
       printf("ADIOI_SVM_Close(): File '%s' not opened.\n",fd->filename);
       *error_code = MPI_SUCCESS;
       free(files_local);
       free(segments_local);
       
       return;
    }
    /* file opened => update file-info and remove it from file-list...*/
    /* determine info-file-name */
    filename = (char *)malloc(strlen(AD_SVM_PATH)+strlen(fd->filename)+20);
    sprintf(filename,"%s%s%s%s",AD_SVM_PATH,".",fd->filename,".adio_file_info");
    
    /* open info-file on disk */
    files_fd = open(filename,O_RDWR,perm);

    if (files_fd == -1) {
       /* cannot open info-file => node uninvolved */
       printf("ADIOI_SVM_Close(): Havn't any segment of file '%s'.\n",fd->filename);
       *error_code = MPI_SUCCESS;
       free(files_local);
       free(segments_local);
       
       return;
    }

    /* update file-info on disk */
    ADIOI_SVM_Update_file_info(files_fd,&files_local,&segments_local);
	
    /* close info-file */
    close(files_fd);
    
    /* update filemappings and delete regions... */
    segments_local = files_local->firstseg;
    name_buf = (char *)malloc(MPI_MAX_PROCESSOR_NAME);
    gethostname(name_buf,MPI_MAX_PROCESSOR_NAME);
    namelen = strlen(name_buf);
    proc_name = (char *)malloc(namelen+10);
    strcpy(proc_name,(const char *)name_buf);
    do {
       seg_name = (char *)malloc(strlen(AD_SVM_PATH)+strlen(files_local->name)+16);
       sprintf(seg_name,"%s%s%s%i%s%i%s%i",AD_SVM_PATH,files_local->name,"_seg_",mynod,"_",
	       files_local->nr_of_nodes,"_",(int)(i/files_local->nr_of_nodes));
	  
       if (strcmp(segments_local->node_name,proc_name)) {
          /* remote-segment => wait for update and then delete it */
	  MPI_Barrier(fd->comm);
	  SVMDeleteRegion(segments_local->RegionHandle);
       }
       else {
          /* segment is local => update and delete it */
	  buf = (char *)malloc(segments_local->bytes_used);
	     
	  memcpy((void *)buf,(const void *)segments_local->RegionPointer,segments_local->bytes_used);
	  msync((void *)segments_local->RegionPointer,segments_local->bytes_used,MS_SYNC);
	     
	  free(buf);

	  /*printf("Region %i now up to date.\n",segments_local->RegionHandle);*/
	  MPI_Barrier(fd->comm);
	  SVMDeleteRegion(segments_local->RegionHandle);	  
       }
       /*printf("Region %i deleted.\n",segments_local->RegionHandle);*/

       /* close segment-file on disk */
       close(segments_local->fd);
       
       segments_buf_local = segments_local;
       if (segments_local->next_fileseg != NULL) {
          segments_local = segments_local->next_fileseg;
	  i++;
       }
    } while(segments_buf_local->next_fileseg != NULL);
    
    /*printf("All Regions deleted.\n");*/
    
    /* remove file from file-list  */
    ADIOI_SVM_STRUCT_Remove_file(&files_local,&segments_local);
    
    MPI_Barrier(fd->comm);

#if 0
    /* finalize SVMlib here or where ? */
    _ad_svm_users--;
    if (_ad_svm_users == 0) {
	ADIOI_SVM_Finalize_ad_svm();
    }
#endif

    *error_code = MPI_SUCCESS;

    free(segments_local);
    free(files_local);
    free(name_buf);
    free(proc_name);
    free(filename);
    free(seg_name);
}
