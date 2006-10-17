#ifndef _USI_TCP_PROTOCOL_H
#define _USI_TCP_PROTOCOL_H

#include "usi_tcp_common.h"

int USI_tcp_Establish(int* argc, char** argv[], USI_rank_t* rank, USI_rank_t* size, USI_Handle* handle, USI_Protocol* this);
int USI_tcp_Send(USI_rank_t dest, void* buffer, USI_size_t size, USI_Byte blocking, USI_Request* request, USI_Handle handle, USI_Protocol* this);
int USI_tcp_Recv(USI_rank_t src,  void* buffer, USI_size_t size, USI_Byte blocking, USI_Request* request, USI_Handle handle, USI_Protocol* this);
int USI_tcp_Test(USI_Byte blocking, USI_Request* request, USI_Handle handle, USI_Protocol* this);
int USI_tcp_Probe(USI_rank_t src, USI_Byte blocking, USI_Handle handle, USI_Protocol* this);

#endif
