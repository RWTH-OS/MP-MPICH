
   /*********************************
--- ** mpxpriv.c                 ** -------------------------------------------------------
    *********************************/

#include "LogMpid.h"
#include "mydebug.h"

#include "mpxpriv.h"

#include "mpid.h"
#include "mpxdev.h"
#include "mpxpackets.h"
#include "mpxdebug.h"
#include "mpimem.h"

#include "stl2c_deque.h"
#include "stl2c_map.h"

#define _MPX_

#ifdef _MPX_
#include "mpx.h"
#endif

#define TRANSFERRING	0
#define HEADER_READ	1
#define CLEAN		2
#define CLOSED		3
    
#define BLOCKING		1
#define NONBLOCKING		0
#define MAX_LOOP 80
#undef  BLOCK
#define THREAD_SIZE 16384

#define WSA_OVERLAPPED      1
#define WSA_NOT_OVERLAPPED  0
    
void MPID_MPX_Error(const char *string,int value);


/*************************************
 ** definitions of local datatypes: **
 *************************************/

struct _ReceiveRequest;

typedef
struct _TransferRequest
{
    void *buf;
    unsigned int size;
    int finished;
    struct _ReceiveRequest* Receive;
} TransferRequest;

typedef
struct _SendRequest
{
#ifndef _MPX_
  WSAOVERLAPPED Overlapped;
  SOCKET fd;
#endif

  unsigned long size;
  WSABUF *buf;
  DWORD NumBlocks;
  MPXHeader h;
  BOOL finished;

#ifdef _MPX_
  MPX_Request mpx_request;
#endif

} SendRequest;


typedef
struct _SelfSend
{
  /*
   |   This structure represents a message that a process sends to itself,
   |   those messages are treated differently
   |   (not sent over a socket, but put into a local queue)
   */
  MPXHeader Header;
  void *buf;
  unsigned int offset;

} SelfSend;
static void MPID_MPX_Init_SelfSend(void*,const unsigned int, SelfSend*);


/* create the stl2c template functions: */
STL_deque_template(SelfSend, SelfSend*); 
STL_deque_template(SendReq, SendRequest*);
STL_deque_template(TraReq, TransferRequest*);
STL_map_template(int2TraReq, int, TransferRequest*);


typedef
struct _ReceiveRequest
{
  WSABUF          buf; /* <-- represents buffer for data, consisting of buf.len (length of buffer) and buf.buf ("real" buffer) */
  MPXHeader     Header;
  unsigned long   received,size;
  BOOL            finished; /* <-- TRUE if the last issued request is done, FALSE otherwise */
  TransferRequest *Transfer;
  int             State;
  BOOL            pending,shuttingDown;
  int             from;

#ifdef _MPX_
  MPX_Request mpx_request;
#endif

} ReceiveRequest;


/* cover all local data in a struct: */
typedef
struct _MPID_MPX_Data_priv_type
{
  STL_map_type TransferRequestMap;
  STL_deque_type SelfSendChannel;
  STL_deque_type SendRequestQueue;
  STL_deque_type TransferRequestQueue;

  int *Channels; /* <-- array of file descriptors of the open sockets */
  ReceiveRequest **ReceiveRequestArray;
  fd_set allFds;  /* <-- set of file descriptors
		   |     (actually a bit field with each bit representing an fd)
		   |     it is used for calls to select()
		   */
  unsigned int numConnections;

  /* this are the former static variables of some local functions: */
  int  _RecvAnyControl_current;
  int  _ControlMsgAvail_current;

} MPID_MPX_Data_priv_type;


/****************************************
 ** definitions of exported functions: **
 ****************************************/

void MPID_MPX_Init( int *, char ***);
void MPID_MPX_Finalize(void);

void MPID_MPX_RecvAnyControl(MPID_PKT_T*, int, int*);
int  MPID_MPX_ControlMsgAvail(void);
int  MPID_MPX_RecvFromChannelAsync(void*, int, int, ASYNCRecvId_t);
void MPID_MPX_RecvFromChannel(void*, int, int);
void MPID_MPX_ConsumeData(int, int);
void MPID_MPX_SendControl(MPID_PKT_T*,int,int);
/*
int  MPID_MPX_SendChannel(WSABUF *,int, unsigned long, int, ASYNCSendId_t, BOOL);
*/
int  MPID_MPX_SendChannel(WSABUF, int, int, ASYNCSendId_t, BOOL);
int  MPID_MPX_TestSend(ASYNCSendId_t);
void MPID_MPX_WaitSend(ASYNCSendId_t);

void MPID_MPX_CreateTransfer(void *, int, int, ASYNCRecvId_t);
void MPID_MPX_ReceiveTransfer(int, ASYNCRecvId_t);
void MPID_MPX_WaitTransfer(ASYNCRecvId_t);
int  MPID_MPX_TestTransfer(ASYNCRecvId_t);

void MPID_MPX_Error(const char *, int);
void MPID_MPX_SysError( const char *, int);


/*************************************
 ** definitions of local functions: **
 *************************************/

static void MPID_MPX_ReadLocal(void*, int);
#ifdef _MPX_
static void MPID_MPX_GenericWait(MPX_Request*);
#else
static void MPID_MPX_GenericWait(OVERLAPPED*);
#endif

static void MPID_MPX_Init_SendRequest(SendRequest* SendRequestPt);
static int  MPID_MPX_Start_SendRequest(WSABUF, int, int, SendRequest*, BOOL);
static void MPID_MPX_Free_SendRequest(SendRequest*);
static SendRequest *MPID_MPX_Get_SendRequest(void);

static void MPID_MPX_Init_ReceiveRequest(SOCKET, ReceiveRequest*);
static void MPID_MPX_Finish_ReceiveRequest(ReceiveRequest*);
static int  MPID_MPX_Start_ReceiveRequest(char*, unsigned long, int, ReceiveRequest*);
static int  MPID_MPX_CheckType_ReceiveRequest(ReceiveRequest*);
static int  MPID_MPX_Test_ReceiveRequest(int, ReceiveRequest*);
static void MPID_MPX_Wait_ReceiveRequest(ReceiveRequest*);
static int  MPID_MPX_GetHeader_ReceiveRequest(ReceiveRequest*);

static void MPID_MPX_Init_TransferRequest(void*, int, TransferRequest*);
static BOOL MPID_MPX_RecvTransferMessage(ReceiveRequest *);
static void MPID_MPX_Free_TransferRequest(TransferRequest*);
static TransferRequest *MPID_MPX_Get_TransferRequest(void*, int);

static MPID_MPX_Data_priv_type* MPID_MPX_Get_priv_data(MPID_Device*);


   /**********************************
--- ** exported (global) functions: ** --------------------------------------------------------
    **********************************/

void MPID_MPX_Init( int *argc, char *** argv )
{
  MPID_MPX_Data_global_type *global_data;
  MPID_MPX_Data_priv_type  *local_data;

  DSECTION("MPID_MPX_Init");

  unsigned long val=1;
  int i;
  
  DSECTENTRYPOINT;

#if 0
  {
    int i;

    printf("MPI_MPX: %d args:\n", *argc);
 
    for(i=0; i<*argc; i++)
      if((*argv)[i]) printf("%s\n",(*argv)[i]);
  }
#endif

  MPID_MPX_Test_device(MPID_devset->active_dev, "Init");

  local_data  = (MPID_MPX_Data_priv_type*)malloc(sizeof(MPID_MPX_Data_priv_type));
  global_data = (MPID_MPX_Data_global_type*)MPID_devset->active_dev->global_data;

  global_data->local_data_priv = (void*)local_data;

  /* initialize the static variables: */
  local_data->_RecvAnyControl_current=0;
  local_data->_ControlMsgAvail_current=0;
  
  STL_deque_constructor(&local_data->TransferRequestQueue);
  STL_deque_constructor(&local_data->SendRequestQueue);
  STL_deque_constructor(&local_data->SelfSendChannel);
  STL_map_constructor(&local_data->TransferRequestMap);

#ifdef _MPX_

  DNOTICE("Setup the MPX environment...");

  MPX_Init(argc, argv);
  MPX_Comm_rank(MPX_COMM_WORLD, &(global_data->MyWorldRank));
  MPX_Comm_size(MPX_COMM_WORLD, &(global_data->MyWorldSize));

  DNOTICE("MPX passed!");
  DNOTICEI("MPX rank", global_data->MyWorldRank);
  DNOTICEI("MPX size", global_data->MyWorldSize);

#else

  CTCPCommunicator_Constructor(&local_data->Comm);
  if(CTCPCommunicator_Create(argc,argv, &local_data->Comm) <0)
  {
    MPID_MPX_Error("Process Startup failed",-1);
  }
  
  global_data->MyWorldRank = CTCPCommunicator_GetMyId(&local_data->Comm);
  global_data->MyWorldSize = CTCPCommunicator_GetNumProcs(&local_data->Comm);

#endif

  DSETRANK(global_data->MyWorldRank);

#ifdef _MPX_ 
   local_data->numConnections =  global_data->MyWorldSize - 1;   
#else
  /* initializes Channels with an array of file descriptors for the open sockets: */
  local_data->numConnections = CTCPCommunicator_GetFDs(&local_data->Channels, &local_data->Comm);
  FD_ZERO(&local_data->allFds); /* set bits for all file descriptors to zero */
#endif

  local_data->ReceiveRequestArray = (ReceiveRequest**)malloc(global_data->MyWorldSize * sizeof(ReceiveRequest*));

  for(i=0;i<global_data->MyWorldSize;i++)
  {
    local_data->ReceiveRequestArray[i] = (ReceiveRequest*)malloc(sizeof(ReceiveRequest));

#ifdef _MPX_
    MPID_MPX_Init_ReceiveRequest(0, local_data->ReceiveRequestArray[i]);
    local_data->ReceiveRequestArray[i]->from=i;
#else
    MPID_MPX_Init_ReceiveRequest(local_data->Channels[i], local_data->ReceiveRequestArray[i]);
    if(local_data->Channels[i] != INVALID_SOCKET)
    {
      WSAEventSelect(local_data->Channels[i],global_data->Events[global_data->MyWorldRank],FD_READ); /* wsock2function */
      FD_SET(local_data->Channels[i],&local_data->allFds); /* sets the bit for the file descriptor Channels[i] in allFds */
    }
#endif
  }  
  MPID_THREAD_DS_LOCK_INIT(&tcp_mutex);
  
  DSECTLEAVE
    return;
}


void MPID_MPX_Finalize( void )
{
  MPID_MPX_Data_global_type *global_data;
  MPID_MPX_Data_priv_type  *local_data;
  
  DSECTION("MPID_MPX_Finalize");

  int i;
  MPXHeader msg;
  MPID_PKT_T   pkt;
  WSABUF SBuf;
  DWORD ToSend = 0;
  BOOL top = TRUE;

   DSECTENTRYPOINT;

  MPID_MPX_Test_device(MPID_devset->active_dev, "Finalize");

  /* get the pointer to the data structs of this device entity: */
  global_data = MPID_MPX_Get_global_data(MPID_devset->active_dev);
  local_data  = MPID_MPX_Get_priv_data(MPID_devset->active_dev);

  DNOTICEI("Shutting down MPX device, numConnections: ",local_data->numConnections);

#ifdef _MPX_
  
  MPX_Finalize();

  return;

#else
  for(i=global_data->MyWorldRank+1;(i<global_data->MyWorldSize)&&top;++i)
    top = (local_data->Channels[i] == INVALID_SOCKET);
  
  while(local_data->numConnections&&top) {	
        MPID_MPX_RecvAnyControl(&pkt,sizeof(pkt),&i);
  }
    
  for(i=0;i<global_data->MyWorldSize;++i) {
    if(i != global_data->MyWorldRank && local_data->Channels[i] != INVALID_SOCKET) {
      SBuf.len=PRIVATESIZE;
      SBuf.buf=(char*)&msg;
      msg.Type=QUIT;
      msg.Size=0;
      DNOTICEI("Sending QUIT to ",i);
      MPID_PKT_PACK(&msg,PRIVATESIZE,i);
      while(WSASend(local_data->Channels[i],&SBuf,1,&ToSend,0,0,0) && WSAGetLastError() == WSAEWOULDBLOCK){
	MPID_MPX_DEBUG_PRINT_MSG2("%d bytes sent",ToSend);
	Sleep( 1000 );
      }
      /* MPID_MPX_SendBlock(&SBuf,PRIVATESIZE,1,i); */
      local_data->ReceiveRequestArray[i]->shuttingDown = TRUE;
      shutdown(local_data->Channels[i],SD_SEND);
    }
  }
  
  while(local_data->numConnections) {	
    MPID_MPX_RecvAnyControl(&pkt,sizeof(pkt),&i);
  }
#endif
  
  /* Sleep(10); */
  
  MPID_THREAD_DS_LOCK_FREE(&tcp_mutex)

    while(STL_deque_size(&local_data->SendRequestQueue)) {
      free(STL_deque_front_TraReq(&local_data->SendRequestQueue));
      STL_deque_pop_front(&local_data->SendRequestQueue);
    }
  
    while(STL_deque_size(&local_data->TransferRequestQueue)) {
      free(STL_deque_front_TraReq(&local_data->TransferRequestQueue));
      STL_deque_pop_front(&local_data->TransferRequestQueue);
    }
    for(i=0;i<global_data->MyWorldSize;i++) {
      free(local_data->ReceiveRequestArray[i]);
    }
    free(local_data->ReceiveRequestArray);

    DSECTLEAVE
      return;
}


void MPID_MPX_RecvAnyControl( MPID_PKT_T *pkt, int size, int *from )
{
  MPID_MPX_Data_global_type *global_data;
  MPID_MPX_Data_priv_type  *local_data;
  
  DSECTION("MPID_MPX_RecvAnyControl");
  
  int current;
  int res,found;
  ReceiveRequest *Request;
  BOOL local;
  
  DSECTENTRYPOINT;

  MPID_MPX_Test_device(MPID_devset->active_dev, "RecvAnyControl");

  /* get the pointer to the data structs of this device entity: */
  global_data = MPID_MPX_Get_global_data(MPID_devset->active_dev);
  local_data  = MPID_MPX_Get_priv_data(MPID_devset->active_dev);
  
  /* get the former static variable from the global struct: */
  current = local_data->_RecvAnyControl_current;
  
  found = 0;
  LOG_ANY_CTRL(0);

#ifdef _MPX_
  while(!found)
  {
    if(current==global_data->MyWorldRank)
    {
      if(STL_deque_size(&(local_data->SelfSendChannel)))
      {
	Request=local_data->ReceiveRequestArray[current];
	found = MPID_MPX_CheckType_ReceiveRequest(Request);
	if(found)
	{
	  *from=current;
	  if(Request->Header.Size > size)
	  { /* XXX SIGNED <-> UNSIGNED CONFLICT !!! */
	    MPID_MPX_Error("Control message too large...\n",Request->Header.Size);
	  }
	  DNOTICEI("Trying to read pkt of size ",Request->Header.Size);
	  MPID_MPX_Start_ReceiveRequest(((char*)pkt)+PRIVATESIZE, Request->Header.Size-PRIVATESIZE,BLOCKING, Request);
	}
      }
    }
    else
    {    
      int probe_flag;
      MPX_Status probe_status;

      MPX_Iprobe(current, 99, MPX_COMM_WORLD, &probe_flag, &probe_status);
      if(probe_flag)
      {
	Request=local_data->ReceiveRequestArray[current];
	found = MPID_MPX_CheckType_ReceiveRequest(Request);
	if(found)
	{
	  *from=current;
	  if(Request->Header.Size > size)
	  { /* XXX SIGNED <-> UNSIGNED CONFLICT !!! */
	    MPID_MPX_Error("Control message too large...\n",Request->Header.Size);
	  }
	  DNOTICEI("Trying to read pkt of size ",Request->Header.Size);
	  MPID_MPX_Start_ReceiveRequest(((char*)pkt)+PRIVATESIZE, Request->Header.Size-PRIVATESIZE,BLOCKING, Request);
	}
      }
    }
    current=(current+1)%global_data->MyWorldSize;
  }
#else
  while(!found) {
    local=FALSE;
    if(STL_deque_size(&local_data->SelfSendChannel)) {
      /* we got a message from ourselves */
      res=1;
      local = TRUE;
      current = global_data->MyWorldRank;
    } else if((global_data->MyWorldSize>1) && (local_data->numConnections)) {
      
      /* select all file descriptors (sockets) as being ready for reading: */
      readfds = local_data->allFds;
      res=select(FD_SETSIZE,&readfds,0,0,0);
      if(res==SOCKET_ERROR) {
	MPID_MPX_SysError("RecvAnyControl: select failed",WSAGetLastError());
      }
      
    } else 
      if(global_data->MyWorldSize>1)
		res = 0;
      else {
	DERROR("Logical error. Waiting for message while num_procs == 1!");
	MPID_MPX_Error("Logical error. Waiting for message while num_procs == 1!",-1);
      }
    while( (res>0) && (!found) ) {
      if(current >= global_data->MyWorldSize) current = 0;
      if(
	 (
	  (local_data->Channels[current] != INVALID_SOCKET)	&&    /* we have a valid socket for communication with current */
	  (FD_ISSET(local_data->Channels[current],&readfds))       /* and it was selected as being ready for reading above */
	  ) || 
	 (
	  local && 
	  (current == global_data->MyWorldRank)
	  )
	 ) {
	--res;
	Request=local_data->ReceiveRequestArray[current];
	found = MPID_MPX_CheckType_ReceiveRequest(Request);
	if(found && (local_data->numConnections || global_data->MyWorldSize<2)) {
	  *from=current;
	  if(Request->Header.Size > size) { /* XXX SIGNED <-> UNSIGNED CONFLICT !!! */
	    MPID_MPX_Error("Control message too large...\n",Request->Header.Size);
	  }
	  DNOTICEI("Trying to read pkt of size ",Request->Header.Size);
	  /*
	    Request->StartRequest(((char*)pkt)+PRIVATESIZE,
					  Request->Header.Size-PRIVATESIZE,BLOCKING);
	  */
	  MPID_MPX_Start_ReceiveRequest(((char*)pkt)+PRIVATESIZE, Request->Header.Size-PRIVATESIZE,BLOCKING, Request);
	}
	
      } /* if( FD_ISSET...)*/
      ++current;
    } /*while(res>0) */
    if(!local_data->numConnections) break;
    } /* while (!found) */
#endif

  LOG_ANY_CTRL(1);

  /* store the former static variable in the global struct: */
  local_data->_RecvAnyControl_current = current;
  
  DSECTLEAVE
    return;
}


int MPID_MPX_ControlMsgAvail( void )
/*
 |   Check if there is a control message available from any process:
 |
 */
{
  MPID_MPX_Data_global_type *global_data;
  MPID_MPX_Data_priv_type  *local_data;


  DSECTION("MPID_MPX_ControlMsgAvail");
 
  int i;
  int current;
  int res,found;
  struct timeval t={0,0};  
  BOOL local=FALSE;

#if 0  
  DSECTENTRYPOINT;
#endif

  MPID_MPX_Test_device(MPID_devset->active_dev, "ControlMsgAvail");

  /* get the pointer to the data structs of this device entity: */
  global_data = MPID_MPX_Get_global_data(MPID_devset->active_dev);
  local_data  = MPID_MPX_Get_priv_data(MPID_devset->active_dev);

  /* get the former static variable from the global struct: */
  current = local_data->_ControlMsgAvail_current; /* <-- ID of process that we check */
    
#ifdef _MPX_

  found=0;
  for(i=0; i<global_data->MyWorldSize; i++, current=(current+1)%global_data->MyWorldSize)
  {
    if(current==global_data->MyWorldRank)
    {
#if 0
      DNOTICEI("checking local channel",current);
#endif
      if(STL_deque_size(&(local_data->SelfSendChannel)))
      {
#if 0
	DNOTICE("checking for incoming type");
#endif
	found = MPID_MPX_CheckType_ReceiveRequest(local_data->ReceiveRequestArray[current]);
	if(found) break;
      }
    }
    else
    {  
      int probe_flag;
      MPX_Status probe_status;

#if 0
      DNOTICEI("checking channel",current);  
#endif
      MPX_Iprobe(current, 99, MPX_COMM_WORLD, &probe_flag, &probe_status);
      if(probe_flag)
      {
#if 0
	DNOTICE("checking for incoming type");
#endif
	found = MPID_MPX_CheckType_ReceiveRequest(local_data->ReceiveRequestArray[current]);
	if(found) break;
      }
    }
  }
#else
  found = 0;
  if(STL_deque_size(&local_data->SelfSendChannel))
  {
    /* chere is a message available that we sent to ourselves: */
    res = 1;
    local = TRUE;
    current = global_data->MyWorldRank;
  } 
  else
  {
    /* check the other procs (if they exist): */
    if((global_data->MyWorldSize>1) && (local_data->numConnections))
    { 
      /* select all file descriptors (sockets) as being ready for reading: */
      readfds = local_data->allFds;

/*      DNOTICE("select on any fds");*/

      res=select(FD_SETSIZE,&readfds,0,0,&t);
      if(res==SOCKET_ERROR)
      {
	MPID_MPX_SysError("ControlMsgAvail: select failed",WSAGetLastError());
      }
/*      DNOTICEI("select successfull, res=",res);*/
    } 
    else res=0;
  }

  while((res>0) && (!found))
  {
    DNOTICE("(res>0) && (!found))");
    
    if(current >= global_data->MyWorldSize)
    {
      DNOTICE("current >= global_data->MyWorldSize");
      current = 0; /* <-- continue with process 0 */
    }
    
    if(/* check for message from another process: */
       ((local_data->Channels[current] != INVALID_SOCKET) && (FD_ISSET(local_data->Channels[current],&readfds)))       
    || /* check for message from ourselves: */
       (local && (current == global_data->MyWorldRank)))
    {
      DNOTICEI("checking channel",current);
      res--;
      DNOTICE("checking for incoming type");
      found = MPID_MPX_CheckType_ReceiveRequest( local_data->ReceiveRequestArray[current] );
    } 	
    current++; /* <-- check for messages from next process */
  }
#endif
  
  /* store the former static variable in the global struct: */
  local_data->_ControlMsgAvail_current = current;
    
#if 0
  DSECTLEAVE
#endif
    return found;
}


int MPID_MPX_RecvFromChannelAsync( void *buf, int size, int channel, ASYNCRecvId_t id )
{ 
  MPID_MPX_Data_global_type *global_data;
  MPID_MPX_Data_priv_type  *local_data;
  
  DSECTION("MPID_MPX_RecvFromChannelAsync");

  ReceiveRequest *Request;
  TransferRequest *Transfer;
  
  DSECTENTRYPOINT;

  MPID_MPX_Test_device(MPID_devset->active_dev, "RecvFromChannelAsync");

  /* get the pointer to the data structs of this device entity: */
  global_data = MPID_MPX_Get_global_data(MPID_devset->active_dev);
  local_data  = MPID_MPX_Get_priv_data(MPID_devset->active_dev);
  
  Request=local_data->ReceiveRequestArray[channel];
  Transfer=MPID_MPX_Get_TransferRequest(buf,size);
  Request->Transfer=Transfer;

  if(!MPID_MPX_Start_ReceiveRequest((char*)buf,size,NONBLOCKING, Request)) {
    memcpy(id,&Transfer,sizeof(Transfer));
    DSECTLEAVE
      return 0;
  }
  /* Request->StartRequest(); */
  MPID_MPX_Free_TransferRequest(Transfer);
#if 0
  DSECTLEAVE
#endif
    return 1;
}


void MPID_MPX_RecvFromChannel( void *buf, int size, int channel)
{
  MPID_MPX_Data_global_type *global_data;
  MPID_MPX_Data_priv_type  *local_data;

  DSECTION("MPID_MPX_RecvFromChannel");

  ReceiveRequest *Request;
  
  DSECTENTRYPOINT;

  MPID_MPX_Test_device(MPID_devset->active_dev, "RecvFromChannel");

  /* get the pointer to the data structs of this device entity: */
  global_data = MPID_MPX_Get_global_data(MPID_devset->active_dev);
  local_data  = MPID_MPX_Get_priv_data(MPID_devset->active_dev);
  
  LOG_RECV(0);
  Request=local_data->ReceiveRequestArray[channel];

  MPID_MPX_Start_ReceiveRequest((char*)buf,size,BLOCKING, Request);
  /*
    while(!Request->TestRequest(TRUE)) ; 
    MPID_DeviceCheck(MPID_NOTBLOCKING);
    */
  /* Request->StartRequest(); */
  LOG_RECV(1);
  DSECTLEAVE
    return;
}


void MPID_MPX_ConsumeData(int size,int channel)
{
  MPID_MPX_Data_global_type *global_data;
  MPID_MPX_Data_priv_type  *local_data;

  DSECTION("MPID_MPX_ConsumeData");

  char buf[4096];
  int toRead;
  ReceiveRequest *Request;
  
  DSECTENTRYPOINT;

  MPID_MPX_Test_device(MPID_devset->active_dev, "ConsumeData");

  /* get the pointer to the data structs of this device entity: */
  global_data = MPID_MPX_Get_global_data(MPID_devset->active_dev);
  local_data  = MPID_MPX_Get_priv_data(MPID_devset->active_dev);
  
  Request=local_data->ReceiveRequestArray[channel];
  
  while(size) {
    toRead = size<4096?size:4096;
   
    MPID_MPX_Start_ReceiveRequest((char*)buf,toRead,BLOCKING, Request);
    size -= toRead;
  }
  
  DSECTLEAVE
    return;
}

	
void MPID_MPX_SendControl(MPID_PKT_T *pkt,int size,int dest)
/*
 |   Send the control packet (short packet) pointed to by 'pkt'  and with 'size'
 |   bytes of effective size (varies according to length of user message) to 
 |   process with ID 'dest'
 */
{
  DSECTION("MPID_MPX_SendControl");
 
  WSABUF SBuf; /* <-- structure representing a data buffer (winsock2) */
  SendRequest *Request;
  
  DSECTENTRYPOINT;

  /* get a send request from the queue of send requests: */
  Request = MPID_MPX_Get_SendRequest();
  
  /* set additional information in packet: */
  pkt->head.TCP_TYPE = CTRL; 
  pkt->head.TCP_SIZE = size;
  
  MPID_PKT_PACK(&pkt->head,PRIVATESIZE,dest);
  
#if 0
  /* initialize buffer structure: */
  SBuf.len = size;
  SBuf.buf = (char*)pkt;
  
  if( MPID_MPX_Start_SendRequest( SBuf, size, dest, Request, WSA_NOT_OVERLAPPED ) < 0 )  /* <- WORKAROUND: 
											  |    Control blocks are now always sent via a 
											  |    blocking (not overlapped) WSASend().
											  |    So the following WaitSend() will be skipped:
											  */
    MPID_MPX_WaitSend( (long *)&Request );

#else
  SBuf.len=12;
  SBuf.buf=(char*)pkt;
  if(MPID_MPX_Start_SendRequest(SBuf, SBuf.len, dest, Request, WSA_OVERLAPPED) < 0 )
    MPID_MPX_WaitSend( (long *)&Request );
  
  Request = MPID_MPX_Get_SendRequest();
  SBuf.len=16;
  SBuf.buf=((char*)pkt)+12;
  if(MPID_MPX_Start_SendRequest(SBuf, SBuf.len, dest, Request, WSA_OVERLAPPED) < 0 )
    MPID_MPX_WaitSend( (long *)&Request );

  SBuf.len=size-28;
  if(SBuf.len>0)
  {
    Request = MPID_MPX_Get_SendRequest();
    SBuf.buf=((char*)pkt)+28;
    if(MPID_MPX_Start_SendRequest(SBuf, SBuf.len, dest, Request, WSA_OVERLAPPED) < 0)
      MPID_MPX_WaitSend( (long *)&Request );
  }

#endif
  
  DSECTLEAVE
    return;
}


int MPID_MPX_SendChannel(WSABUF buf, int size, int dest, ASYNCSendId_t id, BOOL WSAisOverlapped)
/*
 |  Sends 'NumBlocks' WSA-buffers 'buf' with the accumulated length 'size'
 |  to the destination 'dest'.
 |  The 'WSAisOverlapped' flag indicates, if the WSASend() should
 |  be blocking or non-blocking.
 |
 |  This function obtains a SendRequest and passes the whole request to the
 |  SendRequest() function, which handles the WSASend() operation.
 |
 */
{
  DSECTION("MPID_MPX_SendChannel");

  int ret;
  SendRequest *Request;
  
  DSECTENTRYPOINT;
 
  /* Obtain a free SendRequest: */
  Request=MPID_MPX_Get_SendRequest();
  
  MEMCPY(id,&Request,sizeof(Request));

  ret=MPID_MPX_Start_SendRequest(buf,size,dest,Request, WSAisOverlapped);

  DSECTLEAVE
    return ret;
}


int MPID_MPX_TestSend(ASYNCSendId_t id)
/*
 |   Test, if the underlying (channel) send operation has been finished.
 |   The respective send request is retrieved via a send id, maintained by
 |   the MPIR-layer.
 */
{
  DSECTION("MPID_MPX_TestSend");

  SendRequest *Request;
  DWORD ToSend,flags;
  BOOL Finished;
  
  DSECTENTRYPOINT;
  
  LOG_TEST_SEND(0);
  
  /*
   |   The address of the respective 'SendRequest' struct was stored in the 'sid'
   |   member of the 'MPIR_SHANDLE' struct as an 'ASNYCSendId_t' type.
   |   So, we have to do a memcpy() to get back the 'SendRequest' handle from the send-id:
   */
  memcpy(&Request,id,sizeof(&Request));
  
  Finished=Request->finished;
  if(!Finished)
  {
    /* ask the underlying layer: */
#ifdef _MPX_
    int test_flag;
    MPX_Status test_status;
    
    if(Request->mpx_request==MPX_REQUEST_NULL) Finished=true;
    else
    {
      MPX_Test(&(Request->mpx_request), &test_flag, &test_status);
      if(test_flag) Finished=true;
    }
#else
    Finished=HasOverlappedIoCompleted(&Request->Overlapped);
#endif
  } 

  if(Finished)
  {  
#ifndef _MPX_
    /* operation finished, but _whole_ message realy sent? */
    if(!WSAGetOverlappedResult(Request->fd,&Request->Overlapped,&ToSend,FALSE,&flags) || ToSend != Request->size)
    {
      /* this should not happen: */
      MPID_MPX_SysError("TestSend: WSASend truncated message",WSAGetLastError());
    }
#endif
    /* operation finished, so the SendRequest can be freed: */
    MPID_MPX_Free_SendRequest(Request);	    	          
  }

  LOG_TEST_SEND(1);

  /* report the status: */
  DSECTLEAVE
    return (int)Finished;
}


void MPID_MPX_WaitSend(ASYNCSendId_t id)
/*
 |   Wait until the underlying (channel) send operation has been finished.
 |   The respective send request is retrieved via a send id, maintained by
 |   the MPIR-layer.
 */
{
  DSECTION("MPID_MPX_WaitSend");

  SendRequest *Request;
  DWORD ToSend,flags;
  BOOL Finished;
  
  DSECTENTRYPOINT;
  
  LOG_WAIT_SEND(0);
  
  /*
   |   The address of the respective 'SendRequest' struct was stored in the 'sid'
   |   member of the 'MPIR_SHANDLE' struct as an 'ASNYCSendId_t' type.
   |   So, we have to do a memcpy() to get back the 'SendRequest' handle from the send-id:
   */
  memcpy(&Request, id, sizeof(&Request));

  /* if the send operation has not been finished, then wait for it: */
  Finished=Request->finished;

#ifdef _MPX_

  if(!Finished) MPID_MPX_GenericWait(&Request->mpx_request);
  
#else
  if(!Finished) MPID_MPX_GenericWait(&Request->Overlapped);
  
  /* _whole_ message realy, realy sent? */
  if(!WSAGetOverlappedResult(Request->fd,&Request->Overlapped,&ToSend,FALSE,&flags) || ToSend != Request->size)
  {
    /* this should not happen: */
    MPID_MPX_SysError("Wait Send: WSASend truncated message",WSAGetLastError());
  }
#endif
  
  MPID_MPX_Free_SendRequest(Request);
  
  LOG_WAIT_SEND(1);
  
  DSECTLEAVE
    return;
}


void MPID_MPX_CreateTransfer(void * buf, int size,int tag, ASYNCRecvId_t id)
{
  MPID_MPX_Data_global_type *global_data;
  MPID_MPX_Data_priv_type  *local_data;

  DSECTION("MPID_MPX_Create_TransferRequest");

  TransferRequest *t;
  
  DSECTENTRYPOINT;
  
  MPID_MPX_Test_device(MPID_devset->active_dev, "CreateTransfer");
  
  /* get the pointer to the data structs of this device entity: */
  global_data = MPID_MPX_Get_global_data(MPID_devset->active_dev);
  local_data  = MPID_MPX_Get_priv_data(MPID_devset->active_dev);
  
  t=MPID_MPX_Get_TransferRequest(buf,size);
  
  MPID_MPX_DEBUG_PRINT_MSG2("Creating TransferRequest for size %d",size);
  STL_map_insert_int2TraReq(tag, t, &local_data->TransferRequestMap);
  MEMCPY(id,&t,sizeof(t));

  DSECTLEAVE
    return;
}


void MPID_MPX_ReceiveTransfer(int tag, ASYNCRecvId_t id)
{
  MPID_MPX_Data_global_type *global_data;
  MPID_MPX_Data_priv_type  *local_data;
 
  DSECTION("MPID_MPX_Receive_TransferRequest");
  
  TransferRequest *t;
   
  DSECTENTRYPOINT;
    
  MPID_MPX_Test_device(MPID_devset->active_dev, "ReceiveTransfer");
    
  /* get the pointer to the data structs of this device entity: */
  global_data = MPID_MPX_Get_global_data(MPID_devset->active_dev);
  local_data  = MPID_MPX_Get_priv_data(MPID_devset->active_dev);
  
  MEMCPY(&t,id,sizeof(t));
  
  if(tag>0) STL_map_erase(STL_map_find_int2TraReq(tag, &local_data->TransferRequestMap), &local_data->TransferRequestMap);
  
  MPID_MPX_Free_TransferRequest(t);
  
  DSECTLEAVE
    return;
}


int MPID_MPX_TestTransfer(ASYNCRecvId_t id)
{
    DSECTION("MPID_MPX_Test_TransferRequest");
    TransferRequest *t;

    DSECTENTRYPOINT;

    MEMCPY(&t,id,sizeof(t));
    if(t->Receive) {
	DSECTLEAVE
	  return MPID_MPX_Test_ReceiveRequest(NONBLOCKING, t->Receive);
    }
    else {
      DSECTLEAVE
	return t->finished;
    }
}


void MPID_MPX_WaitTransfer(ASYNCRecvId_t id)
{
  DSECTION("MPID_MPX_Wait_TransferRequest");
  TransferRequest *Transfer;

  DSECTENTRYPOINT;

  MPID_MPX_Test_device(MPID_devset->active_dev, "WaitTransfer");
    
  memcpy(&Transfer,id,sizeof(Transfer));
  while(!Transfer->finished) {
    if(!Transfer->Receive) {
      /* The transfer didn't start yet... */
      MPID_DeviceCheck(MPID_NOTBLOCKING);
      MPID_MPX_Test_device(MPID_devset->active_dev, "WaitTransfer II");
    } else
      MPID_MPX_Wait_ReceiveRequest(Transfer->Receive);
  }
  MPID_MPX_Free_TransferRequest(Transfer);
  
  DSECTLEAVE
    return;
}


void MPID_MPX_SysError( const char *string, int value ) 
{
  DSECTION("MPID_MPX_SysError");

  DSECTENTRYPOINT;
  
  MPID_MPX_Error(string, value);

  DSECTLEAVE
    return;
}


void MPID_MPX_Error(const char *string,int value)
{
  DSECTION("MPID_MPX_Error");

  MPID_MPX_Data_global_type *global_data;
  
  DSECTENTRYPOINT;

  MPID_MPX_Test_device(MPID_devset->active_dev, "Error");

  /* get the pointer to the data structs of this device entity: */
  global_data = MPID_MPX_Get_global_data(MPID_devset->active_dev);

  fprintf(stderr,"(%d) %s (%d)\n", global_data->MyWorldRank, string, value); fflush(stderr);

  DSECTLEAVE;

  exit(value);
}


#ifdef _MPX_CHECK_DEV_VALIDITY
/*
 |  This function checks if the pointer to the active device (thus this device)
 |  is valid. The pointer is set during the first call of this function and in
 |  all further calls we check if it is still the same pointer.
 |  (This function must be disabled, when using multiple ch_mpx entities!)
 |
 */
int MPID_MPX_Test_device(void* dev, char* name)
{
  static void* MPID_MPX_Test_device_p = NULL;

  if(MPID_MPX_Test_device_p != NULL)
  {
    if(MPID_MPX_Test_device_p != dev)
    {
      fprintf(stderr, "### MPX-ERROR: Device clashes in \"%s\" detected!\n", name); fflush(stderr);
      exit(-1);
      return -1;
    }
    return 0;
  }
  else MPID_MPX_Test_device_p = dev;
  
  return 0;
}
#endif


   /**********************************
--- ** internal (static) functions: ** --------------------------------------------------------
    **********************************/

/*****************************************
 ** functions to handle 'SendRequests': **
 *****************************************/

static void MPID_MPX_Init_SendRequest(SendRequest* SendRequestPt)
/*
 |   Initialize a SelfRequest structure:
 |
 */
{
  DSECTION("SendRequest_SendRequest");

  DSECTENTRYPOINT;

#ifdef _MPX_
  SendRequestPt->finished=TRUE;
#else
  SendRequestPt->Overlapped.hEvent=CreateEvent(0,FALSE,FALSE,0); 
  SendRequestPt->finished=TRUE;
  if(SendRequestPt->Overlapped.hEvent == WSA_INVALID_EVENT) MPID_MPX_Error("Creation of event failed",WSAGetLastError());
#endif

  DSECTLEAVE
    return;
}

static int MPID_MPX_Start_SendRequest( WSABUF buf, int size, int dest, SendRequest *Request, BOOL WSAisOverlapped )
/*
 |   MPID_MPX_Start_SendRequest() sends data to a destination process 'dest'.
 |   If 'dest' is the ID of the current process, the data is put into 'SelfSendChannel',
 |   otherwise it is sent over a socket by using the WSASend() features.
 |
 |   Parameters:
 | 
 |   WSABUF *buf:             pointer to a array of structures (winsock2 API), describing the data buffers to be sent
 |   int size:                size of data to be sent in bytes (sum of the size of all data buffers)
 |   unsigned long NumBlocks: number of data buffers to be sent (number of WSABUF structures which 'buf' points to)
 |   int dest:                ID of destination process (may be ID of current process
 |   SendRequest *Request:    pointer to SendRequest structure representing send operation to be performed, the structure
 |   is freed, if the send operation can be completed
 | 
 |   WORKAROUND:
 |   'WSAisOverlapped' indicates if a blocking or overlapped WSASend() is to be called
 | 
 |   Return Values:
 | 
 |    0  : send operation successfully performed
 |   -1  : error
 |
*/
{
  MPID_MPX_Data_global_type *global_data;
  MPID_MPX_Data_priv_type  *local_data;

  DSECTION("MPID_MPX_Start_SendRequest");

  DWORD ToSend = size; /* <-- number of bytes that are to be sent */
  int res,error;
  BOOL retry;
  SelfSend* self_msg;
  
  DSECTENTRYPOINT;

  MPID_MPX_Test_device(MPID_devset->active_dev, "Start_SendRequest");

  /* get the pointer to the data structs of this device entity: */
  global_data = MPID_MPX_Get_global_data(MPID_devset->active_dev);
  local_data  = MPID_MPX_Get_priv_data(MPID_devset->active_dev);
  
  LOG_SEND(0);
  
  if( dest==global_data->MyWorldRank )
  {
    /*
     |   if we are sending to ourselves, we make a SelfSend out of each
     |   buffer to be sent and put them into 'SelfSendChannel':
     */
#if 0    
    for( res = 0; res < NumBlocks; res++ ) /* XXX SIGNED UNSIGNED !!! */
#endif
    {
      self_msg=(SelfSend*)malloc(sizeof(SelfSend));
/*    MPID_MPX_Init_SelfSend( buf[res].buf, buf[res].len, self_msg); */
      MPID_MPX_Init_SelfSend( buf.buf, buf.len, self_msg);
      STL_deque_push_back_SelfSend( self_msg, &local_data->SelfSendChannel );
    }
    
    LOG_SEND(1);

    DSECTLEAVE
      return 0;
  } 
  else
  {
    /*
     |   This is a send operation to a different process:
     |
     */

#ifdef _MPX_
    {
      void *dummy;
      int count;
      int test_flag;
      MPX_Status send_status;
      MPX_Request dummy_request;
        
      /* initialize send request: */
      Request->size = size;
    
      DNOTICEI("Sending message of size ", size);

      if(WSAisOverlapped)
      {
	DNOTICEI("MPI_Isend ", buf.len);
	MPX_Isend(buf.buf, buf.len, MPX_CHAR, dest, 99, MPX_COMM_WORLD, &(Request->mpx_request));
/*
	{
	  int i;
	  for(i=0; i<size; i++) printf("S> %d\n", (unsigned char)(buf.buf[i]));
	  printf("\n");	  
	}
*/
      
	MPX_Test(&(Request->mpx_request), &test_flag, &send_status);

	if(test_flag) Request->finished = TRUE;
	else Request->finished = FALSE;
      }
      else
      {
	/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	  XXX  PROBLEM WITH LAM: MPI_Send blocks also for small messages
	  XXX  So, PingPing causes deadlocks, because even the headers can
	  XXX  not be exchanged !!!
	  XXX  But the PKT is placed on the stack (see neager_isend), so
	  XXX  malloc for the dummy is dirty (causes memory leaks), but it
	  XXX  is a way to test for deadlock...                              
	  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/

	DNOTICEI("MPI_Send ...", buf.len);

	MPX_Send(buf.buf, buf.len, MPX_CHAR, dest, 99, MPX_COMM_WORLD);
/*
	{
	  int i;
	  for(i=0; i<size; i++) printf("S> %d\n", (unsigned char)(buf.buf[i]));
	  printf("\n");	  
	}
*/
	DNOTICEI("MPI_Send OK", buf.len);

	Request->finished = TRUE;
      }
    }
#else

    if(local_data->Channels[dest] == INVALID_SOCKET)
    {
      LOG_SEND(1);

     /* isn't this an error? where is it handled? why do we return the same value as in case of success? boris */

      DSECTLEAVE
	return 0;
    }
    
    /* initialize send request: */
    Request->size = size;
    Request->fd = local_data->Channels[dest];
    
    DNOTICEI("Sending message of size ", size);

    do
    {
      retry = FALSE;
      
      /* sending overlapped or not ? */
      if(WSAisOverlapped) res=WSASend( local_data->Channels[dest], &buf, 1, &ToSend, 0, &Request->Overlapped, 0);
      else res=WSASend( local_data->Channels[dest], &buf, 1, &ToSend, 0, 0, 0);
/*
      {
	int i;
	for(i=0; i<size; i++) printf("S> %d\n", (unsigned char)(buf.buf[i]));
	printf("\n");	  
      }
*/
      
      if( res == SOCKET_ERROR )
      {
	/*
	 |   Error handling:
         |
  	 */
	error = WSAGetLastError();
	switch(error) {
	  case WSA_IO_PENDING:
		   /*
		    |   Overlapped operation was successfully initiated,
                    |   completion will be indicated later:
		    |   (no "real" error)
 		    */
		   Request->finished = FALSE;
		   retry = FALSE;
		   break;
	  case WSAENOBUFS:
		   /*
		    |   A blocking send call reports, that there are not enough
                    |   free buffers in the underlying transport layer.
                    |   To avoid a buffer deadlock deadlock, the blocking send
                    |   is omitted and we can hope to solve the deadlock by
                    |   receiving some data:
 		    */
		   MPID_DeviceCheck( MPID_NOTBLOCKING ); /* <-- do something useful and then try again */
		   retry = TRUE;
		   MPID_MPX_Test_device(MPID_devset->active_dev, "SendRequest II");
		   break;
	  default: 
		   DERROR("MPID_MPX_Start_SendRequest: WSASend failed");
		   MPID_MPX_SysError("MPID_MPX_Start_SendRequest: WSASend failed",error);
	}	
      }
      else
      {
	/* maybe not everything was sent: */
	if( ToSend != size )
	{
	  /* this should not happen without an error report! (see above) */
	  DERROR("MPID_MPX_Start_SendRequest: WSASend truncated message");
	  MPID_MPX_Error("MPID_MPX_Start_SendRequest: WSASend truncated message",ToSend);
	}
	
	/* we are done with this send operation: */
	Request->finished = TRUE;
	retry = FALSE;
      }
    } while(retry);
#endif
    
    if( Request->finished )
    {
      /*
       |   The send operation is done.
       |   Free the Request and leave with 0:
       */
      MPID_MPX_Free_SendRequest( Request );
      LOG_SEND(1);
      DSECTLEAVE
	return 0;
    }
  }
  
  /* the send operation is still pending (overlapped send) / we leave with -1 (no error): */
  LOG_SEND(1);
  DSECTLEAVE
    return -1;
}

 
static SendRequest *MPID_MPX_Get_SendRequest(void)
/*
 |   Returns a free SendRequest handle.
 |
 |   To avoid malloc/free overhead the handles are stored by Free_SendRequest() 
 |   in the SendRequestQueue after being used.
 |   So, Get_SendRequest() can recycle them instead of recreate them
 |   in each call.
 */
{
  MPID_MPX_Data_global_type *global_data;
  MPID_MPX_Data_priv_type  *local_data;

  DSECTION("MPID_MPX_Get_SendRequest");

  SendRequest *Request;
  
  DSECTENTRYPOINT;

  MPID_MPX_Test_device(MPID_devset->active_dev, "Get_SendRequest");

  /* get the pointer to the data structs of this device entity: */
  global_data = MPID_MPX_Get_global_data(MPID_devset->active_dev);
  local_data  = MPID_MPX_Get_priv_data(MPID_devset->active_dev);
  
  /* is there a used and free handle in the queue? */
  if(!STL_deque_size(&local_data->SendRequestQueue))
  {
    /* No! Create a new one: */
    Request=(SendRequest*)malloc(sizeof(SendRequest));
    MPID_MPX_Init_SendRequest(Request);
  }
  else
  {
    /* Yes! Reuse this handle: */
    Request=STL_deque_front_SendReq(&local_data->SendRequestQueue);
    STL_deque_pop_front(&local_data->SendRequestQueue);
  }
    
  DSECTLEAVE
    return Request;
}


static void MPID_MPX_Free_SendRequest(SendRequest *Request)
/*
 |   To avoid malloc/free overhead the handles are not deleted but
 |   stored in the SendRequestQueue after being used.
 |   So, Get_SendRequest() can recycle them instead of recreate them
 |   in each call.
 */    
{  
  DSECTION("MPID_MPX_Free_SendRequest");

  MPID_MPX_Data_global_type *global_data;
  MPID_MPX_Data_priv_type  *local_data;
  
  DSECTENTRYPOINT;

  MPID_MPX_Test_device(MPID_devset->active_dev, "Free_SendRequest");

  /* get the pointer to the data structs of this device entity: */
  global_data = MPID_MPX_Get_global_data(MPID_devset->active_dev);
  local_data  = MPID_MPX_Get_priv_data(MPID_devset->active_dev);
  
  /* Are there too much discarded SendRequest handles in the queue? */
  if(STL_deque_size(&local_data->SendRequestQueue) > MAX_BUFFERED_REQUESTS)
  {
    /* Yes! Just delete the handle: */
#ifndef _MPX_
    WSACloseEvent(Request->Overlapped.hEvent);
#endif
    free(Request);
  }
  else
  {
    Request->finished=TRUE;
    /* No! Store the handle in the queue, so that it could be recycled: */
    STL_deque_push_back_SendReq(Request, &local_data->SendRequestQueue);
  }
  
  DSECTLEAVE
    return;
}


/********************************************
 ** functions to handle 'ReceiveRequests': **
 ********************************************/

static void MPID_MPX_Init_ReceiveRequest(SOCKET s, ReceiveRequest* ReceiveRequestPt)
{
  DSECTION("MPID_MPX_Init_ReceiveRequest");

  DSECTENTRYPOINT;

#ifdef _MPX_
  ReceiveRequestPt->finished=FALSE;
  ReceiveRequestPt->Transfer=0;
  ReceiveRequestPt->State=CLEAN;
  memset(&(ReceiveRequestPt->Header),0xFF,sizeof(ReceiveRequestPt->Header));
  ReceiveRequestPt->shuttingDown = FALSE;
#else
  ReceiveRequestPt->fd=s;
  ReceiveRequestPt->Overlapped.hEvent = CreateEvent(0,TRUE,FALSE,0); 
  ReceiveRequestPt->finished=FALSE;
  ReceiveRequestPt->Transfer=0;
  ReceiveRequestPt->State=CLEAN;
  memset(&(ReceiveRequestPt->Header),0xFF,sizeof(ReceiveRequestPt->Header));
  ReceiveRequestPt->shuttingDown = FALSE;
#endif

  DSECTLEAVE
    return;
}


/* this method is called if a receive has been finished, it updates the member variables accordingly */
static void MPID_MPX_Finish_ReceiveRequest(ReceiveRequest* ReceiveRequestPt)
{
  DSECTION("MPID_MPX_Finish_ReceiveRequest");
    
  DSECTENTRYPOINT;

  if(ReceiveRequestPt->Transfer)
  {
    ReceiveRequestPt->Transfer->finished=1;
    ReceiveRequestPt->Transfer=0;
  }

  ReceiveRequestPt->State = CLEAN;
  ReceiveRequestPt->finished = TRUE;
  
  /* commented this out because it leads to problems when a process sends a message to
     itself: this point is reached twice in the Recv and then an Event is raised, can
     be tested e. g. with a call to MPI_Gather(), boris
     if( fd == INVALID_SOCKET ) SetEvent(Overlapped.hEvent);
  */

  MPID_MPX_DEBUG_PRINT_MSG2("Transfer with size %d finished", ReceiveRequestPt->size);
  
  DSECTLEAVE
    return;
}


static int MPID_MPX_Start_ReceiveRequest( char *mem, unsigned long amount, int mode, ReceiveRequest* ReceiveRequestPt)
/*
 |   Try to receive 'amount' bytes into memory location pointed to by 'mem'
 |   using socket fd (member variable)
 */
{ 
  MPID_MPX_Data_global_type *global_data;
  MPID_MPX_Data_priv_type  *local_data;
  
  DSECTION("MPID_MPX_Start_ReceiveRequest");

  unsigned long flags = 0;          /* flags for WSARecv() operation */
  unsigned long amountReceived = 0; /* number of bytes received from socket */
  unsigned long count_nobufs = 0;   /* number of times WSAENOBUFS error occurred */
  int error,res,retry;
  /* WSANETWORKEVENTS NetEvents; */
  
  DSECTENTRYPOINT;

  MPID_MPX_Test_device(MPID_devset->active_dev, "Start_ReceiveRequest");

  global_data = (MPID_MPX_Data_global_type*)MPID_devset->active_dev->global_data;
  local_data  = (MPID_MPX_Data_priv_type*)global_data->local_data_priv;
  
  LOG_START_REQUEST(0);
  
  MPID_MPX_DEBUG_PRINT_MSG2("Starting ReceiveRequest size %d ",amount);
  
  if(ReceiveRequestPt->State == TRANSFERRING) MPID_MPX_Error("Request already in progress.",-1);
  if(ReceiveRequestPt->Transfer) ReceiveRequestPt->Transfer->Receive=ReceiveRequestPt;
  if(amount <=0) MPID_MPX_Finish_ReceiveRequest(ReceiveRequestPt);
  else {
    /* initialize structure that describes our receive buffer */
    ReceiveRequestPt->buf.len = amount; 
    ReceiveRequestPt->buf.buf = mem; 

#ifdef _MPX_    
    if(ReceiveRequestPt->from!=global_data->MyWorldRank)
    {
      int count;
      int test_flag;
      MPX_Status test_status;
      
#if 0
      /* WSARecv emulation of nt2unix also blocks !!! */
      DNOTICEI("MPI_Irecv ", amount);
      MPX_Irecv(mem, amount, MPX_CHAR, ReceiveRequestPt->from, 99, MPX_COMM_WORLD, &(ReceiveRequestPt->mpx_request));
      MPX_Test(&(ReceiveRequestPt->mpx_request), &test_flag, &test_status);
#else

      /* Irecv --> a source of deadlocks ?!? */
      DNOTICEI("MPI_Recv ", amount);
      MPX_Recv(mem, amount, MPX_CHAR, ReceiveRequestPt->from, 99, MPX_COMM_WORLD, &test_status);
/*
      {
	int i;
	for(i=0; i<amount; i++) printf("R> %d\n", (unsigned char)(mem[i]));
	printf("\n");	  
      }
*/
      test_flag=1;
#endif
      
      if(test_flag)
      {
	ReceiveRequestPt->pending = FALSE;

	/* we received as many bytes as we wanted to */
	MPX_Get_count(&test_status, MPX_CHAR, &count);
	ReceiveRequestPt->size=count;
	MPID_MPX_Finish_ReceiveRequest(ReceiveRequestPt);
	
	LOG_START_REQUEST(1);
	DSECTLEAVE
	  return 1;
      } 
      else
      {
	DNOTICE("NOT COMPLETED (Start_ReceiveReq)");

	ReceiveRequestPt->pending = TRUE;

	/* we received some bytes, but not all that we wanted to */
	ReceiveRequestPt->State = TRANSFERRING; /* mark this request as being in progress */
	ReceiveRequestPt->received=0;
	ReceiveRequestPt->finished = FALSE;
	ReceiveRequestPt->size=0; /* <-- MPX: Should be set the actual size ??? */
	if(mode == BLOCKING) MPID_MPX_Wait_ReceiveRequest(ReceiveRequestPt); /* TestRequest(mode); */
	else MPID_MPX_Test_ReceiveRequest(mode, ReceiveRequestPt);
      }
    }
    else
    {
      /* maybe there is packet that we sent to ourselves */
      if(STL_deque_size(&(local_data->SelfSendChannel)))
      {
	MPID_MPX_ReadLocal(mem,amount);
	MPID_MPX_Finish_ReceiveRequest(ReceiveRequestPt);	
      }
    }

#else
    if(ReceiveRequestPt->fd != INVALID_SOCKET) {
      /* Do this to clear the buffer on the socket. */
      /* WSAEnumNetworkEvents(fd,0,&NetEvents); */
      do {
	retry = 0;
	
	/* now we receive buf.len bytes into the buffer pointed to by buf.buf, using socket fd */
	DNOTICEI("WSARecv ", amount);
	res = WSARecv(ReceiveRequestPt->fd, &(ReceiveRequestPt->buf), 1, &amountReceived, &flags, &(ReceiveRequestPt->Overlapped), 0); //wsock2function
/*
	{
	  int i;
	  for(i=0; i<amount; i++) printf("R> %d\n", (unsigned char)(mem[i]));
	  printf("\n");	  
	}
*/
	
	/* error handling */
	if( res == SOCKET_ERROR ) {			    
	  error = WSAGetLastError();
	  switch( error ) {
	    case WSA_IO_PENDING: 
		     /* overlapped operation was successfully initiated, completion will */
		     /* be indicated later */
		     ReceiveRequestPt->pending = TRUE;
		     MPID_MPX_DEBUG_PRINT_MSG2( "ReceiveRequest of size %d is pending...", amount );
		     break;
	    case WSAEWOULDBLOCK:
		     /* Overlapped sockets: too many outstanding overlapped I/O requests */
		     /* Nonoverlapped sockets: socket is nonblocking, operation cannot be completed immediately */
		     fprintf( stderr,"Warning: StartRequest: Got wouldblock error...\n" ); fflush( stderr );
		     retry = 1; Sleep( 10 );				
		     break;
	    case WSAENOBUFS: 
		     /* this error code is defined in nt2unix.h, but I didn't find it in */
		     /* Microsoft's API Documentation for WSARecv(), boris */
		     fprintf(stderr,"Warning: StartRequest: Got nobufs error...\n"); fflush(stderr);
		     retry = 1; count_nobufs++; Sleep( 10 );				
		     if( count_nobufs < 1000 ) break;
		     /* if this has happened too often, don't retry but raise an error */
	    default:
		     fprintf(stderr,"\nmem==%x\namount==%d\nstate==%d, read==%d\n",
			     mem,amount,ReceiveRequestPt->State,amountReceived );
		     MPID_MPX_SysError("Start Request:WSARecv failed",error);
	  } 
	} else {
	  /* this is in case we didn't get a SOCKET_ERROR back, but didn't receive anything also */
	  if( amountReceived == 0 )  {
	    if(!ReceiveRequestPt->shuttingDown) {
				MPID_MPX_DEBUG_PRINT_MSG("Read 0 bytes from network, connection seems to be closed");
				MPID_MPX_Error("StartRequest: Found dead network connection\n",0);
	    } else {
	      MPID_MPX_DEBUG_PRINT_MSG("Read 0 bytes from network, connection seems to be closed");
	      ReceiveRequestPt->pending =FALSE;
	      MPID_MPX_Finish_ReceiveRequest(ReceiveRequestPt);
	      ReceiveRequestPt->State = CLOSED;
	      LOG_START_REQUEST(1);
	      DSECTLEAVE
				    return 1;
	    }
	  }
	  
	  /* everything went fine */
	  ReceiveRequestPt->pending = FALSE;
	  MPID_MPX_DEBUG_PRINT_MSG2("ReceiveRequest size %d received", amountReceived );
		    }
      } while(retry);
      
      if( amountReceived == amount ) {
	/* we received as many bytes as we wanted to */
#ifdef MPID_DEBUG_ALL
	ReceiveRequestPt->size=amount;
#endif
	MPID_MPX_Finish_ReceiveRequest(ReceiveRequestPt);
	
	LOG_START_REQUEST(1);
	DSECTLEAVE
	  return 1;
	
      } else {
	/* we received some bytes, but not all that we wanted to */
	ReceiveRequestPt->State = TRANSFERRING; /* mark this request as being in progress */
	ReceiveRequestPt->received=0;
	ReceiveRequestPt->finished = FALSE;
	ReceiveRequestPt->size=amount;
	if(mode == BLOCKING) MPID_MPX_Wait_ReceiveRequest(ReceiveRequestPt);/* TestRequest(mode); */
	else if(!res) MPID_MPX_Test_ReceiveRequest(mode, ReceiveRequestPt);
      }
    } /* if( fd != INVALID_SOCKET ) */
    else {
      /* maybe there is packet that we sent to ourselves */
      if(STL_deque_size(&local_data->SelfSendChannel)) {
	MPID_MPX_ReadLocal(mem,amount);
	MPID_MPX_Finish_ReceiveRequest(ReceiveRequestPt);	
      } else ResetEvent(ReceiveRequestPt->Overlapped.hEvent);
    }
#endif
  }
  
  LOG_START_REQUEST(1);
	
  DSECTLEAVE
    return ReceiveRequestPt->finished;
}
 

static int MPID_MPX_CheckType_ReceiveRequest( ReceiveRequest *Request )
{
  MPID_MPX_Data_global_type *global_data;
  MPID_MPX_Data_priv_type  *local_data;
  
  DSECTION("MPID_MPX_CheckType_ReceiveRequest");
  
  int again,found=0;

  DSECTENTRYPOINT;

  MPID_MPX_Test_device(MPID_devset->active_dev, "CheckType_ReceiveRequest");

  global_data = (MPID_MPX_Data_global_type*)MPID_devset->active_dev->global_data;
  local_data  = (MPID_MPX_Data_priv_type*)global_data->local_data_priv;
    
  /* if receive operation is in progress */
  if(Request->State == TRANSFERRING) {
    DNOTICEI("CheckType_ReceiveRequest: Continued Transfer request for tag ",Request->Header.Tag);
    MPID_MPX_Test_ReceiveRequest(NONBLOCKING, Request);
    DSECTLEAVE
      return 0;
  }
  do {
    again = 0;
#ifdef _MYDEBUG_ENABLED
    printf("(%d) Header is: Type==%x\nTag==%d\nSize==%d\n",global_data->MyWorldRank, Request->Header.Type,Request->Header.Tag,Request->Header.Size);
#endif
    if(Request->State == CLEAN) {
      /*
	Request->GetHeader();
      */
      DNOTICE("Request State is CLEAN");
      MPID_MPX_GetHeader_ReceiveRequest(Request);
      MPID_PKT_UNPACK(&Request->Header, PRIVATESIZE, current); /* <-- What is 'current' ??? */
      }
    if((Request->Header.Type & CTRL) == CTRL) {
      DNOTICE("Message is a CTRL Header");
      /* found a control packet / short message */
      found = 1;
    } 
    else {
      if((Request->Header.Type & TRANSFER) == TRANSFER) {
	DNOTICEI("CheckType_ReceiveRequest: Found Transfer Message for tag ",Request->Header.Tag);			
	MPID_MPX_RecvTransferMessage(Request);
      } 
      else {
	if((Request->Header.Type & QUIT) == QUIT || Request->State == CLOSED) {
	  if(Request->State == CLOSED) 	DNOTICE("Request closed!");
	  if((Request->Header.Type & QUIT) == QUIT) DNOTICE("Got QUIT header");
	  DNOTICEI("RecvAnyControl: Got QUIT: ", QUIT);
	  local_data->numConnections--;
		DNOTICEI("Got Quit message. Waiting for # procs", local_data->numConnections);
		Request->State = CLEAN;		
#ifndef _MPX_
		ResetEvent(Request->Overlapped.hEvent);
		
		/* if file descriptor is invalid, unset its bit in the file descriptor set allFds */
		if(Request->fd != INVALID_SOCKET)
		  FD_CLR(Request->fd,&local_data->allFds);
		/* if(!local_data->numConnections) found = 1; */
#else
/*		exit(0);*/
#endif      
	} 
	else {
	  printf("Type==%x\nTag==%d\nSize==%d\n",Request->Header.Type,Request->Header.Tag,Request->Header.Size);
	  DERROR("Protocol error: CheckType_ReceiveRequest found illegal message instead of CTRL");
	  MPID_MPX_Error("Protocol error: CheckType_ReceiveRequest found illegal message instead of CTRL",-1);
	}
      }
    }
  } while(again);
  
  DSECTLEAVE
    return found;
}


static int MPID_MPX_Test_ReceiveRequest( int mode, ReceiveRequest* ReceiveRequestPt)
/*
 |   TestRequest() returns TRUE if receive operation is finished, FALSE otherwise
 |   if called with 'mode == BLOCKING', it waits until operation is finished
 */
{
  MPID_MPX_Data_global_type *global_data;
  MPID_MPX_Data_priv_type  *local_data;

  DSECTION("MPID_MPX_Test_ReceiveRequest");

  unsigned long  amountReceived = 0; /* <-- number of bytes that were received during IO operation */
  unsigned long flags,count_nobufs =0;
  int error;
  int res;
  
  DSECTENTRYPOINT;

  MPID_MPX_Test_device(MPID_devset->active_dev, "Test_ReceiveRequest");

  global_data = (MPID_MPX_Data_global_type*)MPID_devset->active_dev->global_data;
  local_data  = (MPID_MPX_Data_priv_type*)global_data->local_data_priv;

  
  LOG_TEST_REQUEST(0);
  
  /* we only need to do something here, if the request is not already marked as being finished: */
  if(!ReceiveRequestPt->finished)
  {
#ifdef _MPX_

    int test_flag;
    MPX_Status test_status;
    
    if(ReceiveRequestPt->from==global_data->MyWorldRank)
    {
      LOG_TEST_REQUEST(1);
      DSECTLEAVE
	return FALSE;
    }

    if(ReceiveRequestPt->mpx_request==MPX_REQUEST_NULL)
    {
      ReceiveRequestPt->finished = TRUE;
      DNOTICE("NULL REQUEST");
      DSECTLEAVE
	return TRUE;
    }
    
    DNOTICE("checking if ovelapped io is completed...");
    MPX_Test(&(ReceiveRequestPt->mpx_request), &test_flag, &test_status);
    DNOTICE("...done");
    
    if(test_flag) ReceiveRequestPt->finished = TRUE;
    else ReceiveRequestPt->finished = FALSE;


    if(!test_flag)
    {
      /* Irecv not completed yet: */
      DNOTICE("IRECV NOT COMPLETED !");
      
      /* if we're not done yet, but running in nonblocking mode, we return now: */
      if(mode==NONBLOCKING)
      {
	DNOTICE("ReceiveRequest not completed / NONBLOCKING");
	LOG_TEST_REQUEST(1);
	DSECTLEAVE
	  return FALSE;
      }
      
      MPX_Wait(&(ReceiveRequestPt->mpx_request), &test_status);
      DNOTICE("WAIT COMPLETED (Test_RecvReq)");
    }

    /* we are done */
    MPID_MPX_Finish_ReceiveRequest(ReceiveRequestPt);
    LOG_TEST_REQUEST(1);
    DSECTLEAVE
      return TRUE;;
  }
  
  LOG_TEST_REQUEST(1);
  DSECTLEAVE
    return ReceiveRequestPt->finished;
#else
 
    /* if we don't have a valid socket, we always return FALSE: */
    if(ReceiveRequestPt->fd == INVALID_SOCKET)
    {
      LOG_TEST_REQUEST(1);
      DSECTLEAVE
	return FALSE;
    }
    
    /* first, we check if an overlapped IO operation has been completed: */
    DNOTICE("checking if ovelapped io is completed...");
    ReceiveRequestPt->finished = HasOverlappedIoCompleted( &(ReceiveRequestPt->Overlapped) );
    DNOTICE("...done");
    
    /* if we're not done yet, but running in nonblocking mode, we return now: */
    if( !ReceiveRequestPt->finished && mode == NONBLOCKING )
    {
      MPID_MPX_DEBUG_PRINT_MSG2("ReceiveRequest not completed, waiting for %d bytes",ReceiveRequestPt->size - ReceiveRequestPt->received);
      LOG_TEST_REQUEST(1);
      DSECTLEAVE
	return FALSE;
    }
    
    /*
     |   either the operation has been completed or we are running in blocking mode
     |   and must wait for it's completion
     |   
     |   we now get the result of the overlapped operation that has been issued before, i. e.
     |   we get the number of bytes that hav been received so far in 'amountReceived';
     |   if 'mode == BLOCKING', then WSAGetOverlappedResult() blocks until the operation is done
     |
     */
    
    LOG_BLOCK(0);
    DNOTICE("getting result of overlapped operation...");
    res = WSAGetOverlappedResult(ReceiveRequestPt->fd, &(ReceiveRequestPt->Overlapped), &amountReceived, mode, &flags); /* wsock2function */
    DNOTICE("...done");
    
    /* in case of success, WSAGetOverlappedResult() returns TRUE: */
    LOG_BLOCK(1);
    if( !res )
    {
      /* some error occurred in WSAGetOverlappedResult(): */
      MPID_MPX_SysError("TestRequest: WSARecv failed",WSAGetLastError());
    } 
    else
      if( amountReceived == 0 )
      {
	/*
         |   call to WSAGetOverlappedResult() succeeded, but we didn't receive anything
         |   during the issued receive operation
 	 */
      if( !ReceiveRequestPt->shuttingDown ) {
	MPID_MPX_Error("TestRequest: Found dead network connection\n",0);
      } else {
	MPID_MPX_DEBUG_PRINT_MSG("Read 0 bytes from network, connection seems to be closed");
	ReceiveRequestPt->pending = FALSE;
	MPID_MPX_Finish_ReceiveRequest(ReceiveRequestPt);
	ReceiveRequestPt->State = CLOSED;
	LOG_TEST_REQUEST(1);
	DSECTLEAVE
	  return 1;
      }
    }
    
    /* call to WSAGetOverlappedResult() succeeded and we already did receive some bytes */
    Over = &(ReceiveRequestPt->Overlapped);
    do {
      ReceiveRequestPt->received += amountReceived;
      
      if(ReceiveRequestPt->received >= ReceiveRequestPt->size) {
	/* we are done */
	MPID_MPX_Finish_ReceiveRequest(ReceiveRequestPt);
	LOG_TEST_REQUEST(1);
	DSECTLEAVE
	  return 1;
      }
      
      /* we are not done yet */
      
      /* update the information about pending operation */
      ReceiveRequestPt->finished = FALSE;
      (ReceiveRequestPt->buf).buf += amountReceived;
      (ReceiveRequestPt->buf).len -= amountReceived;
      
      /* restart the receive operation for the amount of bytes that was not received yet */
      MPID_MPX_DEBUG_PRINT_MSG2("ReceiveRequest restarting recv, waiting for %d bytes", (ReceiveRequestPt->buf).len);
      flags = 0;
      res = WSARecv( ReceiveRequestPt->fd, &(ReceiveRequestPt->buf), 1, &amountReceived, &flags, Over, 0); /* wsock2function */
      
      /* error handling */
      if(res == SOCKET_ERROR) {
	ReceiveRequestPt->pending = TRUE;
	error = WSAGetLastError();
	switch( error ) {
	  case WSA_IO_PENDING: 
		   /* overlapped operation was successfully initiated, completion will
		    * be indicated later */
		   if( mode == BLOCKING ) {
		     /* wait until pending operation is completed */
		     LOG_BLOCK(0);
		     /* blocks until operation is done */
		     if(WSAGetOverlappedResult( ReceiveRequestPt->fd, &(ReceiveRequestPt->Overlapped), &amountReceived, TRUE, &flags)) /* wsock2function */
		       res =0;
		      else
			MPID_MPX_SysError("TestRequest: WSARecv failed",WSAGetLastError());
		     LOG_BLOCK(1);
		   }
		   break;
	  case WSAEWOULDBLOCK:
		   /* Overlapped sockets: too many outstanding overlapped I/O requests
		    * Nonoverlapped sockets: socket is nonblocking, operation cannot be completed immediately */
		   res = 0; amountReceived = 0; Sleep( 10 ); 
		   fprintf(stderr,"Warning: Got wouldblock-error...\n"); fflush(stderr);
		   break;
	  case WSAENOBUFS:
		   /* this error code is defined in nt2unix.h, but I didn't find it in
		    * Microsoft's API Documentation for WSARecv(), boris */
		   res = 0; count_nobufs ++; amountReceived = 0; Sleep( 10 ); 
		   fprintf(stderr,"Warning: TestRequest: Got nobufs-error...\n"); fflush(stderr);
		   MPID_MPX_DEBUG_PRINT_MSG("Got nobufs-error...");
		   if(count_nobufs<1000) break;
		   /* if this has happened too often, don't retry but raise an error */
	  default: MPID_MPX_SysError("Test Request:WSARecv failed",error);
	} /* switch( error ) */
		} 
      else ReceiveRequestPt->pending = FALSE; /* no error in call to WSARecv() */
    } while (!res);
  }
  
  LOG_TEST_REQUEST(1);
  
  DSECTLEAVE
    return ReceiveRequestPt->finished;
#endif
}


/* this method waits until a pending receive operation is finished */
static void MPID_MPX_Wait_ReceiveRequest(ReceiveRequest* ReceiveRequestPt)
{
  DSECTION("MPID_MPX_Wait_ReceiveRequest");

  BOOL Finished;
  
  DSECTENTRYPOINT;
  
  do {
    Finished =MPID_MPX_Test_ReceiveRequest(BLOCKING, ReceiveRequestPt);
  } while( !Finished );
  
  DSECTLEAVE
    return;
}   


static int MPID_MPX_GetHeader_ReceiveRequest(ReceiveRequest* ReceiveRequestPt)
{ 
  DSECTION("MPID_MPX_GetHeader_ReceiveRequest");

  int res;
  
  DSECTENTRYPOINT;
  
  ReceiveRequestPt->Header.Type = QUIT;

  res=MPID_MPX_Start_ReceiveRequest((char*)&(ReceiveRequestPt->Header), PRIVATESIZE, BLOCKING, ReceiveRequestPt);
  if(ReceiveRequestPt->State != CLOSED) 
    ReceiveRequestPt->State=HEADER_READ;
  
  DSECTLEAVE
   return res;
}


/*********************************************
 ** functions to handle 'TransferRequests': **
 *********************************************/

static void MPID_MPX_Init_TransferRequest(void *b, int s, TransferRequest* TransferRequestPt)
{
  TransferRequestPt->buf = b;
  TransferRequestPt->size = s;
  TransferRequestPt->finished = 0;
  TransferRequestPt->Receive = 0;
}


static BOOL MPID_MPX_RecvTransferMessage(ReceiveRequest *Request)
{
  MPID_MPX_Data_global_type *global_data;
  MPID_MPX_Data_priv_type  *local_data;

  DSECTION("MPID_MPX_RecvTransferMessage");

  STL_iterator_type it;
  unsigned int msglen;
  
  DSECTENTRYPOINT;

  MPID_MPX_Test_device(MPID_devset->active_dev, "RecvTransferMessage");

  global_data = (MPID_MPX_Data_global_type*)MPID_devset->active_dev->global_data;
  local_data  = (MPID_MPX_Data_priv_type*)global_data->local_data_priv;
  
  MPID_THREAD_DS_LOCK(&tcp_mutex);
  
  it=STL_map_find_int2TraReq(Request->Header.Tag, &local_data->TransferRequestMap);

  if(it==STL_map_end(&local_data->TransferRequestMap)) {
	MPID_THREAD_DS_UNLOCK(&tcp_mutex);
	/*printf("Size %d, Tag %d\n",Request->Header.Size,Request->Header.Tag);*/
	MPID_MPX_Error("Protocol error: Receive_TransferRequest not found",-1);
    } else {
	MPID_MPX_DEBUG_PRINT_MSG2("Transfer has requested %d bytes",(STL_map_second_int2TraReq(it, &local_data->TransferRequestMap))->size);
	MPID_MPX_DEBUG_PRINT_MSG2("Buf is: %x", (STL_map_second_int2TraReq(it, &local_data->TransferRequestMap))->buf);
    }
  MPID_THREAD_DS_UNLOCK(&tcp_mutex);
  msglen=MPID_MIN((STL_map_second_int2TraReq(it, &local_data->TransferRequestMap))->size,Request->Header.Size);
  if(Request->Header.Size!=(STL_map_second_int2TraReq(it, &local_data->TransferRequestMap))->size) {
    fprintf(stderr,"Warning: Requested size(%d) and sent size(%d) are not matching!!\n",(STL_map_second_int2TraReq(it, &local_data->TransferRequestMap))->size,Request->Header.Size);
    fflush(stderr);
  }
  MPID_MPX_DEBUG_PRINT_MSG2("Reading Transfer message with size %d",msglen);
  Request->Transfer = STL_map_second_int2TraReq(it, &local_data->TransferRequestMap);
  
  DSECTLEAVE
    return MPID_MPX_Start_ReceiveRequest((char*)((STL_map_second_int2TraReq(it, &local_data->TransferRequestMap))->buf),msglen,NONBLOCKING, Request);
}



static TransferRequest *MPID_MPX_Get_TransferRequest(void *buf, int size)
/*
 |   Returns a free TransferRequest handle.
 |
 |   To avoid malloc/free overhead the handles are stored by Free_TransferRequest() 
 |   in the SendRequestQueue after being used.
 |   So, Get_TransferRequest() can recycle them instead of recreate them
 |   in each call.
 */
{
  MPID_MPX_Data_global_type *global_data;
  MPID_MPX_Data_priv_type  *local_data;

  DSECTION("TransferRequest");

  TransferRequest *Request;
  
  DSECTENTRYPOINT;

  MPID_MPX_Test_device(MPID_devset->active_dev, "Get_TransferRequest");

  global_data = (MPID_MPX_Data_global_type*)MPID_devset->active_dev->global_data;
  local_data  = (MPID_MPX_Data_priv_type*)global_data->local_data_priv;
  
  if(!STL_deque_size(&local_data->TransferRequestQueue))
  {
    Request=(TransferRequest*)malloc(sizeof(TransferRequest));
    MPID_MPX_Init_TransferRequest(buf, size, Request);
  }
  else
  {
    Request=STL_deque_front_TraReq(&local_data->TransferRequestQueue);
    STL_deque_pop_front(&local_data->TransferRequestQueue);
    MPID_MPX_Init_TransferRequest(buf, size, Request);
  }
  DNOTICEI("Returning request at ", Request);

  DSECTLEAVE
    return Request;
}


static void MPID_MPX_Free_TransferRequest(TransferRequest *Request)
{
  MPID_MPX_Data_global_type *global_data;
  MPID_MPX_Data_priv_type  *local_data;

  DSECTION("MPID_MPX_Free_TransferRequest");
  
  DSECTENTRYPOINT;

  MPID_MPX_Test_device(MPID_devset->active_dev, "Free_TransferRequest");

  global_data = (MPID_MPX_Data_global_type*)MPID_devset->active_dev->global_data;
  local_data  = (MPID_MPX_Data_priv_type*)global_data->local_data_priv;
  
  if(STL_deque_size(&local_data->TransferRequestQueue) > MAX_BUFFERED_REQUESTS) {
    free(Request);
  } else {
    STL_deque_push_back_TraReq(Request, &local_data->TransferRequestQueue);
  }

  DSECTLEAVE
    return;
}


/***********************************
 ** further internal functions:   **
 ***********************************/ 

static void MPID_MPX_Init_SelfSend(void *mem, const unsigned int s, SelfSend* SelfSend_Pt)
/*
 |   Initialize a SelfSend structure for a self-send-message.
 |   's' is the length of the message, pointed to by 'mem'
 */
{
  DSECTION("SelfSend_SelfSend");

  DSECTENTRYPOINT;
  
  /* allocate memory for message data: */
  SelfSend_Pt->buf = MALLOC( s ); 
  if( ! SelfSend_Pt->buf ) {
    SelfSend_Pt->Header.Size=0;    
    DSECTLEAVE
      return;
  }
  
  /* copy message data and initialize member variables: */
  MEMCPY( SelfSend_Pt->buf, mem, s);
  SelfSend_Pt->Header.Size = s;
  SelfSend_Pt->offset = 0;

  DSECTLEAVE
    return;
}

static void MPID_MPX_ReadLocal( void *buf, int size) 
{
  MPID_MPX_Data_global_type *global_data;
  MPID_MPX_Data_priv_type  *local_data;

  DSECTION("MPID_MPX_ReadLocal");

  SelfSend *msg;
  unsigned int s=size;
  
  DSECTENTRYPOINT;

  MPID_MPX_Test_device(MPID_devset->active_dev,"ReadLocal");

  global_data = (MPID_MPX_Data_global_type*)MPID_devset->active_dev->global_data;
  local_data  = (MPID_MPX_Data_priv_type*)global_data->local_data_priv;

/*
  if(STL_deque_empty(&local_data->SelfSendChannel)) {
*/
  if(!STL_deque_size(&(local_data->SelfSendChannel))) {
    /* should we do a loop here instead of an error? */
    DERROR("protocol error: Local channel is empty");
    MPID_MPX_Error("protocol error: Local channel is empty\n",-1);
  }
  msg=STL_deque_front_SelfSend(&local_data->SelfSendChannel);
  s=MPID_MIN(size,msg->Header.Size); /* XXX SIGNED UNSIGNED CONFLICT !!! */
  MEMCPY(buf,((char*)msg->buf)+msg->offset,s);
  if(s==msg->Header.Size) {
    STL_deque_pop_front(&local_data->SelfSendChannel);	

    if( msg->buf ) FREE( msg->buf );
    msg->buf = 0;
    free(msg);

    if(s != size) {
      /* The user wants more? Give him more 
	 This should not happen within a correct program. */
      MPID_MPX_Error("protocol error: Recursive call of ReadLocal\n",-1);
      MPID_MPX_ReadLocal( ((char*)buf)+s,size-s);
    }
  } else {
    msg->Header.Size-=s;
    msg->offset += s;
  }
  
  DSECTLEAVE
    return;
}

#ifdef _MPX_
static void MPID_MPX_GenericWait(MPX_Request *mpx_request)
#else
static void MPID_MPX_GenericWait(OVERLAPPED *Over)
#endif
{
  DSECTION("MPID_MPX_GenericWait");
    
  int counter =0;
  BOOL Finished=FALSE;
#ifdef BLOCK
  Ev[0]=Over->hEvent;
#endif

  DSECTENTRYPOINT;

  MPID_MPX_Test_device(MPID_devset->active_dev, "GenericWait");

  while(!Finished)
  {
#ifdef _MPX_

    int test_flag;
    MPX_Status test_status;
    
    /* ALWAYS DONT BLOCK! */
    
    MPID_DeviceCheck(MPID_NOTBLOCKING);

    MPID_MPX_Test_device(MPID_devset->active_dev, "GenericWait II");

    if(*mpx_request==MPX_REQUEST_NULL) Finished=TRUE;
    else
    {
#ifdef BLOCK
      DNOTICE("waiting until ovelapped send is completed...");
      MPX_Wait(mpx_request, &test_status);
      DNOTICE("...done");
      Finished = TRUE;
#else
      DNOTICE("checking if ovelapped send is completed...");
      MPX_Test(mpx_request, &test_flag, &test_status);
      DNOTICE("...done");
    
      if(test_flag)
      {
	Finished = TRUE;
	DNOTICE("Finished!");
      }
      else
      {
	DNOTICE("Not finished...");
      }
#endif
    }
    
#else
#ifdef BLOCK
    if(MPID_DeviceCheck(MPID_NOTBLOCKING) <0)
    {
      counter++;
      if(counter>MAX_LOOP)
      {
	LOG_BLOCK(0);
	if(WaitForMultipleObjects(2,Ev,FALSE,INFINITE)==WAIT_FAILED)
	  fprintf(stderr,"Wait failed %d\n",GetLastError());
	LOG_BLOCK(1);
      }
    } else counter =0;
#else
    MPID_DeviceCheck(MPID_NOTBLOCKING);
#endif

    MPID_MPX_Test_device(MPID_devset->active_dev, "GenericWait II");
    Finished=HasOverlappedIoCompleted(Over);
#endif
  }

  DSECTLEAVE
    return;
}

static MPID_MPX_Data_priv_type* MPID_MPX_Get_priv_data(MPID_Device *dev)
{
  MPID_MPX_Data_global_type* global_data;

  global_data = (MPID_MPX_Data_global_type*)dev->global_data;
  return (MPID_MPX_Data_priv_type*)(global_data->local_data_priv);
}
