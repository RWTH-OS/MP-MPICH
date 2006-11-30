/* 
 *   $Id$    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 *
 */

#ifndef __AD_SVM_INCLUDE
#define __AD_SVM_INCLUDE

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/asynch.h>
#include "adio.h"

#define main
#include "svmlibc.h"
#undef main

int ADIOI_SVM_aio(ADIO_File fd, void *buf, int len, ADIO_Offset offset,
                  int wr, void *handle);

/* filename convention:
 * -------------------
 *
 * segments are saved as follows:
 *
 * <AD_SVM_PATH><filename>_seg_<node's rank>_<#nodes>_<segment's nr>
 *
 */

/***** defines *****/
#define AD_SVM_SEGMENT_SIZE (1024*1024)  /* default-segment-size => should be equal to page-size */
#define AD_SVM_PATH ("/ad_svm/")         /* default-path, every file is locally saved to */
#define AD_SVM_MAX_NAME_SIZE (256)       /* maximum (file-)name-size */
#define AD_SVM_MAX_NODE (1024)           /* maximum number of nodes */
#define AD_SVM_FD_OFFSET (3)             /* offset for filedescriptors */
					  /* because of stdout, stderr, etc. */

#define AD_SVM_REGION_OFFSET (3)         /* offset for regionhandles */
					  /* because of regions for global variables */

/*#define AD_SVM_PAGE (RC_SCILRC)*/	  /* SVMlib-Page-Class ==> Lazy release consistency */
#define AD_SVM_PAGE (SSC_MRSW)	  	/* SVMlib-Page-Class ==> Strong sequential consistency/MRSW */
/*** end defines ***/


/***** type-definitions *****/
typedef unsigned long AD_SVM_OFFSET;   /* for 32bit systems, we can only use this offset type */

/* persistent file-specific info */
/* saved to "<DEFAULT_PATH>.<filename>.adio_file_info" */ 
struct HD_FileInfo_D {
  char         node[AD_SVM_MAX_NAME_SIZE];/* node the segment is locally saved to */
  int          seg_nr;                     /* node's <seg_nr>th segment */
  ADIO_Offset  size;                       /* segment-size in bytes */
  int          nr_of_segs;                 /* number of the file's segments */
  int          segs_per_node;              /* number of segments per node */
  ADIO_Offset  bytes_used;                 /* bytes really used in segment <seg_nr> */
  int          requests;                   /* number of requests for new segments */
  ADIO_Offset  request_size;               /* avarage request-size */
};
typedef struct HD_FileInfo_D *HD_FileInfo;

/* list of segments belonging to opened files */
/* hold in working memory */
struct SegmentTable_D {
  int          fd;				  /* segment's fd */
  void         *RegionPointer;                    /* pointer to the dedicated (SVMlib-)region */
  int          RegionHandle;                      /* handle of the dedicated (SVMlib-)region */
  ADIO_Offset  size;                              /* segment-size in bytes */
  ADIO_Offset  bytes_used;                        /* number of bytes really in use */      
  char         node_name[MPI_MAX_PROCESSOR_NAME]; /* name of node the segment is persistently */
                                          	  /* lying on */  
  struct SegmentTable_D *self;            /* pointer to itself */
  struct SegmentTable_D *prev_fileseg;    /* pointer to the file's previous segment */
  struct SegmentTable_D *next_fileseg;    /* pointer to the file's next segment */
  struct SegmentTable_D *prev_seg;        /* pointer to the previous segment */
  struct SegmentTable_D *next_seg;        /* pointer to the next segment */
};
typedef struct SegmentTable_D *SegmentTable;

/* list of opened files */
/* held in working memory */
struct FileTable_D {
  int          fd;  			   /* file-descriptor */
  char         name[AD_SVM_MAX_NAME_SIZE];/* filename */
  ADIO_Offset  size;                       /* file-size in bytes */
  int          nr_of_nodes;                /* number of nodes the file is distributed to */
  int          segs_per_node;              /* number of segments per node */
  int          requests;                   /* number of requests for new segments */
  ADIO_Offset  request_size;               /* avarage request-size */
  struct         SegmentTable_D *firstseg; /* pointer to the file's first segment */
  struct         SegmentTable_D *lastseg;  /* pointer to the file's last segment */
  struct         FileTable_D *self;        /* pointer to itself */
  struct         FileTable_D *prevfile;    /* pointer to the previous file */
  struct         FileTable_D *nextfile;    /* pointer to the next file */
};
typedef struct FileTable_D *FileTable;

/* list of nodes participating the actual MPI-session */
/* hold in working memory */
/* needed for ad_smi_open() */
struct Node_List_D {
  char name[MPI_MAX_PROCESSOR_NAME]; /* node-name - gethostname() */
  int  rank;			     /* node's rank */
  struct Node_List_D *prev;          /* previous node */
  struct Node_List_D *next;          /* next node */
};
typedef struct Node_List_D *Node_List;

/* list of threads created for asynchronous I/O */
struct Thread_List_D {
   ADIO_Request         request;
   pthread_t            tid;
   int                  done;   
   struct Thread_List_D *prev;
   struct Thread_List_D *next;
};
typedef struct Thread_List_D *Thread_List;

/*** end type-definitions ***/


/***** global variables *****/
SegmentTable segments;             /* list of segments belonging to opened files */
SegmentTable segments_buf;         /* segment-buffer */
FileTable    files;                /* list of opened files */
FileTable    files_buf;            /* file-buffer */
Node_List    nodes;                /* list of nodes participating the actual MPI_session */
Thread_List  threads;              /* list of threads created for asynchronous I/O */
int          *RegionHandles;       /* global region-handle */
int          NextRegionHandle;     /* next (SVMlib-)region / file-segments */
int          *FileHandles;         /* global file-handle */
int          NextFileHandle;	   /* next file-descriptor */

extern int _ad_svm_users;          /* current number of users - from ad_svm_fsys.c */


/* Mutexes for data-access-synchronisation */
pthread_mutex_t		NodesLock,
			FilesLock,
			SegmentsLock,
			ThreadsLock;
unsigned long		FileHandleLock,
			RegionHandleLock;
/*** end global variables ***/


#endif
