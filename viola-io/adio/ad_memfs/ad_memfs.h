/**************************************************************************
* MEMFS                                                                   * 
***************************************************************************
*                                                                         * 
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              * 
*                                                                         * 
* See COPYRIGHT notice in base directory for details                      * 
**************************************************************************/
/**************************************************************************
* File:         ad_memfs.h                                                * 
* Description:                                                            *
* Author(s):    Jan Seidel <Jan.Seidel@fh-bonn-rhein-sieg.de>             * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/

#ifndef AD_MEMFS_H
#define AD_MEMFS_H

/* Log messages are only printed if DEBUG_MEMFS is set */
/* #define DEBUG_MEMFS */
/* #define DEBUG_THREADS */
/* #define DEBUG_DATADIST */
/* #define DEBUG_LOCKS */
/* #define MEMFS_TIME */

/*==========================================================================================================*/
/* INCLUDES */

#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <pthread.h>
#include "adio.h"
#include "pario_threads.h"
#include "memfs_time.h"

/*  Assertions mit assert() moeglich */
#include <assert.h>


/*==========================================================================================================*/
/* DEFINES */


/* MEMFS COMMUNICATOR */
#define MPI_COMM_MEMFS_WORLD    MPI_COMM_META_REDUCED

/* Defines for all ADIO functions */
/* Message Tags for Communication of main thread -> service thread */
#define MEMFS_OPEN               0x3002
#define MEMFS_CLOSE              0x3003
#define MEMFS_FCNTL              0x3004
#define MEMFS_SEEKIND            0x3005
#define MEMFS_DELETE             0x3006
#define MEMFS_RESIZE             0x3007
#define MEMFS_FLUSH              0x3008
#define MEMFS_SETINFO            0x3009

#define MEMFS_READCONT           0x3011
#define MEMFS_WRITECONT          0x3012
#define MEMFS_READSTRIDED        0x3013
#define MEMFS_WRITESTRIDED       0x3014
#define MEMFS_IREADCONT          0x3015
#define MEMFS_IWRITECONT         0x3016
#define MEMFS_IREADSTRIDED       0x3017
#define MEMFS_IWRITESTRIDED      0x3018
#define MEMFS_READCONTCOLL       0x3019
#define MEMFS_WRITECONTCOLL      0x301a
#define MEMFS_READSTRIDEDCOLL    0x301b
#define MEMFS_WRITESTRIDEDCOLL   0x301c
#define MEMFS_IREADCONTCOLL      0x301d
#define MEMFS_IWRITECONTCOLL     0x301e
#define MEMFS_IREADSTRIDEDCOLL   0x301f
#define MEMFS_IWRITESTRIDEDCOLL  0x3020


#define MEMFS_IODATA             0x3021

#define MEMFS_DELEGATED_OPEN     0x3030

/* Message Tags for locking and unlocking files (Multiserver) */
#define MEMFS_SETLOCK		 0x3050
#define MEMFS_REMOVELOCK	 0x3060

#define MEMFS_SHUTDOWN           0x3090

/* Message Tags for Communication of service thread -> main thread */
#define MEMFS_REPLY              0x4001
#define MEMFS_REPLY_IODATA       0x4002
#define MEMFS_REPLY_ERRORCODE    0x4003

#define MEMFS_TIME_DATA          0x4009

#define MEMFS_MAX_MSG_SIZE       16777216
#define MEMFS_BLOCKSIZE          104857600 /* 100 * 1024 * 1024 */
#if 0
#define MEMFS_BLOCKSIZE          100000000
#endif
#define DATAINDEX_SIZE           10000000 


#define MEMFS_IO_FAILURE         -3

/*==========================================================================================================*/
/* TYPEDEFS */

/* Definition of memfs specific variables and macros */

/* shared memory segment for communication of threads */
typedef struct {
    int function; /* Unique identifier for a memfs ADIO device function */
    ADIO_File fd;
    char *buffer;
    int error;
    int param;
    MPI_Datatype datatype;
    int file_ptr_type;
    ADIO_Offset offset;
    ADIO_Status *status;
    ADIO_Fcntl_t *fcntl_struct;
    void *extraParam;
} memfs_io_req_t;

/* thread parameters, more may be added */
typedef struct
{
    int num_servers;
    int master_server;
} memfs_param_t;

/* data of a message reply
typedef struct
{
int msg_size;
MPI_Status msg_status;
} reply_param_t;
*/

/*==========================================================================================================*/
/* VARIABLES */

/* Thread variables for main and service thread */
pthread_t thrMain;
pthread_t thrService;
int threads_initialized, memfs_shutdown;

memfs_io_req_t io_req;
/* memfs_param_t pthread_param; */


pthread_mutex_t mutex; 
pthread_mutex_t shutdown_mutex;
pthread_cond_t cond_new_call;
pthread_cond_t cond_finish_call;
pthread_cond_t cond_req_set;

pthread_mutex_t filesystem_mutex;


/*==========================================================================================================*/
/* FUNCTIONS */

void *thread_comm_io(int function, ADIO_File fd, char *buffer, int *error_code,
                    int param, MPI_Datatype datatype, int file_ptr_type,
                    ADIO_Offset offset, ADIO_Status *status, ADIO_Fcntl_t *fcntl_struct, void *extraParam);

void thread_comm(int function, ADIO_File fd, char *buffer, int *error_code);

double gettime(void);

int MEMFS_Init(int num_servers);
int MEMFS_Shutdown();

void ADIOI_MEMFS_Open(ADIO_File fd, int *error_code);
void ADIOI_MEMFS_Close(ADIO_File fd, int *error_code);
void ADIOI_MEMFS_ReadContig(ADIO_File fd, void *buf, int count,
                            MPI_Datatype datatype, int file_ptr_type,
                            ADIO_Offset offset, ADIO_Status *status, 
                            int *error_code);
void ADIOI_MEMFS_WriteContig(ADIO_File fd, void *buf, int count, 
			     MPI_Datatype datatype, int file_ptr_type,
			     ADIO_Offset offset, ADIO_Status *status, 
			     int *error_code);   
void ADIOI_MEMFS_IwriteContig(ADIO_File fd, void *buf, int count, 
			      MPI_Datatype datatype, int file_ptr_type,
			      ADIO_Offset offset, ADIO_Request *request, 
			      int *error_code);   
void ADIOI_MEMFS_IreadContig(ADIO_File fd, void *buf, int count, 
			     MPI_Datatype datatype, int file_ptr_type,
			     ADIO_Offset offset, ADIO_Request *request,
			     int *error_code);   
int ADIOI_MEMFS_ReadDone(ADIO_Request *request, ADIO_Status *status,
			 int *error_code);
int ADIOI_MEMFS_WriteDone(ADIO_Request *request, ADIO_Status *status,
			  int *error_code);
void ADIOI_MEMFS_ReadComplete(ADIO_Request *request, ADIO_Status *status,
			      int *error_code); 
void ADIOI_MEMFS_WriteComplete(ADIO_Request *request, ADIO_Status *status,
			       int *error_code); 
void ADIOI_MEMFS_Fcntl(ADIO_File fd, int flag, ADIO_Fcntl_t *fcntl_struct, 
		       int *error_code); 
void ADIOI_MEMFS_WriteStrided(ADIO_File fd, void *buf, int count,
		              MPI_Datatype datatype, int file_ptr_type,
		              ADIO_Offset offset, ADIO_Status *status,
		              int *error_code);
void ADIOI_MEMFS_ReadStrided(ADIO_File fd, void *buf, int count,
                             MPI_Datatype datatype, int file_ptr_type,
                             ADIO_Offset offset, ADIO_Status *status, 
                             int *error_code);
void ADIOI_MEMFS_WriteStridedColl(ADIO_File fd, void *buf, int count,
			          MPI_Datatype datatype, int file_ptr_type,
				  ADIO_Offset offset, ADIO_Status *status,
				  int *error_code);
void ADIOI_MEMFS_ReadStridedColl(ADIO_File fd, void *buf, int count,
				 MPI_Datatype datatype, int file_ptr_type,
				 ADIO_Offset offset, ADIO_Status *status,
				 int *error_code);
void ADIOI_MEMFS_IreadStrided(ADIO_File fd, void *buf, int count,
			      MPI_Datatype datatype, int file_ptr_type,
			      ADIO_Offset offset, ADIO_Request *request,
			      int *error_code);
void ADIOI_MEMFS_IwriteStrided(ADIO_File fd, void *buf, int count,
			       MPI_Datatype datatype, int file_ptr_type,
			       ADIO_Offset offset, ADIO_Request *request,
			       int *error_code);
void ADIOI_MEMFS_Flush(ADIO_File fd, int *error_code);
void ADIOI_MEMFS_Resize(ADIO_File fd, ADIO_Offset size, int *error_code);
ADIO_Offset ADIOI_MEMFS_SeekIndividual(ADIO_File fd, ADIO_Offset offset, 
			      	       int whence, int *error_code);
void ADIOI_MEMFS_SetInfo(ADIO_File fd, MPI_Info users_info, int *error_code);
void ADIOI_MEMFS_Get_shared_fp(ADIO_File fd, int size, 
			       ADIO_Offset *shared_fp, 
			       int *error_code);
void ADIOI_MEMFS_Set_shared_fp(ADIO_File fd, ADIO_Offset offset, 
			       int *error_code);
void ADIOI_MEMFS_Delete(char *filename, int *error_code);

#endif
