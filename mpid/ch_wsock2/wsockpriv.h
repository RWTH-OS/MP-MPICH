
#ifndef __WSOCKPRIV_H__
#define __WSOCKPRIV_H__

#include <winsock2.h>

#ifndef ANSI_ARGS
#if defined(__STDC__) || defined(__cplusplus) || defined(HAVE_PROTOTYPES)
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif



#define CTRL		0x00000001U
#define TRANSFER	0x10000002U
#define QUIT		0x00000004U
#define RESTART		0x10000000U

#define MAX_BUFFERED_REQUESTS 100

struct WsockHeader {
	unsigned int Type;
	int	     Tag;
	unsigned int Size;
};

#define PRIVATESIZE (3*sizeof(int))


int MPID_WSOCK_Send ANSI_ARGS((WSABUF *,int, unsigned long, int,ASYNCSendId_t));
void MPID_WSOCK_SendBlock ANSI_ARGS((WSABUF *,int, unsigned long, int));
int MPID_WSOCK_TestSend ANSI_ARGS((ASYNCSendId_t));
void MPID_WSOCK_WaitSend ANSI_ARGS((ASYNCSendId_t));
void MPID_WSOCK_SendControl ANSI_ARGS((MPID_PKT_T*,int,int));
int MPID_WSOCK_SendTransfer ANSI_ARGS((void *,int,int,int,ASYNCSendId_t));
void MPID_WSOCK_WaitTransfer ANSI_ARGS((ASYNCRecvId_t));
int MPID_WSOCK_RecvFromChannelAsync ANSI_ARGS(( void*,int,int,ASYNCRecvId_t));



void MPID_WSOCK_RecvFromChannel ANSI_ARGS(( void *, int, int ));
int MPID_WSOCK_ControlMsgAvail ANSI_ARGS(( void ));
void MPID_WSOCK_RecvAnyControl ANSI_ARGS(( MPID_PKT_T *, int , int *));
void wsock_syserror ANSI_ARGS(( const char *, int ));
void MPID_WSOCK_Init ANSI_ARGS(( int *, char *** ));
void MPID_WSOCK_End  ANSI_ARGS((void));
void MPID_WSOCK_CreateTransfer ANSI_ARGS((void *,int,int,ASYNCRecvId_t)) ;
void MPID_WSOCK_SendTransferBlock ANSI_ARGS((void *,int,int,int));
int MPID_WSOCK_TestTransfer ANSI_ARGS((ASYNCRecvId_t));
void MPID_WSOCK_ReceiveTransfer ANSI_ARGS((int,ASYNCRecvId_t));

void MPID_WSOCK_ConsumeData ANSI_ARGS((int size,int channel));





#ifdef __cplusplus
}
#endif


#endif