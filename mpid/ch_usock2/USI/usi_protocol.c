#include "usi.h"
#include "usi_tcp_protocol.h"
#include "usi_self_protocol.h"

char USI_RESULT[4][16]={ "USI_SUCCESS",
			 "USI_ERROR",
			 "USI_RETRY",
			 "USI_PENDING"};

char* USI_Print_ERROR(int error)
{
  switch(error)
  {
    case USI_SUCCESS: return USI_RESULT[0];
    case USI_ERROR:   return USI_RESULT[1];
    case USI_RETRY:   return USI_RESULT[2];
    case USI_PENDING: return USI_RESULT[3];
  }
}

int  USI_protocol_Establish(int* usi_argc, char** usi_argv[], USI_rank_t* rank, USI_rank_t* size, USI_Handle* handle, USI_Protocol* this);
void USI_protocol_Error(char* string, int value);
void USI_protocol_Warning(char* string, int value);

int USI_Establish(int* argc, char** argv[], USI_rank_t* rank, USI_rank_t* size, USI_Handle* handle)
{
  int i;
  char* argument;
  int protocol_count;
  int protocol_number;

  USI_Protocol* first_protocol;
  USI_Protocol* actual_protocol;
  USI_Protocol* last_protocol;

  (*rank)=-1; (*size)=-1;  handle->first_protocol=NULL;  

  /* set up the list of protocols: */
  protocol_count=0;

  /* check for usi-virtual protocol: */
  if(USI_basic_check_arg(argc, argv, "--usi"))
  {
    actual_protocol=(USI_Protocol*)malloc(sizeof(USI_Protocol));
    if(protocol_count==0) first_protocol=actual_protocol;
    else  last_protocol->next_protocol=actual_protocol;
    actual_protocol->next_protocol=NULL;
    last_protocol=actual_protocol;
    protocol_count++;

    actual_protocol->protocol_argc=0;
    actual_protocol->protocol_argv=(char**)malloc(USI_TCP_MAX_ARG_NUM*sizeof(char*));

    while(argument=USI_basic_pop_argv2str(argc, argv, "--usi", "--"))
    {
      printf("VIRTUAL: %s\n", argument);

      actual_protocol->protocol_argv[actual_protocol->protocol_argc]=argument;
      actual_protocol->protocol_argc++;
    }

    actual_protocol->USI_Protocol_Establish=USI_protocol_Establish;    
  }

  /* check for self-send protocol: */
  if(USI_basic_check_arg(argc, argv, "--usi_self"))
  {
    actual_protocol=(USI_Protocol*)malloc(sizeof(USI_Protocol));
    if(protocol_count==0) first_protocol=actual_protocol;
    else  last_protocol->next_protocol=actual_protocol;
    actual_protocol->next_protocol=NULL;
    last_protocol=actual_protocol;
    protocol_count++;
    
    actual_protocol->protocol_argc=0;
    actual_protocol->protocol_argv=(char**)malloc(USI_TCP_MAX_ARG_NUM*sizeof(char*));

    while(argument=USI_basic_pop_argv2str(argc, argv, "--usi_self", "--"))
    {
      printf("SELF: %s\n", argument);
      actual_protocol->protocol_argv[actual_protocol->protocol_argc]=argument;
      actual_protocol->protocol_argc++;
    }

    actual_protocol->USI_Protocol_Establish=USI_self_Establish;
    actual_protocol->USI_Protocol_Send=USI_self_Send;
    actual_protocol->USI_Protocol_Recv=USI_self_Recv;
    actual_protocol->USI_Protocol_Test=USI_self_Test;
    actual_protocol->USI_Protocol_Probe=USI_self_Probe;
  }

  /* check for standard TCP protocol: */
  if(USI_basic_check_arg(argc, argv, "--usi_tcp"))
  {
    actual_protocol=(USI_Protocol*)malloc(sizeof(USI_Protocol));
    if(protocol_count==0) first_protocol=actual_protocol;
    else  last_protocol->next_protocol=actual_protocol;
    actual_protocol->next_protocol=NULL;
    last_protocol=actual_protocol;
    protocol_count++;
    
    actual_protocol->protocol_argc=0;
    actual_protocol->protocol_argv=(char**)malloc(USI_TCP_MAX_ARG_NUM*sizeof(char*));

    while(argument=USI_basic_pop_argv2str(argc, argv, "--usi_tcp", "--"))
    {
      printf("TCP: %s\n", argument);
      actual_protocol->protocol_argv[actual_protocol->protocol_argc]=argument;
      actual_protocol->protocol_argc++;
    }

    actual_protocol->USI_Protocol_Establish=USI_tcp_Establish;
    actual_protocol->USI_Protocol_Send=USI_tcp_Send;
    actual_protocol->USI_Protocol_Recv=USI_tcp_Recv;
    actual_protocol->USI_Protocol_Test=USI_tcp_Test;
    actual_protocol->USI_Protocol_Probe=USI_tcp_Probe;
  }

  if(protocol_count>0)
  {
    for(actual_protocol=first_protocol; actual_protocol!=NULL; actual_protocol=actual_protocol->next_protocol)
    {
      actual_protocol->USI_Protocol_Establish(&actual_protocol->protocol_argc, &actual_protocol->protocol_argv, rank, size, handle, actual_protocol);
    }
  }
  else
  {
    printf("NO USI-PROTOCOL SPECIFIED!\n");
    exit(0);
  }
}

int USI_protocol_Establish(int* usi_argc, char** usi_argv[], USI_rank_t* rank, USI_rank_t* size, USI_Handle* handle, USI_Protocol* this)
{
  USI_rank_t rank_iter;

  if(handle->first_protocol==NULL)
  {
    /* this is the very first protocol: */
    handle->first_protocol=this;

    if((*rank)<0) (*rank)=USI_basic_pop_argv2int(usi_argc, usi_argv, "-rank", "-");
    if((*size)<0) (*size)=USI_basic_pop_argv2int(usi_argc, usi_argv, "-size", "-");

    /* check for illegale values: */
    if( ((*rank)<0) || ((*size)<=0) || ((*rank))>=((*size)) )
    {
      USI_protocol_Error("invalid rank or size supplied", 0);
      exit(-1);
    }

    printf("### SIZE: %d\n", (*size));
    printf("### RANK: %d\n", (*rank));   

    /* store rank and size in the handle: */
    handle->rank=(*rank);  handle->size=(*size);

    /* create the global protocol table in the handle: */
    handle->protocol_table=(USI_Protocol**)malloc(handle->size*sizeof(USI_Protocol*));
    for(rank_iter=0; rank_iter<(handle->size); rank_iter++) handle->protocol_table[rank_iter]=this;

    return 1;
  }
  else
  {
    /* not the first protocol -> there are secondary protocols ... */
    return 0;
  }
}

void USI_protocol_Error(char* string, int value)
{
  if(value!=0) fprintf(stderr, "### USI: Protocol ERROR -> %s (%d)\n", string, value);
  else fprintf(stderr, "### USI: Protocol ERROR -> %s\n", string);
  fflush(stderr);
}

void USI_protocol_Warning(char* string, int value)
{
  if(value!=0) fprintf(stderr, "### USI: Protocol WARNING -> %s (%d)\n", string, value);
  else fprintf(stderr, "### USI: Protocol WARNING -> %s\n", string);
  fflush(stderr);
}

int USI_Send(USI_rank_t dest, void* buffer, USI_size_t size, USI_Byte blocking, USI_Request* request, USI_Handle handle)
{
  USI_Protocol *this_protocol;

  /* XXX ERROR HANDLING (src<0) */

  this_protocol=handle.protocol_table[dest];

  /* XXX ERROR HANDLING (no valid protocol) */
  return this_protocol->USI_Protocol_Send(dest, buffer, size, blocking, request, handle, this_protocol);
}

int USI_Recv(USI_rank_t src,  void* buffer, USI_size_t size, USI_Byte blocking, USI_Request* request, USI_Handle handle)
{
  USI_Protocol *this_protocol;

  /* XXX ERROR HANDLING (src<0) */

  this_protocol=handle.protocol_table[src];

  /* XXX ERROR HANDLING (no valid protocol) */
  return this_protocol->USI_Protocol_Recv(src, buffer, size, blocking, request, handle, this_protocol);
}

int USI_Test(USI_Byte blocking, USI_Request* request, USI_Handle handle)
{
  USI_Protocol *this_protocol;

  this_protocol=handle.protocol_table[request->rank];
  return this_protocol->USI_Protocol_Test(blocking, request, handle, this_protocol);
}

int USI_Probe(USI_rank_t src, USI_Byte blocking, USI_Handle handle)
{
  USI_Protocol *this_protocol;

  this_protocol=handle.protocol_table[src];
  return this_protocol->USI_Protocol_Probe(src, blocking, handle, this_protocol);
}

