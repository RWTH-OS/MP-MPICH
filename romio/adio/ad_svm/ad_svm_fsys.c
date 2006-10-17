/* $Id: ad_svm_fsys.c,v 1.3 2000/09/11 09:48:03 joachim Exp $
  *
  *	ad_svm_fsys.c
  *
  *	routines to manage the virtual parallel filesystem
  *
 */
 
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>

#define main
#include "svmlibc.h"
#undef main

#include "ad_svm_fsys.h"

int _ad_svm_users = 0;

/* Initializing-routine */
void ADIOI_SVM_Init_ad_svm()
{
    int                 svm_argc=7, nprocs, mynod, masterlen, namelen, i,
	page_size, ret;
    char                **svm_argv, *name_buf, *master_name,
	*proc_name;
			
    extern pthread_mutex_t 	NodesLock,
	FilesLock,
	SegmentsLock,
	ThreadsLock;
    extern unsigned long	FileHandleLock,
	RegionHandleLock;
    extern int			*GlobalVarRegion,
	*RegionHandles,
	NextRegionHandle,
	*FileHandles,
	NextFileHandle,
	*MutexRegion;
    pthread_mutexattr_t		*attr;
    

    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &mynod);       

    /*printf("Entering ADIOI_SVM_Init_ad_svm().\n");*/
    
    /* determine master */
    name_buf = (char *)malloc(MPI_MAX_PROCESSOR_NAME);
    gethostname(name_buf,MPI_MAX_PROCESSOR_NAME);
    namelen = strlen(name_buf);
    proc_name = (char *)malloc(namelen+10);
    strcpy(proc_name,(const char *)name_buf);
    if (!mynod) {
        /* process 0 is master - needed for SVMlib */
    	masterlen = namelen;
	master_name = (char *)malloc(masterlen+10);
	strcpy(master_name,(const char *)proc_name);
	MPI_Bcast(&masterlen,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(master_name,masterlen+10,MPI_CHAR,0,MPI_COMM_WORLD);
    }
    else {
	MPI_Bcast(&masterlen,1,MPI_INT,0,MPI_COMM_WORLD);
	master_name = (char *)malloc(masterlen+10);	
	MPI_Bcast(master_name,masterlen+10,MPI_CHAR,0,MPI_COMM_WORLD);
    }
    /*printf("%s - master-name: '%s' - namelen: %i\n",name_buf,master_name,masterlen);*/

    /* Initialize data-structures */
    ADIOI_SVM_STRUCT_Init_nodes(nprocs,mynod,namelen,proc_name);

    /* initialize file-table */
    ADIOI_SVM_STRUCT_Init_files();
     
    /* initialize segment-table */
    ADIOI_SVM_STRUCT_Init_segments();

    /* initialize thread-list */
    ADIOI_SVM_STRUCT_Init_threads();
    
    /* initialize the SVMlib */
    svm_argv = (char**)malloc(svm_argc * sizeof(char *));
    svm_argv[0] = (char*)malloc(10);
    strcpy((char*)svm_argv[0],"./svmrun");
    svm_argv[1] = (char*)malloc(4);
    strcpy((char*)svm_argv[1],"-n");
    svm_argv[2] = (char*)malloc(3);
    sprintf((char*)svm_argv[2],"%i",nprocs);
    svm_argv[3] = (char*)malloc(3);
    strcpy((char*)svm_argv[3],"-m");
    svm_argv[4] = (char*)malloc(masterlen+10);
    strcpy((char*)svm_argv[4],master_name);
    svm_argv[5] = (char*)malloc(4);
    strcpy((char*)svm_argv[5],"-p");
    svm_argv[6] = (char*)malloc(7);
    strcpy((char*)svm_argv[6],"22222");
    
    ad_svm_init(svm_argc, svm_argv);
    
    /* initialize region for global variables */
    page_size = getpagesize();

    RegionHandles = (int *)SVMCreateRegion(0,page_size,SSC_SRSW);
    /*printf("RegionHandles: %x.\n",RegionHandles);fflush(stdout);*/
    FileHandles = (int *)SVMCreateRegion(1,page_size,SSC_SRSW);
    /*printf("FileHandles: %x.\n",FileHandles);fflush(stdout);*/
    
    /* Create synchronisation-objects */
    FileHandleLock = SVMCreateSyncObject(USER_LOCK_ID_START, TRI, 0);
    /*printf("FileHandleLock: %x.\n",FileHandleLock);fflush(stdout);*/
    RegionHandleLock = SVMCreateSyncObject(USER_LOCK_ID_START+1, TRI, 0);
    /*printf("RegionHandleLock: %x.\n",RegionHandleLock);fflush(stdout);*/
    
    ret = pthread_mutex_init(&NodesLock,0);
    if (ret) {
	printf("ADIOI_SVM_Init_ad_svm(): Couldn't init mutex.\n");
	MPI_Abort(MPI_COMM_WORLD,1);
    }
    ret = pthread_mutex_init(&FilesLock,0);
    if (ret) {
	printf("ADIOI_SVM_Init_ad_svm(): Couldn't init mutex.\n");
	MPI_Abort(MPI_COMM_WORLD,1);
    }
    ret = pthread_mutex_init(&SegmentsLock,0);
    if (ret) {
	printf("ADIOI_SVM_Init_ad_svm(): Couldn't init mutex.\n");
	MPI_Abort(MPI_COMM_WORLD,1);
    }
    ret = pthread_mutex_init(&ThreadsLock,0);    
    if (ret) {
	printf("ADIOI_SVM_Init_ad_svm(): Couldn't init mutex.\n");
	MPI_Abort(MPI_COMM_WORLD,1);
    }

    if (!mynod) {
	/* Node 0 has to initialize regions */
	NextRegionHandle = AD_SVM_REGION_OFFSET;
	memcpy((int *)RegionHandles,(const int *)&NextRegionHandle,sizeof(int));
	/*printf("RegionHandles up-to-date...\n");fflush(stdout);*/
	NextFileHandle = AD_SVM_FD_OFFSET;
	memcpy((int *)FileHandles,(const int *)&NextFileHandle,sizeof(int));
	/*printf("FileHandles up-to-date...\n");fflush(stdout);*/
    }

    free(proc_name);
    free(master_name);
    for (i=0;i<svm_argc;i++)
	free(svm_argv[i]);
    free(svm_argv);
    MPI_Barrier(MPI_COMM_WORLD);
}

/* Un-initializing-routine */
void ADIOI_SVM_Finalize_ad_svm()
{
    int  ret, empty=0, mynod;
    extern pthread_mutex_t   	 NodesLock,
	FilesLock,
	SegmentsLock,
	ThreadsLock;
    extern unsigned long	 FileHandleLock,
	RegionHandleLock;
    			
    
    MPI_Barrier(MPI_COMM_WORLD);
    /*printf("Entering ADIOI_SVM_Finalize_ad_svm().\n");*/
    
    /* Delete synchronisation-objects */
    SVMDeleteSyncObject(FileHandleLock);
    SVMDeleteSyncObject(RegionHandleLock);
    
    pthread_mutex_destroy(&NodesLock);
    pthread_mutex_destroy(&FilesLock);
    pthread_mutex_destroy(&SegmentsLock);
    pthread_mutex_destroy(&ThreadsLock);    
    
    
    /* Delete regions for global variables */
    SVMDeleteRegion(1);
    SVMDeleteRegion(0);

    /* Un-initialize SVMlib */ 
    ad_svm_finalize();
         
    /* De-initialize data-structures */
    ADIOI_SVM_STRUCT_Free_nodes();
    ADIOI_SVM_STRUCT_Free_files();
    ADIOI_SVM_STRUCT_Free_segments();
    ADIOI_SVM_STRUCT_Free_threads();
}
    
/* Returns filename without path */
char *ADIOI_SVM_Basename(char *name, int *len)
{
    char *buffer, *result;

    buffer = (char *)malloc(strlen(name));
    result = (char *)malloc(strlen(name));
    strcpy(buffer,name);
    strcpy(result,name);

    do {
	buffer = strstr((const char *)buffer,(const char *)"/");

	if (buffer!=NULL) {
	    buffer++;
	    *len = strlen(buffer);
	    strcpy(result,buffer);
	}
    } while (buffer!=NULL);

    return result;
}

/* Handles file requests from ADIOI_SVM_Redistribute_file() */
void ADIOI_SVM_Handle_file_request(int sender)
{
    int		i=0, mynod, node, nodes, nr, len, done, size, local_fd;
    char		*name, *myname, *filename, *name_buf, *file_buf;
    MPI_Status	status;
	
	
    /*printf("Entering ADIOI_SVM_Handle_file_request().\n");fflush(stdout);*/
    MPI_Comm_rank(MPI_COMM_WORLD,&mynod);
	
    MPI_Bcast(&len,1,MPI_INT,sender,MPI_COMM_WORLD);
    /*printf("Receive bcast from %i: len: %i.\n",sender,len);fflush(stdout);*/
	
    name = (char *)malloc(len+10);
    MPI_Bcast(name,len+10,MPI_CHAR,sender,MPI_COMM_WORLD);
    /*printf("Receive bcast: name: %s.\n",name);fflush(stdout);*/
	
	
    myname = (char *)malloc(MPI_MAX_PROCESSOR_NAME);
    gethostname(myname,MPI_MAX_PROCESSOR_NAME);
    /*printf("I'm node: %s.\n",myname);fflush(stdout);*/
	
	
    if (!strcmp(myname,name)) {
	/* Let master know my rank */ 
	MPI_Allreduce(&mynod,&node,1,MPI_INT,MPI_MAX,MPI_COMM_WORLD);
	/*printf("Allreduce node: %i.\n",node);fflush(stdout);*/
	   
	/* Get filename and other file-attributes */
	MPI_Recv(&len,1,MPI_INT,sender,0,MPI_COMM_WORLD,&status);
	/*printf("Receiving len: %i.\n",len);fflush(stdout);*/
	   
	name_buf = (char *)malloc(len+10);
	MPI_Recv(name_buf,len+10,MPI_CHAR,sender,1,MPI_COMM_WORLD,&status);
	/*printf("Receiving name: %s.\n",name_buf);fflush(stdout);*/
	   
	MPI_Recv(&size,1,MPI_INT,sender,2,MPI_COMM_WORLD,&status);
	/*printf("Receiving size: %i.\n",size);fflush(stdout);*/
	   
	MPI_Recv(&nodes,1,MPI_INT,sender,3,MPI_COMM_WORLD,&status);
	/*printf("Receiving #nodes: %i.\n",nodes);fflush(stdout);*/
	   
	MPI_Recv(&nr,1,MPI_INT,sender,4,MPI_COMM_WORLD,&status);
	/*printf("Receiving node-nr: %i.\n",nr);fflush(stdout);*/
	   
	/* read local data and send it to master */
	filename = (char *)malloc(strlen(AD_SVM_PATH)+strlen(name_buf)+20);
	i = 0;
	nr = nr/nodes;
	do {
	    sprintf(filename,"%s%s%s%i%s%i%s%i",AD_SVM_PATH,name_buf,"_seg_",i,"_",nodes,"_",nr);
	    local_fd = open(filename,O_RDWR,0700);
	    done = local_fd +1;
	    /*printf("filename: %s.\n",filename);fflush(stdout);*/
	    /*printf("local_fd: %i.\n",local_fd);fflush(stdout);*/
	    i++;
	} while (!done);
	   
	if (size) {
	    file_buf = (char *)malloc(size+10);
	    read(local_fd,file_buf,size);
	    MPI_Send(file_buf,size+10,MPI_CHAR,sender,5,MPI_COMM_WORLD);
	    /*printf("Sending file_buf: %s.\n",file_buf);fflush(stdout);*/
	    free(file_buf);
	}
	/* delete data */
	close(local_fd);
	/*printf("Delete %s.\n",filename);fflush(stdout);*/
	unlink(filename);
	sprintf(filename,"%s%s%s%s",AD_SVM_PATH,".",name_buf,".adio_file_info");
	/*printf("Delete %s.\n",filename);fflush(stdout);*/
	unlink(filename);
	/*printf("Done.\n");fflush(stdout);*/
	free(filename);
	free(name_buf);
    }
    else {
	MPI_Allreduce(&i,&node,1,MPI_INT,MPI_MAX,MPI_COMM_WORLD);
	/*printf("Allreduce node: %i.\n",node);fflush(stdout);*/
    }
    free(name);
    free(myname);
}

/* Redistribute existing file */
int ADIOI_SVM_Redistribute_file(ADIO_File file,
				int info_file,
				char *info_name,
				int type,
				int involved)
{
    HD_FileInfo    	FileInfo;
    char 		*name_buf, *file_buf, *tmp_buf, *seg_buf, *filename, *node_name;
    int 		i=0, j=0, mynod, mywnod, empty=0, done=0, local_fd, len, node_len,
	searched_node, size_buf, file_size=0, seg_size, master, seg_nr, nprocs,
	page_size, nodes, nr;
    MPI_Status	status;
	
	
    MPI_Comm_size(file->comm, &nprocs);
    MPI_Comm_rank(file->comm, &mynod);
	
    FileInfo = (HD_FileInfo)malloc(sizeof(struct HD_FileInfo_D));
	
    switch (type) {
	
    case 1:	/* not all data available on involved nodes => fetch rest of the data */
	printf("ADIOI_SVM_Redistribute_file(): Fetch data not implemented, yet.\n");
	free(FileInfo);
	return -1;
	break;
    case 2: /* all data available, but data's distributed among less nodes */
	/* => distribute it to involved nodes */
	/*printf("Distribute file among involved nodes (more).\n");fflush(stdout);*/
		
	if (involved) {
	    /* data available on this node */
	    name_buf = (char *)malloc(MPI_MAX_PROCESSOR_NAME);
	    gethostname(name_buf,MPI_MAX_PROCESSOR_NAME);
		   
	    /* make number of segments known to all nodes */
	    read(info_file,FileInfo,sizeof(struct HD_FileInfo_D));
	    MPI_Allreduce(&FileInfo->nr_of_segs,&seg_nr,1,MPI_INT,MPI_MAX,file->comm);
	    lseek(info_file,0,SEEK_SET);
	    do {
		i = 0;
		done = 0;
		      
		read(info_file,FileInfo,sizeof(struct HD_FileInfo_D));
		      
		if (!strcmp(name_buf,FileInfo->node)) {
		    /* data local */
			 
		    /* read local data and bcast it to other nodes */
		    filename = (char *)malloc(strlen(AD_SVM_PATH)+strlen(file->filename)+16);
		    do {
			sprintf(filename,"%s%s%s%i%s%i%s%i",AD_SVM_PATH,file->filename,
				"_seg_",i,"_",FileInfo->nr_of_segs/FileInfo->segs_per_node
				,"_",FileInfo->seg_nr%(FileInfo->nr_of_segs/FileInfo->segs_per_node));
			local_fd = open(filename,O_RDWR,0700);
			done = local_fd +1;
			i++;
		    } while (!done);
			 
		    MPI_Allreduce(&FileInfo->bytes_used,&size_buf,1,MPI_INT,MPI_MAX,file->comm);
			 
		    if (size_buf) {
			file_size += size_buf;
			seg_buf = (char *)malloc(size_buf+10);
			MPI_Allreduce(&mynod,&master,1,MPI_INT,MPI_MAX,file->comm);
			read(local_fd,seg_buf,size_buf);
			MPI_Bcast(seg_buf,size_buf+10,MPI_CHAR,master,file->comm);
			    
			/* add data to file-buffer */
			tmp_buf = (char *)malloc(file_size-size_buf+10);
			len = file_size-size_buf;
			if (len != 0) {
			    tmp_buf = (char *)malloc(file_size-size_buf+10);
			    sprintf((char *)tmp_buf,"%s",file_buf);
			    free(file_buf);
			    file_buf = (char *)malloc(file_size+10);
			    sprintf((char *)file_buf,"%s%s",tmp_buf,seg_buf);
			    free(tmp_buf);
			}
			else {
			    file_buf = (char *)malloc(file_size+10);
			    strcpy((char *)file_buf,(const char*)seg_buf);
			}
			free(seg_buf);
		    }
			 
		    /* delete local segment */
		    close(local_fd);
		    unlink(filename);
		    free(filename);
		}
		else {
		    /* data not local */
			 
		    /* get data from owner */
		    MPI_Allreduce(&i,&size_buf,1,MPI_INT,MPI_MAX,file->comm);
			 
		    if (size_buf) {
			file_size += size_buf;
			seg_buf = (char *)malloc(size_buf+10);
			MPI_Allreduce(&i,&master,1,MPI_INT,MPI_MAX,file->comm);
			MPI_Bcast(seg_buf,size_buf+10,MPI_CHAR,master,file->comm);
			 
			/* add data to file-buffer */
			tmp_buf = (char *)malloc(file_size-size_buf+10);			    
			len = file_size-size_buf;
			if (len != 0) {
			    tmp_buf = (char *)malloc(file_size-size_buf+10);
			    sprintf((char *)tmp_buf,"%s",file_buf);
			    free(file_buf);
			    file_buf = (char *)malloc(file_size+10);
			    sprintf((char *)file_buf,"%s%s",tmp_buf,seg_buf);
			    free(tmp_buf);
			}
			else {
			    file_buf = (char *)malloc(file_size+10);
			    strcpy((char *)file_buf,(const char *)seg_buf);
			}
			       
			free(seg_buf);
		    }
		}
	    } while (FileInfo->seg_nr<FileInfo->nr_of_segs-1);
		   
	    free(name_buf);
	}
	else {
	    /* no data on this node */
		   
	    /* Get number of segments */
	    MPI_Allreduce(&i,&seg_nr,1,MPI_INT,MPI_MAX,file->comm);
		   
	    do {
		/* get data from owner */
		j++;
		      
		MPI_Allreduce(&i,&size_buf,1,MPI_INT,MPI_MAX,file->comm);
		      
		if (size_buf) {
		    file_size += size_buf;
		    seg_buf = (char *)malloc(size_buf+10);
		    MPI_Allreduce(&i,&master,1,MPI_INT,MPI_MAX,file->comm);
		    MPI_Bcast(seg_buf,size_buf+10,MPI_CHAR,master,file->comm);
			 
		    /* add it to file-buffer */
		    len = file_size-size_buf;
		    if (len != 0) {
			tmp_buf = (char *)malloc(file_size-size_buf+10);
			sprintf((char *)tmp_buf,"%s",file_buf);
			free(file_buf);
			file_buf = (char *)malloc(file_size+10);
			sprintf((char *)file_buf,"%s%s",tmp_buf,seg_buf);
			free(tmp_buf);
		    }
		    else {
			file_buf = (char *)malloc(file_size+10);
			strcpy((char *)file_buf,(const char *)seg_buf);
		    }
		    free(seg_buf);
		}
	    } while (j < seg_nr);
	}
	break;
    case 3: /* all data available, but data's distributed among more nodes */
	/* => distribute it to involved nodes */
#ifndef RAY		
	printf("Distribute file among involved nodes (less).\n");fflush(stdout);
	printf("Not implemented, yet.\n");fflush(stdout);
	free(FileInfo);
	return -1;
#else
	if (involved) {
	    /* data available on this node */
	    name_buf = (char *)malloc(MPI_MAX_PROCESSOR_NAME);
	    gethostname(name_buf,MPI_MAX_PROCESSOR_NAME);
		   
	    /* make number of segments known to all nodes */
	    read(info_file,FileInfo,sizeof(struct HD_FileInfo_D));
	    MPI_Allreduce(&FileInfo->nr_of_segs,&seg_nr,1,MPI_INT,MPI_MAX,file->comm);
	    /*printf("Broadcast nr of segs: %i.\n",FileInfo->nr_of_segs);fflush(stdout);*/
	    lseek(info_file,0,SEEK_SET);
	    do {
		i = 0;
		done = 0;
		      
		read(info_file,FileInfo,sizeof(struct HD_FileInfo_D));
		      
		/* make node-name known to all nodes */
		len = strlen(FileInfo->node);
		MPI_Allreduce(&len,&node_len,1,MPI_INT,MPI_MAX,file->comm);
		MPI_Allreduce(&mynod,&master,1,MPI_INT,MPI_MAX,file->comm);
		MPI_Bcast(FileInfo->node,node_len+10,MPI_CHAR,master,file->comm);
		/*printf("Bcast: len: %i - master: %i - node: %s.\n",len,mynod,FileInfo->node);fflush(stdout);*/

		if (ADIOI_SVM_Is_in_group(file->comm,nprocs,FileInfo->node) == MPI_SUCCESS) {
		    /* => data on involved node */
		    if (!strcmp(name_buf,FileInfo->node)) {
			/* data local */
			/*printf("Node involved and data local.\n");fflush(stdout);*/
			 
			/* read local data and bcast it to other nodes */
			filename = (char *)malloc(strlen(AD_SVM_PATH)+strlen(file->filename)+16);
			do {
			    sprintf(filename,"%s%s%s%i%s%i%s%i",AD_SVM_PATH,file->filename,
				    "_seg_",i,"_",FileInfo->nr_of_segs/FileInfo->segs_per_node,
				    "_",FileInfo->seg_nr%(FileInfo->nr_of_segs/FileInfo->segs_per_node));
			    local_fd = open(filename,O_RDWR,0700);
			    done = local_fd +1;
			    i++;
			} while (!done);
			/*printf("fd: %i.\n",local_fd);fflush(stdout);*/
			    
			MPI_Allreduce(&FileInfo->bytes_used,&size_buf,1,MPI_INT,MPI_MAX,file->comm);
			/*printf("Bcast: bytes: %i.\n",FileInfo->bytes_used);fflush(stdout);*/
			    
			if (size_buf) {
			    file_size += size_buf;
			    seg_buf = (char *)malloc(size_buf+10);
			    read(local_fd,seg_buf,size_buf);
			    MPI_Bcast(seg_buf,size_buf+10,MPI_CHAR,master,file->comm);
			    /*printf("Bcast: seg_buf: %s.\n",seg_buf);fflush(stdout);*/
			 
			    /* add data to file-buffer */
			    tmp_buf = (char *)malloc(file_size-size_buf+10);
			    len = file_size-size_buf;
			    if (len != 0) {
				tmp_buf = (char *)malloc(file_size-size_buf+10);
				sprintf((char *)tmp_buf,"%s",file_buf);
				free(file_buf);
				file_buf = (char *)malloc(file_size+10);
				sprintf((char *)file_buf,"%s%s",tmp_buf,seg_buf);
				free(tmp_buf);
			    }
			    else {
				file_buf = (char *)malloc(file_size+10);
				strcpy((char *)file_buf,(const char*)seg_buf);
			    }
			    free(seg_buf);
			}
			/*printf("file_buf: %s.\n",file_buf);fflush(stdout);*/
			 
			/* delete local segment */
			close(local_fd);
			unlink(filename);
			free(filename);
		    }
		    else {
			/* data not local */
			/*printf("Node involved,but data not local.\n");fflush(stdout);*/
			 
			/* get data from owner */
			MPI_Allreduce(&i,&size_buf,1,MPI_INT,MPI_MAX,file->comm);
			/*printf("Receive bcast: size_buf: %i.\n",size_buf);fflush(stdout);*/
			 
			if (size_buf) {
			    file_size += size_buf;
			    seg_buf = (char *)malloc(size_buf+10);
			    MPI_Bcast(seg_buf,size_buf+10,MPI_CHAR,master,file->comm);
			    /*printf("Receive bcast: seg_buf: %s.\n",seg_buf);fflush(stdout);*/
			 
			    /* add data to file-buffer */
			    tmp_buf = (char *)malloc(file_size-size_buf+10);			    
			    len = file_size-size_buf;
			    if (len != 0) {
				tmp_buf = (char *)malloc(file_size-size_buf+10);
				sprintf((char *)tmp_buf,"%s",file_buf);
				free(file_buf);
				file_buf = (char *)malloc(file_size+10);
				sprintf((char *)file_buf,"%s%s",tmp_buf,seg_buf);
				free(tmp_buf);
			    }
			    else {
				file_buf = (char *)malloc(file_size+10);
				strcpy((char *)file_buf,(const char *)seg_buf);
			    }
			         
			    free(seg_buf);
			}
		    }
		}
		else {
		    /* data on non-involved node */
		    /*printf("Node not involved.\n");fflush(stdout);*/
			 
		    if (mynod == master) {
			/* I'm master => I have to fetch data */
			/*printf("I'm master - sending FileRequestMsg...\n");fflush(stdout);*/
			MPI_Comm_rank(file->comm,&mywnod);
			    
			SVMSendRequestFileMsg(mywnod);
			len = strlen(FileInfo->node);
			MPI_Bcast(&len,1,MPI_INT,mywnod,MPI_COMM_WORLD);
			/*printf("Bcast: len: %i.\n",len);fflush(stdout);*/
			    
			MPI_Bcast(FileInfo->node,len+10,MPI_CHAR,mywnod,MPI_COMM_WORLD);
			/*printf("Bcast: node: %s.\n",FileInfo->node);fflush(stdout);*/
			    
			/* Get rank of searched node and receive data */
			len = 0;
			MPI_Allreduce(&len,&searched_node,1,MPI_INT,MPI_MAX,MPI_COMM_WORLD);
			/*printf("Allreduce: home-node: %i.\n",searched_node);fflush(stdout);*/
			    
			/* send filename and -size */
			len = strlen(file->filename);
			nodes = FileInfo->nr_of_segs/FileInfo->segs_per_node;
			nr = FileInfo->seg_nr%nodes;
			MPI_Send(&len,1,MPI_INT,searched_node,0,MPI_COMM_WORLD);
			/*printf("Sending len: %i.\n",len);fflush(stdout);*/
			    
			MPI_Send(file->filename,len+10,MPI_CHAR,searched_node,1,MPI_COMM_WORLD);
			/*printf("Sending filename: %s.\n",file->filename);fflush(stdout);*/
			    
			MPI_Send(&(FileInfo->bytes_used),1,MPI_INT,searched_node,2,MPI_COMM_WORLD);
			/*printf("Sending bytes: %i.\n",FileInfo->bytes_used);fflush(stdout);*/
			    
			MPI_Send(&nodes,1,MPI_INT,searched_node,3,MPI_COMM_WORLD);
			/*printf("Sending #nodes: %i.\n",nodes);fflush(stdout);*/
			    
			MPI_Send(&nr,1,MPI_INT,searched_node,4,MPI_COMM_WORLD);
			/*printf("Sending node-nr: %i.\n",nr);fflush(stdout);*/
			    
			/* receive data */
			size_buf = FileInfo->bytes_used;
			 
			if (size_buf) {
			    /* segment in use */
			    file_size += size_buf;
			    seg_buf = (char *)malloc(size_buf+10);
			    MPI_Recv(seg_buf,size_buf+10,MPI_CHAR,searched_node,5,MPI_COMM_WORLD,&status);
			    /*printf("Receive seg_buf: %s.\n",seg_buf);fflush(stdout);*/
			       
			    if (nprocs > 1) {
				/* I'm not alone */
				  
				MPI_Barrier(file->comm);
				/*printf("Reached barrier.\n");fflush(stdout);*/
			    
				/* Make size known to all involved nodes */
				MPI_Bcast(&size_buf,1,MPI_INT,mynod,file->comm);
				/*printf("Bcast: size: %i.\n",size_buf);fflush(stdout);*/
			       
				/* Make data known to all involved nodes */
				MPI_Bcast(&seg_buf,size_buf+10,MPI_CHAR,mynod,file->comm);
				/*printf("Bcast: seg_buf: %s.\n",seg_buf);fflush(stdout);*/
			    }
			    /* add data to file-buffer */
			    tmp_buf = (char *)malloc(file_size-size_buf+10);			    
			    len = file_size-size_buf;
			    if (len != 0) {
				tmp_buf = (char *)malloc(file_size-size_buf+10);
				sprintf((char *)tmp_buf,"%s",file_buf);
				free(file_buf);
				file_buf = (char *)malloc(file_size+10);
				sprintf((char *)file_buf,"%s%s",tmp_buf,seg_buf);
				free(tmp_buf);
			    }
			    else {
				file_buf = (char *)malloc(file_size+10);
				strcpy((char *)file_buf,(const char *)seg_buf);
			    }
			         
			    free(seg_buf);
			}
			else {
			    /* segment not used */
			    if (nprocs > 1) {
				/* I'm not alone */
				  
				MPI_Barrier(file->comm);
				/*printf("Reached barrier.\n");fflush(stdout);*/
			    
				/* Make size known to all involved nodes */
				MPI_Bcast(&size_buf,1,MPI_INT,mynod,file->comm);
				/*printf("Bcast: size: %i.\n",size_buf);fflush(stdout);*/
			    }
			}
		    }
		    else {
			/* Wait  for master */
			/*printf("I'm not master - waiting...\n");fflush(stdout);*/
			    
			MPI_Barrier(file->comm);
			    
			/* Get size from master */
			MPI_Bcast(&size_buf,1,MPI_INT,master,file->comm);
			/*printf("Receive bcast: size_buf: %i.\n",size_buf);fflush(stdout);*/
			    
			if (size_buf) {
			    file_size += size_buf;
			    seg_buf = (char *)malloc(size_buf+10);
			       
			    /* Get data from master */
			    MPI_Bcast(seg_buf,size_buf+10,MPI_CHAR,master,file->comm);
			    /*printf("Receive bcast: seg_buf: %s.\n",seg_buf);fflush(stdout);*/
			 
			    /* add data to file-buffer */
			    tmp_buf = (char *)malloc(file_size-size_buf+10);			    
			    len = file_size-size_buf;
			    if (len != 0) {
				tmp_buf = (char *)malloc(file_size-size_buf+10);
				sprintf((char *)tmp_buf,"%s",file_buf);
				free(file_buf);
				file_buf = (char *)malloc(file_size+10);
				sprintf((char *)file_buf,"%s%s",tmp_buf,seg_buf);
				free(tmp_buf);
			    }
			    else {
				file_buf = (char *)malloc(file_size+10);
				strcpy((char *)file_buf,(const char *)seg_buf);
			    }
			         
			    free(seg_buf);
			}
		    }	 
		}
	    } while (FileInfo->seg_nr<FileInfo->nr_of_segs-1);
		   
	    free(name_buf);
	}
	else {
	    /* no data on this node */
	    /*printf("No data available.\n");fflush(stdout);*/
		   
	    /* Get number of segments */
	    MPI_Allreduce(&i,&seg_nr,1,MPI_INT,MPI_MAX,file->comm);
	    /*printf("Allreduce: seg_nr: %i.\n",seg_nr);fflush(stdout);*/
		   
	    do {
		/* get data from owner */
		j++;
		      
		/* Get node_name */
		len = 0;
		MPI_Allreduce(&len,&node_len,1,MPI_INT,MPI_MAX,file->comm);
		/*printf("Allreduce: node_len: %i.\n",node_len);fflush(stdout);*/
		      
		MPI_Allreduce(&len,&master,1,MPI_INT,MPI_MAX,file->comm);
		/*printf("Allreduce: master: %i.\n",master);fflush(stdout);*/
		      
		node_name = (char *)malloc(node_len+10);
		MPI_Bcast(node_name,node_len+10,MPI_CHAR,master,file->comm);
		/*printf("Receive bcast: node_name: %s.\n",node_name);fflush(stdout);*/
		      
		if (ADIOI_SVM_Is_in_group(file->comm,nprocs,node_name) == MPI_SUCCESS) {
		    /* => data on involved node */
		    /*printf("Node involved, but data not local.\n");fflush(stdout);*/
		   
		    MPI_Allreduce(&i,&size_buf,1,MPI_INT,MPI_MAX,file->comm);
		    /*printf("Allreduce: size_buf: %i.\n",size_buf);fflush(stdout);*/
			 
		    if (size_buf != 0) {
			file_size += size_buf;
			seg_buf = (char *)malloc(size_buf+10);
			MPI_Allreduce(&i,&master,1,MPI_INT,MPI_MAX,file->comm);
			/*printf("Allreduce: master: %i.\n",master);fflush(stdout);*/
			    
			MPI_Bcast(seg_buf,size_buf+10,MPI_CHAR,master,file->comm);
			/*printf("Receive bcast: seg_buf: %s.\n",seg_buf);fflush(stdout);*/
			 
			/* add it to file-buffer */
			len = file_size-size_buf;
			if (len != 0) {
			    tmp_buf = (char *)malloc(file_size-size_buf+10);
			    sprintf((char *)tmp_buf,"%s",file_buf);
			    free(file_buf);
			    file_buf = (char *)malloc(file_size+10);
			    sprintf((char *)file_buf,"%s%s",tmp_buf,seg_buf);
			    free(tmp_buf);
			}
			else {
			    file_buf = (char *)malloc(file_size+10);
			    strcpy((char *)file_buf,(const char *)seg_buf);
			}
			free(seg_buf);
		    }
		}
		else {
		    /* data on non-involved node */
		    /*printf("Node not involved and data not local.\n");fflush(stdout);*/
			 
		    /* Wait  for master */
		    MPI_Barrier(file->comm);
		    /*printf("Reached barrier.\n",len);fflush(stdout);*/
			 
		    /* Get size from master */
		    MPI_Bcast(&size_buf,1,MPI_INT,master,file->comm);
		    /*printf("Receive bcast: size_buf: %i.\n",size_buf);fflush(stdout);*/
			    
		    if (size_buf) {
			file_size += size_buf;
			seg_buf = (char *)malloc(size_buf+10);
			      
			/* Get data from master */
			MPI_Bcast(seg_buf,size_buf+10,MPI_CHAR,master,file->comm);
			/*printf("Receive bcast: seg_buf: %s.\n",seg_buf);fflush(stdout);*/
			 
			/* add data to file-buffer */
			tmp_buf = (char *)malloc(file_size-size_buf+10);			    
			len = file_size-size_buf;
			if (len != 0) {
			    tmp_buf = (char *)malloc(file_size-size_buf+10);
			    sprintf((char *)tmp_buf,"%s",file_buf);
			    free(file_buf);
			    file_buf = (char *)malloc(file_size+10);
			    sprintf((char *)file_buf,"%s%s",tmp_buf,seg_buf);
			    free(tmp_buf);
			}
			else {
			    file_buf = (char *)malloc(file_size+10);
			    strcpy((char *)file_buf,(const char *)seg_buf);
			}
			         
			free(seg_buf);
		    }	 
		}
	    } while (j < seg_nr);
	}
	break;
#endif
    default:/* => error */
	printf("ADIOI_SVM_Redistribute_file(): Wrong type: %i.\n",type);
	free(FileInfo);
	return -1;
    }
	
    /* delete old info-file */
    free(FileInfo);
    close(info_file);
    /*printf("info_name: %s.\n",info_name);fflush(stdout);*/
	
    unlink(info_name);
	
    /* redistribute file-data */
	
    /* first create new info-file */
    info_file = open(info_name,O_CREAT | O_RDWR,0700);
    page_size = getpagesize();
    if (((file_size/nprocs)%page_size) != 0)
	seg_size = (ADIO_Offset)(((file_size/(nprocs*page_size))+1)*page_size);
    else
	seg_size = (ADIO_Offset)(file_size/nprocs);
    if (seg_size < AD_SVM_SEGMENT_SIZE)
	seg_size = AD_SVM_SEGMENT_SIZE;
    ADIOI_SVM_Write_new_file_info(file,info_file,nprocs,seg_size,(ADIO_Offset)file_size);

    /* now distribute data to new segments */
	
    for (i=0;i<nprocs;i++)
	if (i == mynod) {
	    filename = (char *)malloc(strlen(AD_SVM_PATH)+strlen(file->filename)+16);
	    sprintf(filename,"%s%s%s%i%s%i%s%i",AD_SVM_PATH,file->filename,"_seg_",mynod,"_",nprocs,"_",0);
	    local_fd = open(filename,O_CREAT | O_RDWR,0700);
	    ftruncate(local_fd,seg_size);
	    if (file_size != 0)
		if (file_size > seg_size)
	            write(local_fd,(char *)file_buf+i*seg_size,seg_size);
		else
		    write(local_fd,(char *)file_buf+i*seg_size,file_size);
	    close(local_fd);
	    /*printf("created new file-segment: %s.\n",filename);fflush(stdout);*/
	    free(filename);
	}
    if (file_size > seg_size)
	file_size -= seg_size;
    else
	file_size = 0;
    free(file_buf);
    return 0;
}

/* Lookup node in node-list */
int ADIOI_SVM_Lookup_node(char *filename, Node_List *p_nodes_local)
{
    /* Get copy of nodes-list */
    ADIOI_SVM_STRUCT_Copy_nodes(p_nodes_local);
	
        /* return to head of node-list */
    while ((*p_nodes_local)->prev != NULL)
	*p_nodes_local = (*p_nodes_local)->prev;

    while (strcmp((*p_nodes_local)->name,filename) && ((*p_nodes_local)->next!=NULL))
	*p_nodes_local = (*p_nodes_local)->next;
    if (strcmp((*p_nodes_local)->name,filename))
	{
	    /* node not found => cannot open file */
	    return MPI_ERR_UNKNOWN;
	}
    /* => node in node-list */

    return MPI_SUCCESS;
}

/* Lookup node's rank in node-list */
int ADIOI_SVM_Lookup_node_rank(int rank, Node_List *p_nodes_local)
{
    /* Get copy of nodes-list */
    ADIOI_SVM_STRUCT_Copy_nodes(p_nodes_local);
	
        /* return to head of node-list */
    while ((*p_nodes_local)->prev != NULL)
	*p_nodes_local = (*p_nodes_local)->prev;

    while (((*p_nodes_local)->rank != rank) && ((*p_nodes_local)->next!=NULL))
	*p_nodes_local = (*p_nodes_local)->next;
    if ((*p_nodes_local)->rank != rank)
	{
	    /* node not found => cannot open file */
	    return MPI_ERR_UNKNOWN;
	}
    /* => node in node-list */
	
    return MPI_SUCCESS;
}


/* Proof, if node is in group */
int ADIOI_SVM_Is_in_group(int comm, int nprocs, char *name)
{
    MPI_Group  		my_group, common_group;
    int        		ret=0, i, j, *ranks, *newranks;
    Node_List		nodes_local, nodes_buf;
	
	
    MPI_Comm_group(comm,&my_group);
    MPI_Comm_group(MPI_COMM_WORLD,&common_group);
	
    ranks = (int *)malloc(nprocs*sizeof(int));
    for (i=0;i<nprocs;i++)
	ranks[i] = i;
    newranks = (int *)malloc(nprocs*sizeof(int));
	
    MPI_Group_translate_ranks(my_group,nprocs,ranks,common_group,newranks);
	
    /* Get copy of nodes-list */
    nodes_local = (Node_List)NULL;
    ADIOI_SVM_STRUCT_Copy_nodes(&nodes_local);
	
    /* start with first node */
    while (nodes_local->prev != NULL)
	nodes_local = nodes_local->prev;
	
    i = 0;
    j = 0;
    do {
	if (i == newranks[j]) {
	    if (!strcmp((char *)nodes_local->name,name))
		ret = 1;
	    j++;
	}
	i++;
	nodes_buf = nodes_local;
	if (nodes_local->next != NULL)
	    nodes_local = nodes_local->next;
    } while ((nodes_buf->next != NULL)&&(j < nprocs));
	
    free(nodes_local);
    free(nodes_buf);
    free(ranks);
    free(newranks);
	
    if (ret)
	/* node found */
	return MPI_SUCCESS;
    else
	/* node not in group */
	return MPI_ERR_UNKNOWN;
}
    
/* Get common rank of node and search it in node-list */
int ADIOI_SVM_Get_common_rank(int comm,int nprocs,int rank, Node_List *p_nodes_local)
{
    MPI_Group  my_group, common_group;
    int        i, *ranks, *newranks;

	
    MPI_Comm_group(comm,&my_group);
    MPI_Comm_group(MPI_COMM_WORLD,&common_group);
	
    ranks = (int *)malloc(nprocs*sizeof(int));
    for (i=0;i<nprocs;i++)
	ranks[i] = i;
    newranks = (int *)malloc(nprocs*sizeof(int));
	
    MPI_Group_translate_ranks(my_group,nprocs,ranks,common_group,newranks);
    /*printf("myrank: %i - commonrank: %i\n",ranks[rank],newranks[rank]);*/
	
    /* Get copy of nodes-list */
    ADIOI_SVM_STRUCT_Copy_nodes(p_nodes_local);
	
    /* search node in node-list */
    while ((*p_nodes_local)->prev != NULL)
	*p_nodes_local = (*p_nodes_local)->prev;
    for (i=0;i<newranks[rank];i++) {
	   
	/*printf("nodes_local->prev: %x\n",(*p_nodes_local)->prev);*/
	/*printf("nodes_local->next: %x\n",(*p_nodes_local)->next);*/
	/*printf("nodes_local->name: %s\n",(*p_nodes_local)->name);*/

	if ((*p_nodes_local)->next != NULL)
	    *p_nodes_local = (*p_nodes_local)->next;
	else {
	    /* node not found => error */
	    free(ranks);
	    free(newranks);
	    return MPI_ERR_UNKNOWN;
	}
    }
	
    /* node found */
    free(ranks);
    free(newranks);
    return MPI_SUCCESS;
}
    
/* Lookup file in file-list */
int ADIOI_SVM_Lookup_file(char *filename, FileTable  *p_files_local)
{
    if (*p_files_local != NULL) {
	/* => at least one file has already been opened */
	/* return to head of file-list */
	while ((*p_files_local)->prevfile != NULL)
	    *p_files_local = (*p_files_local)->prevfile;
	/* search in file-list */
	while (strcmp((*p_files_local)->name,filename) && ((*p_files_local)->nextfile != NULL))
	    *p_files_local = (*p_files_local)->nextfile;
	if (!strcmp((*p_files_local)->name,filename))
	    /* found file in list */
	    return MPI_SUCCESS;
    }
    /* file not found */
    return MPI_ERR_UNKNOWN;
}

/* Lookup fd in file-list */
int ADIOI_SVM_Lookup_fd(int fd, FileTable  *p_files_local)
{
    /* Get copy of file-list */
    ADIOI_SVM_STRUCT_Copy_files(p_files_local);
	
    if (*p_files_local != NULL) {
	/* => at least one file has already been opened */
	/* return to head of file-list */
	while ((*p_files_local)->prevfile != NULL)
	    *p_files_local = (*p_files_local)->prevfile;
	/* search in file-list */
	while (((*p_files_local)->fd != fd) && ((*p_files_local)->nextfile != NULL))
	    *p_files_local = (*p_files_local)->nextfile;
	if ((*p_files_local)->fd == fd)
	    /* found file in list */
	    return MPI_SUCCESS;
    }
    /* file not found */
    return MPI_ERR_UNKNOWN;
}

/* Lookup filename in file-list */
int ADIOI_SVM_Lookup_filename(char *filename, FileTable  *p_files_local)
{
    /* Get copy of file-list */
    ADIOI_SVM_STRUCT_Copy_files(p_files_local);
	
    if (*p_files_local != NULL) {
	/* => at least one file has already been opened */
	/* return to head of file-list */
	while ((*p_files_local)->prevfile != NULL)
	    *p_files_local = (*p_files_local)->prevfile;
	/* search in file-list */
	while ((strcmp((*p_files_local)->name,filename)) && ((*p_files_local)->nextfile != NULL))
	    *p_files_local = (*p_files_local)->nextfile;
	if (!strcmp((*p_files_local)->name,filename))
	    /* found file in list */
	    return MPI_SUCCESS;
    }
    /* file not found */
    return MPI_ERR_UNKNOWN;
}

/* Look, if file exists on disk */
int ADIOI_SVM_Lookup_file_on_disk(char *name)
{
    HD_FileInfo  FileInfo;
    char          *filename;
    int           fd;
    
    /* determine file-name */
    filename = (char *)malloc(strlen(AD_SVM_PATH)+strlen(name)+20);
    sprintf(filename,"%s%s%s%s",AD_SVM_PATH,".",name,".adio_file_info");
    
    /* try to open file on disk */
    fd = open(filename,O_RDWR,0700);
    
    if (fd == -1) {
	/* file doesn't exist */
	return MPI_ERR_UNKNOWN;
    }
    else {
	/* => file on disk - read info */
	close(fd);
    
	return MPI_SUCCESS;
    }
}

/* Write file-info to disk */
void ADIOI_SVM_Write_new_file_info(ADIO_File file, int fd, int nprocs, ADIO_Offset size, ADIO_Offset used)
{
    HD_FileInfo       FileInfo;
    int               i;
    Node_List         nodes_local;
	
	
    FileInfo = (HD_FileInfo)malloc(sizeof(struct HD_FileInfo_D));
	
    for (i=0;i<nprocs;i++) {
	/* Get common rank of node and search it in node-list */
	nodes_local = (Node_List)NULL;
	ADIOI_SVM_Get_common_rank(file->comm,nprocs,i,&nodes_local);
	/*printf("nodes_local->name: %s\n",nodes_local->name);*/
	   
    
	FileInfo->nr_of_segs = nprocs; 	 		     /* nprocs segments alltogether */
	FileInfo->size = size;       			     /* set segment-size */
	if (used > size) {
	    FileInfo->bytes_used = size;     			     /* set number of used bytes */
	    used -= size;
	}
	else {
	    FileInfo->bytes_used = used;     			     /* set number of used bytes */
	    used = 0;
	}
	FileInfo->seg_nr = i;                                     /* i'th segment */
	FileInfo->segs_per_node = 1;                              /* one segment per node */
	   
	strcpy((char *)FileInfo->node,(const char *)nodes_local->name); /* node's name */
	FileInfo->requests = 0;				     /* 0 requests for segments */
	FileInfo->request_size = (ADIO_Offset)0;		     /* => request-size == 0 */

	write(fd,FileInfo,sizeof(struct HD_FileInfo_D));
	free(nodes_local);
    }
    free(FileInfo);
}

/* Update file-info on disk */
void ADIOI_SVM_Update_file_info(int fd, FileTable *p_files_local, SegmentTable *p_segments_local)
{
    HD_FileInfo    FileInfo;
    SegmentTable   segments_buf_local;
    int i=0;
	
    FileInfo = (HD_FileInfo)malloc(sizeof(struct HD_FileInfo_D));
    
    /* start with the file's first segment */
    *p_segments_local = (*p_files_local)->firstseg;
    do {
	FileInfo->nr_of_segs = (*p_files_local)->nr_of_nodes * (*p_files_local)->segs_per_node;
	FileInfo->size = (*p_segments_local)->size;
	FileInfo->bytes_used = (*p_segments_local)->bytes_used;
	FileInfo->seg_nr = i;
	FileInfo->segs_per_node = (*p_files_local)->segs_per_node;
	strcpy((char *)FileInfo->node,(const char *)(*p_segments_local)->node_name);
	FileInfo->requests = (*p_files_local)->requests;
	FileInfo->request_size = (*p_files_local)->request_size;

	write(fd,FileInfo,sizeof(struct HD_FileInfo_D));
	   
	segments_buf_local = *p_segments_local;
	if ((*p_segments_local)->next_fileseg != NULL) {
	    *p_segments_local = (*p_segments_local)->next_fileseg;
	    i++;
	}
    } while (segments_buf_local->next_fileseg != NULL);
	
    free(FileInfo);
}

/* Proof if required data is available */
/* Returns value that indicates how the file is distributed: */
/* 	result == 0:	No data on this node => try to fetch it */
/*	result == 1:	Data on this node, but not all data on involved nodes */
/*	result == 2:	All data avaiable, but distributed among less nodes => redistribute */
/*	result == 3:	All data avaiable, but distributed among more nodes => redistribute */
/*	result == 4:	Data is distributed in optimal order */
/*	result == 5:	Data distributed to more than one segment per node */
int ADIOI_SVM_Proof_data(ADIO_File file,int fd, ADIO_Offset *size)
{
    HD_FileInfo       FileInfo;
    Node_List         nodes_local;
    int		  mynod, nprocs, counter=0, result=-1, i, j, namelen, sender,
	conv_fd, seg_fd;
    char		  *buffer, *name_buf, *proc_name, *segmentname;
    MPI_Status	  status;
    ADIO_Offset	  offset, position;
	
	
    if (fd == -1)
	/* No parallel file-data on this node */
	result = 0;
    else {
	MPI_Comm_size(file->comm, &nprocs);
	MPI_Comm_rank(file->comm, &mynod);
	
	FileInfo = (HD_FileInfo)malloc(sizeof(struct HD_FileInfo_D));
	nodes_local = (Node_List)NULL;
	do {
	    read(fd,FileInfo,sizeof(struct HD_FileInfo_D));
	    /* lookup node-list */
	    if (ADIOI_SVM_Lookup_node(FileInfo->node,&nodes_local) != MPI_SUCCESS) {
		/* node not found => cannot open file */
		free(nodes_local);
		free(FileInfo);
		result = 1;
	    }
	    *size += (ADIO_Offset)FileInfo->bytes_used;
	    counter++;
	} while ((FileInfo->seg_nr<FileInfo->nr_of_segs-1)&&(result!=0));
	/* all segments lying on involved nodes */	   /*printf("size = %i\n",*size);*/

	if ((int)(counter/FileInfo->segs_per_node) < nprocs)
	    /* => all data available, but data distributed among less nodes => redistribute */
	    result = 2;
	else
	    if ((int)(counter/FileInfo->segs_per_node) > nprocs)
		/* => all data available, but data distributed among more nodes => redistribute */
		result = 3;
	    else
#ifdef RAY
		if (FileInfo->segs_per_node > 1) {
		    /* data distributed to more than one segment per node => redistribute */
		    
		    /* determine proc-name */
		    name_buf = (char *)malloc(MPI_MAX_PROCESSOR_NAME);
		    gethostname(name_buf,MPI_MAX_PROCESSOR_NAME);
		    namelen = strlen(name_buf);
		    proc_name = (char *)malloc(namelen+10);
		    strcpy(proc_name,(const char *)name_buf);
		    /*printf("procname: %s\n",proc_name);fflush(stdout);*/
		    
		    /* create conventional file and redistribute it later like any other conventional file */

		    /* node 0 creates conventional file and copies segments to it */
		    conv_fd = open(file->filename,O_CREAT | O_RDWR,0700);
		    if (!mynod)
			ftruncate(conv_fd,*size);
		    lseek(conv_fd,0,SEEK_SET);
		    
		    lseek(fd,0,SEEK_SET);
		    offset = (ADIO_Offset)0;
		    /* each segmet has to be copied */
		    for (i=0;i<FileInfo->nr_of_segs;i++) {
		    	read(fd,FileInfo,sizeof(struct HD_FileInfo_D));
			/*printf("read seg. %i\n",i);fflush(stdout);*/
			
			if (!strcmp(FileInfo->node,proc_name)) {
			    if (!mynod) {
				/* segment is local => copy it */
				/*printf("segment local\n");fflush(stdout);*/
				/*printf("bytes_used: %i\n",FileInfo->bytes_used);fflush(stdout);*/
				/*j = 0;*/
				/*MPI_Reduce(&j, &sender, 1, MPI_INT, MPI_MAX, 0, file->comm);*/
			      
				segmentname = (char *)malloc(strlen(AD_SVM_PATH)+strlen(file->filename)+30);
				sprintf(segmentname,"%s%s%s%i%s%i%s%i",AD_SVM_PATH,file->filename,"_seg_",
					mynod,"_",nprocs,"_",(FileInfo->seg_nr/nprocs));
				/*printf("segname: %s\n",segmentname);fflush(stdout);*/
				seg_fd = open(segmentname,O_RDWR,0700);
				/*printf("seg_fd: %i\n",seg_fd);fflush(stdout);*/
				/*printf("offset: %i\n",offset);fflush(stdout);*/
				buffer = (char*)malloc(FileInfo->bytes_used+10);
				read(seg_fd,buffer,FileInfo->bytes_used);
				close(seg_fd);
				unlink(segmentname);
				free(segmentname);
				position = lseek(conv_fd,offset,SEEK_SET);
				/*printf("lseek returned: %i\n",position);fflush(stdout);*/
				write(conv_fd,buffer,FileInfo->bytes_used);
				free(buffer);
			    }
			    else {
				/* segment local => send data to node 0 */
				/*printf("segment local\n");fflush(stdout);*/
				/*printf("bytes_used: %i\n",FileInfo->bytes_used);fflush(stdout);*/
				segmentname = (char *)malloc(strlen(AD_SVM_PATH)+strlen(file->filename)+30);
				sprintf(segmentname,"%s%s%s%i%s%i%s%i",AD_SVM_PATH,file->filename,"_seg_",
					mynod,"_",nprocs,"_",(FileInfo->seg_nr/nprocs));
				/*printf("segname: %s\n",segmentname);fflush(stdout);*/
				seg_fd = open(segmentname,O_RDWR,0700);
				/*printf("seg_fd: %i\n",seg_fd);fflush(stdout);*/
				/*printf("offset: %i\n",offset);fflush(stdout);*/
				buffer = (char*)malloc(FileInfo->bytes_used+10);
				read(seg_fd,buffer,FileInfo->bytes_used);
				close(seg_fd);
				unlink(segmentname);
				free(segmentname);
				position = lseek(conv_fd,offset,SEEK_SET);
				/*printf("lseek returned: %i\n",position);fflush(stdout);*/
				write(conv_fd,buffer,FileInfo->bytes_used);
				/*MPI_Reduce(&mynod, &sender, 1, MPI_INT, MPI_MAX, 0, file->comm);*/
				/*printf("sending to %i\n",0);fflush(stdout);*/
				/*MPI_Send(buffer,FileInfo->bytes_used,MPI_CHAR,0,0,file->comm);*/
				free(buffer);
			    }
			}
			offset += FileInfo->bytes_used;
		    }
		    close(conv_fd);
		     
		    result = 5;   
		}
		else
#endif
	            /* => data optimally distributed */
		    result = 4;

	free(nodes_local);
	free(FileInfo);
	close(fd);
    }
       
    return result;
}

/* Add a new empty segment to segment-list */
void ADIOI_SVM_Add_new_empty_seg(int mynod, int node, int first, int last, 
				 ADIO_Offset size,
				 int amode, int perm, 
				 Node_List *p_nodes_local, FileTable *p_files_local, 
				 SegmentTable *p_segments_local, SegmentTable *p_segments_buf_local)
{
    char *seg_name;


    /*printf("Entering ADIOI_SVM_Add_new_empty_seg().\n");fflush(stdout);*/
	
    if (*p_segments_local == (SegmentTable)NULL)
	*p_segments_local = (SegmentTable)malloc(sizeof(struct SegmentTable_D));
	   
    (*p_segments_local)->RegionPointer = (void *)NULL;
    (*p_segments_local)->RegionHandle = (int)NULL;
    (*p_segments_local)->size = ADIOI_SVM_Compute_segment_size(size,p_files_local);
	
    (*p_segments_local)->bytes_used = (ADIO_Offset)0;
    strcpy((char *)(*p_segments_local)->node_name,(const char *)(*p_nodes_local)->name);
    (*p_segments_local)->self = *p_segments_local;
	
    /* if segment local, open it */
    if (mynod == node) {
	seg_name = (char *)malloc(strlen(AD_SVM_PATH)+strlen((*p_files_local)->name)+16);
	sprintf(seg_name,"%s%s%s%i%s%i%s%i",AD_SVM_PATH,(*p_files_local)->name,"_seg_",
		mynod,"_",(*p_files_local)->nr_of_nodes,"_",(*p_files_local)->segs_per_node-1);
	(*p_segments_local)->fd = open(seg_name,O_RDWR,perm);
	if ((*p_segments_local)->fd == -1) {
	    /* segment doesn't exist => create it */
	    /*printf("Create new segment '%i' for node '%i'.\n",(*p_files_local)->segs_per_node-1,mynod);*/
	    (*p_segments_local)->fd = open(seg_name,O_CREAT|amode,perm);
	    ftruncate((*p_segments_local)->fd,(*p_segments_local)->size);
	}
	else {
	    /* segment exists => reopen with committed amode */
	    close((*p_segments_local)->fd);
	    (*p_segments_local)->fd = open(seg_name,amode,perm);
	}
	free(seg_name);
    }
    else
	(*p_segments_local)->fd = -1;
	   
    (*p_segments_local)->prev_seg = *p_segments_buf_local;
    if (first) {
	(*p_segments_local)->prev_fileseg = (SegmentTable)NULL;
	(*p_files_local)->firstseg = *p_segments_local;
    }
    else {
	(*p_segments_local)->prev_fileseg = (*p_segments_local)->prev_seg;
    }
    if (last) {
	(*p_segments_local)->next_fileseg = (SegmentTable)NULL;
	(*p_segments_local)->next_seg = (SegmentTable)NULL;
	(*p_files_local)->lastseg = *p_segments_local;
    }
    else {
	(*p_segments_local)->next_fileseg = (SegmentTable)malloc(sizeof(struct SegmentTable_D));
	(*p_segments_local)->next_seg = (*p_segments_local)->next_fileseg;
	*p_segments_buf_local = *p_segments_local;
	*p_segments_local = (*p_segments_local)->next_seg;
    }
}


/* Add an empty segment to segment-list */
void ADIOI_SVM_Add_empty_seg(char *node, char *filename, int rank, int nr_of_nodes, int seg_nr, int first, int last, ADIO_Offset bytes_used, ADIO_Offset size, int amode, int perm, Node_List *p_nodes_local, FileTable *p_files_local, SegmentTable *p_segments_local, SegmentTable *p_segments_buf_local){
    char *seg_name, *proc_name, *name_buf;
    int  namelen;


    if (*p_segments_local == NULL)
	*p_segments_local = (SegmentTable)malloc(sizeof(struct SegmentTable_D));
	   
    (*p_segments_local)->RegionPointer = (void *)NULL;
    (*p_segments_local)->RegionHandle = (int)NULL;
    (*p_segments_local)->size = size;
    (*p_segments_local)->bytes_used = bytes_used;
    strcpy((char *)(*p_segments_local)->node_name,(const char *)(*p_nodes_local)->name);
    (*p_segments_local)->self = *p_segments_local;
	
    /* get node-name */
    name_buf = (char *)malloc(MPI_MAX_PROCESSOR_NAME);
    gethostname(name_buf,MPI_MAX_PROCESSOR_NAME);
    namelen = strlen(name_buf);
    proc_name = (char *)malloc(namelen+10);
    strcpy(proc_name,(const char *)name_buf);
	
    /* if segment local, open it */
    if (!strcmp(node,proc_name)) {
	seg_name = (char *)malloc(strlen(AD_SVM_PATH)+strlen((*p_files_local)->name)+16);
	sprintf(seg_name,"%s%s%s%i%s%i%s%i",AD_SVM_PATH,filename,"_seg_",rank,"_",nr_of_nodes,"_",seg_nr);
	(*p_segments_local)->fd = open(seg_name,O_RDWR,perm);
	if ((*p_segments_local)->fd == -1) {
	    /* segment doesn't exist => create it */
	    /*printf("Create new segment '%i' for node '%i'.\n",seg_nr,rank);*/
	    (*p_segments_local)->fd = open(seg_name,O_CREAT|amode,perm);
	    ftruncate((*p_segments_local)->fd,(*p_segments_local)->size);
	}
	else {
	    /* segment exists => reopen with committed amode */
	    close((*p_segments_local)->fd);
	    (*p_segments_local)->fd = open(seg_name,amode,perm);
	}
	free(seg_name);
    }
    else
	(*p_segments_local)->fd = -1;
	
    (*p_segments_local)->prev_seg = *p_segments_buf_local;
    if (first) {
	(*p_segments_local)->prev_fileseg = (SegmentTable)NULL;
	(*p_files_local)->firstseg = *p_segments_local;
    }
    else {
	(*p_segments_local)->prev_fileseg = (*p_segments_local)->prev_seg;
    }
    if (last) {
	(*p_segments_local)->next_fileseg = (SegmentTable)NULL;
	(*p_segments_local)->next_seg = (SegmentTable)NULL;
	(*p_files_local)->lastseg = *p_segments_local;
    }
    else {
	(*p_segments_local)->next_fileseg = (SegmentTable)malloc(sizeof(struct SegmentTable_D));
	(*p_segments_local)->next_seg = (*p_segments_local)->next_fileseg;
	*p_segments_buf_local = *p_segments_local;
	*p_segments_local = (*p_segments_local)->next_seg;
    }
    free(proc_name);
    free(name_buf);
}

/* Reads in file <fd> from position <offset> <len> bytes to <buf> */
/* if error => return -1 */
int ADIOI_SVM_Read(ADIO_File fd, void *buf, ADIO_Offset offset, ADIO_Offset len)
{
    ADIO_Offset         start_pos, rest_len, new_offset, read_sz, off_buf, max_off, size_buf;
    int                 err, nprocs, mynod;
    FileTable           files_local;
    SegmentTable        segments_local;


    /*printf("Entering ADIOI_SVM_Read().\n");*/
	
    files_local = (FileTable)NULL;
    if (ADIOI_SVM_Lookup_fd(fd->fd_sys,&files_local)!=MPI_SUCCESS) {
	/* fd not found => error */
	printf("ADIOI_SVM_Read(): File-Descriptor %i not found.\n",fd->fd_sys);
	free(files_local);
	return -1;
    }
	
    MPI_Comm_size(fd->comm, &nprocs);
    MPI_Comm_rank(fd->comm, &mynod);       

    off_buf = offset+len;
    MPI_Allreduce(&off_buf,&max_off,1,ADIO_OFFSET,MPI_MAX,fd->comm);
	
    /* Get copy of file's segments-structure */
    segments_local = (SegmentTable)NULL;
    ADIOI_SVM_STRUCT_Copy_segments(&files_local,&segments_local);
	
    size_buf = ADIOI_SVM_Compute_file_size(files->nr_of_nodes*files->segs_per_node,&files_local,&segments_local);
	
    /*printf("ADIOI_SVM_Read(): Max.Offset: %i.\n",max_off);*/
    /*printf("ADIOI_SVM_Read(): Filesize: %i.\n",size_buf);*/
    /*if (size_buf<off_buf) {*/
    /* => read until end of file */
    /*len = size_buf - offset;*/
    ADIOI_SVM_Resize(fd, max_off, &err);
    if (err != MPI_SUCCESS) {
	/* => error */
	printf("ADIOI_SVM_Read(): Couldn't resize file '%s'.\n",fd->filename);
	free(segments_local);
	free(files_local);
	return -1;
    }
    /* Get new copy of data-structure */
    ADIOI_SVM_Lookup_fd(fd->fd_sys,&files_local);
    ADIOI_SVM_STRUCT_Copy_segments(&files_local,&segments_local);
    /*}*/
	
    /* search start-position <offset> in file's segments */
    segments_local = files_local->firstseg;
    start_pos = segments_local->size;
    /*printf("ADIOI_SVM_Read(): Offset: %i.\n",offset);*/
    while (start_pos < offset) {
	if (segments_local->next_fileseg == NULL) {
	    printf("ADIOI_SVM_Read(): Error while searching startposition.\n");
	    free(segments_local);
	    free(files_local);
	    return -1;
	}
	/*printf("ADIOI_SVM_Read(): Startposition: %i.\n",start_pos);*/
	segments_local = segments_local->next_fileseg;
	start_pos += segments_local->size;
    }
    /*  found start-segment */
    start_pos -= segments_local->size;

    /* rest_len and new_offset start with committed values */
    rest_len = len;
    new_offset = offset;
	
    while (start_pos < offset+len) {
	if (start_pos+segments_local->size < offset+len) {
	    /* Cannot read everything from this segment => split reading */
	    read_sz = (ADIO_Offset) (segments_local->size+start_pos-new_offset);
	      
	    if (fd->atomicity == 1) {
		/*SVMLockFraction(segments_local->RegionHandle,new_offset-start_pos,read_sz);*/
		/*printf("Locking from %i %i Bytes.\n",new_offset-start_pos,read_sz);*/
	    }
		 
	    memcpy((char *)buf+len-rest_len,(const char *)segments_local->RegionPointer+new_offset-start_pos,read_sz);
	      
	    if (fd->atomicity == 1) {
		/*SVMUnlockFraction(segments_local->RegionHandle,new_offset-start_pos,read_sz);*/
		/*printf("Unlocking from %i %i Bytes.\n",new_offset-start_pos,read_sz);*/
	    }
		 
	    /* adapt pointer */
	    rest_len -=  segments_local->size-(new_offset-start_pos);
	    start_pos += segments_local->size;
	    new_offset = start_pos;
	    /*printf("ADIOI_SVM_Read(): Read %i bytes.\n",read_sz);*/
	    /*printf("ADIOI_SVM_Read(): %i bytes left to read.\n",rest_len);*/
	      
	    /* goto next segment */
	    segments_local = segments_local->next_fileseg;
	      
	}
	else {
	    /* rest_len matches the segment => read rest from segment */
	      
	    if (fd->atomicity == 1) {
		/*SVMLockFraction(segments_local->RegionHandle,new_offset-start_pos,rest_len);*/
		/*printf("Locking from %i %i Bytes.\n",new_offset-start_pos,rest_len);*/
	    }
		 
	    memcpy((char *)buf+len-rest_len,(const char *)segments_local->RegionPointer+new_offset-start_pos,rest_len);
	      
	    if (fd->atomicity == 1) {
		/*SVMUnlockFraction(segments_local->RegionHandle,new_offset-start_pos,rest_len);*/
		/*printf("Unlocking from %i %i Bytes.\n",new_offset-start_pos,rest_len);*/
	    }
		 
	    start_pos += segments_local->size;
	    /*printf("ADIOI_SVM_Read(): Read %i bytes.\n",rest_len);*/
	}
    }
    /*printf("Leaving ADIOI_SVM_Read().\n");*/
				
    free(segments_local);
    free(files_local);
	
    return 0;
}

/* Writes to file <fd> from position <offset> <len> bytes from <buf> */
/* if error => return -1 */
int ADIOI_SVM_Write(ADIO_File fd, void *buf, ADIO_Offset offset, ADIO_Offset len)
{
    int                 i, nprocs, mynod, namelen, err;
    ADIO_Offset         start_pos, rest_len, new_offset, write_sz,
	size_buf, max_off, off_buf;
    char                *name_buf, *node_name, *seg_name;
    SegmentTable        next_seg_buf;
    FileTable           files_local;
    SegmentTable        segments_local;


    /*printf("Entering ADIOI_SVM_Write().\n");*/
	
    files_local = (FileTable)NULL;
    if (ADIOI_SVM_Lookup_fd(fd->fd_sys,&files_local)!=MPI_SUCCESS) {
	/* fd not found => error */
	printf("ADIOI_SVM_Write(): File-Descriptor %i not found.\n",fd->fd_sys);
	free(files_local);
	return -1;
    }

    MPI_Comm_size(fd->comm, &nprocs);
    MPI_Comm_rank(fd->comm, &mynod);       

    off_buf = offset+len;
    MPI_Allreduce(&off_buf,&max_off,1,ADIO_OFFSET,MPI_MAX,fd->comm);
	
    /* Get copy of file's segments-structure */
    segments_local = (SegmentTable)NULL;
    ADIOI_SVM_STRUCT_Copy_segments(&files_local,&segments_local);
	
    size_buf = ADIOI_SVM_Compute_file_size(files->nr_of_nodes*files->segs_per_node,&files_local,&segments_local);

    ADIOI_SVM_Resize(fd, max_off, &err);
    if (err != MPI_SUCCESS) {
	/* => error */
	printf("ADIOI_SVM_Write(): Couldn't resize file '%s'.\n",fd->filename);
	free(segments_local);
	free(files_local);
	return -1;
    }
    /* Get new copy of data-structure */
    ADIOI_SVM_Lookup_fd(fd->fd_sys,&files_local);
    ADIOI_SVM_STRUCT_Copy_segments(&files_local,&segments_local);

    /* search start-position <offset> in file's segments */
    segments_local = files_local->firstseg;
    start_pos = segments_local->size;
    while (start_pos < offset) {
	if (segments_local->next_fileseg == NULL) {
	    printf("ADIOI_SVM_Write(): Error while searching startposition.\n");
	    free(segments_local);
	    free(files_local);
	    return -1;
	}
	segments_local = segments_local->next_fileseg;
	start_pos += segments_local->size;
    }
    /*  found start-segment */
    start_pos -= segments_local->size;
    /* rest_len and new_offset start with committed values */
    rest_len = len;
    new_offset = offset;
			
    while (start_pos < offset+len) {
	if (start_pos+segments_local->size < offset+len) {
	    /* Cannot write everything to this segment => split writing */
	    write_sz = (ADIO_Offset) (segments_local->size+start_pos-new_offset);
	      
	    if (fd->atomicity == 1) {
		/*SVMLockFraction(segments_local->RegionHandle,new_offset-start_pos,write_sz);*/
		/*printf("Locking from %i %i Bytes.\n",new_offset-start_pos,write_sz);*/
	    }

	    /* XXX is this really char* ? */
	    memcpy((char *)segments_local->RegionPointer+new_offset-start_pos,
		   (const char *)buf+len-rest_len,write_sz);
	      
	    if (fd->atomicity == 1) {
		/*SVMUnlockFraction(segments_local->RegionHandle,new_offset-start_pos,write_sz);*/
		/*printf("Unlocking from %i %i Bytes.\n",new_offset-start_pos,write_sz);*/
	    }
		 	      
	    /* adapt pointer */
	    rest_len -=  segments_local->size-(new_offset-start_pos);
	    start_pos += segments_local->size;
	    new_offset = start_pos;
	      
	    /* goto next segment */
	    segments_local = segments_local->next_fileseg;
	}
	else {
	    /* rest_len matches the segment => write rest to segment */
	      
	    if (fd->atomicity == 1) {
		/*SVMLockFraction(segments_local->RegionHandle,new_offset-start_pos,rest_len);*/
		/*printf("Locking from %i %i Bytes.\n",new_offset-start_pos,rest_len);*/
	    }
		 
	    /* XXX is this really char* ? */
	    memcpy((char *)segments_local->RegionPointer+new_offset-start_pos,
		   (const char *)buf+len-rest_len, rest_len);
	      
	    if (fd->atomicity == 1) {
		/*SVMUnlockFraction(segments_local->RegionHandle,new_offset-start_pos,rest_len);*/
		/*printf("Unlocking from %i %i Bytes.\n",new_offset-start_pos,rest_len);*/
	    }
		 	      
	    start_pos += segments_local->size;
	}
    }
    ADIOI_SVM_STRUCT_Update_files(0,&files_local,&segments_local);
    free(segments_local);
    free(files_local);
	
    return 0;
}

/* Sends delete-message for file <filename> */
int ADIOI_SVM_Delete(char *filename)
{
    int	mynod;

     
    /*printf("Entering ADIOI_SVM_Delete().\n");*/
   
    MPI_Comm_rank(MPI_COMM_WORLD, &mynod);
   
    if (SVMSendDelFileMsg(mynod) == -1) {
	/* => error */
	printf("ADIOI_SVM_Delete(): Sending delete-messages failed.\n");
      
	return MPI_ERR_UNKNOWN;
    }
    ADIOI_SVM_Delete_file_master(filename);

    return MPI_SUCCESS;
}

/* Deletes file <filename> - the master has to broadcast filename to other processes */
int ADIOI_SVM_Delete_file_master(char *filename)
{
    MPI_File            	     fh;
    int                 	     files_fd, i, nprocs, mynod, master, ret, len;
    char                	     *seg_name, *info_filename;
    HD_FileInfo	    	     FileInfo;
    FileTable           	     files_local;
    SegmentTable        	     segments_local;


    /*printf("Entering ADIOI_SVM_Delete_file_master().\n");*/
	
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &mynod);
	
    /*** first of all show, that I'm master and then broadcast filename to other processes */
	
    len = strlen(filename);
    /*printf("(%i) Broadcasting: type: MPI_INT - value: %i - root: %i.\n",mynod,len,mynod);*/
    MPI_Bcast(&len,1,MPI_INT,mynod,MPI_COMM_WORLD);
    /*printf("(%i) Broadcasting: type: MPI_CHAR - value: %s - len: %i - root: %i.\n",mynod,filename,len+10,mynod);*/
    MPI_Bcast(filename,len+10,MPI_CHAR,mynod,MPI_COMM_WORLD);

    /*** second: proof, if file exist ***/
	
    /* determine info-file-name */
    info_filename = (char *)malloc(strlen(AD_SVM_PATH)+strlen(filename)+20);
    sprintf(info_filename,"%s%s%s%s",AD_SVM_PATH,".",filename,".adio_file_info");
    
    /* open info-file on disk */
    files_fd = open(info_filename,O_RDWR,0700);
	
    if (files_fd == -1) {
	/* file doesn't exist => done */
	printf("ADIOI_SVM_Delete_file_master(): File '%s' doesn't exist => done.\n",filename);
	   
	free(info_filename);
	   
	return MPI_SUCCESS;
    }
	
    /*** proof, if file is opened ***/
	
    files_local = (FileTable)NULL;
    segments_local = (SegmentTable)NULL;
	
    if (ADIOI_SVM_Lookup_file(filename,&files_local) == MPI_SUCCESS) {
	/* file open => remove it from file-list */
	/*printf("File open => remove from file-list...\n");*/
	ADIOI_SVM_STRUCT_Copy_segments(&files_local,&segments_local);
	ADIOI_SVM_Update_file_info(files_fd,&files_local,&segments_local);
	   
	/* delete (SVMlib-)regions */
	segments_local = files_local->firstseg;
	for (i=0;i<(files_local->segs_per_node*files_local->nr_of_nodes);i++) {
	    SVMDeleteRegion(segments_local->RegionHandle);
	    /* next file-segment */
	    if (segments_local->next_fileseg != NULL)
		segments_local = segments_local->next_fileseg;   
	}
	/* remove file and segments from lists */	
	ADIOI_SVM_STRUCT_Remove_file(&files_local,&segments_local);
    }
	
    /*** now, remove segments and info-file from disk ***/
	
    seg_name = (char *)malloc(strlen(AD_SVM_PATH)+strlen(filename)+16);
    FileInfo = (HD_FileInfo)malloc(sizeof(struct HD_FileInfo_D));
    lseek(files_fd,0,SEEK_SET);
    do {
	read(files_fd,FileInfo,sizeof(struct HD_FileInfo_D));
	/*printf("Read: Seg-Nr.: %i - my rank: %i.\n",FileInfo->seg_nr,mynod);*/
	if ((FileInfo->seg_nr % nprocs) == mynod) {
	    /* => segment local */  
	    /* determine segment-name */
	    sprintf(seg_name,"%s%s%s%i%s%i%s%i",AD_SVM_PATH,filename,"_seg_",
		    mynod,"_",FileInfo->nr_of_segs/FileInfo->segs_per_node,"_",
		    FileInfo->seg_nr/(FileInfo->nr_of_segs/FileInfo->segs_per_node));
	      
	    /* delete segment from file */
	    if (unlink(seg_name) != 0) {
		/* => error */
		printf("ADIOI_SVM_Delete_file(): Error while deleting segment '%i' of file '%s'\n",i,filename);
		 
		free(info_filename);
		free(files_local);
		free(segments_local);
		free(seg_name);
		free(FileInfo);
		 
		return MPI_ERR_UNKNOWN;
	    }
	    /*printf("ADIOI_SVM_Delete_file(): Deleted segment '%i' of file '%s'\n",FileInfo->seg_nr/nprocs,filename);*/
	}  
    } while (FileInfo->seg_nr < FileInfo->nr_of_segs-1);
	   
    /* delete info-file */
    if (unlink(info_filename) != 0) {
	/* => error */
	printf("ADIOI_SVM_Delete_file(): Error while deleting file-info of file '%s'\n",filename);
	   
	free(info_filename);
	free(files_local);
	free(segments_local);
	free(seg_name);
	free(FileInfo);
	   
	return MPI_ERR_UNKNOWN;
    }	
    /*printf("Deleted file '%s'\n",filename);*/
	   
      
    free(info_filename);
    free(files_local);
    free(segments_local);
    free(seg_name);
    free(FileInfo);

    return MPI_SUCCESS;
}

/* Deletes file <filename> */
int ADIOI_SVM_Delete_file(int sender)
{
    MPI_File            	     fh;
    int                 	     files_fd, i, nprocs, mynod, ret, len;
    char                	     *seg_name, *info_filename, *filename;
    HD_FileInfo	    	     FileInfo;
    FileTable           	     files_local;
    SegmentTable        	     segments_local;


    /*printf("Entering ADIOI_SVM_Delete_file().\n");*/
	
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &mynod);
	
    /*** first of all get filename ***/
	
    MPI_Bcast(&len,1,MPI_INT,sender,MPI_COMM_WORLD);
    /*printf("(%i) Broadcasting: type: MPI_INT - value: %i - root: %i.\n",mynod,len,sender);*/
    filename = (char *)malloc(len+10);	
    MPI_Bcast(filename,len+10,MPI_CHAR,sender,MPI_COMM_WORLD);
    /*printf("(%i) Broadcasting: type: MPI_CHAR - value: %s - len: %i - root: %i.\n",mynod,filename,len+10,sender);*/
	
    /*** second: proof, if file exist ***/
	
    /* determine info-file-name */
    info_filename = (char *)malloc(strlen(AD_SVM_PATH)+strlen(filename)+20);
    sprintf(info_filename,"%s%s%s%s",AD_SVM_PATH,".",filename,".adio_file_info");
    
    /* open info-file on disk */
    files_fd = open(info_filename,O_RDWR,0700);
	
    if (files_fd == -1) {
	/* file doesn't exist => done */
	printf("ADIOI_SVM_Delete_file(): File '%s' doesn't exist => done.\n",filename);
	   
	free(info_filename);
	   
	return MPI_SUCCESS;
    }
	
    /*** proof, if file is opened ***/
	
    files_local = (FileTable)NULL;
    segments_local = (SegmentTable)NULL;
	
    if (ADIOI_SVM_Lookup_file(filename,&files_local) == MPI_SUCCESS) {
	/* file open => remove it from file-list */
	/*printf("File open => remove from file-list...\n");*/
	ADIOI_SVM_STRUCT_Copy_segments(&files_local,&segments_local);
	ADIOI_SVM_Update_file_info(files_fd,&files_local,&segments_local);
	   
	/* delete (SVMlib-)regions */
	segments_local = files_local->firstseg;
	for (i=0;i<(files_local->segs_per_node*files_local->nr_of_nodes);i++) {
	    SVMDeleteRegion(segments_local->RegionHandle);
	    /* next file-segment */
	    if (segments_local->next_fileseg != NULL)
		segments_local = segments_local->next_fileseg;   
	}
	/* remove file and segments from lists */	
	ADIOI_SVM_STRUCT_Remove_file(&files_local,&segments_local);
    }
	
    /*** now, remove segments and info-file from disk ***/
	
    seg_name = (char *)malloc(strlen(AD_SVM_PATH)+strlen(filename)+16);
    FileInfo = (HD_FileInfo)malloc(sizeof(struct HD_FileInfo_D));
    lseek(files_fd,0,SEEK_SET);
    do {
	read(files_fd,FileInfo,sizeof(struct HD_FileInfo_D));
	/*printf("Read: Seg-Nr.: %i - my rank: %i.\n",FileInfo->seg_nr,mynod);*/
	if ((FileInfo->seg_nr % nprocs) == mynod) {
	    /* => segment local */  
	    /* determine segment-name */
	    sprintf(seg_name,"%s%s%s%i%s%i%s%i",AD_SVM_PATH,filename,"_seg_",mynod,
		    "_",FileInfo->nr_of_segs/FileInfo->segs_per_node,"_",
		    FileInfo->seg_nr/(FileInfo->nr_of_segs/FileInfo->segs_per_node));
	      
	    /* delete segment from file */
	    if (unlink(seg_name) != 0) {
		/* => error */
		printf("ADIOI_SVM_Delete_file(): Error while deleting segment '%i' of file '%s'\n",i,filename);
		 
		free(info_filename);
		free(files_local);
		free(segments_local);
		free(seg_name);
		free(FileInfo);
		 
		return MPI_ERR_UNKNOWN;
	    }
	    /*printf("ADIOI_SVM_Delete_file(): Deleted segment '%i' of file '%s'\n",FileInfo->seg_nr/nprocs,filename);*/
	}  
    } while (FileInfo->seg_nr < FileInfo->nr_of_segs-1);
	   
    /* delete info-file */
    if (unlink(info_filename) != 0) {
	/* => error */
	printf("ADIOI_SVM_Delete_file(): Error while deleting file-info of file '%s'\n",filename);
	   
	free(info_filename);
	free(files_local);
	free(segments_local);
	free(seg_name);
	free(FileInfo);
	   
	return MPI_ERR_UNKNOWN;
    }	
    /*printf("Deleted file '%s'\n",filename);*/
	   
      
    free(info_filename);
    free(files_local);
    free(segments_local);
    free(seg_name);
    free(FileInfo);

    return MPI_SUCCESS;
}

/* Synchronize a file's in-memory state with that on the physical medium */
int ADIOI_SVM_Sync(ADIO_File fd)
{
    FileTable           files_local;
    SegmentTable        segments_local,
	segments_buf_local;
    char		    *buf;


    /*printf("Entering ADIOI_SVM_Sync().\n");*/
	
    files_local = (FileTable)NULL;
    if (ADIOI_SVM_Lookup_fd(fd->fd_sys,&files_local) != MPI_SUCCESS) {
	/* file not found */
	printf("ADIOI_SVM_Sync(): File '%s' not opened.\n",fd->filename);
	free(files_local);
	return MPI_ERR_UNKNOWN;
    }
    /* Get copy of file's segments-structure */
    segments_local = (SegmentTable)NULL;
    ADIOI_SVM_STRUCT_Copy_segments(&files_local,&segments_local);
	
    segments_local = files_local->firstseg;
    do {
	if (segments_local->fd != -1) {
	    /* segment local => update segment */
	    buf = (char *)malloc(segments_local->bytes_used);
			
	    /* actualize SVMlib-region */
	    memcpy((void *)buf,(const void *)segments_local->RegionPointer,segments_local->bytes_used);
	    /* synchronize file contents to match the current contents of the memory region */ 
	    msync((void *)segments_local->RegionPointer,segments_local->bytes_used,MS_SYNC);

	    free(buf);	      
	}

	segments_buf_local = segments_local;
	if (segments_local->next_fileseg != NULL)
	    segments_local = segments_local->next_fileseg;
    } while (segments_buf_local != files_local->lastseg);
	
    ADIOI_SVM_STRUCT_Update_files(0,&files_local,&segments_local);
	
    free(files_local);
    free(segments_local);
	
    return MPI_SUCCESS;
}

/* lseek-replacement */
ADIO_Offset ADIOI_SVM_Lseek(ADIO_File fd, ADIO_Offset offset)
{
    SegmentTable  next_seg_buf;
    int           i, nprocs, mynod, namelen, err;
    ADIO_Offset   start_pos, rest_len, new_offset, write_sz, 
	size_buf, max_off;
    char          *name_buf, *node_name, *seg_name;
    FileTable           files_local;
    SegmentTable        segments_local;

	
    /*printf("Entering ADIOI_SVM_Lseek().\n");*/

    files_local = (FileTable)NULL;
    if (ADIOI_SVM_Lookup_fd(fd->fd_sys,&files_local) != MPI_SUCCESS) {
	/* file not found */
	printf("ADIOI_SVM_Lseek(): File '%s' not opened.\n",fd->filename);
	free(files_local);
	return -1;
    }
	
    MPI_Comm_size(fd->comm, &nprocs);
    MPI_Comm_rank(fd->comm, &mynod); 
	
    MPI_Allreduce(&offset,&max_off,1,ADIO_OFFSET,MPI_MAX,fd->comm);
	
    /* Get copy of file's segments-structure */
    segments_local = (SegmentTable)NULL;
    ADIOI_SVM_STRUCT_Copy_segments(&files_local,&segments_local);
	
    size_buf = ADIOI_SVM_Compute_file_size(files->nr_of_nodes*files->segs_per_node,&files_local,&segments_local);
    if (size_buf<max_off) {
	/* => add new segments */
	ADIOI_SVM_Resize(fd, max_off, &err);
	if (err != MPI_SUCCESS) {
	    /* => error */
	    printf("ADIOI_SVM_Lseek(): Couldn't resize file '%s'.\n",fd->filename);
	    free(files_local);
	    free(segments_local);
	    return -1;
	}
    }	
    free(files_local);
    free(segments_local);

    return offset;
}

/* Computes the optimal segment-size at a time (as multiple of the pagesize) */
ADIO_Offset ADIOI_SVM_Compute_segment_size(ADIO_Offset size, FileTable *p_files_local)
{
    int page_size;
	
    if ((*p_files_local)->requests == 0) {
	/* first request */
	(*p_files_local)->request_size = size;
    }
    else {
	/* compute request-size as maximum of avarage request-size and requested size */
	/* the new segment-size has to be equal on all nodes, thus compute it only one */
	/* time */
	if ((*p_files_local)->requests%(*p_files_local)->nr_of_nodes == 0)
	    (*p_files_local)->request_size = (ADIO_Offset)(((*p_files_local)->request_size*(*p_files_local)->requests+size)/(ADIO_Offset)((*p_files_local)->requests+1));
	else
	    (*p_files_local)->request_size = size;
	if ((*p_files_local)->request_size < size)
	    (*p_files_local)->request_size = size;
    }
    (*p_files_local)->requests++;

    /* segment-size has to be a multiple of AD_SVM_SEGMENT_SIZE */
    /*page_size = getpagesize();*/
    if ((ADIO_Offset)((*p_files_local)->request_size%(ADIO_Offset)AD_SVM_SEGMENT_SIZE) != (ADIO_Offset)0) {
	(*p_files_local)->request_size = (ADIO_Offset)(((*p_files_local)->request_size/
							(ADIO_Offset)AD_SVM_SEGMENT_SIZE+(ADIO_Offset)1)*AD_SVM_SEGMENT_SIZE);
    }
    /*printf("New segment-size: %i bytes.\n",(*p_files_local)->request_size);*/

    return (*p_files_local)->request_size;
}

/* Computes the total size of the actual file's first <num> segments */
ADIO_Offset ADIOI_SVM_Compute_file_size(int num, FileTable *p_files_local, SegmentTable *p_segments_local)
{
    ADIO_Offset  ret=(ADIO_Offset)0;
    int          i;
	
	
    if (num > ((*p_files_local)->nr_of_nodes*(*p_files_local)->segs_per_node)) {
	/* error => return filesize */
	num = (*p_files_local)->nr_of_nodes*(*p_files_local)->segs_per_node;
    }
	
    /* start with first file-segment */
    *p_segments_local = (*p_files_local)->firstseg;

    for (i=0;i<num;i++) {
	ret += (*p_segments_local)->size;
	if ((*p_segments_local)->next_fileseg != NULL)
	    *p_segments_local = (*p_segments_local)->next_fileseg;
    }
	
    return ret;
}

/* Returns the actual RegionHandle */
int ADIOI_SVM_Get_region_handle()
{
    int                     ret;
    extern int              NextRegionHandle;
    extern int              *RegionHandles;
    extern unsigned long   	RegionHandleLock;


    /*printf("Get_region_handle().\n");fflush(stdout);*/
    /**************************/   
    /* Enter critical section */
    /*printf("Aquire Lock...\n");*/
    SVMAcquireSyncObject(RegionHandleLock,0);
    /* locked... */
   		
    memcpy((int *)&NextRegionHandle,(const int *)RegionHandles,sizeof(int));
	
    ret = (NextRegionHandle-1);
    /*printf("GetRegionHandle(): RegionHandle: %i\n",ret);*/
	
    /* unlock... */
    /*printf("Release Lock...\n");*/
    SVMReleaseSyncObject(RegionHandleLock);
    /* Left critical section */
    /**************************/   
   
    return ret;
}

/* Updates the actual RegionHandle */
int ADIOI_SVM_Update_region_handle()
{
    int                     ret;
    extern int              NextRegionHandle;
    extern int              *RegionHandles;
    extern unsigned long   	RegionHandleLock;

		
    /*printf("Update_region_handle().\n");fflush(stdout);*/
    /**************************/   
    /* Enter critical section */
    /*printf("Aquire Lock...\n");*/
    SVMAcquireSyncObject(RegionHandleLock,0);
    /* locked... */
   		
    memcpy((int *)&NextRegionHandle,(const int *)RegionHandles,sizeof(int));
    NextRegionHandle++;
    memcpy((int *)RegionHandles,(const int *)&NextRegionHandle,sizeof(int));
	
    /*printf("UpdateRagionHandle(): NextRegionHandle == %i\n",NextRegionHandle);*/
	
    ret = (NextRegionHandle-1);
    /*printf("RegionHandle: %i\n",ret);*/
	
    /* unlock... */
    /*printf("Release Lock...\n");*/
    SVMReleaseSyncObject(RegionHandleLock);
    /* Left critical section */
    /**************************/   
   
    return ret;
}
