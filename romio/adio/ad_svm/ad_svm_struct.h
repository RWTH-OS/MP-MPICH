/* $Id: ad_svm_struct.h 51 2000-04-19 17:42:21Z joachim $
 *	ad_svm_struct.h
 *
 *	routines to synchronize data-structure-accesses
 *
 */

#ifndef __AD_SVM_STRUCT_INCLUDE
#define __AD_SVM_STRUCT_INCLUDE

#include "ad_svm.h"

/************************************/
/* Routines for the nodes-structure */

/* Initialize the nodes-structure */
int ADIOI_SVM_STRUCT_Init_nodes(int nprocs, int mynod, int namelen, char *proc_name);

/* De-initialize nodes-structure */
int ADIOI_SVM_STRUCT_Free_nodes();

/* Frees the local copy of the nodes-structure */
int ADIOI_SVM_STRUCT_Free_nodes_copy(Node_List *p_nodes_local);

/* Creates a local copy of nodes-structure */
int ADIOI_SVM_STRUCT_Copy_nodes(Node_List *p_nodes_local);


/************************************/
/* Routines for the files-structure */

/* Initialize the files-structure */
int ADIOI_SVM_STRUCT_Init_files();

/* De-initialize files-structure */
int ADIOI_SVM_STRUCT_Free_files();

/* Frees the local copy of the files-structure */
int ADIOI_SVM_STRUCT_Free_files_copy(FileTable *p_files_local);

/* Creates a local copy of files-structure */
int ADIOI_SVM_STRUCT_Copy_files(FileTable *p_files_local);

/* Update files-structure */
int ADIOI_SVM_STRUCT_Update_files(int is_new, FileTable *p_files_local, SegmentTable *p_segments_local);

/* Create new entry in files-structure */
int ADIOI_SVM_STRUCT_Create_file(int nprocs, int mynod, ADIO_File file, int is_new, int files_fd, FileTable *p_files_local, SegmentTable *p_segments_local, ADIO_Offset size);

/* Remove entry from files-structure */
int ADIOI_SVM_STRUCT_Remove_file(FileTable *p_files_local, SegmentTable *p_segments_local);


/************************************/
/* Routines for the segments-structure */

/* Initialize the segments-structure */
int ADIOI_SVM_STRUCT_Init_segments();

/* De-initialize segments-structure */
int ADIOI_SVM_STRUCT_Free_segments();

/* Frees the local copy of a file's segments-structure */
int ADIOI_SVM_STRUCT_Free_segments_copy(SegmentTable *p_segments_local);

/* Creates a local copy of a file's segments-structure */
int ADIOI_SVM_STRUCT_Copy_segments(FileTable *p_files_local, SegmentTable *p_segments_local);


/************************************/
/* Routines for the threads-structure */

/* Initialize the threads-structure */
int ADIOI_SVM_STRUCT_Init_threads();

/* De-initialize threads-structure */
int ADIOI_SVM_STRUCT_Free_threads();

/* Create new entry in threads-structure */
int ADIOI_SVM_STRUCT_Create_thread(pthread_t *tid, ADIO_Request *request);

/* Remove entry from threads-structure */
int ADIOI_SVM_STRUCT_Remove_thread(ADIO_Request *request);

/* Set done-attribute of entry in threads-structure */
int ADIOI_SVM_STRUCT_Set_thread_done(pthread_t tid);

/* Get done-attribute of entry in threads-structure */
int ADIOI_SVM_STRUCT_Get_thread_done(ADIO_Request *request);

#endif
