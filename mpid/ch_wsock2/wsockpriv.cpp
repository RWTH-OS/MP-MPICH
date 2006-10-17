/*
 * $Id$
 *
 */

/*#define _WIN32_WINNT 0x0403*/
#define EXT "KSLF1255"

#define NOTHREAD

#pragma warning(disable : 4786) 



#if (_MSC_VER < 1100)
#define NOMINMAX 
  #include <winsock2.h>

  #include <windows.h>
  #define HasOverlappedIoCompleted(lpOverlapped) ((lpOverlapped)->Internal != STATUS_PENDING)
  #include <new.h>
  #include <iostream>
  namespace std{
  #include <function>
  #include <deque>
  #include <map>
  } 
  #ifndef min
  #define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#else
  /*#include <wtypes.h>
  #include <winbase.h>*/
  #include <winsock2.h>
  

  #include <deque>
  #include <map>
#endif

#include <iostream>
#include "TCPCommunicator.h"
#include "LogMpid.h"

extern "C" {
    // Avoid inclusion of "dev.h"
    #define MPID_DEV_H
    
    #include "mpid.h"
    #include "mpiddev.h"
    #include "packets.h"
    #include "mpid_debug.h"
    #include "mpimem.h"
    
    #define TRANSFERRING	0
    #define HEADER_READ	1
    #define CLEAN		2
    #define CLOSED		3
    
    #define BLOCKING		1
    #define NONBLOCKING		0
    #define MAX_LOOP 80
    #define BLOCK
    #define THREAD_SIZE 16384
        
    void wsock_error(const char *string,int value);
    HANDLE *MPID_Events;
    extern int mixed;
  }

#ifdef _MSC_VER
  #pragma comment(exestr,__DATE__ ": " DEV " " EXT)
#endif

struct CReceiveRequest;

struct TransferRequest {
	TransferRequest() {Init(0,0);}
	TransferRequest(void *b,int s) {Init(b,s);}
	void Init(void *b, int s) {buf=b;size=s;finished=0;Receive=0;}
	void *buf;
	unsigned int size;
	int finished;
	CReceiveRequest* Receive;
};

struct SendRequest {
	SendRequest() {
		DBG("Entering SendRequest()");
		Overlapped.hEvent=CreateEvent(0,FALSE,FALSE,0); 
		finished=TRUE;
		if(Overlapped.hEvent == WSA_INVALID_EVENT) 
			wsock_error("Creation of event failed",WSAGetLastError());
	}

	virtual ~SendRequest() {DBG("Entering ~SendRequest()");WSACloseEvent(Overlapped.hEvent);} //wsock2function
	WSAOVERLAPPED Overlapped;
	SOCKET fd;
	unsigned long size;
	WSABUF *buf;
	DWORD NumBlocks;
	struct WsockHeader h;
#ifndef NOTHREAD
	volatile BOOL finished;
	DWORD Error;
	BOOL Queued;
	unsigned long dest;
#else
	BOOL finished;
#endif
};



struct TCP_SELF_MESSAGE;

typedef std::map<int,TransferRequest*,std::less<int> > CTransferMap;
typedef std::deque<TCP_SELF_MESSAGE*> CSelfQueue;
typedef std::deque<SendRequest*> CRequestQueue;
typedef std::deque<TransferRequest*> CTransferQueue;


static CTransferMap ReceiveTransfers;
static CSelfQueue LocalChannel;
static CTransferQueue Transfers;
static CRequestQueue RequestQueue;

#ifndef NOTHREAD
static CRequestQueue SendQueue;
static HANDLE SendSem,hThread;
static CRITICAL_SECTION CS;
static volatile BOOL run;
static volatile unsigned long *ChannelForThread;

#endif

#ifdef __cplusplus
  extern "C" {
#endif
    static void MPID_WSOCK_FreeRequest(SendRequest*);
    static SendRequest *MPID_WSOCK_GetRequest(void);
    static void MPID_WSOCK_ReadLocal( void*, int);
    static TransferRequest *MPID_WSOCK_GetTransfer(void*,int);
    static void MPID_WSOCK_FreeTransfer(TransferRequest *);
    static int MPID_WSOCK_SendR(WSABUF *, int , unsigned long ,int ,SendRequest *);
#ifndef NOTHREAD
    DWORD WINAPI ThreadFunc(void* dummy);
    int MPID_WSOCK_Send_thread(WSABUF *, int, unsigned long,int,SendRequest*);
#endif


void GenericWait(OVERLAPPED *Over) {
	HANDLE Ev[2];
	int counter =0;
	BOOL Finished=FALSE;
#ifdef BLOCK
		Ev[0]=Over->hEvent;
		Ev[1]=MPID_Events[MPID_MyWorldRank];
#endif
	while(!Finished) {
#ifdef BLOCK
		if(MPID_DeviceCheck(MPID_NOTBLOCKING) <0) {
			counter++;
			if(counter>MAX_LOOP) {
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
}

struct CReceiveRequest {
	CReceiveRequest(SOCKET s) {
		DBG("Entering CReceiveRequest(SOCKET s) ");
		fd=s;Overlapped.hEvent=CreateEvent(0,TRUE,FALSE,0); 
		finished=FALSE;
		Transfer=0;
		State=CLEAN;
		memset(&Header,0xFF,sizeof(Header));
		shuttingDown = false;
	}
	~CReceiveRequest() {
		DBG("Entering ~CReceiveRequest()");
		WSACloseEvent(Overlapped.hEvent);}
	
	int GetHeader() { 
		Header.Type = Quit;
		
		int res=StartRequest((char*)&Header,PRIVATESIZE,BLOCKING);		
		if(State != CLOSED) 
		    State=HEADER_READ;
		return res;
	}

	int StartRequest(char *mem,unsigned long amount,int mode=NONBLOCKING) {
	    unsigned long flags=0,dummy=0,count=0;
	    int error,res,retry;
	    //WSANETWORKEVENTS NetEvents;
	    
	    LOG_START_REQUEST(0);
	    
	    DEBUG_PRINT_MSG2("Starting ReceiveRequest size %d ",amount);
	    
	    if(State == TRANSFERRING) wsock_error("Request already in progress.",-1);
	    if(Transfer) Transfer->Receive=this;
	    if(amount <=0) finish();
	    else {
		buf.len=amount; 
		buf.buf=mem; 
		if(fd != INVALID_SOCKET) {
		    // Do this to clear the buffer on the socket.
		    //WSAEnumNetworkEvents(fd,0,&NetEvents);
		    do {
			retry = 0;
			res=WSARecv(fd,&buf,1,&dummy,&flags,&Overlapped,0); //wsock2function
			if(res == SOCKET_ERROR) {			    
			    error=WSAGetLastError();
			    switch(error) {
			    case WSA_IO_PENDING: 
				pending = true;
				DEBUG_PRINT_MSG2("ReceiveRequest of size %d is pending...",amount);
				break;
			    case WSAEWOULDBLOCK:
				fprintf(stderr,"Warning: StartRequest: Got wouldblock error...\n"); fflush(stderr);
				retry=1; Sleep(10);				
				break;
			    case WSAENOBUFS: 
				fprintf(stderr,"Warning: StartRequest: Got nobufs error...\n"); fflush(stderr);
				retry=1; count++; Sleep(10);				
				if(count<1000) break;
			    default:
				fprintf(stderr,"\nmem==%p\namount==%d\nstate==%d, read==%d\n",mem,amount,State,dummy);
				wsock_syserror("Start Request:WSARecv failed",error);
			    } 
			} else {
			    if(dummy == 0)  {
				if(!shuttingDown) {
				    DEBUG_PRINT_MSG("Read 0 bytes from network, connection seems to be closed");
				    wsock_error("StartRequest: Found dead network connection\n",0);
				} else {
				    DEBUG_PRINT_MSG("Read 0 bytes from network, connection seems to be closed");
				    pending =false;
				    finish();
				    State = CLOSED;
				    LOG_START_REQUEST(1);
				    return 1;
				}
			    }
			    pending = false;
			    DEBUG_PRINT_MSG2("ReceiveRequest size %d received",dummy);
			}
		    } while(retry);

		    if(dummy==amount) {
#ifdef MPID_DEBUG_ALL
			size=amount;
#endif
			finish();
			
			LOG_START_REQUEST(1);
			
			return 1;
		    } else {
			State=TRANSFERRING;
			received=0;
			finished = FALSE;
			size=amount;
			if(mode == BLOCKING) WaitRequest();//TestRequest(mode);
			else if(!res) TestRequest(mode);
		    }
		} else {
		    if(LocalChannel.size()) {
			MPID_WSOCK_ReadLocal(mem,amount);
			finish();	
		    } else ResetEvent(Overlapped.hEvent);
		}
	    }
	    
	    LOG_START_REQUEST(1);
	    
	    return finished;
	}
	
	int TestRequest(int mode = NONBLOCKING) {
	    unsigned long RSize=0;
	    unsigned long flags,counter =0;
	    WSAOVERLAPPED *Over;
	    int error;
	    int res;
	    LOG_TEST_REQUEST(0);
	    if(!finished) {
		if(fd == INVALID_SOCKET) {
		    
		    LOG_TEST_REQUEST(1);
		    
		    return FALSE;
		}
		finished=HasOverlappedIoCompleted(&Overlapped);
		if(!finished && mode == NONBLOCKING) {
		    DEBUG_PRINT_MSG2("ReceiveRequest not completed, waiting for %d bytes",size-received);
		    
		    LOG_TEST_REQUEST(1);
		    
		    return 0;
		}
		LOG_BLOCK(0);
		res=WSAGetOverlappedResult(fd,&Overlapped,&RSize,mode,&flags); //wsock2function
		LOG_BLOCK(1);
		if(!res) {
		    wsock_syserror("TestRequest: WSARecv failed",WSAGetLastError());
		} else if(RSize == 0 ) {
		    if(!shuttingDown) {
		    //fprintf(stderr,"Read 0 bytes from network, connection seems to be closed\n");
			wsock_error("TestRequest: Found dead network connection\n",0);
		    } else {
			DEBUG_PRINT_MSG("Read 0 bytes from network, connection seems to be closed");
			pending = false;
			finish();
			State = CLOSED;
			LOG_TEST_REQUEST(1);
			return 1;
		    }
		}
		/*if(mode == NONBLOCKING)*/ Over=&Overlapped;
		/*else Over=0;*/
		do {
		    received += RSize;
		    if(received >= size) {
			finish();
			
			LOG_TEST_REQUEST(1);
			
			return 1;
		    }
		    finished = FALSE;
		    buf.buf += RSize;
		    buf.len -= RSize;
		    //printf("[%lf] Received %d bytes, waiting for %d bytes\n",MPI_Wtime(),RSize,buf.len);
		    DEBUG_PRINT_MSG2("ReceiveRequest restarting recv, waiting for %d bytes",buf.len);
		    //Sleep(2);
		    flags=0;
		    res=WSARecv(fd,&buf,1,&RSize,&flags,Over,0); //wsock2function
		    if(res == SOCKET_ERROR) {
			pending = true;
			error=WSAGetLastError();
			switch(error) {
			case WSA_IO_PENDING: 
			    if(mode==BLOCKING) {
				LOG_BLOCK(0);
				if(WSAGetOverlappedResult(fd,&Overlapped,&RSize,TRUE,&flags)) //wsock2function
				    res =0;
				else							
				    wsock_syserror("TestRequest: WSARecv failed",WSAGetLastError());
				LOG_BLOCK(1);
			    }
			    break;
			case WSAEWOULDBLOCK: 
			    res =0; RSize=0; Sleep(10); 
			    fprintf(stderr,"Warning: Got wouldblock-error...\n"); fflush(stderr);
			    break;
			case WSAENOBUFS: res =0; counter ++; RSize=0; Sleep(10); 
			    fprintf(stderr,"Warning: TestRequest: Got nobufs-error...\n"); fflush(stderr);
			    DEBUG_PRINT_MSG("Got nobufs-error...");
			    if(counter<1000) break;
			default: wsock_syserror("Test Request:WSARecv failed",error);
			}
		    } else pending = false;
		} while (!res);
	    }
	    
	    LOG_TEST_REQUEST(1);
	    
	    return finished;
	}
	
	void WaitRequest() {
	    BOOL Finished=TestRequest(BLOCKING);
	    
	    while(!Finished) {
		//GenericWait(&Overlapped);
		Finished=TestRequest(BLOCKING);
	    }
	}
	
	void finish() {
	    if(Transfer) {
		Transfer->finished=1;
		Transfer=0;
	    }
	    State=CLEAN;
	    finished=TRUE;
	    if(fd == INVALID_SOCKET) SetEvent(Overlapped.hEvent);
	    else if(pending&&mixed) recv(fd,(char*)&received,0,MSG_PEEK);
	    DEBUG_PRINT_MSG2("Transfer with size %d finished",size);
	}
	
	WSAOVERLAPPED Overlapped;
	WSABUF buf;
	SOCKET fd;
	WsockHeader Header;
	unsigned long received,size;
	BOOL finished;
	TransferRequest *Transfer;
	int State;
	bool pending,shuttingDown;
};


static CTCPCommunicator Comm;
static int *Channels;
static CReceiveRequest **ReceiveRequests;
static WSAEVENT *Events;
static fd_set allFds;
static unsigned int numConnections;
static LARGE_INTEGER TimerFrequ;




#ifdef THREAD_SAFE
static struct CTCP_MUTEX {
	MPID_THREAD_DS_LOCK_DECLARE;
} tcp_mutex;
#endif

struct TCP_SELF_MESSAGE {
	TCP_SELF_MESSAGE() {buf=0;Header.Size=offset=0; Header.Type=-1;}
	TCP_SELF_MESSAGE(void *mem, const unsigned int s) {
		buf = MALLOC(s); 
		if(!buf) {
			Header.Size=0;
			return;
		}
		MEMCPY(buf,mem,s); Header.Size=s;
		offset=0;
	}
	TCP_SELF_MESSAGE(const unsigned int s) {
		DBG("Entering TCP_SELF_MESSAGE(const unsigned int s)) ");
		Header.Size=s; buf=MALLOC(s);
		offset=0;
	}
	~TCP_SELF_MESSAGE() {DBG("Entering ~TCP_SELF_MESSAGE()");if(buf) FREE(buf); buf=0;}
	operator void*() { return buf;}
	bool operator !() {return (buf==0);}
	struct WsockHeader Header;
	void *buf;
	unsigned int offset;
};


void wsock_error(const char *string,int value)
{
    fprintf(stdout,"%s (%d)\n",string, value); fflush(stdout);
    exit(value);

}

char *makeExceptionMessage(DWORD code,void *address,DWORD *info) {
	static char m[255];

	switch(code) {
	case EXCEPTION_ACCESS_VIOLATION : sprintf(m,"Invalid %s of address 0x%X at 0x%p",(info[0]?"write":"read"),info[1],address); break;
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED : sprintf(m,"Index out of bounds at 0x%p",address); break;
	case EXCEPTION_BREAKPOINT :	sprintf(m,"Breakpoint at 0x%p",address); break;
	case EXCEPTION_DATATYPE_MISALIGNMENT : sprintf(m,"Datatype misalignement at 0x%p",address); break;
	case EXCEPTION_FLT_DENORMAL_OPERAND : sprintf(m,"Floatingpoint denormal operand at 0x%p",address);break;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO :  sprintf(m,"Floatingpoint division by zero at 0x%p",address); break;
	case EXCEPTION_FLT_INEXACT_RESULT : sprintf(m,"Inexact floatingpoint result at 0x%p",address); break;
	case EXCEPTION_FLT_INVALID_OPERATION : sprintf(m,"Invalid floatingpoint operation at 0x%p",address); break;
	case EXCEPTION_FLT_OVERFLOW : sprintf(m,"Floatingpoint overflow at 0x%p",address);  break;
	case EXCEPTION_FLT_STACK_CHECK : sprintf(m,"Floatingpoint stack over/underflow at 0x%p",address); break;
	case EXCEPTION_FLT_UNDERFLOW : sprintf(m,"Floatingpoint underflow at 0x%p",address); break;
	case EXCEPTION_ILLEGAL_INSTRUCTION : sprintf(m,"Illegal instruction at 0x%p",address); break;
	case EXCEPTION_IN_PAGE_ERROR : sprintf(m,"Invalid page at 0x%p",address); break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO : sprintf(m,"Integer division by zero at 0x%p",address); break;
	case EXCEPTION_INT_OVERFLOW : sprintf(m,"Integer overflow at 0x%p",address); break;
 	case EXCEPTION_INVALID_DISPOSITION : sprintf(m,"Invalid disposition at 0x%p (How did you do that ???) ",address); break;
 	case EXCEPTION_NONCONTINUABLE_EXCEPTION : sprintf(m,"noncontinuable exception at 0x%p",address); break;
	case EXCEPTION_PRIV_INSTRUCTION : sprintf(m,"Privileged instruction at 0x%p",address); break;
	case EXCEPTION_SINGLE_STEP : sprintf(m,"Single step at 0x%p",address); break;
	case EXCEPTION_STACK_OVERFLOW : sprintf(m,"Stack overflow at 0x%p",address); break;
	default: sprintf(m,"Unknown exception 0x%X at 0x%p",code,address); break;
	}

	return m;
}

LONG GlobalExceptionFilter(_EXCEPTION_POINTERS *ExceptionInfo) {
	EXCEPTION_RECORD *ExRec=ExceptionInfo->ExceptionRecord;
	fprintf(stderr,"Exception of type:\n%s\n",makeExceptionMessage(ExRec->ExceptionCode,
		        ExRec->ExceptionAddress, (DWORD*) ExRec->ExceptionInformation));
	fflush(stderr);
	fflush(stdout);

	return EXCEPTION_CONTINUE_SEARCH;
}


void wsock_syserror( const char *string, int value )
{
	static char all[1024];
	void *lpMsgBuf;
	if(string) {
		strcpy(all,string);
		strcat(all,":\n");
	}
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,value,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR) &lpMsgBuf,1,NULL);
	
	if(lpMsgBuf) strcat(all,(char*)lpMsgBuf);
	wsock_error(all,value);
}

void MPID_WSOCK_Init( int *argc, char *** argv) {
	unsigned long val=1;
	int i;
#ifndef NOTHREAD
	DWORD tid;
#endif

	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)GlobalExceptionFilter);
	SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX);
	try{
		// Create: Initializing CTCPCommunicator with commandline arguments
		// fill struct with information about master, number of nodes,
		// port, address (switches -m -n -p -b)
		
		if(Comm.Create(argc,argv) <0) {
			wsock_error("Process Startup failed",-1);
		}
	} catch (socketException e) {
		wsock_error((const char*)e,-1);
	}
	MPID_MyWorldRank = Comm.GetMyId();
    MPID_MyWorldSize = Comm.GetNumProcs();
	numConnections=Comm.GetFDs(&Channels);//nur lokal 0
	FD_ZERO(&allFds);	
	ReceiveRequests= new CReceiveRequest*[MPID_MyWorldSize];

	for(i=0;i<MPID_MyWorldSize;i++) {
		ReceiveRequests[i] = new CReceiveRequest(Channels[i]);
		if(Channels[i] != INVALID_SOCKET) {
			FD_SET(Channels[i],&allFds);
			WSAEventSelect(Channels[i],MPID_Events[MPID_MyWorldRank],FD_READ); //wsock2function
		}
	} 
	MPID_THREAD_DS_LOCK_INIT(&tcp_mutex)
	QueryPerformanceFrequency(&TimerFrequ);


#ifndef NOTHREAD
	InitializeCriticalSectionAndSpinCount(&CS,3000);
	ChannelForThread = new unsigned long[MPID_MyWorldSize];
	memset((void*)ChannelForThread,0,MPID_MyWorldSize*sizeof(int));
	SendSem=CreateSemaphore(0,0,SendQueue.max_size()-1,0);
	run=1;
	if(!SendSem) wsock_syserror("CreateSemaphore failed",GetLastError());
	hThread=CreateThread(0,0,(LPTHREAD_START_ROUTINE)ThreadFunc,0,0,&tid);
	if(!hThread) wsock_syserror("CreateThread failed",GetLastError());
	SetThreadPriority(hThread,THREAD_PRIORITY_HIGHEST);
#endif
}

void MPID_WSOCK_End(void) {
    int i;
    WsockHeader msg;
    MPID_PKT_T   pkt;
    WSABUF SBuf;
    DWORD ToSend = 0;
    bool top = true;

    
    DEBUG_PRINT_MSG2("Shutting down WSOCK device, numConnections: %d\n",numConnections);
    for(i=MPID_MyWorldRank+1;(i<MPID_MyWorldSize)&&top;++i)
	top = (Channels[i] == INVALID_SOCKET);

    
    while(numConnections&&top) {	
        MPID_WSOCK_RecvAnyControl(&pkt,sizeof(pkt),&i);
    }

    
    for(i=0;i<MPID_MyWorldSize;++i) {
	if(i != MPID_MyWorldRank && Channels[i] != INVALID_SOCKET) {
	    SBuf.len=PRIVATESIZE;
	    SBuf.buf=(char*)&msg;
	    msg.Type=QUIT;
	    msg.Size=0;
	    DEBUG_PRINT_MSG2("Sending QUIT to %d",i);
	    MPID_PKT_PACK(&msg,PRIVATESIZE,i);
	    while(WSASend(Channels[i],&SBuf,1,&ToSend,0,0,0) && WSAGetLastError() == WSAEWOULDBLOCK) {
		DEBUG_PRINT_MSG2("%d bytes sent",ToSend);
		Sleep(1);
	    }
	    //MPID_WSOCK_SendBlock(&SBuf,PRIVATESIZE,1,i);
	    ReceiveRequests[i]->shuttingDown = true;
	    shutdown(Channels[i],SD_SEND);
	}
    }
    
#ifndef NOTHREAD
    run=FALSE;
    ReleaseSemaphore(SendSem,1,0);
#endif
    while(numConnections) {	
	MPID_WSOCK_RecvAnyControl(&pkt,sizeof(pkt),&i);
    }
    
    //Sleep(10);
    
    MPID_THREAD_DS_LOCK_FREE(&tcp_mutex)
#ifndef NOTHREAD
	WaitForSingleObject(hThread,INFINITE);
    CloseHandle(hThread);
    CloseHandle(SendSem);
    DeleteCriticalSection(&CS);
#endif
    
    while(RequestQueue.size()) {
	delete RequestQueue.front();
	RequestQueue.pop_front();
    }
    
    while(Transfers.size()) {
	delete Transfers.front();
	Transfers.pop_front();
    }
    for(i=0;i<MPID_MyWorldSize;i++) {
	delete ReceiveRequests[i];
    }
    delete ReceiveRequests;
}

extern int MPID_DebugFlag;
void MPID_WSOCK_CreateTransfer(void * buf, int size,int tag, ASYNCRecvId_t id) {
	//MPID_DebugFlag =1;
	TransferRequest *t=MPID_WSOCK_GetTransfer(buf,size);
	DEBUG_PRINT_MSG2("Creating TransferRequest for size %d",size);
	ReceiveTransfers.insert(CTransferMap::value_type(tag,t));
	MEMCPY(id,&t,sizeof(t));
}

void MPID_WSOCK_ReceiveTransfer(int tag,ASYNCRecvId_t id) {
	TransferRequest *t;
	MEMCPY(&t,id,sizeof(t));
	if(tag>0) ReceiveTransfers.erase(tag);
	MPID_WSOCK_FreeTransfer(t);
	//MPID_DebugFlag =0;
}

int MPID_WSOCK_SendTransfer(void *buf, int size,int partner,int id,ASYNCSendId_t sid) {
	//struct WsockHeader h;
	SendRequest *Request;
	WSABUF SBuf[2];
	DWORD ToSend=PRIVATESIZE+size;
	Request = MPID_WSOCK_GetRequest();
	Request->h.Type = TRANSFER; 
	Request->h.Tag=id; 
	Request->h.Size=size;
	MPID_PKT_PACK(&Request->h,PRIVATESIZE,partner);
	//MPID_DebugFlag =1;
	SBuf[0].len=PRIVATESIZE;
	SBuf[0].buf=(char*)&Request->h;
	SBuf[1].len=size;
	SBuf[1].buf=(char*)buf;
	MEMCPY(sid,&Request,sizeof(Request));
	return MPID_WSOCK_SendR( SBuf, PRIVATESIZE+size,2, partner,Request);
}
 
int MPID_WSOCK_TestTransfer(ASYNCRecvId_t id) {
	TransferRequest *t;
	MEMCPY(&t,id,sizeof(t));
	if(t->Receive) return t->Receive->TestRequest(NONBLOCKING);
	else return t->finished;
}

void MPID_WSOCK_WaitTransfer(ASYNCRecvId_t id) {
	TransferRequest *Transfer;
	memcpy(&Transfer,id,sizeof(Transfer));
	while(!Transfer->finished) {
		if(!Transfer->Receive) {
			// The transfer didn't start yet...
			MPID_DeviceCheck(MPID_NOTBLOCKING);
		} else
			Transfer->Receive->WaitRequest();
	}
	MPID_WSOCK_FreeTransfer(Transfer);
}
 
BOOL MPID_WSOCK_RecvTransferMessage(CReceiveRequest *Request) {
	CTransferMap::iterator i;
	unsigned int msglen;
	MPID_THREAD_DS_LOCK(&tcp_mutex)
	i=ReceiveTransfers.find(Request->Header.Tag);
	if(i==ReceiveTransfers.end()) {
		MPID_THREAD_DS_UNLOCK(&tcp_mutex);
		//printf("Size %d, Tag %d\n",Request->Header.Size,Request->Header.Tag);
		wsock_error("Protocol error: ReceiveTransfer not found",-1);
	} else {
		DEBUG_PRINT_MSG2("Transfer has requested %d bytes",(*i).second->size);
		DEBUG_PRINT_MSG2("Buf is: %p",(*i).second->buf);
	}
	MPID_THREAD_DS_UNLOCK(&tcp_mutex);
	msglen=min((*i).second->size,Request->Header.Size);
	if(Request->Header.Size!=(*i).second->size) {
		fprintf(stderr,"Warning: Requested size(%d) and sent size(%d) are not matching!!\n",(*i).second->size,Request->Header.Size);
		fflush(stderr);
	}
	DEBUG_PRINT_MSG2("Reading Transfer message with size %d",msglen);
	Request->Transfer = (*i).second;
	return Request->StartRequest((char*)((*i).second->buf),msglen,NONBLOCKING);
}


static int MPID_WSOCK_CheckIncomingType(CReceiveRequest *Request) {
    
    int again,found=0;
    
    if(Request->State == TRANSFERRING) {
	DEBUG_PRINT_MSG2("CheckIncomingType: Continued Transfer request for tag %d",Request->Header.Tag);
	Request->TestRequest(NONBLOCKING);
	return 0;
    }
    do {
	again = 0;
	if(Request->State == CLEAN) {
	    Request->GetHeader();
	    MPID_PKT_UNPACK(&Request->Header,PRIVATESIZE,current);
	}
	if((Request->Header.Type & CTRL) == CTRL) {
	    found = 1;
	} else if((Request->Header.Type & TRANSFER) == TRANSFER) {
	    DEBUG_PRINT_MSG2("CheckIncomingType: Found Transfer Message for tag %d",Request->Header.Tag);			
	    MPID_WSOCK_RecvTransferMessage(Request);
	} else if((Request->Header.Type & QUIT) == QUIT || Request->State == CLOSED) {
	    //MPID_SetDebugFlag( 1 );
	    DEBUG_PRINT_MSG2("RecvAnyControl: Got QUIT on %d",Request->fd);
	    numConnections--;
	    DEBUG_PRINT_MSG2("Got Quit message. Waiting for %d procs.",numConnections);
	    ResetEvent(Request->Overlapped.hEvent);
	    Request->State = CLEAN;
	    if(Request->fd != INVALID_SOCKET) FD_CLR(Request->fd,&allFds);
	    //if(!numConnections) found = 1;
	} else {
	    printf("Type==%d\nTag==%d\nSize==%d\n",Request->Header.Type,Request->Header.Tag,Request->Header.Size);
	    wsock_error("Protocol error: CheckIncomingType found illegal message instead of CTRL",-1);
	}
    } while(again);
    return found;
}

void MPID_WSOCK_RecvAnyControl( MPID_PKT_T *pkt, int size, int *from ) {
    static int current=0;
    FD_SET readfds;
    int res,found;
    CReceiveRequest *Request;
    bool local;
    
    found = 0;
    LOG_ANY_CTRL(0);
    while(!found) {
	local=false;
	if(LocalChannel.size()) {
	    res=1;
	    local = true;
	    current = MPID_MyWorldRank;
	} else if((MPID_MyWorldSize>1) && (numConnections)) {
	    readfds = allFds;
	    res=select(0,&readfds,0,0,0);
	    if(res==SOCKET_ERROR)
		wsock_syserror("RecvAnyControl: select failed",WSAGetLastError());
	} else if(MPID_MyWorldSize>1) res=0;
	       else wsock_error("Logical error. Waiting for message while num_procs == 1!",-1);
	while( (res>0) && (!found) ) {
	    if(current >= MPID_MyWorldSize) current = 0;
	    if(
		(
		(Channels[current] != INVALID_SOCKET)	&& 
		(FD_ISSET(Channels[current],&readfds))
		) || 
		(
		local && 
		(current == MPID_MyWorldRank)
		)
		) {
		--res;
		Request=ReceiveRequests[current];
		found = MPID_WSOCK_CheckIncomingType(Request);
		if(found && (numConnections || MPID_MyWorldSize<2)) {
		    *from=current;
		    if(Request->Header.Size > size) {
			wsock_error("Control message too large...\n",Request->Header.Size);
		    }
		    DEBUG_PRINT_MSG2("Trying to read pkt of size %d",Request->Header.Size);
		    Request->StartRequest(((char*)pkt)+PRIVATESIZE,
			Request->Header.Size-PRIVATESIZE,BLOCKING);
		}
		
	    } /* if( FD_ISSET...)*/
	    ++current;
	} /*while(res>0) */
	if(!numConnections) break;
    } /* while (!found) */
    LOG_ANY_CTRL(1);
}

int MPID_WSOCK_ControlMsgAvail( void ) {
	/*
	|   Check if there is a control message available from any process:
	|
	*/
	static int current=0;
	int res,found;
	FD_SET readfds;
	timeval t={0,0};
	bool local=false;

	// LOG_MSG_AVAIL(0);

	found = 0;
	if(LocalChannel.size()) {
		/* here is a message available that we sent to ourselves: */
		res=1;
		local = true;
		current = MPID_MyWorldRank;
		/* check the other procs (if they exist): */
	} else if((MPID_MyWorldSize>1) && (numConnections)) { 
		/* select all file descriptors (sockets) as being ready for reading: */
		readfds = allFds;
		res=select(0,&readfds,0,0,&t);
		if(res==SOCKET_ERROR)
			wsock_syserror("ControlMsgAvail: select failed",WSAGetLastError());
	} else res=0;

	while( (res>0) && (!found)) {
		if(current >= MPID_MyWorldSize) 
			current = 0;/* <-- continue with process 0 */
		if(/* check for message from another process: */
			(Channels[current] != INVALID_SOCKET && FD_ISSET(Channels[current],&readfds))
			|| /* check for message from ourselves: */ 
			(local && current == MPID_MyWorldRank)) {
			  --res;
			  found = MPID_WSOCK_CheckIncomingType(ReceiveRequests[current]);
		} 	
		current++;/* <-- check for messages from next process */
	}
	// LOG_MSG_AVAIL(1);

	return found;	
}

static void MPID_WSOCK_ReadLocal( void *buf, int size) {
	TCP_SELF_MESSAGE *msg;
	unsigned int s=size;
	if(LocalChannel.empty())
		// should we do a loop here instead of an error?
		wsock_error("protocol error: Local channel is empty\n",-1);
	msg=LocalChannel.front();
	s=min(size,msg->Header.Size);
	MEMCPY(buf,((char*)msg->buf)+msg->offset,s);
	if(s==msg->Header.Size) {
		LocalChannel.pop_front();	
		delete msg;
		if(s != size) {
			// The user wants more? Give him more
			// This should not happen within a correct program.
			MPID_WSOCK_ReadLocal( ((char*)buf)+s,size-s);
		}
	} else {
		msg->Header.Size-=s;
		msg->offset += s;
	}
}

int MPID_WSOCK_RecvFromChannelAsync( void *buf, int size, int channel,ASYNCRecvId_t id ) {
	CReceiveRequest *Request;
	TransferRequest *Transfer;
	Request=ReceiveRequests[channel];
	Transfer=MPID_WSOCK_GetTransfer(buf,size);
	Request->Transfer=Transfer;
	if(!Request->StartRequest((char*)buf,size,NONBLOCKING)) {
		memcpy(id,&Transfer,sizeof(Transfer));
		return 0;
	}
	//Request->StartRequest();
	MPID_WSOCK_FreeTransfer(Transfer);
	return 1;
}

void MPID_WSOCK_RecvFromChannel( void *buf, int size, int channel) {
	CReceiveRequest *Request;

	LOG_RECV(0);
	Request=ReceiveRequests[channel];
	Request->StartRequest((char*)buf,size,BLOCKING);
		/*
		while(!Request->TestRequest(TRUE)) ; 
			MPID_DeviceCheck(MPID_NOTBLOCKING);
		*/
	//Request->StartRequest();
	LOG_RECV(1);
}

void MPID_WSOCK_ConsumeData(int size,int channel) {
    char buf[4096];
    int toRead;
    CReceiveRequest *Request;
    
    Request=ReceiveRequests[channel];
	
    while(size) {
	toRead = size<4096?size:4096;
	Request->StartRequest((char*)buf,toRead,BLOCKING);
	size -= toRead;
    }
}
    

void MPID_WSOCK_SendBlock(WSABUF *buf, int size, unsigned long NumBlocks,int dest) {
	//ASYNCSendId_t id;
	SendRequest *r;
	r = MPID_WSOCK_GetRequest();
	if(MPID_WSOCK_SendR(buf,size,NumBlocks,dest,r)<0)
		MPID_WSOCK_WaitSend((long *)&r);
}

	
	
void MPID_WSOCK_SendControl(MPID_PKT_T *pkt,int size,int dest) {	
	WSABUF SBuf;
	SendRequest *r;
	//ASYNCSendId_t id;
	r = MPID_WSOCK_GetRequest();
	pkt->head.TCP_TYPE=CTRL; 
	pkt->head.TCP_SIZE=size;
	
	MPID_PKT_PACK(&pkt->head,PRIVATESIZE,dest);
	
	SBuf.len=size;
	SBuf.buf=(char*)pkt;
	
	if(MPID_WSOCK_SendR(&SBuf,size,1,dest,r)<0)
		MPID_WSOCK_WaitSend((long *)&r);
}



static int MPID_WSOCK_SendR(WSABUF *buf, int size, unsigned long NumBlocks,int dest,SendRequest *Request) {

	/*
	|   MPID_WSOCK_SendR() sends data to a destination process 'dest'.
	|   If 'dest' is the ID of the current process, the data is put into 'LocalChannel',
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
	|   
	| 
	|   Return Values:
	| 
	|    0  : send operation successfully performed
	|   -1  : error
	|
	*/
    DWORD ToSend=size;
    int res,error;
    //SendRequest *Request=0;
    BOOL retry;
    
    LOG_SEND(0);
    
	if(dest==MPID_MyWorldRank) {
		for(res=0;res<NumBlocks;res++) {
			LocalChannel.push_back(new TCP_SELF_MESSAGE(buf[res].buf,buf[res].len));
		}

		LOG_SEND(1);
		return 0;
	} else {
		if(Channels[dest] == INVALID_SOCKET) {
			LOG_SEND(1);
			return 0;
		}

#ifndef NOTHREAD
	if(ChannelForThread[dest] || size>=THREAD_SIZE)
	    return MPID_WSOCK_Send_thread(buf,size,NumBlocks,dest,Request);
#endif
	
	Request->size=size;
	Request->fd=Channels[dest];
#ifndef NOTHREAD
	Request->Queued = FALSE;
#endif
	
	DEBUG_PRINT_MSG2("Sending message of size %d",size);
	//SetThreadIdealProcessor(GetCurrentThread(),0);
	do {
	    retry=FALSE;
		//sending overlapped
	    res=WSASend(Channels[dest],buf,NumBlocks,&ToSend,0,&Request->Overlapped,0); //wsock2function
	    if(res==SOCKET_ERROR) {
		error=WSAGetLastError();
		switch(error) {
		case WSA_IO_PENDING: Request->finished=FALSE; break;
		case WSAENOBUFS: MPID_DeviceCheck(MPID_NOTBLOCKING); retry=TRUE; break;
		default: wsock_syserror("MPID_WSOCK_Send: WSASend failed",error);
		}	
	    } else {
		if(ToSend != size) {
		    wsock_error("Send: WSASend truncated message",ToSend);
		}
		Request->finished=TRUE;
	    }
	} while(retry);
	if(Request->finished) {
	    MPID_WSOCK_FreeRequest(Request);
	    LOG_SEND(1);
	    return 0;
	}
    }
    LOG_SEND(1);
    return -1;
}

int MPID_WSOCK_Send(WSABUF *buf, int size, unsigned long NumBlocks,int dest,ASYNCSendId_t id) {
    	SendRequest *Request;
	Request=MPID_WSOCK_GetRequest();
	MEMCPY(id,&Request,sizeof(Request));
	return MPID_WSOCK_SendR(buf,size,NumBlocks,dest,Request);
}

#ifndef NOTHREAD
int MPID_WSOCK_Send_thread(WSABUF *buf, int size, unsigned long NumBlocks,int dest,SendRequest *Request) {
    DWORD ToSend=size;
    int res,error;
    //SendRequest *Request=0;
    
    //printf("\n");
    //fflush(stdout);
    
    
    if(Channels[dest] == INVALID_SOCKET) {
	LOG_SEND(1);
	return 0;
    }
    InterlockedIncrement((long*)(ChannelForThread+dest));
    //Request=MPID_WSOCK_GetRequest();
    Request->size=size;
    Request->fd=Channels[dest];
    Request->dest=dest;
    Request->Queued = TRUE;
    Request->buf=(WSABUF*)malloc(sizeof(WSABUF)*NumBlocks);
    for(int i=0;i<NumBlocks;i++) Request->buf[i]=buf[i];
    Request->NumBlocks=NumBlocks;
    Request->Overlapped.Internal=STATUS_PENDING;
    Request->finished=FALSE;
    ResetEvent(Request->Overlapped.hEvent);
    
    DEBUG_PRINT_MSG2("Queueing message of size %d",size);
    //SetThreadIdealProcessor(GetCurrentThread(),0);
    EnterCriticalSection(&CS);
    SendQueue.push_back(Request);
    LeaveCriticalSection(&CS);
    while(!ReleaseSemaphore(SendSem,1,0))
	Sleep(2);
    //MEMCPY(id,&Request,sizeof(Request));
    return -1;
}
#endif




int MPID_WSOCK_TestSend(ASYNCSendId_t id) {
	/*
	|   Test, if the underlying (channel) send operation has been finished.
	|   The respective send request is retrieved via a send id, maintained by
	|   the MPIR-layer.
	*/
    SendRequest *Request;
    DWORD ToSend,flags;
    BOOL Finished;
    
    LOG_TEST_SEND(0);
	/*
	|   The address of the respective 'SendRequest' struct was stored in the 'sid'
	|   member of the 'MPIR_SHANDLE' struct as an 'ASNYCSendId_t' type.
	|   So, we have to do a memcpy() to get back the 'SendRequest' handle from the send-id:
	*/
    memcpy(&Request,id,sizeof(&Request));
    Finished=Request->finished;

#ifndef NOTHREAD
	if(Request->Queued) {
		if(Finished && (Request->size != Request->Overlapped.Offset)) {
			wsock_syserror("TestSend: WSASend truncated message",Request->Error);   
		}
		if(Finished) MPID_WSOCK_FreeRequest(Request);
	} else
#endif
	{

		if(!Finished) {
			Finished=HasOverlappedIoCompleted(&Request->Overlapped);		
		} 

		if(Finished) {
			if(!WSAGetOverlappedResult(Request->fd,&Request->Overlapped,&ToSend,FALSE,&flags) ||
				ToSend != Request->size) //wsock2function
				wsock_syserror("TestSend: WSASend truncated message",WSAGetLastError());
			MPID_WSOCK_FreeRequest(Request);	    	    
		}
	}
    LOG_TEST_SEND(1);
    
    return (int)Finished;
}

void MPID_WSOCK_WaitSend(long *id) {
	/*
	|   Wait until the underlying (channel) send operation has been finished.
	|   The respective send request is retrieved via a send id, maintained by
	|   the MPIR-layer.
	*/
	SendRequest *Request; 
	DWORD ToSend,flags;
	BOOL Finished;
	//int counter =0;
	 LOG_WAIT_SEND(0);
	 /*
	 |   The address of the respective 'SendRequest' struct was stored in the 'sid'
	 |   member of the 'MPIR_SHANDLE' struct as an 'ASNYCSendId_t' type.
	 |   So, we have to do a memcpy() to get back the 'SendRequest' handle from the send-id:
	 */
	memcpy(&Request,id,sizeof(&Request));
	Finished=Request->finished;
	
	/* if the send operation has not been finished, then wait for it: */
	if(!Finished) 
		GenericWait(&Request->Overlapped);

#ifndef NOTHREAD
	if(Request->Queued) {
	    if(Request->size != Request->Overlapped.Offset)
		wsock_syserror("Wait Send: WSASend truncated message",Request->Error);
	} else
#endif
	{
	ToSend=0;//Request->size;
	if(!WSAGetOverlappedResult(Request->fd,&Request->Overlapped,&ToSend,FALSE,&flags) ||
	    ToSend != Request->size) wsock_syserror("Wait Send: WSASend truncated message",WSAGetLastError()); //wsock2function
	}
	MPID_WSOCK_FreeRequest(Request);
	
	LOG_WAIT_SEND(1);
}

/*
int MPID_WSOCK_TestSend(ASYNCSendId_t id) {
	SendRequest *Request;
	DWORD ToSend,flags;
	BOOL Finished;
	
	 LOG_TEST_SEND(0);

	memcpy(&Request,id,sizeof(&Request));
	Finished=Request->finished;

	//if(!Finished) {
	//	Finished=HasOverlappedIoCompleted(&Request->Overlapped);		
	//} 

	if(Finished) {
		if(Request->size != Request->Overlapped.Offset) 
			wsock_error("TestSend: WSASend truncated message",Request->Error);
		MPID_WSOCK_FreeRequest(Request);
	}// else SetThreadIdealProcessor(GetCurrentThread(),0);
	
	LOG_TEST_SEND(1);
	
	return (int)Finished;
}


void MPID_WSOCK_WaitSend(ASYNCSendId_t id) {
	SendRequest *Request;
	DWORD ToSend,flags;
	BOOL Finished;
	DWORD counter =0;
	
	 LOG_WAIT_SEND(0);

	memcpy(&Request,id,sizeof(&Request));

	Finished=Request->finished;
	
	if(!Finished) 
		GenericWait(&Request->Overlapped);
		
	if(Request->size != Request->Overlapped.Offset) 
		wsock_error("Wait Send: WSASend truncated message",Request->Error);
	MPID_WSOCK_FreeRequest(Request);
	
	 LOG_WAIT_SEND(1);
}

#endif
  */
static SendRequest *MPID_WSOCK_GetRequest(void) {
	SendRequest *Request;
	if(!RequestQueue.size()) {
		Request=new SendRequest;
	} else {
		Request=RequestQueue.front();
		RequestQueue.pop_front();
	}
	return Request;
}
 
static void MPID_WSOCK_FreeRequest(SendRequest *Request) {
	if(RequestQueue.size() > MAX_BUFFERED_REQUESTS) {
		delete Request;
	} else {
		RequestQueue.push_back(Request);
	}
}

static TransferRequest *MPID_WSOCK_GetTransfer(void *buf, int size) {
	TransferRequest *Request;
	if(!Transfers.size()) {
		Request=new TransferRequest(buf,size);
	} else {
		Request=Transfers.front();
		Transfers.pop_front();
		Request->Init(buf,size);
	}
	DEBUG_PRINT_MSG2("Returning request at %p",Request);
	return Request;
}

static void MPID_WSOCK_FreeTransfer(TransferRequest *Request) {
	if(Transfers.size() > MAX_BUFFERED_REQUESTS) {
		delete Request;
	} else {
		Transfers.push_back(Request);
	}
	
}

void MPID_WSOCK_Wtime(double *a) {
	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);
	*a = (double)t.QuadPart/(double)TimerFrequ.QuadPart;
}

/*
 as  NOTHREAD is defined this thread is not created
 it would block on an semaphore and then send messages
 which are queued in SendQueue
*/
#ifndef NOTHREAD
/*
DWORD WINAPI ThreadFunc(void* dummy) {
	DWORD res;

	SendRequest *Request;
	WaitForSingleObject(SendSem,INFINITE);
	while(run) {
		EnterCriticalSection(&CS);
		Request=SendQueue.front();
		SendQueue.pop_front();
		LeaveCriticalSection(&CS);
		 
		Request->Overlapped.Offset=0;
		//DEBUG_PRINT_MSG2("Thread sending message of size %d",Request->size);

		res=WSASend(Request->fd,Request->buf,Request->NumBlocks,&Request->Overlapped.Offset,0,0,0); //wsock2function
		Request->Error=WSAGetLastError();
		if(res == SOCKET_ERROR && Request->Error == WSAEWOULDBLOCK) {
		}
		//if(MPID_DebugFlag) 
		// cerr<<"Thread sent message ("<<Request->size<<") sent "<<Request->Overlapped.Offset<<" res is: "<<Request->Error<<endl<<flush;
		//MPID_DebugFlag = 0;
		InterlockedDecrement((long*)(ChannelForThread+Request->dest));
		Request->Overlapped.OffsetHigh=0;
		Request->Overlapped.InternalHigh=0;
		free(Request->buf);
		Request->Overlapped.Internal = STATUS_WAIT_0;
		Request->finished = TRUE;
		SetEvent(Request->Overlapped.hEvent);
		WaitForSingleObject(SendSem,INFINITE);
	}
	return 0;
}*/
#endif

int MAGPIE_cluster_of_process(MPI_Comm comm, int rank, int *cluster){
    *cluster = Comm.GetSMPId();
    return MPI_SUCCESS;
}

void MAGPIE_reset_cluster_info(void){
  /* do nothing -- will always be the same */
}

#ifdef __cplusplus
}
#endif
