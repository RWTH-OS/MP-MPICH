#ifndef _USI_SELF_PROTOCOL_H
#define _USI_SELF_PROTOCOL_H

#include "usi.h"

typedef struct _USI_self_Data
{
  void     *buffer;
  USI_size_t size;
  USI_Byte active_send;
  USI_Byte active_recv;
  USI_Byte message_sent;
} USI_self_Data;


int USI_self_Establish(int* argc, char** argv[], USI_rank_t* rank, USI_rank_t* size, USI_Handle* handle, USI_Protocol* this);
int USI_self_Send(USI_rank_t dest, void* buffer, USI_size_t size, USI_Byte blocking, USI_Request* request, USI_Handle handle, USI_Protocol* this);
int USI_self_Recv(USI_rank_t src,  void* buffer, USI_size_t size, USI_Byte blocking, USI_Request* request, USI_Handle handle, USI_Protocol* this);
int USI_self_Test(USI_Byte blocking, USI_Request* request, USI_Handle handle, USI_Protocol* this);
int USI_self_Probe(USI_rank_t src, USI_Byte blocking, USI_Handle handle, USI_Protocol* this);

#endif
