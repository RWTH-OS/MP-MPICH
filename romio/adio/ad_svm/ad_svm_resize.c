/* 
 *   $Id: ad_svm_resize.c,v 1.3 2000/09/11 09:48:05 joachim Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 *
 */

#include "ad_svm_fsys.h"

void ADIOI_SVM_Resize(ADIO_File fd, ADIO_Offset size, int *error_code)
{
    int           i, mynod, nprocs, namelen, old_seg_nr, amode, perm, old_mask;
    ADIO_Offset   size_buf;
    char          *name_buf, *node_name, *seg_name;
    Node_List     nodes_local;
    FileTable     files_local;	
    SegmentTable  segments_local, segments_buf_local, next_seg_buf;	
	

    /*printf("Entering ADIOI_SVM_Resize().\n");*/
    
    files_local = (FileTable)NULL;
    segments_local = (SegmentTable)NULL;
		
    if (ADIOI_SVM_Lookup_fd(fd->fd_sys,&files_local) != MPI_SUCCESS) {
	/* file not found => error */
	printf("ADIOI_SVM_Resize(): File '%s' not found.\n",fd->filename);
	*error_code = MPI_ERR_UNKNOWN;
       
	free(files_local);
	free(segments_local);

	return;
    }
    
    if (size < (ADIO_Offset)0) {
	/* error in size */
	printf("ADIOI_SVM_Resize(): Size for file '%s' < 0.\n",fd->filename);
	*error_code = MPI_ERR_UNKNOWN;
       
	free(files_local);
	free(segments_local);
       
	return;    
    }
    /* Get copy of the file's segments-structure */
    ADIOI_SVM_STRUCT_Copy_segments(&files_local,&segments_local);
    
    MPI_Comm_size(fd->comm, &nprocs);
    MPI_Comm_rank(fd->comm, &mynod);       

    if (size < files_local->size) {
	/* file shrinks... */
	size_buf = ADIOI_SVM_Compute_file_size(files_local->nr_of_nodes*(files_local->segs_per_node-1),
					       &files_local,&segments_local);
	while (size < size_buf) {
	    /* delete segments */
	    files_local->segs_per_node--;
	  
	    /* remove segments */
	    segments_local = files_local->lastseg;
	    seg_name = (char *)malloc(strlen(AD_SVM_PATH)+strlen(files_local->name)+16);
	    for (i=0;i<files_local->nr_of_nodes;i++) {
		/* delete SVMRegion */
		SVMDeleteRegion(segments_local->RegionHandle);
		/* remove local segment from disk */
		if (i == mynod) {
		    sprintf(seg_name,"%s%s%s%i%s%i%s%i", AD_SVM_PATH, files_local->name, "_seg_", mynod,"_",
			    files_local->nr_of_nodes,"_",files_local->segs_per_node);
		    unlink(seg_name);
		    /*printf("Region %i deleted.\n",segments_local->RegionHandle);*/
		}
		segments_local = segments_local->prev_seg;
	    }
	    segments_local->next_fileseg = (SegmentTable)NULL;
	    segments_local->next_seg = segments_buf_local;
	    if (segments_local->next_seg != NULL)
		(segments_local->next_seg)->prev_seg = segments_local;
	    files_local->lastseg = segments_local;
	    size_buf = ADIOI_SVM_Compute_file_size(files_local->nr_of_nodes*(files_local->segs_per_node-1),
						   &files_local,&segments_local);
	  
	    free(seg_name);
	}
	/* update segments' size */
	segments_local = files_local->lastseg;
	i = 1;
	size_buf = ADIOI_SVM_Compute_file_size(files_local->segs_per_node*files_local->nr_of_nodes-i,
					       &files_local,&segments_local);
	while (size_buf >= size) {
	    /* actual segment not used */
	    segments_local->bytes_used = 0;
	    /* go to previous segment */
	    segments_local = segments_local->prev_fileseg;
	    i++;
	    size_buf = ADIOI_SVM_Compute_file_size(files_local->segs_per_node*files_local->nr_of_nodes-i,
						   &files_local,&segments_local);
	}
	if ((size%segments_local->size) == (ADIO_Offset)0)
	    segments_local->bytes_used = segments_local->size;
	else {
	    segments_local->bytes_used = (ADIO_Offset)(size%segments_local->size);
	}
	files_local->size = size;
    }
    else {
	/* expand file */
	if (fd->perm == ADIO_PERM_NULL) {
	    old_mask = umask(022);
	    umask(old_mask);
	    perm = old_mask ^ 0666;
    	}
    	else perm = fd->perm;
    
    	amode = 0;
    	if (fd->access_mode & ADIO_CREATE)
	    amode = amode | O_CREAT;
	if (fd->access_mode & ADIO_RDONLY)
	    amode = amode | O_RDONLY;
    	if (fd->access_mode & ADIO_WRONLY)
	    amode = amode | O_WRONLY;
    	if (fd->access_mode & ADIO_RDWR)
	    amode = amode | O_RDWR;
    	if (fd->access_mode & ADIO_EXCL)
	    amode = amode | O_EXCL;
	
	old_seg_nr = files_local->nr_of_nodes*files_local->segs_per_node;
	size_buf = ADIOI_SVM_Compute_file_size(old_seg_nr,&files_local,&segments_local);
	if (size > size_buf) {
	    /* New segments are needed... */
	    files_local->segs_per_node++;
	  
	    segments_local = files_local->lastseg;
	    size_buf = (ADIO_Offset)((size-size_buf)/nprocs);
	    /* remind pointer to next file's first segment */
	    next_seg_buf = segments_local->next_seg;
	  
	    segments_local->next_fileseg = (SegmentTable)malloc(sizeof(struct SegmentTable_D));
	    segments_local->next_seg = segments_local->next_fileseg;
	    segments_buf_local = segments_local;
	    segments_local = segments_local->next_seg;
	    for (i=0;i<nprocs;i++) {
		/* Get common rank of node and search it in node-list */
		nodes_local = (Node_List)NULL;
		ADIOI_SVM_Get_common_rank(fd->comm,nprocs,i,&nodes_local);
		if (i==nprocs-1)
		    /* last segment */
		    ADIOI_SVM_Add_new_empty_seg(mynod,i,0,1,size_buf,amode,perm,&nodes_local,
						&files_local,&segments_local,&segments_buf_local);
		else
		    /* neither first, nor last segment */
		    ADIOI_SVM_Add_new_empty_seg(mynod,i,0,0,size_buf,amode,perm,&nodes_local,
						&files_local,&segments_local,&segments_buf_local);
		free(nodes_local);
	    }  
	    /* close segment-string */
	    segments_local->next_seg = next_seg_buf;
	    files_local->lastseg = segments_local;
	  
	    /* now: for each segment create a region */
		 
	    /* go to the right segment */
	    for (i=0;i<nprocs-1;i++)
		segments_local = segments_local->prev_fileseg;
		    
	    /* get node-name */
	    name_buf=(char *)malloc(MPI_MAX_PROCESSOR_NAME);
	    gethostname(name_buf,MPI_MAX_PROCESSOR_NAME);
	    namelen = strlen(name_buf);
	    node_name = (char *)malloc(namelen+10);
	    strcpy(node_name,(const char *)name_buf);
	    i = (files_local->segs_per_node-1)*files_local->nr_of_nodes;
  
	    do {
		/*printf("segments_local->node_name: %s - node name: %s\n",segments_local->node_name,node_name);*/
		if (strcmp(segments_local->node_name,node_name)) {
		    /* remote-segment => create reagion with virtual mapping */
		    MPI_Barrier(fd->comm);
		    /* get global region-handle */
		    segments_local->RegionHandle = ADIOI_SVM_Get_region_handle();
		    segments_local->RegionPointer = (void *)SVMCreateRegion(segments_local->RegionHandle,
									    segments_local->size,AD_SVM_PAGE);
		}
		else {
		    /* segment is local => create region with file-mapping */
		    /* first update global region-handle */
		    segments_local->RegionHandle = ADIOI_SVM_Update_region_handle();
		    seg_name = (char *)malloc(strlen(AD_SVM_PATH)+strlen(files_local->name)+16);
		    sprintf(seg_name,"%s%s%s%i%s%i%s%i",AD_SVM_PATH,files_local->name,"_seg_",mynod,
			    "_",files_local->nr_of_nodes,"_",(int)(i/files_local->nr_of_nodes));
		    segments_local->RegionPointer = 
			(void *)SVMCreateFileRegion(segments_local->RegionHandle,
						    (AD_SVM_OFFSET)segments_local->size,
						    AD_SVM_PAGE, seg_name);
		    MPI_Barrier(fd->comm);
		    /*printf("Region %i created.\n",segments_local->RegionHandle);*/
		    free(seg_name);
		}
		segments_buf_local = segments_local;
		if (segments_local->next_fileseg != NULL) {
		    segments_local = segments_local->next_fileseg;
		    i++;
		}
	    } while(segments_buf_local->next_fileseg != NULL);
	  
	    size_buf = ADIOI_SVM_Compute_file_size(files_local->nr_of_nodes*files_local->segs_per_node,
						   &files_local,&segments_local);
	  
	    free(name_buf);
	    free(node_name);
	}
	/* update segment- and file-size */
	segments_local = files_local->firstseg;
	files_local->size = (ADIO_Offset)0;
	while ((segments_local->next_fileseg != NULL)&&((files_local->size+segments_local->size)<=size)) {
	    segments_local->bytes_used = segments_local->size;
	    files_local->size += segments_local->bytes_used;
	    segments_local = segments_local->next_fileseg;
	}
	segments_local->bytes_used = size - files_local->size;
	files_local->size = size;
	while (segments_local->next_fileseg != NULL) {
	    segments_local = segments_local->next_fileseg;
	    segments_local->bytes_used = (ADIO_Offset)0;
	}
    }
    /*printf("New file-size: %i.\n",files_local->size);*/
    /* update data-structures */
    ADIOI_SVM_STRUCT_Update_files(0,&files_local,&segments_local);
    
    free(segments_local);
    free(files_local);
    
    *error_code = MPI_SUCCESS;
}

