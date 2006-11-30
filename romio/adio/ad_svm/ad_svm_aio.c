/* $Id$
 *	ad_svm_aio.c
 *
 *	routines to manage the asynchronous file-accesses to the virtual parallel filesystem
 *
 */
 
#include "ad_svm_aio.h"


/* Threadfunction for asynchronous read */
void *ADIOI_SVM_aread(aio_arg arg)
{
  int        ret;
  pthread_t  tid;
  
  /*printf("Entering ADIOI_SVM_aread().\n");*/
  
  /*printf("Starting aread with fd = %i ...\n",(arg->fd)->fd_sys);*/
  (arg->result)->aio_return = (int) ADIOI_SVM_Read(arg->fd,arg->buf,arg->offset,arg->len);
  /*printf("Finished aread with ret = %i and errno = %i...\n",(arg->result)->aio_return,errno);*/
  (arg->result)->aio_errno = errno;
  
  tid = (pthread_t) SVMGetGlobalThreadId();
  /*printf("Global thread id: %x.\n",tid);*/ 
  ADIOI_SVM_STRUCT_Set_thread_done(tid); 
}

/* Threadfunction for asynchronous write */
void *ADIOI_SVM_awrite(aio_arg arg)
{
  int        ret;
  pthread_t  tid;
  
  /*printf("Entering ADIOI_SVM_awrite().\n");*/
  
  /*printf("Starting awrite with fd = %i ...\n",(arg->fd)->fd_sys);*/
  
  (arg->result)->aio_return = (int) ADIOI_SVM_Write(arg->fd,arg->buf,arg->offset,arg->len);
  
  /*printf("Finished awrite with ret = %i and errno = %i...\n",(arg->result)->aio_return,errno);*/
  (arg->result)->aio_errno = errno;
  
  tid = (pthread_t) SVMGetGlobalThreadId();
  /*printf("Global thread id: %x.\n",tid);*/
  ADIOI_SVM_STRUCT_Set_thread_done(tid); 
}

/* Asynchronous read */
int ADIOI_SVM_Aioread(ADIO_File fd,void *buf,ADIO_Offset len,ADIO_Offset offset,aio_result_t *result)
{
   aio_arg        args;
   unsigned long  tid;
   int            ret;
   
   /*printf("Entering ADIOI_SVM_Aioread().\n");*/
   
   args = (aio_arg)malloc(sizeof(struct aio_arg_D));
   args->fd = fd;
   args->buf = buf;
   args->offset = offset;
   args->len = len;
   args->result = result;
   
   /* create thread to execute asyn. read */
   ret = (int)SVMStartThread((SVMTHREAD_START_ROUTINE)ADIOI_SVM_aread, args, NULL, &tid);
 
   if (ret <= 0) {
      /* => error => try again */
      printf("ADIOI_SVM_Aioread(): Couldn't start thread...\n");
      errno = EAGAIN;
      return -1;
   }
   else 
      /* => succesful */
      return (int)tid;
}

/* Asynchronous write */
int ADIOI_SVM_Aiowrite(ADIO_File fd,void *buf,ADIO_Offset len,ADIO_Offset offset,aio_result_t *result)
{
   aio_arg        args;
   unsigned long  tid;
   int            ret;
   
   /*printf("Entering ADIOI_SVM_Aiowrite().\n");*/
   
   args = (aio_arg)malloc(sizeof(struct aio_arg_D));
   args->fd = fd;
   args->buf = buf;
   args->offset = offset;
   args->len = len;
   args->result = result;
   
   /* create thread to execute asyn. read */
   ret = (int)SVMStartThread((SVMTHREAD_START_ROUTINE)ADIOI_SVM_awrite, args, NULL, &tid);
 
   if (ret <= 0) {
      /* => error => try again */
      printf("ADIOI_SVM_Aiowrite(): Couldn't start thread...\n");
      errno = EAGAIN;
      return -1;
   }
   else 
      /* => succesful */
      return (int)tid;
}

