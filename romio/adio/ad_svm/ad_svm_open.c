/* 
 *   $Id: ad_svm_open.c,v 1.3 2000/09/11 09:48:04 joachim Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 *
 */

#include "ad_svm_fsys.h"
#include "ad_svm.h"


void ADIOI_SVM_Open(ADIO_File fd, int *error_code)
{
    int files_fd;                    /* Filespecific info-file */
    int conv_fd, seg_fd;	     /* file in conventional file-system */

    int  i, j=0, mynod, nprocs, namelen, result, max_res, len, ret;
    char *filename, *seg_name, *node_name, *name_buf, *buffer;
    ADIO_Offset   size, seg_size, offset;
    Node_List	  nodes_local;
    FileTable     files_local;
    SegmentTable  segments_local;
    SegmentTable  segments_buf_local;
    HD_FileInfo   FileInfo;

#if 0
    /* call the init from here or where ? */
    if (_ad_svm_users == 0) {
	ADIOI_SVM_Init_ad_svm();
	/* XXX what to do if it fails? */
	_ad_svm_users++;
    }
#endif
	
    /*printf("Entering ADIOI_SVM_Open().\n");*/
    MPI_Barrier(fd->comm);
    
    /* Get copy of data-structures */
    files_local = (FileTable)NULL;
    segments_local = (SegmentTable)NULL;
    ADIOI_SVM_STRUCT_Copy_files(&files_local);
    ADIOI_SVM_STRUCT_Copy_segments(&files_local,&segments_local);
		
    /* Lookup file in file-table */
    if (ADIOI_SVM_Lookup_file(fd->filename,&files_local) == MPI_SUCCESS) {
       printf("ADIOI_SVM_Open(): File '%s' already opened.\n",fd->filename);
       fd->fd_sys = files_local->fd;
       if (fd->access_mode == MPI_MODE_EXCL)
          *error_code = MPI_ERR_UNKNOWN;
       else
          *error_code = MPI_SUCCESS;
       
       free(segments_local);
       free(files_local);
       
       return;
    }
    
    MPI_Comm_size(fd->comm, &nprocs);
    MPI_Comm_rank(fd->comm, &mynod);
    
    /* file hasn't been opened, yet => add it to list and open it...*/
    /* build info-file-name */
    filename = (char *)malloc(strlen(AD_SVM_PATH)+strlen(fd->filename)+20);
    sprintf(filename,"%s%s%s%s",AD_SVM_PATH,".",fd->filename,".adio_file_info");
    
    /* open info-file on disk */
    files_fd = open(filename,O_RDWR,0700);
    if (files_fd != -1) {
       j = 1;
       if (fd->access_mode == MPI_MODE_EXCL) {
         *error_code = MPI_ERR_UNKNOWN;
	 return;
       }
    }
    
    size = (ADIO_Offset)0;
    
    result = ADIOI_SVM_Proof_data(fd,files_fd, &size);
    if ((result == 0) || (result == 5)) {
	/* check if data exists in conventional file-system */
	conv_fd = open(fd->filename,O_RDWR,0700);
	if (conv_fd != -1) {
	    /* data exists */
	    size = (ADIO_Offset)lseek(conv_fd, 0, SEEK_END);
	    unlink(filename);
	    result = 5;
	}
    }
    MPI_Allreduce(&result,&max_res,1,MPI_INT,MPI_MAX,fd->comm);
    
    switch (result) {
    case 0:	
	if (max_res == 0) {
	    /* new file => create it */
	    /*printf("new file => create it.\n");*/
	    
	    files_fd = open(filename,O_CREAT | O_RDWR,0700);
	    if (files_fd==-1) {
		printf("ADIOI_SVM_Open(): Cannot create file-info.\n");
		*error_code = MPI_SUCCESS;
		
		free(filename);
		free(segments_local);
		free(files_local);
		
		return;
	    }		   
	    /* create file-info and write it to file */
	    ADIOI_SVM_Write_new_file_info(fd,files_fd,nprocs,
					  (ADIO_Offset)AD_SVM_SEGMENT_SIZE,(ADIO_Offset)0);
	    /* add it to file-list  */
	    fd->fd_sys = ADIOI_SVM_STRUCT_Create_file(nprocs,mynod,fd,1,files_fd,
						      &files_local,&segments_local,size);
	} else {
	    /* file exists, but not on this node */
	    /*printf("No file-data on this node.\n");*/
	    files_fd = open(filename, O_CREAT|O_RDWR, 0700);
	    if (files_fd==-1) {
		printf("ADIOI_SVM_Open(): Cannot create file-info.\n");
		*error_code = MPI_SUCCESS;
		
		free(filename);
		free(segments_local);
		free(files_local);
		
		return;
	    }
	    /* redistribute file to involved nodes */
	    ADIOI_SVM_Redistribute_file(fd, files_fd, filename, max_res, 0);
	    /* add it to file-list  */
	    fd->fd_sys = ADIOI_SVM_STRUCT_Create_file(nprocs, mynod, fd, 1, files_fd,
						      &files_local, &segments_local, size);
	}
	break;
    case 1:	
	/*printf("Not all data available.\n");*/
	files_fd = open(filename, O_CREAT|O_RDWR, 0700);
	/* redistribute file to involved nodes */
	if (files_fd == -1) {
	    printf("ADIOI_SVM_Open(): Cannot create file-info.\n");
	    *error_code = MPI_SUCCESS;
	    
	    free(filename);
	    free(segments_local);
	    free(files_local);
	    
	    return;
	}
	ADIOI_SVM_Redistribute_file(fd, files_fd, filename, max_res, 1);
	/* add it to file-list  */
	fd->fd_sys = ADIOI_SVM_STRUCT_Create_file(nprocs, mynod, fd, 1, files_fd,
						  &files_local, &segments_local, size);
	break;
    case 2: 
	/*printf("File exists, but data's distributed among less nodes.\n");*/
	files_fd = open(filename,O_CREAT | O_RDWR,0700);
	if (files_fd==-1) {
	    printf("ADIOI_SVM_Open(): Cannot create file-info.\n");
	    *error_code = MPI_SUCCESS;
	    
	    free(filename);
	    free(segments_local);
	    free(files_local);
	    
	    return;
	}
	/* redistribute file to involved nodes */
	ADIOI_SVM_Redistribute_file(fd, files_fd, filename, max_res, 1);
	/* add it to file-list  */
	fd->fd_sys = ADIOI_SVM_STRUCT_Create_file(nprocs, mynod, fd, 1, files_fd,
						  &files_local, &segments_local, size);
	break;
    case 3: 
	/*printf("File exists, but data's distributed among more nodes.\n");*/
	files_fd = open(filename,O_CREAT | O_RDWR,0700);
	if (files_fd==-1) {
	    printf("ADIOI_SVM_Open(): Cannot create file-info.\n");
	    *error_code = MPI_SUCCESS;
	    
	    free(filename);
	    free(segments_local);
	    free(files_local);
	    
	    return;
	}
	/* redistribute file to involved nodes */
	ADIOI_SVM_Redistribute_file(fd, files_fd, filename, max_res, 1);
	/* add it to file-list  */
	fd->fd_sys = ADIOI_SVM_STRUCT_Create_file(nprocs,mynod,fd,1,files_fd,
						  &files_local,&segments_local,size);
	break;
    case 4: 
	/*printf("File exists and data optimally distributed.\n");*/
	/* add file to file-list  */
	files_fd = open(filename,O_CREAT | O_RDWR,0700);
	if (files_fd==-1) {
	    printf("ADIOI_SVM_Open(): Cannot create file-info.\n");
	    *error_code = MPI_SUCCESS;
	    
	    free(filename);
	    free(segments_local);
	    free(files_local);
	    
	    return;
	}
	fd->fd_sys = ADIOI_SVM_STRUCT_Create_file(nprocs,mynod,fd,0,files_fd,
						  &files_local,&segments_local,size);
	break;
    case 5:	
	/* data exists in conventional file-system => convert it */
	/*printf("Data exists in conventional file-system => convert it.\n");*/
	size = (ADIO_Offset)lseek(conv_fd,0,SEEK_END);
	lseek(conv_fd,0,SEEK_SET);
	
	buffer = (char *)malloc(strlen(fd->filename));
	buffer = ADIOI_SVM_Basename(fd->filename, &len);
	free(fd->filename);
	fd->filename = (char *)malloc(len);
	strcpy(fd->filename,buffer);
	
	/*printf("fd->filename: %s.\n",fd->filename);*/
	free(filename);
	filename = (char *)malloc(strlen(AD_SVM_PATH)+strlen(fd->filename)+20);
	sprintf(filename,"%s%s%s%s",AD_SVM_PATH,".",fd->filename,".adio_file_info");
	
	/* open new info-file */
	files_fd = open(filename,O_CREAT | O_RDWR,0700);
	if (files_fd==-1) {
	    printf("ADIOI_SVM_Open(): Cannot create file-info.\n");
	    *error_code = MPI_SUCCESS;
	    
	    free(filename);
	    free(segments_local);
	    free(files_local);
	    
	    return;
	}
	/* create file-info and write it to file */
	seg_size = (ADIO_Offset)(size/nprocs);
	if ((ADIO_Offset)(seg_size%AD_SVM_SEGMENT_SIZE) != (ADIO_Offset)0)
	    seg_size = (ADIO_Offset)((seg_size/AD_SVM_SEGMENT_SIZE+1)*AD_SVM_SEGMENT_SIZE);
	
	ADIOI_SVM_Write_new_file_info(fd,files_fd,nprocs,(ADIO_Offset)seg_size,(ADIO_Offset)size);
	
	/* now create parallel file-segments */
	nodes_local = (Node_List)NULL;
	offset = (ADIO_Offset)0;
	buffer = (char *)malloc(size);
	read(conv_fd,buffer,size);
	/*printf("read %i bytes.\n",(int)size);*/
	
	for (i=0;i<nprocs;i++) {
	    /* Get common rank of node and search it in node-list */
	    ADIOI_SVM_Get_common_rank(fd->comm,nprocs,i,&nodes_local);
	    /* get node-name */
	    name_buf=(char *)malloc(MPI_MAX_PROCESSOR_NAME);
	    gethostname(name_buf,MPI_MAX_PROCESSOR_NAME);
	    namelen = strlen(name_buf);
	    node_name = (char *)malloc(namelen+10);
	    strcpy(node_name,(const char *)name_buf);
	    
	    if (!strcmp(nodes_local->name,node_name)) {
		/* => segment local => copy data from conventional file to segment */
		seg_name = (char *)malloc(strlen(AD_SVM_PATH)+strlen(fd->filename)+16);
		sprintf(seg_name,"%s%s%s%i%s%i%s%i",AD_SVM_PATH,fd->filename,"_seg_",i,"_",nprocs,"_",0);
		
		seg_fd = open(seg_name,O_CREAT|O_RDWR,0700);
		ftruncate(seg_fd,seg_size);
		lseek(seg_fd,0,SEEK_SET);
		if ((size-offset) < seg_size) {
		    write(seg_fd,buffer+offset,(size-offset));
		    /*printf("wrote %i bytes from %i to %i.\n",(int)(size-offset),(int)offset,(int)(size));*/
		}
		else {
		    write(seg_fd,buffer+offset,seg_size);
		    /*printf("wrote %i bytes from %i to %i.\n",(int)seg_size,(int)offset,(int)(offset+seg_size));*/
		}
		free(buffer);
		close(seg_fd);
	    }
	    else
		offset += seg_size;	   
	}
	/* add it to file-list  */
	fd->fd_sys = ADIOI_SVM_STRUCT_Create_file(nprocs, mynod, fd, 1, files_fd,
						  &files_local, &segments_local, size);
	if ((j)&(!mynod))
	    unlink(fd->filename);
	break;
    default:
	printf("ADIOI_SVM_Proof_data() failed.\n");
	*error_code = MPI_ERR_UNKNOWN;
	return;
    }
    /* close InfoFile */
    close(files_fd);

    /* for each segment create a region */
    /* go to file's first segment */
    if (fd->fd_sys != -1) {
       segments_local = files_local->firstseg;
       name_buf       = (char *)malloc(MPI_MAX_PROCESSOR_NAME);
       gethostname(name_buf, MPI_MAX_PROCESSOR_NAME);
       namelen   = strlen(name_buf);
       node_name = (char *)malloc(namelen+10);
       strcpy(node_name, (const char *)name_buf);

       i=0;  
       do {
          /*printf("segments_local->node_name: %s - node name: %s\n",segments_local->node_name,node_name);*/
	  if (strcmp(segments_local->node_name, node_name)) {
             /* remote-segment => create region with virtual mapping */
	     MPI_Barrier(fd->comm);
	     /* get global region-handle */
	     segments_local->RegionHandle = ADIOI_SVM_Get_region_handle();
	     segments_local->RegionPointer = (void *)SVMCreateRegion(segments_local->RegionHandle,
								     segments_local->size,AD_SVM_PAGE);
	     /*printf("Remote-region %i created.\n",segments_local->RegionHandle);*/
          }
          else {
             /* segment is local => create region with file-mapping */
	     /* first update global region-handle */
	     segments_local->RegionHandle = ADIOI_SVM_Update_region_handle();
    	     seg_name = (char *)malloc(strlen(AD_SVM_PATH)+strlen(files_local->name)+16);
    	     sprintf(seg_name,"%s%s%s%i%s%i%s%i",AD_SVM_PATH,files_local->name,"_seg_",
		     mynod,"_",files_local->nr_of_nodes,"_",(int)(i/files_local->nr_of_nodes));

	     segments_local->RegionPointer 
		 = (void *)SVMCreateFileRegion(segments_local->RegionHandle,
					       (AD_SVM_OFFSET)segments_local->size,
					       AD_SVM_PAGE, seg_name);
	     /* Handle for next region*/
	     MPI_Barrier(fd->comm);
	     /*printf("Region %i created.\n",segments_local->RegionHandle);*/
          }
          segments_buf_local = segments_local;
          if (segments_local->next_fileseg != NULL) {
             segments_local = segments_local->next_fileseg;
	     i++;
          }
       } while(segments_buf_local->next_fileseg != NULL);
    }

    if ((fd->fd_sys != -1) && (fd->access_mode & ADIO_APPEND)) {
        /* changed by RAY */ 
	fd->fp_ind = fd->fp_sys_posn = files_local->size;
    }
    ADIOI_SVM_STRUCT_Update_files(0,&files_local,&segments_local);
    
    MPI_Barrier(fd->comm);
    /* the barrier ensures that no process races ahead and modifies
       the file size before all processes have opened the file. */
    /*printf("Leaving ADIOI_SVM_Open().\n");*/

    *error_code = (fd->fd_sys == -1) ? MPI_ERR_UNKNOWN : MPI_SUCCESS;

    free(segments_local);
    free(files_local);
    free(name_buf);
    free(node_name);
    free(filename);
    free(seg_name);
}
