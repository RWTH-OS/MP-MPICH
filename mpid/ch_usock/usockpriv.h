/*
 * $Id$
 *
 */

#ifndef __USOCKPRIV_H__
#define __USOCKPRIV_H__

#include <winsock2.h>
#include "mpid.h"
#include "usockpackets.h"

#ifndef ANSI_ARGS
#if defined(__STDC__) || defined(__cplusplus) || defined(HAVE_PROTOTYPES)
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif
#endif

#define CTRL		0x00000001U
#define TRANSFER	0x10000002U
#define QUIT		0x00000004U
#define RESTART		0x10000000U

#define MAX_BUFFERED_REQUESTS 100

#define PRIVATESIZE (3*sizeof(int))

typedef 
struct _UsockHeader {
	unsigned int Type;
	int	     Tag;
	unsigned int Size;
} UsockHeader;


void MPID_USOCK_Init ANSI_ARGS(( int *, char *** ));
void MPID_USOCK_Finalize  ANSI_ARGS((void));
void MPID_USOCK_RecvAnyControl ANSI_ARGS(( MPID_PKT_T *, int , int *));
int  MPID_USOCK_ControlMsgAvail ANSI_ARGS(( void ));
int  MPID_USOCK_RecvFromChannelAsync ANSI_ARGS(( void*,int,int,ASYNCRecvId_t));
void MPID_USOCK_RecvFromChannel ANSI_ARGS(( void *, int, int ));
void MPID_USOCK_ConsumeData ANSI_ARGS((int, int ));
void MPID_USOCK_SendControl ANSI_ARGS((MPID_PKT_T*,int,int));
int  MPID_USOCK_SendChannel ANSI_ARGS((WSABUF *,int, unsigned long, int,ASYNCSendId_t, BOOL));
int  MPID_USOCK_TestSend ANSI_ARGS((ASYNCSendId_t));
void MPID_USOCK_WaitSend ANSI_ARGS((ASYNCSendId_t));
void MPID_USOCK_CreateTransfer ANSI_ARGS((void *,int,int,ASYNCRecvId_t)) ;
void MPID_USOCK_ReceiveTransfer ANSI_ARGS((int,ASYNCRecvId_t));
int  MPID_USOCK_TestTransfer ANSI_ARGS((ASYNCRecvId_t));
void MPID_USOCK_WaitTransfer ANSI_ARGS((ASYNCRecvId_t));

/* Error handling */
void MPID_USOCK_Error ANSI_ARGS(( const char *, int ));
void MPID_USOCK_SysError ANSI_ARGS(( const char *, int ));

#endif
