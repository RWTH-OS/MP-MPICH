/* $Id$
 *	ad_smi_struct.c
 *
 *	routines to synchronize data-structure-accesses
 *
 */
 
#include "ad_svm_struct.h"


/************************************/
/* Routines for the nodes-structure */

/* Initialize nodes-structure */
int ADIOI_SVM_STRUCT_Init_nodes(int nprocs, int mynod, int namelen, char *proc_name)
{
    Node_List  nodes_buf;
    char       *node_name;
    int        nodelen, i;
    
    extern Node_List	nodes;
    
    
    nodes = (Node_List)malloc(sizeof(struct Node_List_D));
    nodes->prev = (Node_List)NULL;
    /*printf("nodes->prev: %x\n",nodes->prev);*/
    
    for (i=0;i<nprocs;i++) {
	/*printf("nodes: %x\n",nodes);*/
	if (i == mynod) {
	    nodelen = namelen;
	    node_name = (char *)malloc(nodelen+10);
	    strcpy(node_name,(const char *)proc_name);
	    MPI_Bcast(&nodelen,1,MPI_INT,i,MPI_COMM_WORLD);
	    MPI_Bcast(node_name,nodelen+10,MPI_CHAR,i,MPI_COMM_WORLD);
	}
	else {
	    MPI_Bcast(&nodelen,1,MPI_INT,i,MPI_COMM_WORLD);
	    node_name = (char *)malloc(nodelen+10);
	    MPI_Bcast(node_name,nodelen+10,MPI_CHAR,i,MPI_COMM_WORLD);
	}
	/*printf("Process %i's namelen %i\n",i,nodelen);*/
	strcpy(nodes->name,(const char *)node_name);
	nodes->rank = i;
       
	/*printf("Process %i on '%s'\n",i,nodes->name);*/
       
	nodes_buf = nodes;
	if (i<(nprocs-1)) {
	    nodes->next = (Node_List)malloc(sizeof(struct Node_List_D));
	    /*printf("nodes->next: %x\n",nodes->next);*/
	    nodes = nodes->next;
	    nodes->prev = nodes_buf;
	    /*printf("nodes->prev: %x\n",nodes->prev);*/
	}       
    }
    nodes->next = (Node_List)NULL;
    /*printf("nodes->next: %x\n",nodes->next);*/
    
    free(node_name);
    
    return 0;
}

/* De-initialize nodes-structure */
int ADIOI_SVM_STRUCT_Free_nodes()
{
    Node_List  nodes_buf;
    
    extern Node_List	nodes;

    
    if (nodes != NULL) {
	while (nodes->prev != NULL)
	    nodes = nodes->prev;
	while (nodes->next != NULL) {
	    nodes_buf = nodes->next;
	    free(nodes);
	    nodes = nodes_buf;
	}
    }
    free(nodes);

    return 0;
}

/* Frees the local copy of the nodes-structure */
int ADIOI_SVM_STRUCT_Free_nodes_copy(Node_List *p_nodes_local)
{
    Node_List  nodes_buf_local;
   
   
    /* start with first node */
    while ((*p_nodes_local)->prev != NULL)
	*p_nodes_local = (*p_nodes_local)->prev;
      
    while (*p_nodes_local != NULL) {
	nodes_buf_local = *p_nodes_local;
	*p_nodes_local = (*p_nodes_local)->next;
	free(nodes_buf_local);
    }
    *p_nodes_local = (Node_List)malloc(sizeof(struct Node_List_D));
    (*p_nodes_local)->prev = (Node_List)NULL;
    (*p_nodes_local)->next = (Node_List)NULL;
   
    return 0;
}

/* Creates a local copy of nodes-structure */
int ADIOI_SVM_STRUCT_Copy_nodes(Node_List *p_nodes_local)
{
    Node_List nodes_buf;
    int  ret;
    
    extern Node_List		nodes;
    extern pthread_mutex_t 	NodesLock;

   
    if (*p_nodes_local == NULL) {
	*p_nodes_local = (Node_List)malloc(sizeof(struct Node_List_D));
	(*p_nodes_local)->prev = (Node_List)NULL;
	(*p_nodes_local)->next = (Node_List)NULL;
    }
    else
	ADIOI_SVM_STRUCT_Free_nodes_copy(p_nodes_local);
   
    /**************************/   
    /* Enter critical section */
    ret = pthread_mutex_trylock(&NodesLock);
    if (ret)
	/* => error */
	do {
	    ret = pthread_mutex_lock(&NodesLock);
	    /* is 100 a good value ? */
	    usleep(113);
	} while (ret);
    /* locked... */
   
    /* return to head of node-list */
    while (nodes->prev != NULL)
	nodes = nodes->prev;
      
    **p_nodes_local = *nodes;
    (*p_nodes_local)->prev = (Node_List)NULL;
   
    while (nodes->next != NULL) {
	nodes = nodes->next;
	(*p_nodes_local)->next = (Node_List)malloc(sizeof(struct Node_List_D));
	nodes_buf = *p_nodes_local;
	*p_nodes_local = (*p_nodes_local)->next;
	**p_nodes_local = *nodes;
	(*p_nodes_local)->prev = nodes_buf;
    } 
    (*p_nodes_local)->next = (Node_List)NULL;
   
    /* unlock... */
    pthread_mutex_unlock(&NodesLock);
    /* Left critical section */
    /**************************/   
   
    return 0;
}


/************************************/
/* Routines for files-structure */

/* Initialize files-structure */
int ADIOI_SVM_STRUCT_Init_files()
{
    extern FileTable	files,
	files_buf;
			
			
    files = (FileTable)NULL;
    files_buf = (FileTable)NULL;
    
    return 0;
}

/* De-initialize files-structure */
int ADIOI_SVM_STRUCT_Free_files()
{
    extern FileTable	files,
	files_buf;
			

    if (files != NULL) {
	while (files->prevfile != NULL)
	    files = files->prevfile;
	while (files->nextfile != NULL) {
	    files_buf = files->nextfile;
	    free(files);
	    files = files_buf;
	}
    }
    free(files);
    free(files_buf);
    
    return 0;
}

/* Frees the local copy of the files-structure */
int ADIOI_SVM_STRUCT_Free_files_copy(FileTable *p_files_local)
{
    FileTable  files_buf_local;
   
   
    if (*p_files_local == NULL)
	/* nothing to do... */
	return 0;
      
    /* start with first file */
    while ((*p_files_local)->prevfile != NULL)
	*p_files_local = (*p_files_local)->prevfile;
      
    while (*p_files_local != NULL) {
	files_buf_local = *p_files_local;
	*p_files_local = (*p_files_local)->nextfile;
	free(files_buf_local);
    }
    *p_files_local = (FileTable)malloc(sizeof(struct FileTable_D));
    (*p_files_local)->prevfile = (FileTable)NULL;
    (*p_files_local)->nextfile = (FileTable)NULL;
   
    return 0;
}

/* Creates a local copy of files-structure */
int ADIOI_SVM_STRUCT_Copy_files(FileTable *p_files_local)
{
    int  ret;
   
    extern FileTable		files,
	files_buf;
    extern pthread_mutex_t 	FilesLock;
			
			
    if (*p_files_local == NULL) {
	*p_files_local = (FileTable)malloc(sizeof(struct FileTable_D));
	(*p_files_local)->prevfile = (FileTable)NULL;
	(*p_files_local)->nextfile = (FileTable)NULL;
    }
    else
	ADIOI_SVM_STRUCT_Free_files_copy(p_files_local);
      
    /**************************/   
    ret = pthread_mutex_trylock(&FilesLock);
    if (ret)
	/* => error */
	do {
	    ret = pthread_mutex_lock(&FilesLock);
	    /* is 100 a good value ? */
	    usleep(113);
	} while (ret);
    /* locked... */
   
    if (files != NULL) {
	/* return to head of file-list */
	while (files->prevfile != NULL)
	    files = files->prevfile;
	 
	**p_files_local = *files;
	(*p_files_local)->self = *p_files_local;
	(*p_files_local)->prevfile = (FileTable)NULL;
	(*p_files_local)->firstseg = (SegmentTable)NULL;
	(*p_files_local)->lastseg = (SegmentTable)NULL;
	/* copy elements */
	while (files->nextfile != NULL) {
	    files = files->nextfile;
	    (*p_files_local)->nextfile = (FileTable)malloc(sizeof(struct FileTable_D));
	    files_buf = *p_files_local;
	    *p_files_local = (*p_files_local)->nextfile;
	    **p_files_local = *files;
	    (*p_files_local)->self = *p_files_local;
	    (*p_files_local)->prevfile = files_buf;
	    (*p_files_local)->firstseg = (SegmentTable)NULL;
	    (*p_files_local)->lastseg = (SegmentTable)NULL;
	} 
	(*p_files_local)->nextfile = (FileTable)NULL;
    }
    else {
	free(*p_files_local);
	*p_files_local = (FileTable)NULL;
    }
   
    /* unlock... */
    pthread_mutex_unlock(&FilesLock);
    /* Left critical section */
    /**************************/   
   
    return 0;
}

/* Update files-structure */
int ADIOI_SVM_STRUCT_Update_files(int is_new, FileTable *p_files_local, SegmentTable *p_segments_local)
{
    int           ret;
    SegmentTable  segments_buf_local;
   
    extern FileTable		files,
	files_buf;
    extern SegmentTable		segments,
	segments_buf;
    extern pthread_mutex_t 	FilesLock,
	SegmentsLock;
			   
    /**************************/   
    /* Enter critical section */
    ret = pthread_mutex_trylock(&FilesLock);
    if (ret)
	/* => error */
	do {
	    ret = pthread_mutex_lock(&FilesLock);
	    /* is 100 a good value ? */
	    usleep(113);
	} while (ret);
    /* locked... */
   
    if (files != NULL) {
	/* => at least one file has already been opened */
	/* return to head of file-list */
	while (files->prevfile != NULL)
	    files = files->prevfile;
	/* search in file-list */
	while ((files->fd != (*p_files_local)->fd) && (files->nextfile != NULL))
	    files = files->nextfile;
	if (files->fd != (*p_files_local)->fd)
	    /* file not found */
	    return MPI_ERR_UNKNOWN;
    }
    else {
	files = (FileTable)malloc(sizeof(struct FileTable_D));
	files->fd = (*p_files_local)->fd;
	files->prevfile = (FileTable)NULL;
	files->nextfile = (FileTable)NULL;
	files->self = files;
    }
    /* got file => update data */
    files->size = (*p_files_local)->size;
    files->nr_of_nodes = (*p_files_local)->nr_of_nodes;
    files->segs_per_node = (*p_files_local)->segs_per_node;
    files->requests = (*p_files_local)->requests;
    files->request_size = (*p_files_local)->request_size;
   
   
    /**************************/   
    ret = pthread_mutex_trylock(&SegmentsLock);
    if (ret)
	/* => error */
	do {
	    ret = pthread_mutex_lock(&SegmentsLock);
	    /* is 100 a good value ? */
	    usleep(113);
	} while (ret);
    /* locked... */
   

    /* start with first segment */
    while ((*p_segments_local)->prev_fileseg != NULL)
	*p_segments_local = (*p_segments_local)->prev_fileseg;

    if (is_new) {
	if (segments != NULL) {
	    /* => at least one segment is opened */
	    /* add segments at end of segment-list */
	    while(segments->next_seg != NULL)
		segments = segments->next_seg;
	    segments->next_seg = (SegmentTable)malloc(sizeof(struct SegmentTable_D));
	    (segments->next_seg)->prev_seg = segments;
	    segments = segments->next_seg;
	}
	else {
	    /* => first segment that will be opened */
	    segments = (SegmentTable)malloc(sizeof(struct SegmentTable_D));
	    segments->prev_seg = (SegmentTable)NULL;
	    segments->next_seg = (SegmentTable)NULL;
	}
	segments->prev_fileseg = (SegmentTable)NULL;
	segments->next_fileseg = (SegmentTable)NULL;
	segments->self = segments;
	files->firstseg = segments;
    }
    else {
	segments = files->firstseg;
    }

    do  {
	/* copy data */
	segments->fd = (*p_segments_local)->fd;
	segments->RegionPointer = (*p_segments_local)->RegionPointer;
	segments->RegionHandle = (*p_segments_local)->RegionHandle;
	segments->size = (*p_segments_local)->size;
	segments->bytes_used = (*p_segments_local)->bytes_used;
	strcpy((char *)segments->node_name,(const char *)(*p_segments_local)->node_name);
      
	/* organize data-structure */
	segments_buf_local = *p_segments_local;
	if ((*p_segments_local)->next_fileseg != NULL) {
	    *p_segments_local = (*p_segments_local)->next_fileseg;
	    if (segments->next_fileseg == NULL) {
		/* => add new entry to segment-structure */
		segments->next_fileseg = (SegmentTable)malloc(sizeof(struct SegmentTable_D));
		(segments->next_fileseg)->prev_fileseg = segments;
		segments_buf = segments->next_seg;
		segments->next_seg = segments->next_fileseg;
		segments = segments->next_fileseg;
		segments->self = segments;
		segments->prev_seg = segments->prev_fileseg;
		segments->next_seg = segments_buf;
		segments->next_fileseg = (SegmentTable)NULL;
	    }
	    else
		segments = segments->next_fileseg;
	}
    } while (segments_buf_local->next_fileseg != NULL);
   
    files->lastseg = segments;

    /* copied segment-structure - remove file's remaining segments */
    while (segments->next_fileseg != NULL) {
	segments_buf = segments->next_fileseg;
	segments->next_fileseg = (segments->next_fileseg)->next_fileseg;
	segments->next_seg = (segments->next_seg)->next_seg;
	free(segments_buf);
    }
    /* segments-structure up to date...*/
   
    /* unlock... */
    pthread_mutex_unlock(&SegmentsLock);
    pthread_mutex_unlock(&FilesLock);
    /* Left critical sections */
    /**************************/   
    /**************************/   
   
    return 0;
}


/* Create new entry in files-structure */
int ADIOI_SVM_STRUCT_Create_file(int nprocs, int mynod, ADIO_File file, int is_new, int fd, FileTable *p_files_local, SegmentTable *p_segments_local, ADIO_Offset size)
{
    Node_List    nodes_local;
    SegmentTable segments_buf_local;
    HD_FileInfo  FileInfo;
    int          i, node_nr, amode, perm, old_mask, ret;
	
    extern FileTable        files,
	files_buf;
    extern SegmentTable	segments,
	segments_buf;
    extern int 		*FileHandles,
	NextFileHandle;
    extern unsigned long 	FileHandleLock;
    extern pthread_mutex_t	FilesLock;
	

    segments_buf_local = (SegmentTable)NULL;
    /**************************/   
    /* Enter critical section */
    ret = pthread_mutex_trylock(&FilesLock);
    if (ret)
	/* => error */
	do {
	    ret = pthread_mutex_lock(&FilesLock);
	    /* is 100 a good value ? */
	    usleep(113);
	} while (ret);
    /* locked... */
   
    if (files != NULL) {
	/* => at least one file is opened */
	/* add file at end of file-list */
	while(files->nextfile != NULL)
	    files = files->nextfile;
	   
	/* remind last segment in list for first new segment */
	segments_buf_local = files->lastseg;
	   
	files->nextfile = (FileTable)malloc(sizeof(struct FileTable_D));
	files_buf = files;
	files = files->nextfile;
	files->prevfile = files_buf;
    }
    else {
	/* first file that will be opened */
	files = (FileTable)malloc(sizeof(struct FileTable_D));
	files->prevfile = (FileTable)NULL;
    }
    files->requests = 0;
    files->request_size = (ADIO_Offset)0;
    files->nextfile = (FileTable)NULL;
    strcpy((char *)files->name,(const char *)file->filename);
    files->size = (ADIO_Offset)0;
    files->self = files;
    files->nr_of_nodes = nprocs;
    files->segs_per_node = 1;
    files->firstseg = (SegmentTable)NULL;
    files->lastseg = (SegmentTable)NULL;

    /* update global file-handle (only one process) */
    if (!mynod) {

	/**************************/   
	/* Enter critical section */
	/*printf("Aquire Lock...\n");*/
	SVMAcquireSyncObject(FileHandleLock,0);
	/* locked... */
   
	/*printf("Updating global file-handle.\n");fflush(stdout);*/

	memcpy((int *)&NextFileHandle,(const int *)FileHandles,sizeof(int));
	files->fd = NextFileHandle++;
	memcpy((int *)FileHandles,(const int *)&NextFileHandle,sizeof(int));
	   
	/* unlock... */
	/*printf("Release Lock...\n");*/
	SVMReleaseSyncObject(FileHandleLock);
	/* Left critical section */
	/*************************/   
	   
	ret = files->fd;
	   
	if (*p_files_local == NULL) {
	    *p_files_local = (FileTable)malloc(sizeof(struct FileTable_D));
	}
	else
	    ADIOI_SVM_STRUCT_Free_files_copy(p_files_local);
      
	**p_files_local = *files;
	
	/* unlock... */
	pthread_mutex_unlock(&FilesLock);
	/* Left critical section */
	/*************************/   
	   
	MPI_Barrier(file->comm);
	/*printf("NextFileHandle == %i\n",NextFileHandle);*/
    }
    else {
	MPI_Barrier(file->comm);
	   
	/**************************/   
	/* Enter critical section */
	/*printf("Aquire Lock...\n");*/
	SVMAcquireSyncObject(FileHandleLock,0);
	/* locked... */
   
	/*printf("Get global file-handle.\n");fflush(stdout);*/
	   
	memcpy((int *)&NextFileHandle,(const int *)FileHandles,sizeof(int));
	files->fd = NextFileHandle-1;
	/*printf("Nextfilehandle == %i.\n",NextFileHandle);fflush(stdout);*/
	   
	/* unlock... */
	/*printf("Release Lock...\n");*/
	SVMReleaseSyncObject(FileHandleLock);
	/* Left critical section */
	/*************************/   
	   
	ret = files->fd;
	   
	if (*p_files_local == NULL)
	    *p_files_local = (FileTable)malloc(sizeof(struct FileTable_D));
	else
	    ADIOI_SVM_STRUCT_Free_files_copy(p_files_local);
	      
	**p_files_local = *files;
	
	/* unlock... */
	pthread_mutex_unlock(&FilesLock);
	/* Left critical section */
	/*************************/   
    }
   
	
    if (file->perm == ADIO_PERM_NULL) {
	old_mask = umask(022);
	umask(old_mask);
	perm = old_mask ^ 0666;
    }
    else perm = file->perm;
    
    amode = 0;
    if (file->access_mode & ADIO_CREATE)
	amode = amode | O_CREAT;
    if (file->access_mode & ADIO_RDONLY)
	amode = amode | O_RDONLY;
    if (file->access_mode & ADIO_WRONLY)
	amode = amode | O_WRONLY;
    if (file->access_mode & ADIO_RDWR)
	amode = amode | O_RDWR;
    if (file->access_mode & ADIO_EXCL)
	amode = amode | O_EXCL;
	
    /********************************/
    /* add segments to segment-list */

    nodes_local = (Node_List)NULL;
    if (is_new) {
	/* file doesn't exist on disk => "create" it */
	(*p_files_local)->nr_of_nodes = nprocs;
	(*p_files_local)->segs_per_node = 1;
	for (i=0;i<nprocs;i++) {
	    /* Get common rank of node and search it in node-list */
	    ADIOI_SVM_Get_common_rank(file->comm,nprocs,i,&nodes_local);
	      
	    /*printf("node-name: %s\n",nodes_local->name);*/
	      
	    /* Determine segment-size */
	    if ((ADIO_Offset)(size/nprocs) < (ADIO_Offset)AD_SVM_SEGMENT_SIZE)
		size = (ADIO_Offset)(AD_SVM_SEGMENT_SIZE*nprocs);
	    if (size%((ADIO_Offset)(AD_SVM_SEGMENT_SIZE*nprocs)) != 0)
		size = (ADIO_Offset)((size%AD_SVM_SEGMENT_SIZE+1) * AD_SVM_SEGMENT_SIZE);
		 
	    if (i==0)
		if (i==nprocs-1)
		    /* first and last segment */
		    ADIOI_SVM_Add_new_empty_seg(mynod,i,1,1,(ADIO_Offset)(size/nprocs),
						amode,perm,&nodes_local,p_files_local,
						p_segments_local,&segments_buf_local);
		else
		    /* first, but not last segment */
		    ADIOI_SVM_Add_new_empty_seg(mynod,i,1,0,(ADIO_Offset)(size/nprocs),
						amode,perm,&nodes_local,p_files_local,
						p_segments_local,&segments_buf_local);
	    else
		if (i==nprocs-1)
		    /* last, but not first segment */
		    ADIOI_SVM_Add_new_empty_seg(mynod,i,0,1,(ADIO_Offset)(size/nprocs),
						amode,perm,&nodes_local,p_files_local,
						p_segments_local,&segments_buf_local);
		else
		    /* neither first, nor last segment */
		    ADIOI_SVM_Add_new_empty_seg(mynod,i,0,0,(ADIO_Offset)(size/nprocs),
						amode,perm,&nodes_local,p_files_local,
						p_segments_local,&segments_buf_local);
	}
    }
    else {
	/* file already exists on disk => fetch info from info-file */
	FileInfo = (HD_FileInfo)malloc(sizeof(struct HD_FileInfo_D));
	lseek(fd,0,SEEK_SET);
	do {
	    read(fd,FileInfo,sizeof(struct HD_FileInfo_D));
	    (*p_files_local)->nr_of_nodes = (int)(FileInfo->nr_of_segs/FileInfo->segs_per_node);
	    (*p_files_local)->segs_per_node = FileInfo->segs_per_node;
	    (*p_files_local)->size += FileInfo->bytes_used;
	    (*p_files_local)->requests = FileInfo->requests;
	    (*p_files_local)->request_size = FileInfo->request_size;
	    /* lookup node in node-list */
	    ADIOI_SVM_Lookup_node(FileInfo->node,&nodes_local);
	    if (FileInfo->seg_nr==0)
		if (FileInfo->seg_nr==FileInfo->nr_of_segs-1)
		    /* first and last segment */
		    ADIOI_SVM_Add_empty_seg(FileInfo->node,file->filename,
					    (FileInfo->seg_nr%(FileInfo->nr_of_segs/FileInfo->segs_per_node)),
					    (FileInfo->nr_of_segs/FileInfo->segs_per_node),
					    (FileInfo->seg_nr/(FileInfo->nr_of_segs/FileInfo->segs_per_node)),1,1,
					    FileInfo->bytes_used,FileInfo->size,amode,perm,&nodes_local,
					    p_files_local,p_segments_local,&segments_buf_local);
		else
		    /* first, but not last segment */
		    ADIOI_SVM_Add_empty_seg(FileInfo->node,file->filename,
					    (FileInfo->seg_nr%(FileInfo->nr_of_segs/FileInfo->segs_per_node)),
					    (FileInfo->nr_of_segs/FileInfo->segs_per_node),
					    (FileInfo->seg_nr/(FileInfo->nr_of_segs/FileInfo->segs_per_node)),1,0,
					    FileInfo->bytes_used,FileInfo->size,amode,perm,&nodes_local,
					    p_files_local,p_segments_local,&segments_buf_local);
	    else
		if (FileInfo->seg_nr==FileInfo->nr_of_segs-1)
		    /* last, but not first segment */
		    ADIOI_SVM_Add_empty_seg(FileInfo->node,file->filename,
					    (FileInfo->seg_nr%(FileInfo->nr_of_segs/FileInfo->segs_per_node)),
					    (FileInfo->nr_of_segs/FileInfo->segs_per_node),
					    (FileInfo->seg_nr/(FileInfo->nr_of_segs/FileInfo->segs_per_node)),0,1,
					    FileInfo->bytes_used,FileInfo->size,amode,perm,&nodes_local,
					    p_files_local,p_segments_local,&segments_buf_local);
		else
		    /* neither first, nor last segment */
		    ADIOI_SVM_Add_empty_seg(FileInfo->node,file->filename,
					    (FileInfo->seg_nr%(FileInfo->nr_of_segs/FileInfo->segs_per_node)),
					    (FileInfo->nr_of_segs/FileInfo->segs_per_node),
					    (FileInfo->seg_nr/(FileInfo->nr_of_segs/FileInfo->segs_per_node)),0,0,
					    FileInfo->bytes_used,FileInfo->size,amode,perm,&nodes_local,
					    p_files_local,p_segments_local,&segments_buf_local);
	      
	} while (FileInfo->seg_nr<FileInfo->nr_of_segs-1);
	   
	free(FileInfo);
    }
	
    /* end add segments to segment-list */
    /************************************/
	
    ADIOI_SVM_STRUCT_Update_files(1,p_files_local,p_segments_local);
	
    free(nodes_local);
	
    return ret;
}

/* Remove entry from files-structure */
int ADIOI_SVM_STRUCT_Remove_file(FileTable *p_files_local, SegmentTable *p_segments_local)
{
    int  ret;
   
    FileTable     files_buf_local;
    SegmentTable  segments_buf_local, next_seg_buf;
   
    extern FileTable     	files,
	files_buf;
    extern SegmentTable		segments,
	segments_buf;
    extern pthread_mutex_t 	FilesLock,
	SegmentsLock;
	

    /**************************/   
    /* Enter critical sections */
    ret = pthread_mutex_trylock(&FilesLock);
    if (ret)
	/* => error */
	do {
	    ret = pthread_mutex_lock(&FilesLock);
	    /* is 100 a good value ? */
	    usleep(113);
	} while (ret);
    ret = pthread_mutex_trylock(&SegmentsLock);
    if (ret)
	/* => error */
	do {
	    ret = pthread_mutex_lock(&SegmentsLock);
	    /* is 100 a good value ? */
	    usleep(113);
	} while (ret);
    /* locked... */
   
    /* first update segment-list */
   
    while (files->prevfile != NULL)
	files = files->prevfile;
    while ((files->fd != (*p_files_local)->fd) && (files->nextfile != NULL))
	files = files->nextfile;
      
    if (files->fd != (*p_files_local)->fd) {
	printf("ADIOI_SVM_STRUCT_Remove_file(): Couldn't find file.\n");
	return -1;
    }

    if ((files->firstseg)->prev_seg != NULL) {
	segments = (files->firstseg)->prev_seg;
	segments->next_seg = (files->lastseg)->next_seg;
	if ((files->lastseg)->next_seg != NULL) {
	    segments = (files->lastseg)->next_seg;
	    segments->prev_seg = (files->firstseg)->prev_seg;
	}
    }
    else {
	if ((files->lastseg)->next_seg != NULL) {
	    segments = (files->lastseg)->next_seg;
	    segments->prev_seg = (files->firstseg)->prev_seg;
	}
	else
	    segments = (SegmentTable)NULL;
    }
    /*printf("Segments up to date: %x\n",segments);*/
    segments = (SegmentTable)NULL;
   
	
    /* deallocate memory for deleted segments */
    segments_buf = files->firstseg;
    do {
	next_seg_buf = segments_buf->next_fileseg;
	free(segments_buf);
	segments_buf = next_seg_buf;
    } while(segments_buf != NULL);

    /* update file-list */
    files_buf = (FileTable)NULL;
    if (files->prevfile != NULL) {
	files_buf = files->prevfile;
	(files->prevfile)->nextfile = files->nextfile;
    }
    if (files->nextfile != NULL) {
	if (files_buf == NULL)
	    files_buf = files->nextfile;
	(files->nextfile)->prevfile = files->prevfile;
    }
    /* reinitialize file-list */
    free(files);
    files = files_buf;
    /*printf("Files up to date: %x\n",files);*/
   
    /* unlock... */
    pthread_mutex_unlock(&SegmentsLock);
    pthread_mutex_unlock(&FilesLock);
    /* Left critical sections */
    /**************************/   
   
   
    /* Free local structures */
    files_buf_local = (FileTable)NULL;
    if ((*p_files_local)->prevfile != NULL) {
	((*p_files_local)->prevfile)->nextfile = (*p_files_local)->nextfile;
	files_buf_local = (*p_files_local)->prevfile;
    }
    if ((*p_files_local)->nextfile != NULL) {
	((*p_files_local)->nextfile)->prevfile = (*p_files_local)->prevfile;
	files_buf_local = (*p_files_local)->nextfile;
    }
    free(*p_files_local);
    *p_files_local = files_buf_local;
    /*printf("Files_local up to date: %x\n",*p_files_local);*/
   
    next_seg_buf = (SegmentTable)NULL;
    while ((*p_segments_local)->prev_fileseg != NULL)
	*p_segments_local = (*p_segments_local)->prev_fileseg;
    if ((*p_segments_local)->prev_seg != NULL)
	next_seg_buf = (*p_segments_local)->prev_seg;
    while ((*p_segments_local)->next_fileseg != NULL) {
	segments_buf_local = *p_segments_local;
	*p_segments_local = (*p_segments_local)->next_fileseg;
	free(segments_buf_local);
    }
    if ((*p_segments_local)->next_seg != NULL)
	next_seg_buf = (*p_segments_local)->next_seg;
    free(*p_segments_local);
    *p_segments_local = next_seg_buf;
    /*printf("Segments_local up to date: %x\n",*p_segments_local);*/
   
    return 0;
}


/************************************/
/* Routines for segments-structure */

/* Initialize segments-structure */
int ADIOI_SVM_STRUCT_Init_segments()
{
    extern SegmentTable	segments,
	segments_buf;
			
			
    segments = (SegmentTable)NULL;
    segments_buf = (SegmentTable)NULL;
    
    return 0;
}

/* De-initialize segments-structure */
int ADIOI_SVM_STRUCT_Free_segments()
{
    extern SegmentTable	segments,
	segments_buf;
			
			
    if (segments != NULL) {
	while (segments->prev_seg != NULL)
	    segments = segments->prev_seg;
	while (segments->next_seg != NULL) {
	    segments_buf = segments->next_seg;
	    free(segments);
	    segments = segments_buf;
	}
    }
    free(segments);
    free(segments_buf);
    
    return 0;
}

/* Frees the local copy of a file's segments-structure */
int ADIOI_SVM_STRUCT_Free_segments_copy(SegmentTable *p_segments_local)
{
    SegmentTable  segments_buf_local;
   
    if (*p_segments_local == NULL)
	/* nothing to do... */
	return 0;
      
    /* start with first segment */
    while ((*p_segments_local)->prev_seg != NULL)
	*p_segments_local = (*p_segments_local)->prev_seg;
      
    while (*p_segments_local != NULL) {
	segments_buf_local = *p_segments_local;
	*p_segments_local = (*p_segments_local)->next_seg;
	free(segments_buf_local);
    }
    *p_segments_local = (SegmentTable)malloc(sizeof(struct SegmentTable_D));
    (*p_segments_local)->prev_seg = (SegmentTable)NULL;
    (*p_segments_local)->next_seg = (SegmentTable)NULL;
    (*p_segments_local)->prev_fileseg = (SegmentTable)NULL;
    (*p_segments_local)->next_fileseg = (SegmentTable)NULL;
       
    return 0;
}

/* Creates a local copy of a file's segments-structure */
int ADIOI_SVM_STRUCT_Copy_segments(FileTable *p_files_local, SegmentTable *p_segments_local)
{
    int           ret;
    SegmentTable  segments_buf_local;
   
    extern SegmentTable		segments,
	segments_buf;
    extern pthread_mutex_t 	FilesLock,
	SegmentsLock;
			
			   
    if (*p_segments_local == NULL) {
	*p_segments_local = (SegmentTable)malloc(sizeof(struct SegmentTable_D));
	(*p_segments_local)->prev_seg = (SegmentTable)NULL;
	(*p_segments_local)->next_seg = (SegmentTable)NULL;
	(*p_segments_local)->prev_fileseg = (SegmentTable)NULL;
	(*p_segments_local)->next_fileseg = (SegmentTable)NULL;
    }
    else
	ADIOI_SVM_STRUCT_Free_segments_copy(p_segments_local);
   
    /**************************/   
    /* Enter critical sections */
    ret = pthread_mutex_trylock(&FilesLock);
    if (ret)
	/* => error */
	do {
	    ret = pthread_mutex_lock(&FilesLock);
	    /* is 100 a good value ? */
	    usleep(113);
	} while (ret);
    ret = pthread_mutex_trylock(&SegmentsLock);
    if (ret)
	/* => error */
	do {
	    ret = pthread_mutex_lock(&SegmentsLock);
	    /* is 100 a good value ? */
	    usleep(113);
	} while (ret);
    /* locked... */
   
    if (segments != NULL) {
	/* start with file's first segment */
	while (files->prevfile != NULL)
	    files = files->prevfile;
	while (((*p_files_local)->fd != files->fd) && (files->nextfile != NULL))
	    files = files->nextfile;
	if ((*p_files_local)->fd != files->fd) {
	    printf("ADIOI_SVM_STRUCT_Copy_segments(): Couldn't find file.\n");
	 
	    return;
	}
	segments = files->firstseg;
      
	**p_segments_local = *segments;
	(*p_files_local)->firstseg = *p_segments_local;
	(*p_segments_local)->self = *p_segments_local;
	(*p_segments_local)->prev_fileseg = (SegmentTable)NULL;
	(*p_segments_local)->prev_seg = (SegmentTable)NULL;
       
	while (segments->next_fileseg != NULL) {
	    segments = segments->next_fileseg;
	    (*p_segments_local)->next_fileseg = (SegmentTable)malloc(sizeof(struct SegmentTable_D));
	    (*p_segments_local)->next_seg = (*p_segments_local)->next_fileseg;
	    segments_buf_local = *p_segments_local;
	    *p_segments_local = (*p_segments_local)->next_fileseg;
	    **p_segments_local = *segments;
	    (*p_segments_local)->self = *p_segments_local;
	    (*p_segments_local)->prev_fileseg = segments_buf_local;
	    (*p_segments_local)->prev_seg = segments_buf_local;
	} 
	(*p_segments_local)->next_fileseg = (SegmentTable)NULL;
	(*p_segments_local)->next_seg = (SegmentTable)NULL;

	(*p_files_local)->lastseg = *p_segments_local;
    }
    else {
	free(*p_segments_local);
	*p_segments_local = (SegmentTable)NULL;
    }
   
    /* unlock... */
    pthread_mutex_unlock(&SegmentsLock);
    pthread_mutex_unlock(&FilesLock);
    /* Left critical sections */
    /**************************/   
   
    return 0;
}



/************************************/
/* Routines for threads-structure */

/* Initialize threads-structure */
int ADIOI_SVM_STRUCT_Init_threads()
{
    extern Thread_List	threads;
   
   
    threads = (Thread_List)NULL;

    return 0;
}

/* De-initialize threads-structure */
int ADIOI_SVM_STRUCT_Free_threads()
{
    Thread_List  threads_buf;
    
    extern Thread_List	threads;
   
    if (threads != NULL) {
	while (threads->prev != NULL)
	    threads = threads->prev;
	while (threads->next != NULL) {
	    threads_buf = threads->next;
	    free(threads);
	    threads = threads_buf;
	}
    }
    free(threads);
    
    return 0;
}

/* Create new entry in threads-structure */
int ADIOI_SVM_STRUCT_Create_thread(pthread_t *tid, ADIO_Request *request)
{
    int  ret;
   
    extern Thread_List		threads;
    extern pthread_mutex_t 	ThreadsLock;
   
   
    /**************************/   
    /* Enter critical section */
    ret = pthread_mutex_trylock(&ThreadsLock);
    if (ret)
	/* => error */
	do {
	    ret = pthread_mutex_lock(&ThreadsLock);
	    /* is 100 a good value ? */
	    usleep(113);
	} while (ret);
    /* locked... */
   
    if (threads != NULL) {
	/* => at least one thread exists - add to end of list */
	while (threads->next != NULL)
	    threads = threads->next;
	threads->next = (Thread_List)malloc(sizeof(struct Thread_List_D));
	(threads->next)->prev = threads;
	threads = threads->next;
    }
    else {
	/* first thread that is add to list */
	threads = (Thread_List)malloc(sizeof(struct Thread_List_D));
	threads->prev = (Thread_List)NULL;
    }
    threads->next = (Thread_List)NULL;
    threads->request = *request;
    threads->tid = *tid;
    threads->done = 0;
    /*printf("Created new thread:\n");*/
    /*printf("Thread-ID: %x.\n",threads->tid);*/
    /*printf("Thread->done: %i.\n",threads->done);*/
    /*printf("(Thread->request)->ptr_in_async_list: %x\n",(threads->request)->ptr_in_async_list);*/


    /* unlock... */
    pthread_mutex_unlock(&ThreadsLock);
    /* Left critical section */
    /**************************/   
   
    return 0;   
}

/* Remove entry from threads-structure */
int ADIOI_SVM_STRUCT_Remove_thread(ADIO_Request *request)
{
    int          ret;
    Thread_List  threads_buf;
   
    extern Thread_List		threads;
    extern pthread_mutex_t 	ThreadsLock;

      
    /**************************/   
    /* Enter critical section */
    ret = pthread_mutex_trylock(&ThreadsLock);
    if (ret)
	/* => error */
	do {
	    ret = pthread_mutex_lock(&ThreadsLock);
	    /* is 100 a good value ? */
	    usleep(113);
	} while (ret);
    /* locked... */
   
    if (threads != NULL) {
	/* => at least one thread exists - start at begin of list */
	while (threads->prev != NULL)
	    threads = threads->prev;
	while (((threads->request)->ptr_in_async_list != (*request)->ptr_in_async_list)&&(threads->next != NULL))
	    threads = threads->next;
	if ((threads->request)->ptr_in_async_list != (*request)->ptr_in_async_list) {
	    /* => thread not found */
	    /*printf("ADIOI_SVM_STRUCT_Remove_thread(): Thread not found => nothing to do...\n");*/
	}
	else {
	    /* => thread found - remove it from list */
	    /*printf("Remove thread:\n");*/
	    /*printf("Thread-ID: %x.\n",threads->tid);*/
	    /*printf("Thread->done: %i.\n",threads->done);*/
	    /*printf("(Thread->request)->ptr_in_async_list: %x\n",(threads->request)->ptr_in_async_list);*/
	    threads_buf = threads;
	    if (threads_buf->prev != NULL) {
		(threads_buf->prev)->next = threads->next;
		if (threads_buf->next != NULL) {
		    (threads_buf->next)->prev = threads->prev;
		}
		free(threads);
		threads = threads_buf->prev;
	    }
	    else
		if (threads_buf->next != NULL) {
		    (threads_buf->next)->prev = (Thread_List)NULL;
		    free(threads);
		    threads = threads_buf->next;
		}
		else {
		    free(threads);
		    threads = (Thread_List)NULL;
		}
	}
    }
    else {	 
	/* => no thread left */
	/*printf("ADIOI_SVM_STRUCT_Remove_thread(): Thread not found => nothing to do...\n");*/
    }

    /* unlock... */
    pthread_mutex_unlock(&ThreadsLock);
    /* Left critical section */
    /**************************/   
   
    return 0;   
}


/* Set done-attribute of entry in threads-structure */
int ADIOI_SVM_STRUCT_Set_thread_done(pthread_t tid)
{
    int  ret;
   
    extern Thread_List		threads;
    extern pthread_mutex_t 	ThreadsLock;

      
    /**************************/   
    /* Enter critical section */
    ret = pthread_mutex_trylock(&ThreadsLock);
    if (ret)
	/* => error */
	do {
	    ret = pthread_mutex_lock(&ThreadsLock);
	    /* is 100 a good value ? */
	    usleep(113);
	} while (ret);
    /* locked... */
   
    if (threads == NULL) {
	/* => no thread in list */
	printf("ADIOI_SVM_STRUCT_Set_thread_done(): Thread not found => nothing to do...\n");
      
	/* unlock... */
	pthread_mutex_unlock(&ThreadsLock);
	/* Left critical section */   
	/**************************/   
      
	return 0;
    }
   
    /* start with first thread in list */
    while (threads->prev != NULL)
	threads = threads->prev;
    /* search for the right one */
    while ((threads->tid != tid)&&(threads->next != NULL))
	threads = threads->next;
    if (threads->tid == tid) {
	/* => thread found */
	threads->done = 1;
	ret = threads->done;
      
	printf("Set threads->done of thread %x.\n",threads->tid);
      
	/* unlock... */
	pthread_mutex_unlock(&ThreadsLock);
	/* Left critical section */   
	/**************************/   
      
	return ret;
    }
    else {
	/* => thread not in list */
	/*printf("ADIOI_SVM_STRUCT_Set_thread_done(): Thread not found => nothing to do...\n");*/
      
	/* unlock... */
	pthread_mutex_unlock(&ThreadsLock);
	/* Left critical section */   
	/**************************/   
         
	return 0;
    }
}

/* Get done-attribute of entry in threads-structure */
int ADIOI_SVM_STRUCT_Get_thread_done(ADIO_Request *request)
{
    int  ret;
   
    extern Thread_List		threads;
    extern pthread_mutex_t 	ThreadsLock;

      
    /**************************/   
    /* Enter critical section */
    ret = pthread_mutex_trylock(&ThreadsLock);
    if (ret)
	/* => error */
	do {
	    ret = pthread_mutex_lock(&ThreadsLock);
	    /* is 100 a good value ? */
	    usleep(113);
	} while (ret);
    /* locked... */
   
    if (threads == NULL) {
	/* => no thread in list */
	/*printf("ADIOI_SVM_STRUCT_Get_thread_done(): Thread not found => nothing to do...\n");*/
      
	/* unlock... */
	pthread_mutex_unlock(&ThreadsLock);
	/* Left critical section */   
	/**************************/   
   
	return 1;
    }
    /* start with first thread in list */
    while (threads->prev != NULL)
	threads = threads->prev;
    /* search for the right one */
    while (((threads->request)->ptr_in_async_list != (*request)->ptr_in_async_list)&&(threads->next != NULL))
	threads = threads->next;
    if ((threads->request)->ptr_in_async_list == (*request)->ptr_in_async_list) {
	/* => thread found */
	ret = threads->done;
      
	printf("Threads->done of thread %x: %x.\n",threads->tid,threads->done);
      
	/* unlock... */
	pthread_mutex_unlock(&ThreadsLock);
	/* Left critical section */   
	/**************************/   
   
	return ret;
    }
    else {
	/* => thread not in list */
	/*printf("ADIOI_SVM_STRUCT_Get_thread_done(): Thread not found => nothing to do...\n");*/
      
	/* unlock... */
	pthread_mutex_unlock(&ThreadsLock);
	/* Left critical section */   
	/**************************/   
    
	return 1;
    }
}

