/*
 * $Id: mpxpriv.h 3227 2005-03-15 13:51:10Z carsten $
 *
 */

#ifndef __MPXPRIV_H__
#define __MPXPRIV_H__

/* OLD NT2UNIX STUFF: */
typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef struct _WSABUF{
  unsigned long  len;
  char           *buf;
}WSABUF;

typedef int SOCKET;


#include "mpid.h"
#include "mpxpackets.h"

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
struct _MPXHeader {
	unsigned int Type;
	int	     Tag;
	unsigned int Size;
} MPXHeader;


void MPID_MPX_Init ANSI_ARGS(( int *, char *** ));
void MPID_MPX_Finalize  ANSI_ARGS((void));
void MPID_MPX_RecvAnyControl ANSI_ARGS(( MPID_PKT_T *, int , int *));
int  MPID_MPX_ControlMsgAvail ANSI_ARGS(( void ));
int  MPID_MPX_RecvFromChannelAsync ANSI_ARGS(( void*,int,int,ASYNCRecvId_t));
void MPID_MPX_RecvFromChannel ANSI_ARGS(( void *, int, int ));
void MPID_MPX_ConsumeData ANSI_ARGS((int, int ));
void MPID_MPX_SendControl ANSI_ARGS((MPID_PKT_T*,int,int));
/*
int  MPID_MPX_SendChannel ANSI_ARGS((WSABUF *,int, unsigned long, int,ASYNCSendId_t, BOOL));
*/
int  MPID_MPX_SendChannel(WSABUF, int, int, ASYNCSendId_t, BOOL);
int  MPID_MPX_TestSend ANSI_ARGS((ASYNCSendId_t));
void MPID_MPX_WaitSend ANSI_ARGS((ASYNCSendId_t));
void MPID_MPX_CreateTransfer ANSI_ARGS((void *,int,int,ASYNCRecvId_t)) ;
void MPID_MPX_ReceiveTransfer ANSI_ARGS((int,ASYNCRecvId_t));
int  MPID_MPX_TestTransfer ANSI_ARGS((ASYNCRecvId_t));
void MPID_MPX_WaitTransfer ANSI_ARGS((ASYNCRecvId_t));

/* Error handling */
void MPID_MPX_Error ANSI_ARGS(( const char *, int ));
void MPID_MPX_SysError ANSI_ARGS(( const char *, int ));

#endif
