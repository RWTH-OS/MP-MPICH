
#include "usi_self_protocol.h"

int USI_self_Establish(int* usi_argc, char** usi_argv[], USI_rank_t* rank, USI_rank_t* size, USI_Handle* handle, USI_Protocol* this)
{

  if(USI_protocol_Establish(usi_argc, usi_argv, rank, size, handle, this))
  {
    /* this is the first protocol: */
    *rank=0;  *size=1;
  }

  this->private.size=sizeof(USI_self_Data);
  this->private.data=(USI_Pointer)malloc(sizeof(USI_self_Data));

  ((USI_self_Data*)(this->private.data))->active_send=0;
  ((USI_self_Data*)(this->private.data))->active_recv=0;
  ((USI_self_Data*)(this->private.data))->message_sent=0;

  return USI_SUCCESS;
}

int USI_self_Send(USI_rank_t dest, void* buffer, USI_size_t size, USI_Byte blocking, USI_Request* request, USI_Handle handle, struct _USI_Protocol* this)
{
  if(dest!=handle.rank)
  {
    printf("ERROR: destination for self-send must be myself!\n");
    exit(1);
  }
  if(blocking)
  {
    /* XXXX temp buffer !?! */
    printf("ERROR: self-send must be non-blocking!\n");
    exit(1);
  }

  /* already sending? */
  if(((USI_self_Data*)(this->private.data))->active_send) return USI_RETRY;
  ((USI_self_Data*)(this->private.data))->active_send=1;
  ((USI_self_Data*)(this->private.data))->message_sent=0;

  /* Initialize the USI_Request: */
  request->buffer=buffer;
  request->final_size=size;
  request->actual_size=0;
  request->rank=dest;
  request->handle=&handle;
  request->active_send=1;
  request->active_recv=0;
  
  /* store the message size and the pointer to the buffer: */
  ((USI_self_Data*)(this->private.data))->size=size;
  ((USI_self_Data*)(this->private.data))->buffer=buffer;

  return USI_PENDING;
}

int USI_self_Recv(USI_rank_t src,  void* buffer, USI_size_t size, USI_Byte blocking, USI_Request* request, USI_Handle handle, struct _USI_Protocol* this)
{ 
  if(src!=handle.rank)
  {
    printf("ERROR: source for self-send must be myself!\n");
    exit(1);
  }

  /* already receiving? */
  if(((USI_self_Data*)(this->private.data))->active_recv) return USI_RETRY;
  ((USI_self_Data*)(this->private.data))->active_recv=1;

  /* Initialize the USI_Request: */
  request->buffer=buffer;
  request->final_size=size;
  request->actual_size=0;
  request->rank=src;
  request->handle=&handle;
  request->active_send=0;
  request->active_recv=1;

  /* check, if the send request is already initiated: */
  if(((USI_self_Data*)(this->private.data))->active_send==1)
  {
    /* check for a valid message size: */
    if(((USI_self_Data*)(this->private.data))->size!=size)
    {
      /* XXX ERROR: Message truncated! */
    }
    
    /* copy the message: */
    memcpy(buffer, ((USI_self_Data*)(this->private.data))->buffer, size);

    request->actual_size=request->final_size;
    ((USI_self_Data*)(this->private.data))->message_sent=1;

    return USI_SUCCESS;
  }
  else return USI_PENDING;
}

int USI_self_Test(USI_Byte blocking, USI_Request* request, USI_Handle handle, USI_Protocol* this)
{
  /* is this a send request? */
  if(request->active_send)
  {
    /* check, if the message is already sent: */
    if(((USI_self_Data*)(this->private.data))->message_sent)
    {
      request->actual_size=request->final_size;
      ((USI_self_Data*)(this->private.data))->active_send=0;
      return USI_SUCCESS;
    }
    else return USI_PENDING;
  }
  else
  {
    /* is this a recv request? */
    if(request->active_recv)
    {
      /* check, if the send request is already initiated: */
      if(((USI_self_Data*)(this->private.data))->active_send==1)
      {
	/* copy the message: */
	memcpy(request->buffer, ((USI_self_Data*)(this->private.data))->buffer, request->final_size);

	request->actual_size=request->final_size;
	((USI_self_Data*)(this->private.data))->message_sent=1;	

	return USI_SUCCESS;
      }
      else return USI_PENDING;
    }
    else return USI_ERROR;
  }
}

int USI_self_Probe(USI_rank_t src, USI_Byte blocking, USI_Handle handle, USI_Protocol* this)
{
}
