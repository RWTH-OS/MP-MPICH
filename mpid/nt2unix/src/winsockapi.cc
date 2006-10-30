#include "nt2unix.h"
#include <debugnt2u.h>
#include <deque>

#if !defined(linux)
#include <synch.h>
#endif


#include "mydebug.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#if !defined(linux)
#include <sys/systeminfo.h>
#include <stropts.h>
#include <poll.h>
#include <sys/mman.h>
#else 
#include <sys/ioctl.h>
#include <fstream>
#endif

#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <string>
#include <vector>
#include <map>

#ifndef __cplusplus
#error "nt2unix.cc: this file is C++."
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Switches and definitions: -----------------------------------------

#define _REPORT_NOBUFS_IMMEDIATELY
#undef  _OVERLAPPED_MESSAGE_BACKUP

// maximal size (in byte) of messages to be backuped
#ifdef MAX_MSG_BACKUP_SIZE
#undef MAX_MSG_BACKUP_SIZE
#endif
#define MAX_MSG_BACKUP_SIZE 256

// maximal number of overlapped messages to be stored in the queue
#ifdef MAX_OVERLAPPED_MESSAGES
#undef MAX_OVERLAPPED_MESSAGES
#endif
#define MAX_OVERLAPPED_MESSAGES 16

/*
#ifdef DBG
#undef DBG
#endif
#define DBG(x) {printf("[%s] ",  MessageStruct.hostname); printf(x); printf("\n"); fflush(stdout);}

#ifdef DNOTICE
#undef DNOTICE
#endif
#define DNOTICE(x) {printf("[%s] ",  MessageStruct.hostname); printf(x); printf("\n"); fflush(stdout);}

#ifdef DNOTICEI
#undef DNOTICEI
#endif
#define DNOTICEI(x,i) {printf("[%s] ", MessageStruct.hostname); printf(x); printf(": %d\n",i); fflush(stdout);}
*/

// internal functions:
long int send_all(int s, const void *msg, size_t len, int flags, BOOL OverlappedFlag, void (*)(int));
long int recv_all(int s, const void *msg, size_t len, int flags);
long BuffSender (void*);
void BuffSenderWouldBlock(int);
void SenderWouldBlock(int);


// WinSock API -------------------------------------------------------

// use a STL queue for storing overlapped messages:
typedef std::deque<MessageSocket_t*> MessageQueue_t;

// cover all global variables of this module in a struct:
struct _Message_Struct
{
  _Message_Struct()
  {
    LastError     = 0;
    LastOverlapped = FALSE;
    ThreadExists  = FALSE;
    WouldBlock    = FALSE;
    InitializeCriticalSection (&QueueMutex);
    InitializeCriticalSection (&OverlappedMutex);
    gethostname(hostname, 16);
  }

  volatile int       LastError;
  volatile BOOL      ThreadExists;  
  volatile BOOL      LastOverlapped;
  volatile BOOL      WouldBlock;
  volatile int       MessagesStored;
  HANDLE             QueueSemaphore, OverlappedSemaphore;
  CRITICAL_SECTION   QueueMutex,     OverlappedMutex;  
  MessageQueue_t     MessageQueue;
  HANDLE             BuffHandle;
  char               hostname[16];
} ;

typedef _Message_Struct Message_Struct;
Message_Struct MessageStruct;


int PASCAL FAR WSACleanup(void)
{
 /* if (!CloseHandle (MessageStruct.BuffHandle))
  {	
    perror ("CloseHandle(MessageStruct.BuffHandle) failed");
    return 1;
  }
  else*/
    return 0;
}

int PASCAL FAR ioctlsocket (SOCKET s, long cmd, u_long FAR *argp)
{
  // This is untested. 
    
  if (ioctl(s, cmd, argp) == -1)
    return SOCKET_ERROR; 
    
  return 0; 
}

BOOL HasOverlappedIoCompleted (LPOVERLAPPED lpOverlapped)
{
  return lpOverlapped->Completed;
}

BOOL WSAGetOverlappedResult (SOCKET s,
			     LPWSAOVERLAPPED lpOverlapped,
			     LPDWORD lpcbTransfer,
			     BOOL fWait,
			     LPDWORD lpdwFlags)
{
  DBG("Entered GetOverlappedResult()");
  
  if (!lpOverlapped)
    return FALSE;
  
  if (fWait)
  {
    if (!lpOverlapped->hEvent)
      return FALSE;
    WaitForSingleObject (lpOverlapped->hEvent, INFINITE);
  }

  *(lpcbTransfer) = lpOverlapped->BytesTransferred;

  DNOTICEI("Leaving GetOverlappedResult", lpOverlapped->Completed);

  if (!lpOverlapped->Completed)
  {
    MessageStruct.LastError = WSA_IO_INCOMPLETE;  
    return FALSE;
  }
  else
    return TRUE;
}


int PASCAL FAR WSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData)
{
  
  if(!lpWSAData)
    return WSAEFAULT;

  lpWSAData->wVersion =
    lpWSAData->wHighVersion = 0x0002;

  strcpy(lpWSAData->szDescription, "This is WinFake 0.1\n");
  strcpy(lpWSAData->szSystemStatus, "[still buggy]");

  return 0;
}
  

int PASCAL FAR WSASocket (int af,
			  int type,
			  int protocol,
			  LPWSAPROTOCOL_INFO lpProtocolInfo,
			  GROUP g,
			  DWORD dwFlags)
{
  SOCKET S;
  const char Error = 0;


  S = socket (af, type, protocol);
  if (S < 0)
    {
      perror (&Error);
      return SOCKET_ERROR;
    }
 
  return S;
}


WSAEVENT WSACreateEvent(void)
{
  return CreateEvent (0, 1, 0, 0);
}


BOOL WSACloseEvent(WSAEVENT hEvent)
{
  return CloseHandle((HANDLE) hEvent);
}

  
// needed for CreateThread to accept BuffSender:
typedef DWORD (*helpfcn)(void*);
 
int PASCAL FAR _CREATETHREAD(void)
{
  DWORD ThreadId;
       
  MessageStruct.QueueSemaphore  = CreateSemaphore (0, 0, 1000, 0);
  MessageStruct.OverlappedSemaphore = CreateSemaphore (0, 0, 1, 0);

  if (!MessageStruct.QueueSemaphore)
  {
    DBG ("CreateSemaphore FAILED");
    return 1;
  }
      
  MessageStruct.BuffHandle = CreateThread(0, 0, (helpfcn) BuffSender, 0, 0, &ThreadId);
  if (!MessageStruct.BuffHandle)
  {
    DBG("CreateThread for BuffSender() FAILED !!!");
    return 1;
  }
  else
    DBG ("Thread BuffSender() is created ...");
  
  return 0;
}


int PASCAL FAR WSASend (SOCKET s,
		        LPWSABUF lpBuffers,
			DWORD dwBufferCount,
			LPDWORD lpNumberOfBytesSent,
			DWORD dwFlags,
			LPWSAOVERLAPPED lpOverlapped,
			LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
  DSECTION("WSASend");

  int ret  = 0;
  long int length = 0;
  
  DSECTENTRYPOINT;
  
  *lpNumberOfBytesSent = 0;  //  initialize bytecounter
  
  if (dwFlags == MSG_DONTROUTE)
    dwFlags = MSG_EOR;
  
  if (dwFlags == MSG_PARTIAL)
  {
    DBG("WSASend : MSG_PARTIAL Flag not supported !\n");
    DSECTLEAVE
      return WSAEOPNOTSUPP;
  }

  // determine the size of the whole message: (multiple buffers)
  for (unsigned int i = 0; i < dwBufferCount; i++) length+=lpBuffers[i].len;
      

  if (lpOverlapped) // branch for overlapped messages
  {
    /*
     |   This is an overlapped WSASend:
     |   The message parameters are stored in a FIFO queue and the send call
     |   is handled afterwards by thread in the function BuffSender() 
     */

    MessageSocket_t* MessageInfo;
    
    DNOTICEI("Overlapped Send / Size", length);

    if(!MessageStruct.ThreadExists)
    {
      /*   If this is the first call for an overlapped send we must
       |   initialize the thread and some mutexes for its handling:
       */  
      if (_CREATETHREAD())
      {
        DBG("CreateThread() failed !!!");
        DSECTLEAVE
	  return WSAEFAULT;
      }
      MessageStruct.ThreadExists   = TRUE;
      MessageStruct.MessagesStored = 0;
    }
 
    if(MessageStruct.MessagesStored < MAX_OVERLAPPED_MESSAGES) // <- This is to avoid the possibility of a memory overflow
    {   
      // store the information of this send call:
      MessageInfo = new MessageSocket_t;
      MessageInfo->BufferCount = dwBufferCount;
      MessageInfo->Socket = s;
      MessageInfo->NumberOfBytesSent = *(lpNumberOfBytesSent);
      MessageInfo->Flags = dwFlags;
      MessageInfo->lpOverlapped = lpOverlapped; 

      MessageInfo->Message = new WSABUF[dwBufferCount];
      for (unsigned int i=0; i<dwBufferCount; i++)
      { 
         MessageInfo->Message[i].len=lpBuffers[i].len;       
         MessageInfo->Message[i].buf=lpBuffers[i].buf;

#ifdef _OVERLAPPED_MESSAGE_BACKUP

       /*   The buffer has to remain valid for WSAsend during the overlapped communication.
        |   To avoid validity problems you may want to copy the message in a backup buffer.
        |   -> If desired, define the _OVERLAPPED_MESSAGE_BACKUP macro!
        |      Messages with a length lesser than MAX_MSG_BACKUP_SIZE will then be copied.
        */

        if(MessageInfo->Message[i].len<MAX_MSG_BACKUP_SIZE)
        {
           MessageInfo->Message[i].buf = new char[MessageInfo->Message[i].len];
           memcpy (MessageInfo->Message[i].buf, lpBuffers[i].buf, MessageInfo->Message[i].len * sizeof(char));
        }
#endif
      }
  
      // set the return values:
      lpOverlapped->Completed = FALSE;
      lpOverlapped->BytesTransferred = 0;
 
      // store the message information in the queue:
      EnterCriticalSection (&MessageStruct.QueueMutex);  // <- mutex for working on the queue
      {
        DBG("Process got queue mutex ...");
        if (lpOverlapped->hEvent)
        {
          ResetEvent (lpOverlapped->hEvent); 
        }
        MessageStruct.MessageQueue.push_back (MessageInfo);  // <- append message at the end of the queue
        MessageStruct.MessagesStored++;
        ReleaseSemaphore (MessageStruct.QueueSemaphore, 1, 0);
        DBG("Process has stored the message in the queue.");
      }
      LeaveCriticalSection (&MessageStruct.QueueMutex);

      DBG("Process left the critical section."); 
 
      MessageStruct.LastOverlapped=TRUE;  /* <- Remember, that the this (and so the last) call to WSASend() was overlapped
                                           |    This is necessary to be able to check in a further blocking send, if the 
                                           |    process has to wait until the sender thread has entered its critical section.
                                           */

      MessageStruct.LastError = WSA_IO_PENDING;

      DBG("Process is leaving the WSASend() function and IO is pending...");

      DSECTLEAVE
        return SOCKET_ERROR;  /* <- No real error! (check WSAGetLastError() for WSA_IO_PENDING) 
                               |    It indicates that the operation has been initiated and that 
                               |    completion will be indicated at a later time.
                               */
    }
    else
    {
      /*
       |   There are too many outstanding overlapped sends.
       |   So, we report a WOULDBLOCK error.
       |   You may want to increase MAX_OVERLAPPED_MESSAGES ...
       */

       DBG("Too many overlapped IOs are pending -> return WSAEWOULDBLOCK");

       MessageStruct.LastError = WSAEWOULDBLOCK;

       DSECTLEAVE
         return SOCKET_ERROR;
    }
  }
  else 
  {
    /*
     |   This is a non-overlapped WSASend:
     |   A message is directly send by a call to the send_all() function, if there
     |   are no pending overlapped-messages in the queue.
     |   To avoid blocking in case of stucked overlapped-messages, the 'WouldBlock'
     |   flag is checked and WSAENOBUFS is returned via WSAGetLastError() if necessary.
     */

    DNOTICEI("Non Overlapped Send / Size", length);

    if(MessageStruct.WouldBlock)
    { 
      /*
       |   A pending overlapped send has detected a lack of socket buffers:
       |   We report a WSANOBUFS-Error and do not try to enter the critical section.
       */
 
        DBG("Process would block -> return WSAENOBUFS");
     
        MessageStruct.LastError = WSAENOBUFS;
        return SOCKET_ERROR;
    }

    if(MessageStruct.LastOverlapped==TRUE)
    {
       /*   Wait until the thread has entered its critical section:
        |   This is nessesary to avoid that a further (and very quick) blocking send can
        |   reach the section before the thread does -- and so this is only necessary, if
        |   the last call to WSASend() was an overlapped send.
        */

       DBG("Process is waiting for the overlapped semaphore...");

       WaitForSingleObject(MessageStruct.OverlappedSemaphore, INFINITE); 

       DBG("Process got the overlapped semaphore !!!");
    }

    DBG("Process is waiting for the overlapped semaphore...");

    EnterCriticalSection (&MessageStruct.OverlappedMutex); /* <- A blocking send must wait here until all former overlapped sends
                                                            |    are finished or until blocking situation is detected by the thread
                                                            |    (we must leave this function if a call to send_all() would block)
                                                            */
    {    
      DBG("Process got the overlapped mutex ...");

      /*    
       |   Now check, if a call to send_all() may block:
       |   (This is because a stuck situation may be occured while waiting for the overlapped mutex)
       */

      if((MessageStruct.WouldBlock==FALSE)&&(MessageStruct.MessagesStored==0)) // <- be sure, that the queue really is empty
      {
        for (unsigned int i = 0; i < dwBufferCount; i++) // <- send multiple buffers
        {
          DNOTICEI("Process is calling the send_all() function", lpBuffers[i].len);
          *lpNumberOfBytesSent += send_all (s, lpBuffers[i].buf, lpBuffers[i].len, dwFlags, FALSE, SenderWouldBlock); 
        }
        DBG("Process has sent all buffers ...")
        MessageStruct.LastOverlapped=FALSE;
      }
      else
      {
        // a pending overlapped send has detected a lack of socket buffers:
        DBG("Process would block -> return WSAENOBUFS");
     
        MessageStruct.LastError = WSAENOBUFS;
        ret = SOCKET_ERROR;
      }
    }
    LeaveCriticalSection (&MessageStruct.OverlappedMutex);   
    
    DBG("Process is leaving the WSASend() function ..."); 

    DSECTLEAVE
      return ret;
  }
}

int WSARecv (SOCKET s,
             LPWSABUF lpBuffers,
	     DWORD dwBufferCount,
	     LPDWORD lpNumberOfBytesRecvd,
	     LPDWORD lpFlags,
	     LPWSAOVERLAPPED lpOverlapped,
	     LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionROUTINE)
{
  DSECTION("WSARecv");
 
  long int Received;
  long int length = 0;

  DSECTENTRYPOINT;
  // *lpFlags = *lpFlags || MSG_WAITALL; /* This is an errornous parameter to recv() */
  *lpNumberOfBytesRecvd = 0; 

  // determine the size of the whole message: (multiple buffers)
  for (unsigned int i = 0; i < dwBufferCount; i++) length+=lpBuffers[i].len; // <- this is just for debug purpose ...
  DNOTICEI("WSARecv() / Size", length);

  for (unsigned int i = 0; i < dwBufferCount; i++) // <- receive multiple buffers
  { 
    DNOTICEI("Process is calling the recv_all() function",lpBuffers[i].len);  
    Received = recv_all (s, lpBuffers[i].buf, lpBuffers[i].len, *lpFlags);
    DNOTICEI("Received", Received);

    if(Received<0)
    {
      MessageStruct.LastError = WSAEWOULDBLOCK;

#if 1
      switch(errno)
      {
	case EAGAIN:
	{
	  fprintf(stderr, "WSARecv-ERROR: EAGAIN The socket is marked non-blocking and the receive operation would block, or a receive timeout had been set and the timeout expired before data was received.\n"); 
	  fflush(stderr);
	  break;
	}
	case EBADF:
	{
	  fprintf(stderr, "WSARecv-ERROR: EBADF The argument s is an invalid descriptor.\n");
	  fflush(stderr);
	  break;
	}
	case ECONNREFUSED:
	{
	  fprintf(stderr, "WSARecv-ERROR: ECONNREFUSEDA remote host refused to allow the network connection (typically because it is not running the requested service).\n");
	  fflush(stderr);
	  break;
	}
	case EFAULT:
	{
	  fprintf(stderr, "WSARecv-ERROR: EFAULT The receive buffer pointer(s) point outside the process's address space.\n");
	  fflush(stderr);
	  break;
	}
	case EINTR:
	{
	  fprintf(stderr, "WSARecv-ERROR: EINTR The receive was interrupted by delivery of a signal before any data were available.\n");
	  fflush(stderr);
	  break;
	}
	case EINVAL:
	{
	  fprintf(stderr, "WSARecv-ERROR: EINVAL Invalid argument passed.\n");
	  fflush(stderr);
	  break;
	}
	case ENOMEM:
	{
	  fprintf(stderr, "WSARecv-ERROR: ENOMEM Could not allocate memory for recvmsg.\n");
	  fflush(stderr);
	  break;
	}
	case ENOTCONN:
	{
	  fprintf(stderr, "WSARecv-ERROR: ENOTCONN The socket is associated with a connection-oriented protocol and has not been connected.\n");
	  fflush(stderr);
	  break;
	}
	case ENOTSOCK:
	{
	  fprintf(stderr, "WSARecv-ERROR: ENOTSOCK The argument s does not refer to a socket.\n");
	  fflush(stderr);
	  break;
	}
	default:
	{
	  char *errstr;
	  fprintf(stderr, "WSARecv-ERROR: Unknown socket error: %d\n", errno);
	  errstr=strerror(errno);
	  fprintf(stderr, "WSARecv-ERROR: %s\n", errstr);
	  fflush(stderr);
	  break;
	}
      }
      exit(-1);
#endif

      DSECTLEAVE;
        return SOCKET_ERROR;
    }

    *lpNumberOfBytesRecvd += Received;  
  }
  DBG("WSARecv has received all buffers ...") 

  DSECTLEAVE;
    return 0;
}


int PASCAL FAR WSAGetLastError(void)
{
   return(MessageStruct.LastError);
}

void PASCAL FAR WSASetLastError(int iError)
{
   MessageStruct.LastError = iError;
}


// Internal functions: -------------------------------------------------------

#undef _DEBUG_CONTENT_SEND  //  define this to print the content of each message sent
#undef _DEBUG_CONTENT_RECV  //  define this to print the content of each message received

/* 
 |  Unix send/recv does not guarantee to perform the whole transfer in one step.
 |  Thus this functions are used to retry until all data is transfered.
 */

long int send_all(int s, const void *msg, size_t len, int flags, BOOL OverlappedFlag, void (*WouldBlock)(int))
{
  ssize_t current = 0;
  ssize_t actual  = 0;

  /*
   |   To be able to report a WSAENOBUFS to an application we use the non-blocking
   |   send-operation (MSG_DONTWAIT) and check for return values lesser than zero.
   |
   |   Now there are two alternatives:
   |   - We can report this immediately by calling the (*WouldBlock)-function
   |     (to enable this behavior define _REPORT_NOBUFS_IMMEDIATELY).
   |   - Or we set a timer and try again. 
   |     If the timer function still detects a deadlock (the WouldBlock flag is still set),
   |     this state is reported via the (*WouldBlock)-function to further WSASend() calls.
   */

#ifndef __REPORT_NOBUFS_IMMEDIATELY
  BOOL   TimerFlag = FALSE;
  struct itimerval timeout;
  struct sigaction sALRM;

  sALRM.sa_handler = WouldBlock;
  sALRM.sa_flags   = 0;
  sigaction( SIGALRM, &sALRM, NULL);
#endif

  //  try as long as the whole message is sent:
  while (current < len)
  {
    // this is the non-blocking UNIX send() call:
    actual = send(s, (char*)msg + current, len - current, flags | MSG_DONTWAIT);

#ifdef _DEBUG_CONTENT_SEND
    int k;
    unsigned char *c;

    if(OverlappedFlag) printf("OVER: %d / Actual  %d / Current %d >>> ", len, actual, current);
    else printf("SEND: %d / Actual  %d / Current %d >>> ", len, actual, current); 
    fflush(stdout);
    c=(unsigned char*)msg;
    if(actual<=64) for(k=0; k<actual; k++) printf("[%d]",c[k]);
    printf("\n"); fflush(stdout);
#endif
    
    if((int)actual < 0) // currently no part of the message could be sent
    {
       if(WouldBlock!=NULL)
       {
         MessageStruct.WouldBlock = TRUE;

#ifndef _REPORT_NOBUFS_IMMEDIATELY
         if(!TimerFlag)
         {
           // set a timer to check for a deadlock:
           timeout.it_value.tv_sec = 0;
           timeout.it_value.tv_usec = 100;  //  probably lesser than the system's timer resolution ...
           timeout.it_interval.tv_sec = 0;
           timeout.it_interval.tv_usec = 0;
           setitimer( ITIMER_REAL, &timeout, NULL);
           TimerFlag = TRUE;
         }
#else 
         (*WouldBlock)(current);
#endif
       }
    } 
    else
    { 
      MessageStruct.WouldBlock = FALSE;

#ifndef _REPORT_NOBUFS_IMMEDIATELY
      if(TimerFlag)
      {
        TimerFlag = FALSE;
        setitimer( ITIMER_REAL, NULL, NULL);
      }
#endif

      current += actual;
    }
  }

  return ((long int)current);
}


long BuffSender (void*)
{
  /* 
   |  This is the function for the overlapped sending thread:
   |  The thread waits for a message to send and calls 'send_all'
   |  for all buffers of this message:
   */

  MessageSocket_t* MessageInfo;
  long int Sent = 0;

  DBG("Thread is waiting for overlapped mutex ...");

  // wait until the queue is not empty:
  while (WaitForSingleObject (MessageStruct.QueueSemaphore, INFINITE) == WAIT_OBJECT_0)
  {
    EnterCriticalSection (&MessageStruct.OverlappedMutex);
    {
      DBG("Thread got the overlapped mutex !!!");     

     // this thread is working, so the main thread can leave WSASend():
      ReleaseSemaphore (MessageStruct.OverlappedSemaphore, 1, 0);

      // let the thread work until there is no overlapped msg left in the queue:
      do 
      {         
        DBG("Thread works on the queue ...");
	MessageInfo = MessageStruct.MessageQueue.front();
	
	// perform a send_all() for all available buffers of this message: 
	for (unsigned int i = 0; i < MessageInfo->BufferCount; i++)
	{ 
	  DBG ("Thread is calling the send_all() function ...");
  
	  Sent=send_all (MessageInfo->Socket, (MessageInfo->Message[i]).buf, (MessageInfo->Message[i]).len, MessageInfo->Flags, TRUE, BuffSenderWouldBlock); 
	  
          // update the current state of this overlapped send:
	  MessageInfo->NumberOfBytesSent += Sent;
	  MessageInfo->lpOverlapped->BytesTransferred = MessageInfo->NumberOfBytesSent;

          // indicate that there is a proper flow:
	  MessageStruct.WouldBlock = FALSE;        

          DBG ("Thread has successfully sent the overlapped message.");         
	}
	
	DBG("Thread is waiting for queue mutex ...");
       
	// remove the finished send call from the queue:
	EnterCriticalSection (&MessageStruct.QueueMutex);
	{ 
          DBG("Thread got queue mutex !!!");

          // set event for overlapped finished:
          if (MessageInfo->lpOverlapped->hEvent)
	  {
	    SetEvent (MessageInfo->lpOverlapped->hEvent);
            DBG("Thread has set the overlapped event.");
          }
      
          MessageInfo->lpOverlapped->Completed = TRUE;
	  MessageStruct.MessageQueue.pop_front();
          MessageStruct.MessagesStored--;
 
          //  free the queue object:
#ifdef _OVERLAPPED_MESSAGE_BACKUP 
          for (unsigned int i = 0; i < MessageInfo->BufferCount; i++)
	  {
             if(MessageInfo->Message[i].len<MAX_MSG_BACKUP_SIZE) delete (MessageInfo->Message[i].buf);
          }
#endif
          delete (MessageInfo->Message);
          delete (MessageInfo);
          DBG("Thread has deleted the queued message.");
	}
	LeaveCriticalSection (&MessageStruct.QueueMutex);

	DNOTICEI("Thread is testing for further messages ...", MessageStruct.MessagesStored);

      }while (WaitForSingleObject (MessageStruct.QueueSemaphore, 0) == WAIT_OBJECT_0);   /* <- Check, if there are further msg in the queue.
                                                                                          |    This is a immediate call of WaitForSingleObject()
                                                                                          |    (dwTimeout=0), so we'll leave the critical section
                                                                                          |    if the queue is empty:
                                                                                          */     
    }
    LeaveCriticalSection (&MessageStruct.OverlappedMutex);  //  now a possibly waiting blocking-send can be handled ...
  }

  DBG("Thread finished !!!");

  MessageStruct.ThreadExists = FALSE;

  return 0;
}


void BuffSenderWouldBlock(int current)
{ 
  /*
   |   This function is called by the BuffSender thread in case of a stuck situation:
   |   The 'WouldBlock' flag is set and the critical section is left for a short time
   |   to allow a waiting not-overlapped send to return a WSAENOBUFS indication.
   */

  if(MessageStruct.WouldBlock == TRUE)
  {
    LeaveCriticalSection (&MessageStruct.OverlappedMutex);

    DBG("BuffSender() stucks ...");

    EnterCriticalSection (&MessageStruct.OverlappedMutex);

    ReleaseSemaphore (MessageStruct.OverlappedSemaphore, 1, 0);
  }
}


// just for debug purpose:
void SenderWouldBlock(int current)
{
  DNOTICEI("Non-overlapped Send stucks", current);
}


long int recv_all(int s, const void *msg, size_t len, int flags)
{
  ssize_t current = 0;
  ssize_t actual  = 0;
  int    count   = 0;

  while (current < len)
  {
    actual = recv(s, (char*)msg + current, len - current, flags);

#ifdef _DEBUG_CONTENT_RECV
    int k;
    unsigned char *c;

    printf("RECV: %d / Actual  %d / Current %d >>> ", len, actual, current); fflush(stdout);
    c=(unsigned char*)msg;
    if(actual<=64) for(k=0; k<actual; k++) printf("[%d]",c[k]);
    printf("\n"); fflush(stdout); 
#endif

    if ((int)actual < 0) return (long int)actual;
    current += actual;

    //  This is a simple watchdog feature: (you may want to omit this ...)
#if 0
    {
      if ((int)actual == 0)
      {
        count++;
        if(count > 10)
        {
          MessageStruct.LastError = WSAEWOULDBLOCK;

          exit(-16);  //  Hey, just quit it !!!                   
       
          return -1;
        }
      }
    }
#endif
   // -------------------------------------------------------------------

  }
  return ((long int)current);
}

/*
int
WSAEventSelect (SOCKET s, WSAEVENT hEventObject, long lNetworkEvent)
{
}
*/


#ifdef __cplusplus 
}
#endif
