
#define USE_USI

   /*********************************
--- ** usockpriv.c                 ** -------------------------------------------------------
    *********************************/

#define _WIN32_WINNT 0x0403
#define EXT "KSLF1255"

#pragma warning(disable : 4786) 

#if (_MSC_VER < 1100) && !(defined USE_NT2UNIX)
#define NOMINMAX
#include <winsock2.h>
#include <windows.h>
#define HasOverlappedIoCompleted(lpOverlapped) ((lpOverlapped)->Internal != STATUS_PENDING)
#include <new.h>
#include <iostream>
namespace std{
#include <function>
} 
#else
# include <wtypes.h>
# include <winbase.h>
# include <winsock2.h>
#endif

#include "LogMpid.h"
#include "mydebug.h"

#include "usocktcpcomm.h"
#include "usockpriv.h"

#include "mpid.h"
#include "usockdev.h"
#include "usockpackets.h"
#include "usockdebug.h"
#include "mpimem.h"

#include "stl2c_deque.h"
#include "stl2c_map.h"

#include "USI/usi.h"
   
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
    
void MPID_USOCK_Error(const char *string,int value);

#ifdef _MSC_VER
#pragma comment(exestr,__DATE__ ": " DEV " " EXT)
#endif


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
  WSAOVERLAPPED Overlapped;
  SOCKET fd;
  unsigned long size;
  WSABUF *buf;
  DWORD NumBlocks;
  UsockHeader h;
  BOOL finished;

  USI_Request usi_request;

} SendRequest;


typedef
struct _SelfSend
{
  /*
   |   This structure represents a message that a process sends to itself,
   |   those messages are treated differently
   |   (not sent over a socket, but put into a local queue)
   */
  UsockHeader Header;
  void *buf;
  unsigned int offset;

} SelfSend;
static void MPID_USOCK_Init_SelfSend(void*,const unsigned int, SelfSend*);


/* create the stl2c template functions: */
STL_deque_template(SelfSend, SelfSend*); 
STL_deque_template(SendReq, SendRequest*);
STL_deque_template(TraReq, TransferRequest*);
STL_map_template(int2TraReq, int, TransferRequest*);


typedef
struct _ReceiveRequest
{
  WSAOVERLAPPED   Overlapped;
  WSABUF          buf; /* <-- represents buffer for data, consisting of buf.len (length of buffer) and buf.buf ("real" buffer) */
  SOCKET          fd;
  UsockHeader     Header;
  unsigned long   received,size;
  BOOL            finished; /* <-- TRUE if the last issued request is done, FALSE otherwise */
  TransferRequest *Transfer;
  int             State;
  BOOL            pending,shuttingDown;
  int             from;

  USI_Request usi_request;

} ReceiveRequest;

MPID_USOCK_Data_global_type MPID_USOCK_Data_global;

/* cover all local data in a struct: */
typedef
struct _MPID_USOCK_Data_local_type
{
  STL_map_type TransferRequestMap;
  STL_deque_type SelfSendChannel;
  STL_deque_type SendRequestQueue;
  STL_deque_type TransferRequestQueue;

  CTCPCommunicator Comm;
  int *Channels; /* <-- array of file descriptors of the open sockets */
  ReceiveRequest **ReceiveRequestArray;
  fd_set allFds;  /* <-- set of file descriptors
		   |     (actually a bit field with each bit representing an fd)
		   |     it is used for calls to select()
		   */
  unsigned int numConnections;

} MPID_USOCK_Data_local_type;
static MPID_USOCK_Data_local_type MPID_USOCK_Data_local;

/****************************************
 ** definitions of exported functions: **
 ****************************************/

void MPID_USOCK_Init( int *, char ***);
void MPID_USOCK_Finalize(void);

void MPID_USOCK_RecvAnyControl(MPID_PKT_T*, int, int*);
int  MPID_USOCK_ControlMsgAvail(void);
int  MPID_USOCK_RecvFromChannelAsync(void*, int, int, ASYNCRecvId_t);
void MPID_USOCK_RecvFromChannel(void*, int, int);
void MPID_USOCK_ConsumeData(int, int);
void MPID_USOCK_SendControl(MPID_PKT_T*,int,int);
int  MPID_USOCK_SendChannel(WSABUF *,int, unsigned long, int, ASYNCSendId_t, BOOL);
int  MPID_USOCK_TestSend(ASYNCSendId_t);
void MPID_USOCK_WaitSend(ASYNCSendId_t);

void MPID_USOCK_CreateTransfer(void *, int, int, ASYNCRecvId_t);
void MPID_USOCK_ReceiveTransfer(int, ASYNCRecvId_t);
void MPID_USOCK_WaitTransfer(ASYNCRecvId_t);
int  MPID_USOCK_TestTransfer(ASYNCRecvId_t);

void MPID_USOCK_Error(const char *, int);
void MPID_USOCK_SysError( const char *, int);


/*************************************
 ** definitions of local functions: **
 *************************************/

static void MPID_USOCK_ReadLocal(void*, int);
static void MPID_USOCK_GenericWait(OVERLAPPED*);

static void MPID_USOCK_Init_SendRequest(SendRequest* SendRequestPt);
static int  MPID_USOCK_Start_SendRequest(WSABUF*, int, unsigned long, int, SendRequest*, BOOL);
static void MPID_USOCK_Free_SendRequest(SendRequest*);
static SendRequest *MPID_USOCK_Get_SendRequest(void);

static void MPID_USOCK_Init_ReceiveRequest(SOCKET, ReceiveRequest*);
static void MPID_USOCK_Finish_ReceiveRequest(ReceiveRequest*);
static int  MPID_USOCK_Start_ReceiveRequest(char*, unsigned long, int, ReceiveRequest*);
static int  MPID_USOCK_CheckType_ReceiveRequest(ReceiveRequest*);
static int  MPID_USOCK_Test_ReceiveRequest(int, ReceiveRequest*);
static void MPID_USOCK_Wait_ReceiveRequest(ReceiveRequest*);
static int  MPID_USOCK_GetHeader_ReceiveRequest(ReceiveRequest*);

static void MPID_USOCK_Init_TransferRequest(void*, int, TransferRequest*);
static void MPID_USOCK_Free_TransferRequest(TransferRequest*);
static TransferRequest *MPID_USOCK_Get_TransferRequest(void*, int);


   /**********************************
--- ** exported (global) functions: ** --------------------------------------------------------
    **********************************/

void MPID_USOCK_Init( int *argc, char *** argv)
{
  DSECTION("MPID_USOCK_Init");

  unsigned long val=1;
  int i;
  
  DSECTENTRYPOINT;
  
  STL_deque_constructor(&MPID_USOCK_Data_local.TransferRequestQueue);
  STL_deque_constructor(&MPID_USOCK_Data_local.SendRequestQueue);
  STL_deque_constructor(&MPID_USOCK_Data_local.SelfSendChannel);
  STL_map_constructor(&MPID_USOCK_Data_local.TransferRequestMap);

#if 0
  /* currently deactivated: ! needs functions in usockunused.c ! */
  SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)GlobalExceptionFilter);
  SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX);
#endif
  
#ifdef USE_USI
  USI_Establish(argc, argv, &(MPID_USOCK_Data_global.MyWorldRank), &(MPID_USOCK_Data_global.MyWorldSize), &(MPID_USOCK_Data_global.usi_handle));
  printf("### USI: USI established! (%d/%d)\n", MPID_USOCK_Data_global.MyWorldRank, MPID_USOCK_Data_global.MyWorldSize);

#else
  CTCPCommunicator_Constructor(&MPID_USOCK_Data_local.Comm);
  if(CTCPCommunicator_Create(argc,argv, &MPID_USOCK_Data_local.Comm) <0)
  {
    MPID_USOCK_Error("Process Startup failed",-1);
  }
  
  MPID_USOCK_Data_global.MyWorldRank = CTCPCommunicator_GetMyId(&MPID_USOCK_Data_local.Comm);
  MPID_USOCK_Data_global.MyWorldSize = CTCPCommunicator_GetNumProcs(&MPID_USOCK_Data_local.Comm);
#endif
  
  if( MPID_MyWorldRank == -1 )
    MPID_MyWorldRank = MPID_USOCK_Data_global.MyWorldRank;
    
  if( MPID_MyWorldSize == -1 )
    MPID_MyWorldSize = MPID_USOCK_Data_global.MyWorldSize;

#ifdef USE_USI
  MPID_USOCK_Data_local.numConnections =  MPID_MyWorldSize - 1;   
#else
  MPID_USOCK_Data_local.numConnections = CTCPCommunicator_GetFDs(&MPID_USOCK_Data_local.Channels, &MPID_USOCK_Data_local.Comm); /* initializes Channels with an array of file descriptors for the open sockets */
  FD_ZERO(&MPID_USOCK_Data_local.allFds); /* set bits for all file descriptors to zero */
#endif

  MPID_USOCK_Data_local.ReceiveRequestArray = (ReceiveRequest**)malloc(MPID_USOCK_Data_global.MyWorldSize * sizeof(ReceiveRequest*));

  for(i=0; i<MPID_USOCK_Data_global.MyWorldSize; i++)
  {
    MPID_USOCK_Data_local.ReceiveRequestArray[i] = (ReceiveRequest*)malloc(sizeof(ReceiveRequest));


#ifdef USE_USI
    MPID_USOCK_Init_ReceiveRequest(0, MPID_USOCK_Data_local.ReceiveRequestArray[i]);
    MPID_USOCK_Data_local.ReceiveRequestArray[i]->from=i;
#else
    MPID_USOCK_Init_ReceiveRequest(MPID_USOCK_Data_local.Channels[i], MPID_USOCK_Data_local.ReceiveRequestArray[i]);
    if(MPID_USOCK_Data_local.Channels[i] != INVALID_SOCKET)
    {
      WSAEventSelect(MPID_USOCK_Data_local.Channels[i],MPID_USOCK_Data_global.Events[MPID_USOCK_Data_global.MyWorldRank],FD_READ); /* wsock2function */
      FD_SET(MPID_USOCK_Data_local.Channels[i],&MPID_USOCK_Data_local.allFds); /* sets the bit for the file descriptor Channels[i] in allFds */
    }
#endif
  }  
  MPID_THREAD_DS_LOCK_INIT(&tcp_mutex);

  
  DSECTLEAVE
    return;
}

void MPID_USOCK_Finalize(void)
{
  DSECTION("MPID_USOCK_Finalize");
  int i;
  UsockHeader msg;
  MPID_PKT_T   pkt;
  WSABUF SBuf;
  DWORD ToSend = 0;
  BOOL top = TRUE;
  
  DSECTENTRYPOINT;

  exit(0);

  DNOTICEI("Shutting down USOCK device, numConnections: ",MPID_USOCK_Data_local.numConnections);
  for(i=MPID_USOCK_Data_global.MyWorldRank+1;(i<MPID_USOCK_Data_global.MyWorldSize)&&top;++i)
    top = (MPID_USOCK_Data_local.Channels[i] == INVALID_SOCKET);
  
  
  while(MPID_USOCK_Data_local.numConnections&&top) {	
        MPID_USOCK_RecvAnyControl(&pkt,sizeof(pkt),&i);
  }
  
    
  for(i=0;i<MPID_USOCK_Data_global.MyWorldSize;++i) {
    if(i != MPID_USOCK_Data_global.MyWorldRank && MPID_USOCK_Data_local.Channels[i] != INVALID_SOCKET) {
      SBuf.len=PRIVATESIZE;
      SBuf.buf=(char*)&msg;
      msg.Type=QUIT;
      msg.Size=0;
      DNOTICEI("Sending QUIT to ",i);
      MPID_PKT_PACK(&msg,PRIVATESIZE,i);
      while(WSASend(MPID_USOCK_Data_local.Channels[i],&SBuf,1,&ToSend,0,0,0) && WSAGetLastError() == WSAEWOULDBLOCK){
	MPID_USOCK_DEBUG_PRINT_MSG2("%d bytes sent",ToSend);
	Sleep( 1000 );
      }
      /* MPID_USOCK_SendBlock(&SBuf,PRIVATESIZE,1,i); */
      MPID_USOCK_Data_local.ReceiveRequestArray[i]->shuttingDown = TRUE;
      shutdown(MPID_USOCK_Data_local.Channels[i],SD_SEND);
    }
  }
  
  while(MPID_USOCK_Data_local.numConnections) {	
    MPID_USOCK_RecvAnyControl(&pkt,sizeof(pkt),&i);
  }
  
  /* Sleep(10); */
  
  MPID_THREAD_DS_LOCK_FREE(&tcp_mutex)

    while(STL_deque_size(&MPID_USOCK_Data_local.SendRequestQueue)) {
      free(STL_deque_front_TraReq(&MPID_USOCK_Data_local.SendRequestQueue));
      STL_deque_pop_front(&MPID_USOCK_Data_local.SendRequestQueue);
    }
  
    while(STL_deque_size(&MPID_USOCK_Data_local.TransferRequestQueue)) {
      free(STL_deque_front_TraReq(&MPID_USOCK_Data_local.TransferRequestQueue));
      STL_deque_pop_front(&MPID_USOCK_Data_local.TransferRequestQueue);
    }
    for(i=0;i<MPID_USOCK_Data_global.MyWorldSize;i++) {
      free(MPID_USOCK_Data_local.ReceiveRequestArray[i]);
    }
    free(MPID_USOCK_Data_local.ReceiveRequestArray);

    DSECTLEAVE;
}

void MPID_USOCK_RecvAnyControl( MPID_PKT_T *pkt, int size, int *from ) {
  DSECTION("MPID_USOCK_RecvAnyControl");
  static int current=0;
  FD_SET readfds;
  int res,found;
  ReceiveRequest *Request;
  BOOL local;

  DSECTENTRYPOINT;
  
  found = 0;
  LOG_ANY_CTRL(0);

#ifdef USE_USI
  while(!found)
  {
    if(current==MPID_USOCK_Data_global.MyWorldRank)
    {
      if(STL_deque_size(&MPID_USOCK_Data_local.SelfSendChannel))
      {
	Request=MPID_USOCK_Data_local.ReceiveRequestArray[current];
	found = MPID_USOCK_CheckType_ReceiveRequest(Request);
	if(found)
	{
	  *from=current;
	  if(Request->Header.Size > size)
	  { /* XXX SIGNED <-> UNSIGNED CONFLICT !!! */
	    MPID_USOCK_Error("Control message too large...\n",Request->Header.Size);
	  }
	  DNOTICEI("Trying to read pkt of size ",Request->Header.Size);
	  MPID_USOCK_Start_ReceiveRequest(((char*)pkt)+PRIVATESIZE, Request->Header.Size-PRIVATESIZE,BLOCKING, Request);
	}
      }
    }
    else
    {    
      if(USI_Probe(current, USI_DONT_BLOCK, MPID_USOCK_Data_global.usi_handle)==USI_SUCCESS)
      {
	Request=MPID_USOCK_Data_local.ReceiveRequestArray[current];
	found = MPID_USOCK_CheckType_ReceiveRequest(Request);
	if(found)
	{
	  *from=current;
	  if(Request->Header.Size > size)
	{ /* XXX SIGNED <-> UNSIGNED CONFLICT !!! */
	  MPID_USOCK_Error("Control message too large...\n",Request->Header.Size);
	}
	  DNOTICEI("Trying to read pkt of size ",Request->Header.Size);
	  MPID_USOCK_Start_ReceiveRequest(((char*)pkt)+PRIVATESIZE, Request->Header.Size-PRIVATESIZE,BLOCKING, Request);
	}
      }
    }
    current=(current+1)%MPID_USOCK_Data_global.MyWorldSize;
  }

#else
  while(!found)
  {
    local=FALSE;
    if(STL_deque_size(&MPID_USOCK_Data_local.SelfSendChannel))
    {
      /* we got a message from ourselves: */
      res=1;
      local = TRUE;
      current = MPID_USOCK_Data_global.MyWorldRank;
    }
    else
    {
      if((MPID_USOCK_Data_global.MyWorldSize>1) && (MPID_USOCK_Data_local.numConnections))
      {
	/* select all file descriptors (sockets) as being ready for reading: */
	readfds = MPID_USOCK_Data_local.allFds;
	res=select(FD_SETSIZE,&readfds,0,0,0);
	if(res==SOCKET_ERROR)
	{
	  MPID_USOCK_SysError("RecvAnyControl: select failed",WSAGetLastError());
	}
      }
      else
      {
	if(MPID_USOCK_Data_global.MyWorldSize>1) res = 0;
	else
	{
	  DERROR("Logical error. Waiting for message while num_procs == 1!");
	  MPID_USOCK_Error("Logical error. Waiting for message while num_procs == 1!",-1);
	}
      }
    }
    
    while( (res>0) && (!found) ) {
      if(current >= MPID_USOCK_Data_global.MyWorldSize) current = 0;
      if(
	 (
	  (MPID_USOCK_Data_local.Channels[current] != INVALID_SOCKET)	&&    /* we have a valid socket for communication with current */
	  (FD_ISSET(MPID_USOCK_Data_local.Channels[current],&readfds))       /* and it was selected as being ready for reading above */
	  ) || 
	 (
	  local &&
	  (current == MPID_USOCK_Data_global.MyWorldRank)
	  )
	 ) {
	--res;
	Request=MPID_USOCK_Data_local.ReceiveRequestArray[current];
	found = MPID_USOCK_CheckType_ReceiveRequest(Request);
	if(found && (MPID_USOCK_Data_local.numConnections || MPID_USOCK_Data_global.MyWorldSize<2)) {
	  *from=current;
	  if(Request->Header.Size > size) { /* XXX SIGNED <-> UNSIGNED CONFLICT !!! */
	    MPID_USOCK_Error("Control message too large...\n",Request->Header.Size);
	  }
	  DNOTICEI("Trying to read pkt of size ",Request->Header.Size);
	  /*
	    Request->StartRequest(((char*)pkt)+PRIVATESIZE,
					  Request->Header.Size-PRIVATESIZE,BLOCKING);
	  */
	  MPID_USOCK_Start_ReceiveRequest(((char*)pkt)+PRIVATESIZE, Request->Header.Size-PRIVATESIZE,BLOCKING, Request);
	}
	
      } /* if( FD_ISSET...)*/
      ++current;
    } /*while(res>0) */
    if(!MPID_USOCK_Data_local.numConnections) break;
  } /* while (!found) */
#endif

  LOG_ANY_CTRL(1);
  
  DSECTLEAVE;
}

int MPID_USOCK_ControlMsgAvail( void )
/*
 |   Check if there is a control message available from any process:
 |
 */
{
  DSECTION("MPID_USOCK_ControlMsgAvail");

  static int start=0;
  int current = 0; /* <-- ID of process that we check */
  int res,found;
  FD_SET readfds;
  struct timeval t={0,0};  
  BOOL local=FALSE;
  
  DSECTENTRYPOINT;

#ifdef USE_USI
  for(current=0; current<MPID_USOCK_Data_global.MyWorldSize; current++)
  {
    printf("CURRENT: %d\n", current);

    found=0;
    if(current==MPID_USOCK_Data_global.MyWorldRank)
    {
      DNOTICEI("checking local channel",current);
      if(STL_deque_size(&MPID_USOCK_Data_local.SelfSendChannel))
      {
	DNOTICE("checking for incoming type");
	found = MPID_USOCK_CheckType_ReceiveRequest( MPID_USOCK_Data_local.ReceiveRequestArray[current] );
      }
    }
    else
    {  
      DNOTICEI("checking channel",current);  
      if(USI_Probe(current, USI_DONT_BLOCK, MPID_USOCK_Data_global.usi_handle)==USI_SUCCESS)
      {
	DNOTICE("checking for incoming type");
	found = MPID_USOCK_CheckType_ReceiveRequest( MPID_USOCK_Data_local.ReceiveRequestArray[current] );
	if(found==TRUE) break;
      }
    }
  }
  
#else
    
  sleep(1);
  found = 0;
  if(STL_deque_size(&MPID_USOCK_Data_local.SelfSendChannel))
  {
    /* chere is a message available that we sent to ourselves: */
    res = 1;
    local = TRUE;
    current = MPID_USOCK_Data_global.MyWorldRank;
  } 
  else
  {
    /* check the other procs (if they exist): */
    if((MPID_USOCK_Data_global.MyWorldSize>1) && (MPID_USOCK_Data_local.numConnections))
    { 
      /* select all file descriptors (sockets) as being ready for reading: */
      readfds = MPID_USOCK_Data_local.allFds;

      DNOTICE("select on any fds");

      res=select(FD_SETSIZE,&readfds,0,0,&t);
      if(res==SOCKET_ERROR)
      {
	MPID_USOCK_SysError("ControlMsgAvail: select failed",WSAGetLastError());
      }
      DNOTICEI("select successfull, res=",res);
    } 
    else res=0;
  }

  while((res>0) && (!found))
  {
    DNOTICE("(res>0) && (!found))");
    
    if(current >= MPID_USOCK_Data_global.MyWorldSize)
    {
      DNOTICE("current >= MPID_USOCK_Data_global.MyWorldSize");
      current = 0; /* <-- continue with process 0 */
    }
    
    if(/* check for message from another process: */
       ((MPID_USOCK_Data_local.Channels[current] != INVALID_SOCKET) && (FD_ISSET(MPID_USOCK_Data_local.Channels[current],&readfds)))       
    || /* check for message from ourselves: */
       (local && (current == MPID_USOCK_Data_global.MyWorldRank)))
    {
      DNOTICEI("checking channel",current);
      res--;
      DNOTICE("checking for incoming type");
      found = MPID_USOCK_CheckType_ReceiveRequest( MPID_USOCK_Data_local.ReceiveRequestArray[current] );
    } 	
    current++; /* <-- check for messages from next process */
  }
#endif

  printf("FOUND: %d\n", found);
    
  DSECTLEAVE
    return found;	
}


int MPID_USOCK_RecvFromChannelAsync( void *buf, int size, int channel, ASYNCRecvId_t id )
{
  DSECTION("MPID_USOCK_RecvFromChannelAsync");

  ReceiveRequest *Request;
  TransferRequest *Transfer;
  
  DSECTENTRYPOINT;

  printf("!!!!!!!!!!!!!!!!!\n");
  exit(-1);
  
  Request=MPID_USOCK_Data_local.ReceiveRequestArray[channel];
  Transfer=MPID_USOCK_Get_TransferRequest(buf,size);
  Request->Transfer=Transfer;

  if(!MPID_USOCK_Start_ReceiveRequest((char*)buf,size,NONBLOCKING, Request)) {
    memcpy(id,&Transfer,sizeof(Transfer));
    DSECTLEAVE
      return 0;
  }
  /* Request->StartRequest(); */
  MPID_USOCK_Free_TransferRequest(Transfer);
  DSECTLEAVE
    return 1;
}

void MPID_USOCK_RecvFromChannel( void *buf, int size, int channel)
{
  DSECTION("MPID_USOCK_RecvFromChannel");

  ReceiveRequest *Request;
  
  DSECTENTRYPOINT;
  
  LOG_RECV(0);
  Request=MPID_USOCK_Data_local.ReceiveRequestArray[channel];

  MPID_USOCK_Start_ReceiveRequest((char*)buf,size,BLOCKING, Request);
  /*
    while(!Request->TestRequest(TRUE)) ; 
    MPID_DeviceCheck(MPID_NOTBLOCKING);
    */
  /* Request->StartRequest(); */
  LOG_RECV(1);
  DSECTLEAVE;
}

void MPID_USOCK_ConsumeData(int size,int channel) {
  DSECTION("MPID_USOCK_ConsumeData");
  char buf[4096];
  int toRead;
  ReceiveRequest *Request;
  
  DSECTENTRYPOINT;
  
  Request=MPID_USOCK_Data_local.ReceiveRequestArray[channel];
  
  while(size) {
    toRead = size<4096?size:4096;
   
    MPID_USOCK_Start_ReceiveRequest((char*)buf,toRead,BLOCKING, Request);
    size -= toRead;
  }
  
  DSECTLEAVE;
}

	
void MPID_USOCK_SendControl(MPID_PKT_T *pkt,int size,int dest)
/*
 |   Send the control packet (short packet) pointed to by 'pkt'  and with 'size'
 |   bytes of effective size (varies according to length of user message) to 
 |   process with ID 'dest'
 */
{
  DSECTION("MPID_USOCK_SendControl");
 
  WSABUF SBuf; /* <-- structure representing a data buffer (winsock2) */
  SendRequest *Request;
  
  DSECTENTRYPOINT;

  /* get a send request from the queue of send requests: */
  Request = MPID_USOCK_Get_SendRequest();
  
  /* set additional information in packet: */
  pkt->head.TCP_TYPE = CTRL; 
  pkt->head.TCP_SIZE = size;
  
  MPID_PKT_PACK(&pkt->head,PRIVATESIZE,dest);
  
  /* initialize buffer structure: */
  SBuf.len = size;
  SBuf.buf = (char*)pkt;
  
  if( MPID_USOCK_Start_SendRequest( &SBuf, size, 1, dest, Request, WSA_NOT_OVERLAPPED ) < 0 )  /* <- WORKAROUND: 
											  |    Control blocks are now always sent via a 
											  |    blocking (not overlapped) WSASend().
											  |    So the following WaitSend() will be skipped:
											  */
    MPID_USOCK_WaitSend( (long *)&Request );
  
  DSECTLEAVE;
}


int MPID_USOCK_SendChannel(WSABUF *buf, int size, unsigned long NumBlocks,int dest, ASYNCSendId_t id, BOOL WSAisOverlapped)
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
  DSECTION("MPID_USOCK_SendChannel");
  SendRequest *Request;
  
  DSECTENTRYPOINT;
 
  /* Obtain a free SendRequest: */
  Request=MPID_USOCK_Get_SendRequest();
  
  MEMCPY(id,&Request,sizeof(Request));
  DSECTLEAVE
    return MPID_USOCK_Start_SendRequest(buf,size,NumBlocks,dest,Request, WSAisOverlapped);
}


int MPID_USOCK_TestSend(ASYNCSendId_t id)
/*
 |   Test, if the underlying (channel) send operation has been finished.
 |   The respective send request is retrieved via a send id, maintained by
 |   the MPIR-layer.
 */
{
  DSECTION("MPID_USOCK_TestSend");
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
#ifdef USE_USI
    if(USI_Test(USI_DONT_BLOCK, &(Request->usi_request), MPID_USOCK_Data_global.usi_handle)==USI_SUCCESS) Finished=true;
#else
    Finished=HasOverlappedIoCompleted(&Request->Overlapped);
#endif
  } 
    
  if(Finished)
  {
#ifndef USE_USI  
    /* operation finished, but _whole_ message realy sent? */
    if(!WSAGetOverlappedResult(Request->fd,&Request->Overlapped,&ToSend,FALSE,&flags) || ToSend != Request->size)
    {
      /* this should not happen: */
      MPID_USOCK_SysError("TestSend: WSASend truncated message",WSAGetLastError());
    }
#endif
    /* operation finished, so the SendRequest can be freed: */
    MPID_USOCK_Free_SendRequest(Request);	    	          
  }

  LOG_TEST_SEND(1);

  /* report the status: */
  DSECTLEAVE
    return (int)Finished;
}


void MPID_USOCK_WaitSend(ASYNCSendId_t id)
/*
 |   Wait until the underlying (channel) send operation has been finished.
 |   The respective send request is retrieved via a send id, maintained by
 |   the MPIR-layer.
 */
{
  DSECTION("MPID_USOCK_WaitSend");

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
 
#ifdef USE_USI

  if(!Finished) USI_Test(USI_BLOCK, &(Request->usi_request), MPID_USOCK_Data_global.usi_handle);

#else
  if(!Finished) MPID_USOCK_GenericWait(&Request->Overlapped);
  
  /* _whole_ message realy, realy sent? */
  if(!WSAGetOverlappedResult(Request->fd,&Request->Overlapped,&ToSend,FALSE,&flags) || ToSend != Request->size)
  {
    /* this should not happen: */
    MPID_USOCK_SysError("Wait Send: WSASend truncated message",WSAGetLastError());
  }
#endif
  
  MPID_USOCK_Free_SendRequest(Request);
  
  LOG_WAIT_SEND(1);
  
  DSECTLEAVE
    return;
}


void MPID_USOCK_CreateTransfer(void * buf, int size,int tag, ASYNCRecvId_t id)
{
    DSECTION("MPID_USOCK_Create_TransferRequest");

    TransferRequest *t;
    
    DSECTENTRYPOINT;

    t=MPID_USOCK_Get_TransferRequest(buf,size);
   
    MPID_USOCK_DEBUG_PRINT_MSG2("Creating TransferRequest for size %d",size);
    STL_map_insert_int2TraReq(tag, t, &MPID_USOCK_Data_local.TransferRequestMap);
    MEMCPY(id,&t,sizeof(t));
    DSECTLEAVE;
}


void MPID_USOCK_ReceiveTransfer(int tag, ASYNCRecvId_t id)
{
    DSECTION("MPID_USOCK_Receive_TransferRequest");
    TransferRequest *t;

    DSECTENTRYPOINT;
    MEMCPY(&t,id,sizeof(t));

    if(tag>0) STL_map_erase(STL_map_find_int2TraReq(tag, &MPID_USOCK_Data_local.TransferRequestMap), &MPID_USOCK_Data_local.TransferRequestMap);
 
    MPID_USOCK_Free_TransferRequest(t);

    DSECTLEAVE;
}


int MPID_USOCK_TestTransfer(ASYNCRecvId_t id)
{
    DSECTION("MPID_USOCK_Test_TransferRequest");
    TransferRequest *t;

    DSECTENTRYPOINT;

    MEMCPY(&t,id,sizeof(t));
    if(t->Receive) {
	DSECTLEAVE
	  return MPID_USOCK_Test_ReceiveRequest(NONBLOCKING, t->Receive);
    }
    else {
      DSECTLEAVE
	return t->finished;
    }
}


void MPID_USOCK_WaitTransfer(ASYNCRecvId_t id)
{
    DSECTION("MPID_USOCK_Wait_TransferRequest");
    TransferRequest *Transfer;
    
    DSECTENTRYPOINT;

    memcpy(&Transfer,id,sizeof(Transfer));
    while(!Transfer->finished) {
	if(!Transfer->Receive) {
	    /* The transfer didn't start yet... */
	    MPID_DeviceCheck(MPID_NOTBLOCKING);
	} else
            MPID_USOCK_Wait_ReceiveRequest(Transfer->Receive);
    }
    MPID_USOCK_Free_TransferRequest(Transfer);
    
    DSECTLEAVE
      return;
}


void MPID_USOCK_SysError( const char *string, int value ) 
{
  DSECTION("MPID_USOCK_SysError");
  static char all[1024];
  void *lpMsgBuf;
  
  DSECTENTRYPOINT;
  
  if(string) {
    strcpy(all,string);
    strcat(all,":\n");
  }
  FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		 NULL,value,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		 (LPTSTR) &lpMsgBuf,1,NULL);
  
  if(lpMsgBuf) strcat(all,(char*)lpMsgBuf);
  MPID_USOCK_Error(all,value);
  DSECTLEAVE;
}


void MPID_USOCK_Error(const char *string,int value)
{
  DSECTION("MPID_USOCK_Error");

  DSECTENTRYPOINT;

  fprintf(stderr,"(%d) %s (%d)\n", MPID_USOCK_Data_global.MyWorldRank, string, value); fflush(stderr);

  DSECTLEAVE;

  exit(value);
}

   /**********************************
--- ** internal (static) functions: ** --------------------------------------------------------
    **********************************/

/*****************************************
 ** functions to handle 'SendRequests': **
 *****************************************/

static void MPID_USOCK_Init_SendRequest(SendRequest* SendRequestPt)
/*
 |   Initialize a SelfRequest structure:
 |
 */
{
  DSECTION("SendRequest_SendRequest");

  DSECTENTRYPOINT;

#ifdef USE_USI

#else
  SendRequestPt->Overlapped.hEvent=CreateEvent(0,FALSE,FALSE,0); 
  SendRequestPt->finished=TRUE;
  if(SendRequestPt->Overlapped.hEvent == WSA_INVALID_EVENT) MPID_USOCK_Error("Creation of event failed",WSAGetLastError());
#endif

  DSECTLEAVE
    return;
}

static int MPID_USOCK_Start_SendRequest( WSABUF *buf, int size, unsigned long NumBlocks, int dest, SendRequest *Request, BOOL WSAisOverlapped )
/*
 |   MPID_USOCK_Start_SendRequest() sends data to a destination process 'dest'.
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
  DSECTION("MPID_USOCK_Start_SendRequest");

  DWORD ToSend = size; /* <-- number of bytes that are to be sent */
  int res, error, count;
  BOOL retry;
  SelfSend* self_msg;
  
  DSECTENTRYPOINT;
  
  LOG_SEND(0);
  
  if( dest==MPID_USOCK_Data_global.MyWorldRank )
  {
    /*
     |   if we are sending to ourselves, we make a SelfSend out of each
     |   buffer to be sent and put them into 'SelfSendChannel':
     */
    for( count = 0; count < NumBlocks; count++ ) /* XXX SIGNED UNSIGNED !!! */
    {
      self_msg=(SelfSend*)malloc(sizeof(SelfSend));
      MPID_USOCK_Init_SelfSend( buf[count].buf, buf[count].len, self_msg);
      STL_deque_push_back_SelfSend( self_msg, &MPID_USOCK_Data_local.SelfSendChannel );
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
     
    /* initialize send request: */
    Request->size = size;
    
    DNOTICEI("Sending message of size ", size);

#ifdef USE_USI
    for(count=0; count < NumBlocks; count++ )
    {
      do
      {

	retry = FALSE;
	res=USI_Send(dest, buf[count].buf, buf[count].len, USI_DONT_BLOCK, &(Request->usi_request), MPID_USOCK_Data_global.usi_handle);
	
	switch(res)
	{
	  case USI_PENDING:
		   /*
		    |   Overlapped operation was successfully initiated,
		    |   completion will be indicated later:
		    |   (no "real" error)
		    */
		   Request->finished = FALSE;
		   retry = FALSE;
		   break;
	  case USI_RETRY:
		   /*
		    |   A blocking send call reports, that there are not enough
                    |   free buffers in the underlying transport layer.
                    |   To avoid a buffer deadlock deadlock, the blocking send
                    |   is omitted and we can hope to solve the deadlock by
                    |   receiving some data:
 		    */
		   MPID_DeviceCheck( MPID_NOTBLOCKING ); /* <-- do something useful and then try again */
		   retry = TRUE;
		   break;
	  case USI_SUCCESS:
		   /* we are done with this send operation: */
		   Request->finished = TRUE;
		   retry = FALSE;
		   break;
	  default:
		   DERROR("MPID_USOCK_Start_SendRequest: WSASend failed");
		   MPID_USOCK_SysError("MPID_USOCK_Start_SendRequest: WSASend failed",error);
		   
	}
      } while(retry);
      USI_Test(USI_BLOCK, &(Request->usi_request), MPID_USOCK_Data_global.usi_handle);
    }

#else      
    /*
     |   This is a send operation to a different process:
     |
     */
     
    if(MPID_USOCK_Data_local.Channels[dest] == INVALID_SOCKET)
    {
      LOG_SEND(1);

     /* isn't this an error? where is it handled? why do we return the same value as in case of success? boris */

      DSECTLEAVE
	return 0;
    }
    
    /* initialize send request: */
    Request->size = size;
    Request->fd = MPID_USOCK_Data_local.Channels[dest];
    
    DNOTICEI("Sending message of size ", size);

    do
    {
      retry = FALSE;

      /* sending overlapped or not ? */
      if(WSAisOverlapped) res=WSASend( MPID_USOCK_Data_local.Channels[dest], buf, NumBlocks, &ToSend, 0, &Request->Overlapped, 0);
      else res=WSASend( MPID_USOCK_Data_local.Channels[dest], buf, NumBlocks, &ToSend, 0, 0, 0);
      
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
		   break;
	  default: 
		   DERROR("MPID_USOCK_Start_SendRequest: WSASend failed");
		   MPID_USOCK_SysError("MPID_USOCK_Start_SendRequest: WSASend failed",error);
	}	
      }
      else
      {
	/* maybe not everything was sent: */
	if( ToSend != size )
	{
	  /* this should not happen without an error report! (see above) */
	  DERROR("MPID_USOCK_Start_SendRequest: WSASend truncated message");
	  MPID_USOCK_Error("MPID_USOCK_Start_SendRequest: WSASend truncated message",ToSend);
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
      MPID_USOCK_Free_SendRequest( Request );
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

 
static SendRequest *MPID_USOCK_Get_SendRequest(void)
/*
 |   Returns a free SendRequest handle.
 |
 |   To avoid malloc/free overhead the handles are stored by Free_SendRequest() 
 |   in the SendRequestQueue after being used.
 |   So, Get_SendRequest() can recycle them instead of recreate them
 |   in each call.
 */
{
  DSECTION("MPID_USOCK_Get_SendRequest");

  SendRequest *Request;
  
  DSECTENTRYPOINT;
 
  /* is there a used and free handle in the queue? */
  if(!STL_deque_size(&MPID_USOCK_Data_local.SendRequestQueue))
  {
    /* No! Create a new one: */
    Request=(SendRequest*)malloc(sizeof(SendRequest));
    MPID_USOCK_Init_SendRequest(Request);
  }
  else
  {
    /* Yes! Reuse this handle: */
    Request=STL_deque_front_SendReq(&MPID_USOCK_Data_local.SendRequestQueue);
    STL_deque_pop_front(&MPID_USOCK_Data_local.SendRequestQueue);
  }
    
  DSECTLEAVE
    return Request;
}

static void MPID_USOCK_Free_SendRequest(SendRequest *Request)
/*
 |   To avoid malloc/free overhead the handles are not deleted but
 |   stored in the SendRequestQueue after being used.
 |   So, Get_SendRequest() can recycle them instead of recreate them
 |   in each call.
 */    
{  
  DSECTION("MPID_USOCK_Free_SendRequest");

  DSECTENTRYPOINT;
  
  /* Are there too much discarded SendRequest handles in the queue? */
  if(STL_deque_size(&MPID_USOCK_Data_local.SendRequestQueue) > MAX_BUFFERED_REQUESTS)
  {
#ifdef USE_USI

#else
    /* Yes! Just delete the handle: */
    WSACloseEvent(Request->Overlapped.hEvent);
#endif
    free(Request);
  }
  else
  {
    /* No! Store the handle in the queue, so that it could be recycled: */
    STL_deque_push_back_SendReq(Request, &MPID_USOCK_Data_local.SendRequestQueue);
  }
  
  DSECTLEAVE
    return;
}


/********************************************
 ** functions to handle 'ReceiveRequests': **
 ********************************************/

static void MPID_USOCK_Init_ReceiveRequest(SOCKET s, ReceiveRequest* ReceiveRequestPt)
{
  DSECTION("MPID_USOCK_Init_ReceiveRequest");

  DSECTENTRYPOINT;

#ifdef USE_USI
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
static void MPID_USOCK_Finish_ReceiveRequest(ReceiveRequest* ReceiveRequestPt)
{
  DSECTION("MPID_USOCK_Finish_ReceiveRequest");
    
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

  MPID_USOCK_DEBUG_PRINT_MSG2("Transfer with size %d finished", ReceiveRequestPt->size);
  
  DSECTLEAVE
    return;
}


static int MPID_USOCK_Start_ReceiveRequest( char *mem, unsigned long amount, int mode, ReceiveRequest* ReceiveRequestPt)
/*
 |   Try to receive 'amount' bytes into memory location pointed to by 'mem'
 |   using socket fd (member variable)
 */
{
  DSECTION("MPID_USOCK_Start_ReceiveRequest");

  unsigned long flags = 0;          /* flags for WSARecv() operation */
  unsigned long amountReceived = 0; /* number of bytes received from socket */
  unsigned long count_nobufs = 0;   /* number of times WSAENOBUFS error occurred */
  int error,res,retry;
  /* WSANETWORKEVENTS NetEvents; */
  
  DSECTENTRYPOINT;
  
  LOG_START_REQUEST(0);
  
  MPID_USOCK_DEBUG_PRINT_MSG2("Starting ReceiveRequest size %d ",amount);
  
  if(ReceiveRequestPt->State == TRANSFERRING) MPID_USOCK_Error("Request already in progress.",-1);
  if(ReceiveRequestPt->Transfer) ReceiveRequestPt->Transfer->Receive=ReceiveRequestPt;
  if(amount <=0) MPID_USOCK_Finish_ReceiveRequest(ReceiveRequestPt);
  else {
    /* initialize structure that describes our receive buffer */
    ReceiveRequestPt->buf.len = amount; 
    ReceiveRequestPt->buf.buf = mem; 

#ifdef USE_USI
    if(ReceiveRequestPt->from!=MPID_USOCK_Data_global.MyWorldRank)
    {
      do
      {
	retry = 0;

	res=USI_Recv(ReceiveRequestPt->from, mem, amount, USI_DONT_BLOCK, &(ReceiveRequestPt->usi_request), MPID_USOCK_Data_global.usi_handle);

	switch(res)
	{
	  case USI_PENDING:
		   /* overlapped operation was successfully initiated, completion will
		    * be indicated later */
		   ReceiveRequestPt->pending = TRUE;
		   MPID_USOCK_DEBUG_PRINT_MSG2( "ReceiveRequest of size %d is pending...", amount );
		   break;
	  case USI_RETRY:
		   /* Overlapped sockets: too many outstanding overlapped I/O requests
		    * Nonoverlapped sockets: socket is nonblocking, operation cannot be completed immediately */
		   fprintf( stderr,"Warning: StartRequest: Got wouldblock error...\n" ); fflush( stderr );
		   retry = 1; Sleep( 10 );				
		   break;
	  case USI_SUCCESS:
		   /* everything went fine */
		   ReceiveRequestPt->pending = FALSE;
		   MPID_USOCK_DEBUG_PRINT_MSG2("ReceiveRequest size %d received", amountReceived );
		   break;
	  default:
		   fprintf(stderr,"\nmem==%x\namount==%d\nstate==%d, read==%d\n",
			   mem,amount,ReceiveRequestPt->State,amountReceived );
		   MPID_USOCK_SysError("Start Request:WSARecv failed",error);
	}
      }
      while(retry);
    
      if( USI_Test(USI_DONT_BLOCK, &(ReceiveRequestPt->usi_request), MPID_USOCK_Data_global.usi_handle)==USI_SUCCESS)
      {
	/* we received as many bytes as we wanted to */
	ReceiveRequestPt->size=(ReceiveRequestPt->usi_request).actual_size;
	MPID_USOCK_Finish_ReceiveRequest(ReceiveRequestPt);
	
	LOG_START_REQUEST(1);
	DSECTLEAVE
	  return 1;
      }
      else
      {
	/* we received some bytes, but not all that we wanted to */
	ReceiveRequestPt->State = TRANSFERRING; /* mark this request as being in progress */
	ReceiveRequestPt->received=0;
	ReceiveRequestPt->finished = FALSE;
	ReceiveRequestPt->size=(ReceiveRequestPt->usi_request).actual_size;
	if(mode == BLOCKING) MPID_USOCK_Wait_ReceiveRequest(ReceiveRequestPt); /* TestRequest(mode); */
	else if(!res) MPID_USOCK_Test_ReceiveRequest(mode, ReceiveRequestPt);
      }
    }
    else
    {
      /* maybe there is packet that we sent to ourselves */
      if(STL_deque_size(&MPID_USOCK_Data_local.SelfSendChannel))
      {
	MPID_USOCK_ReadLocal(mem,amount);
	MPID_USOCK_Finish_ReceiveRequest(ReceiveRequestPt);	
      }
    }
  
#else
    if(ReceiveRequestPt->fd != INVALID_SOCKET) {
      /* Do this to clear the buffer on the socket. */
      /* WSAEnumNetworkEvents(fd,0,&NetEvents); */
      do {
	retry = 0;
	
	/* now we receive buf.len bytes into the buffer pointed to by buf.buf, using socket fd */
	res = WSARecv(ReceiveRequestPt->fd, &(ReceiveRequestPt->buf), 1, &amountReceived, &flags, &(ReceiveRequestPt->Overlapped), 0); /* wsock2function */
	
	/* error handling */
	if( res == SOCKET_ERROR ) {
	  error = WSAGetLastError();
	  switch( error ) {
	    case WSA_IO_PENDING:
		     /* overlapped operation was successfully initiated, completion will
		      * be indicated later */
		     ReceiveRequestPt->pending = TRUE;
		     MPID_USOCK_DEBUG_PRINT_MSG2( "ReceiveRequest of size %d is pending...", amount );
		     break;
	    case WSAEWOULDBLOCK:
		     /* Overlapped sockets: too many outstanding overlapped I/O requests
		      * Nonoverlapped sockets: socket is nonblocking, operation cannot be completed immediately */
		     fprintf( stderr,"Warning: StartRequest: Got wouldblock error...\n" ); fflush( stderr );
		     retry = 1; Sleep( 10 );
		     break;
	    case WSAENOBUFS:
		     /* this error code is defined in nt2unix.h, but I didn't find it in
		      * Microsoft's API Documentation for WSARecv(), boris */
		     fprintf(stderr,"Warning: StartRequest: Got nobufs error...\n"); fflush(stderr);
		     retry = 1; count_nobufs++; Sleep( 10 );
		     if( count_nobufs < 1000 ) break;
		     /* if this has happened too often, don't retry but raise an error */
	    default:
		     fprintf(stderr,"\nmem==%x\namount==%d\nstate==%d, read==%d\n",
			     mem,amount,ReceiveRequestPt->State,amountReceived );
		     MPID_USOCK_SysError("Start Request:WSARecv failed",error);
	  } 
	} else {
	  /* this is in case we didn't get a SOCKET_ERROR back, but didn't receive anything also */
	  if( amountReceived == 0 )  {
	    if(!ReceiveRequestPt->shuttingDown) {
				MPID_USOCK_DEBUG_PRINT_MSG("Read 0 bytes from network, connection seems to be closed");
				MPID_USOCK_Error("StartRequest: Found dead network connection\n",0);
	    } else {
	      MPID_USOCK_DEBUG_PRINT_MSG("Read 0 bytes from network, connection seems to be closed");
	      ReceiveRequestPt->pending =FALSE;
	      MPID_USOCK_Finish_ReceiveRequest(ReceiveRequestPt);
	      ReceiveRequestPt->State = CLOSED;
	      LOG_START_REQUEST(1);
	      DSECTLEAVE
				    return 1;
	    }
	  }
	  
	  /* everything went fine */
	  ReceiveRequestPt->pending = FALSE;
	  MPID_USOCK_DEBUG_PRINT_MSG2("ReceiveRequest size %d received", amountReceived );
	}
      } while(retry);
      
      if( amountReceived == amount ) {
	/* we received as many bytes as we wanted to */
#ifdef MPID_DEBUG_ALL
	ReceiveRequestPt->size=amount;
#endif
	MPID_USOCK_Finish_ReceiveRequest(ReceiveRequestPt);
	
	LOG_START_REQUEST(1);
	DSECTLEAVE
	  return 1;
	
      } else {
	/* we received some bytes, but not all that we wanted to */
	ReceiveRequestPt->State = TRANSFERRING; /* mark this request as being in progress */
	ReceiveRequestPt->received=0;
	ReceiveRequestPt->finished = FALSE;
	ReceiveRequestPt->size=amount;
	if(mode == BLOCKING) MPID_USOCK_Wait_ReceiveRequest(ReceiveRequestPt); /* TestRequest(mode); */
	else if(!res) MPID_USOCK_Test_ReceiveRequest(mode, ReceiveRequestPt);
      }
    } /* if( fd != INVALID_SOCKET ) */
    else {
      /* maybe there is packet that we sent to ourselves */
      if(STL_deque_size(&MPID_USOCK_Data_local.SelfSendChannel)) {
	MPID_USOCK_ReadLocal(mem,amount);
	MPID_USOCK_Finish_ReceiveRequest(ReceiveRequestPt);	
      } else ResetEvent(ReceiveRequestPt->Overlapped.hEvent);
    }
#endif
  }
  
  LOG_START_REQUEST(1);
	
  DSECTLEAVE
    return ReceiveRequestPt->finished;
}

static int MPID_USOCK_CheckType_ReceiveRequest( ReceiveRequest *Request ) {
    DSECTION("MPID_USOCK_CheckType_ReceiveRequest");
    int again,found=0;
    
    DSECTENTRYPOINT;
    
    /* if receive operation is in progress */
    if(Request->State == TRANSFERRING) {
      MPID_USOCK_DEBUG_PRINT_MSG2("CheckType_ReceiveRequest: Continued Transfer request for tag %d",Request->Header.Tag);
      MPID_USOCK_Test_ReceiveRequest(NONBLOCKING, Request);
      DSECTLEAVE
	return 0;
    }
    do {
      again = 0;
      if(Request->State == CLEAN) {
	/*
	  Request->GetHeader();
	*/
	MPID_USOCK_GetHeader_ReceiveRequest(Request);
	MPID_PKT_UNPACK(&Request->Header,PRIVATESIZE,current);
      }
      if((Request->Header.Type & CTRL) == CTRL) {
	/* found a control packet / short message */
	found = 1;
      } 
      else {
	if((Request->Header.Type & TRANSFER) == TRANSFER) {
	  DNOTICEI("CheckType_ReceiveRequest: Found Transfer Message for tag ",Request->Header.Tag);			
	  MPID_USOCK_RecvTransferMessage(Request);
	} 
	    else {
	      if((Request->Header.Type & QUIT) == QUIT || Request->State == CLOSED) {
		DNOTICEI("RecvAnyControl: Got QUIT on ",Request->fd);
		MPID_USOCK_Data_local.numConnections--;
		DNOTICEI("Got Quit message. Waiting for # procs", MPID_USOCK_Data_local.numConnections);
		ResetEvent(Request->Overlapped.hEvent);
		    Request->State = CLEAN;
		    
		    /* if file descriptor is invalid, unset its bit in the file descriptor set allFds */
		    if(Request->fd != INVALID_SOCKET)
		      FD_CLR(Request->fd,&MPID_USOCK_Data_local.allFds);
		    /* if(!MPID_USOCK_Data_local.numConnections) found = 1; */
	      } 
	      else {
		printf("Type==%x\nTag==%d\nSize==%d\n",Request->Header.Type,Request->Header.Tag,Request->Header.Size);
		DERROR("Protocol error: CheckType_ReceiveRequest found illegal message instead of CTRL");
		MPID_USOCK_Error("Protocol error: CheckType_ReceiveRequest found illegal message instead of CTRL",-1);
	      }
	    }
      }
    } while(again);
    
    DSECTLEAVE
	return found;
}


static int MPID_USOCK_Test_ReceiveRequest( int mode, ReceiveRequest* ReceiveRequestPt)
/*
 |   TestRequest() returns TRUE if receive operation is finished, FALSE otherwise
 |   if called with 'mode == BLOCKING', it waits until operation is finished
 */
{
  DSECTION("MPID_USOCK_Test_ReceiveRequest");

  unsigned long  amountReceived = 0; /* <-- number of bytes that were received during IO operation */
  unsigned long flags,count_nobufs =0;
  WSAOVERLAPPED *Over;
  int error;
  int res;
  
  DSECTENTRYPOINT;
  
  LOG_TEST_REQUEST(0);
  
  /* we only need to do something here, if the request is not already marked as being finished: */
  if( !ReceiveRequestPt->finished ) {
    
#ifdef USE_USI
    if(ReceiveRequestPt->from==MPID_USOCK_Data_global.MyWorldRank)
    {
      LOG_TEST_REQUEST(1);
      DSECTLEAVE
	return FALSE;
    }
    DNOTICE("checking if ovelapped io is completed...");
    res=USI_Test(USI_DONT_BLOCK, &(ReceiveRequestPt->usi_request), MPID_USOCK_Data_global.usi_handle);
    DNOTICE("...done");


    switch(res)
    {
      case USI_SUCCESS:
	       ReceiveRequestPt->pending = FALSE;
	        DSECTLEAVE
		  return TRUE;
      case USI_PENDING:
	       if( mode == NONBLOCKING )
	       {
		 MPID_USOCK_DEBUG_PRINT_MSG2("ReceiveRequest not completed, waiting for %d bytes",ReceiveRequestPt->size - ReceiveRequestPt->received);
		 LOG_TEST_REQUEST(1);
		 DSECTLEAVE
		   return FALSE;
	       }
	       else
	       {
		 res=USI_Test(USI_BLOCK, &(ReceiveRequestPt->usi_request), MPID_USOCK_Data_global.usi_handle);
		 ReceiveRequestPt->pending = FALSE;
		 DSECTLEAVE
		   return TRUE;
	       }
      default:
	       MPID_USOCK_SysError("Test Request:WSARecv failed",error);
    }
	           
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
#endif
    
    /* if we're not done yet, but running in nonblocking mode, we return now: */
    if( !ReceiveRequestPt->finished && mode == NONBLOCKING )
    {
      MPID_USOCK_DEBUG_PRINT_MSG2("ReceiveRequest not completed, waiting for %d bytes",ReceiveRequestPt->size - ReceiveRequestPt->received);
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
    
#ifdef USE_USI

#else
    LOG_BLOCK(0);
    DNOTICE("getting result of overlapped operation...");
    res = WSAGetOverlappedResult(ReceiveRequestPt->fd, &(ReceiveRequestPt->Overlapped), &amountReceived, mode, &flags); /* wsock2function */
    DNOTICE("...done");
    
    /* in case of success, WSAGetOverlappedResult() returns TRUE: */
    LOG_BLOCK(1);
    if( !res )
    {
      /* some error occurred in WSAGetOverlappedResult(): */
      MPID_USOCK_SysError("TestRequest: WSARecv failed",WSAGetLastError());
    } 
    else
      if( amountReceived == 0 )
      {
	/*
         |   call to WSAGetOverlappedResult() succeeded, but we didn't receive anything
         |   during the issued receive operation
 	 */
      if( !ReceiveRequestPt->shuttingDown ) {
	MPID_USOCK_Error("TestRequest: Found dead network connection\n",0);
      } else {
	MPID_USOCK_DEBUG_PRINT_MSG("Read 0 bytes from network, connection seems to be closed");
	ReceiveRequestPt->pending = FALSE;
	MPID_USOCK_Finish_ReceiveRequest(ReceiveRequestPt);
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
	MPID_USOCK_Finish_ReceiveRequest(ReceiveRequestPt);
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
      MPID_USOCK_DEBUG_PRINT_MSG2("ReceiveRequest restarting recv, waiting for %d bytes", (ReceiveRequestPt->buf).len);
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
			MPID_USOCK_SysError("TestRequest: WSARecv failed",WSAGetLastError());
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
		   MPID_USOCK_DEBUG_PRINT_MSG("Got nobufs-error...");
		   if(count_nobufs<1000) break;
		   /* if this has happened too often, don't retry but raise an error */
	  default: MPID_USOCK_SysError("Test Request:WSARecv failed",error);
	} /* switch( error ) */
		} 
      else ReceiveRequestPt->pending = FALSE; /* no error in call to WSARecv() */
    } while (!res);
#endif
  }

  
  LOG_TEST_REQUEST(1);
  
  DSECTLEAVE
	    return ReceiveRequestPt->finished;
}


/* this method waits until a pending receive operation is finished */
static void MPID_USOCK_Wait_ReceiveRequest(ReceiveRequest* ReceiveRequestPt)
{
  DSECTION("MPID_USOCK_Wait_ReceiveRequest");
  BOOL Finished=MPID_USOCK_Test_ReceiveRequest(BLOCKING, ReceiveRequestPt);
  
  DSECTENTRYPOINT;
  
  while( !Finished )
    Finished =MPID_USOCK_Test_ReceiveRequest(BLOCKING, ReceiveRequestPt);
  
  DSECTLEAVE;
}   


static int MPID_USOCK_GetHeader_ReceiveRequest(ReceiveRequest* ReceiveRequestPt)
{
  int res;
  DSECTION("MPID_USOCK_GetHeader_ReceiveRequest");
  
  DSECTENTRYPOINT;
  
  ReceiveRequestPt->Header.Type = Quit;

  res=MPID_USOCK_Start_ReceiveRequest((char*)&(ReceiveRequestPt->Header), PRIVATESIZE, BLOCKING, ReceiveRequestPt);
  if(ReceiveRequestPt->State != CLOSED) 
    ReceiveRequestPt->State=HEADER_READ;
  
  DSECTLEAVE
   return res;
}


/*********************************************
 ** functions to handle 'TransferRequests': **
 *********************************************/

static void MPID_USOCK_Init_TransferRequest(void *b, int s, TransferRequest* TransferRequestPt)
{
  TransferRequestPt->buf = b;
  TransferRequestPt->size = s;
  TransferRequestPt->finished = 0;
  TransferRequestPt->Receive = 0;
}


BOOL MPID_USOCK_RecvTransferMessage(ReceiveRequest *Request)
{
  DSECTION("MPID_USOCK_RecvTransferMessage");
  STL_iterator_type it;
  unsigned int msglen;
  
  DSECTENTRYPOINT;
  
  MPID_THREAD_DS_LOCK(&tcp_mutex);
  
  it=STL_map_find_int2TraReq(Request->Header.Tag, &MPID_USOCK_Data_local.TransferRequestMap);

  if(it==STL_map_end(&MPID_USOCK_Data_local.TransferRequestMap)) {
	MPID_THREAD_DS_UNLOCK(&tcp_mutex);
	/* printf("Size %d, Tag %d\n",Request->Header.Size,Request->Header.Tag); */
	MPID_USOCK_Error("Protocol error: Receive_TransferRequest not found",-1);
    } else {
	MPID_USOCK_DEBUG_PRINT_MSG2("Transfer has requested %d bytes",(STL_map_second_int2TraReq(it, &MPID_USOCK_Data_local.TransferRequestMap))->size);
	MPID_USOCK_DEBUG_PRINT_MSG2("Buf is: %x", (STL_map_second_int2TraReq(it, &MPID_USOCK_Data_local.TransferRequestMap))->buf);
    }
  MPID_THREAD_DS_UNLOCK(&tcp_mutex);
  msglen=MPID_MIN((STL_map_second_int2TraReq(it, &MPID_USOCK_Data_local.TransferRequestMap))->size,Request->Header.Size);
  if(Request->Header.Size!=(STL_map_second_int2TraReq(it, &MPID_USOCK_Data_local.TransferRequestMap))->size) {
    fprintf(stderr,"Warning: Requested size(%d) and sent size(%d) are not matching!!\n",(STL_map_second_int2TraReq(it, &MPID_USOCK_Data_local.TransferRequestMap))->size,Request->Header.Size);
    fflush(stderr);
  }
  MPID_USOCK_DEBUG_PRINT_MSG2("Reading Transfer message with size %d",msglen);
  Request->Transfer = STL_map_second_int2TraReq(it, &MPID_USOCK_Data_local.TransferRequestMap);
  
  DSECTLEAVE
    return MPID_USOCK_Start_ReceiveRequest((char*)((STL_map_second_int2TraReq(it, &MPID_USOCK_Data_local.TransferRequestMap))->buf),msglen,NONBLOCKING, Request);
}


static TransferRequest *MPID_USOCK_Get_TransferRequest(void *buf, int size)
/*
 |   Returns a free TransferRequest handle.
 |
 |   To avoid malloc/free overhead the handles are stored by Free_TransferRequest() 
 |   in the SendRequestQueue after being used.
 |   So, Get_TransferRequest() can recycle them instead of recreate them
 |   in each call.
 */
{
  DSECTION("TransferRequest");

  TransferRequest *Request;
  
  DSECTENTRYPOINT;
  
  if(!STL_deque_size(&MPID_USOCK_Data_local.TransferRequestQueue))
  {
    Request=(TransferRequest*)malloc(sizeof(TransferRequest));
    MPID_USOCK_Init_TransferRequest(buf, size, Request);
  }
  else
  {
    Request=STL_deque_front_TraReq(&MPID_USOCK_Data_local.TransferRequestQueue);
    STL_deque_pop_front(&MPID_USOCK_Data_local.TransferRequestQueue);
    MPID_USOCK_Init_TransferRequest(buf, size, Request);
  }
  DNOTICEI("Returning request at ", Request);

  DSECTLEAVE
    return Request;
}

static void MPID_USOCK_Free_TransferRequest(TransferRequest *Request) {
  DSECTION("MPID_USOCK_Free_TransferRequest");
  DSECTENTRYPOINT;
  
  if(STL_deque_size(&MPID_USOCK_Data_local.TransferRequestQueue) > MAX_BUFFERED_REQUESTS) {
    free(Request);
  } else {
    STL_deque_push_back_TraReq(Request, &MPID_USOCK_Data_local.TransferRequestQueue);
  }
  DSECTLEAVE;
}


/***********************************
 ** further internal functions:   **
 ***********************************/ 

static void MPID_USOCK_Init_SelfSend(void *mem, const unsigned int s, SelfSend* SelfSend_Pt)
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

static void MPID_USOCK_ReadLocal( void *buf, int size) {
  DSECTION("MPID_USOCK_ReadLocal");
  SelfSend *msg;
  unsigned int s=size;
  
  DSECTENTRYPOINT;
  
  if(STL_deque_empty(&MPID_USOCK_Data_local.SelfSendChannel)) {
    /* should we do a loop here instead of an error? */
    DERROR("protocol error: Local channel is empty");
    MPID_USOCK_Error("protocol error: Local channel is empty\n",-1);
  }
  msg=STL_deque_front_SelfSend(&MPID_USOCK_Data_local.SelfSendChannel);
  s=MPID_MIN(size,msg->Header.Size); /* XXX SIGNED UNSIGNED CONFLICT !!! */
  MEMCPY(buf,((char*)msg->buf)+msg->offset,s);
  if(s==msg->Header.Size) {
    STL_deque_pop_front(&MPID_USOCK_Data_local.SelfSendChannel);	

    if( msg->buf ) FREE( msg->buf );
    msg->buf = 0;
    free(msg);

    if(s != size) {
      /* The user wants more? Give him more
       * This should not happen within a correct program. */
      MPID_USOCK_ReadLocal( ((char*)buf)+s,size-s);
    }
  } else {
    msg->Header.Size-=s;
    msg->offset += s;
  }
  
  DSECTLEAVE;
}


static void MPID_USOCK_GenericWait(OVERLAPPED *Over)
{
    DSECTION("MPID_USOCK_GenericWait");
    
    HANDLE Ev[2];
    int counter =0;
    BOOL Finished=FALSE;
#ifdef BLOCK
    Ev[0]=Over->hEvent;
#endif
    
    DSECTENTRYPOINT;
    
    while(!Finished)
    {
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
	Finished=HasOverlappedIoCompleted(Over);
    }
    DSECTLEAVE
      return;
}
