/*
 *	ad_svm_fsys.h
 *
 *	added by RAY - 02.08.99
 *	last modification: 15.11.99
 *
 *	routines to manage the virtual parallel filesystem
 *
 */
 
#ifndef __AD_SVM_FSYS_INCLUDE
#define __AD_SVM_FSYS_INCLUDE

#include <strings.h>
#include <sys/mman.h>

#include "ad_svm.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Initializing-Routine */
void ADIOI_SVM_Init_ad_svm();

/* Un-initializing-routine */
void ADIOI_SVM_Finalize_ad_svm();

/* Returns filename without path */
char *ADIOI_SVM_Basename(char *name,int *len);

/* Handles file requests from ADIOI_SVM_Redistribute_file() */
void ADIOI_SVM_Handle_file_request(int sender);

/* Redistribute existing file */
int ADIOI_SVM_Redistribute_file(ADIO_File file,
				int info_file,
				char *info_name,
				int type,
				int involved);

/* Lookup node in node-list */
int ADIOI_SVM_Lookup_node(char *filename, 
			  Node_List *p_nodes_local);

/* Lookup node's rank in node-list */
int ADIOI_SVM_Lookup_node_rank(int rank,
			       Node_List *p_nodes_local);

/* Proof, if node is in group */
int ADIOI_SVM_Is_in_group(int comm, int nprocs, char *name);

/* Get common rank of node and search it in node-list */
int ADIOI_SVM_Get_common_rank(int comm,
			      int nprocs,
			      int rank, 
			      Node_List *p_nodes_local);

/* Lookup file in file-list */
int ADIOI_SVM_Lookup_file(char *filename, 
			  FileTable  *p_files_local);

/* Lookup fd in file-list */
int ADIOI_SVM_Lookup_fd(int fd, 
			FileTable  *p_files_local);

/* Lookup filename in file-list */
int ADIOI_SVM_Lookup_filename(char *filename, FileTable  *p_files_local);

/* Look, if file exists on disk */
int ADIOI_SVM_Lookup_file_on_disk(char *name);

/* Write file-info to disk */
void ADIOI_SVM_Write_new_file_info(ADIO_File file, 
				   int fd,
				   int nprocs,
				   ADIO_Offset size,
				   ADIO_Offset used);

/* Update file-info on disk */
void ADIOI_SVM_Update_file_info(int fd, 
				FileTable *p_files_local, 
				SegmentTable *p_segments_local);

/* Proof if required data is available */
int ADIOI_SVM_Proof_data(ADIO_File file,int fd, ADIO_Offset *size);

/* Add an empty segment to segment-list */
void ADIOI_SVM_Add_new_empty_seg(int mynod, 
				 int node,
				 int first,
				 int last,
				 ADIO_Offset size,
				 int amode,
				 int perm,
				 Node_List *p_nodes_local,
				 FileTable *p_files_local,
				 SegmentTable *p_segments_local,
				 SegmentTable *p_segments_buf_local);

/* Add an empty segment to segment-list */
void ADIOI_SVM_Add_empty_seg(char *node,
			     char *filename,
			     int rank,
			     int nr_of_nodes,
			     int seg_nr,
			     int first,
			     int last,
			     ADIO_Offset bytes_used,
			     ADIO_Offset size,
			     int amode,
			     int perm,
			     Node_List *p_nodes_local,
			     FileTable *p_files_local,
			     SegmentTable *p_segments_local,
			     SegmentTable *p_segments_buf_local);

/* Reads in file <fd> from position <offset> <len> bytes to <buf> */
/* if error => return -1 */
int ADIOI_SVM_Read(ADIO_File fd,
		       void *buf,
		       ADIO_Offset offset,
		       ADIO_Offset len);

/* Writes to file <fd> from position <offset> <len> bytes from <buf> */
/* if error => return -1 */
int ADIOI_SVM_Write(ADIO_File fd,
			void *buf,
			ADIO_Offset offset,
			ADIO_Offset len);

/* Broadcasts delete-message for file <filename> */
int ADIOI_SVM_Delete(char *filename);

/* Deletes file <filename> - the master has to broadcast filename to other processes */
int ADIOI_SVM_Delete_file_master(char *filename);

/* Delete file */
int ADIOI_SVM_Delete_file(int sender);

/* Synchronize a file's in-memory state with that on the physical medium */
int ADIOI_SVM_Sync(ADIO_File fd);

/* lseek-replacement */
ADIO_Offset ADIOI_SVM_Lseek(ADIO_File fd,
			    ADIO_Offset offset);

/* Computes the optimal segment-size at a time */
ADIO_Offset ADIOI_SVM_Compute_segment_size(ADIO_Offset size,
					   FileTable *p_files_local);

/* Computes the total size of the actual file's first <num> segments */
ADIO_Offset ADIOI_SVM_Compute_file_size(int num,
					FileTable *p_files_local,
					SegmentTable *p_segments_local);

/* Returns the actual RegionHandle */
int ADIOI_SVM_Get_region_handle();

/* Updates the actual RegionHandle */
int ADIOI_SVM_Update_region_handle();

#ifdef __cplusplus
}
#endif

#endif
