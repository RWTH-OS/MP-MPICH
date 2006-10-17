/*
 *	ad_smi_aio.h
 *
 *	added by RAY - 18.08.99
 *	last modification: 25.08.99
 *
 *	routines to manage the asynchronous file-accesses to the virtual parallel filesystem
 *
 */
 
#ifndef __AD_SVM_AIO_INCLUDE
#define __AD_SVM_AIO_INCLUDE

#include <errno.h>

#include "ad_svm.h"

/* own struct needed for pthread-function-call */
struct aio_arg_D {
   ADIO_File    fd;
   void         *buf;
   ADIO_Offset  offset;
   ADIO_Offset  len;
   aio_result_t *result;
};
typedef struct aio_arg_D *aio_arg;

/* Threadfunction for asynchronous read */
void *ADIOI_SVM_aread(aio_arg arg);

/* Threadfunction for asynchronous write */
void *ADIOI_SVM_awrite(aio_arg arg);

/* Asynchronous read */
int ADIOI_SVM_Aioread(ADIO_File fd,void *buf,ADIO_Offset len,ADIO_Offset offset,aio_result_t *result);

/* Asynchronous write */
int ADIOI_SVM_Aiowrite(ADIO_File fd,void *buf,ADIO_Offset len,ADIO_Offset offset,aio_result_t *result);

#endif
