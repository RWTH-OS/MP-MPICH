/* Definitions for the device only 
   This is an example that can can be used by channel codes
 */
#ifndef MPID_DEV_H
#define MPID_DEV_H

#if !defined(VOLATILE)
#if (HAS_VOLATILE || defined(__STDC__))
#define VOLATILE volatile
#else
#define VOLATILE
#endif
#endif

#include "statistic.h"
#include "dev.h"

/* Globals - For the device */
extern int          MPID_n_pending;
extern MPID_DevSet *MPID_devset;
extern MPID_INFO   *MPID_tinfo;


#include "shpackets.h"
#include "mpid_debug.h"

#define MPIDTRANSPORT "ch_ntshmem"
#define MPIDPATCHLEVEL 2.0

#include "shdef.h"
#define BLOCK
#if defined(WSOCK2) && defined(BLOCK)
extern p2p_lock_t *MPID_Events;
extern int mixed;

#define IDLE(c) {\
	if(mixed&&(c)++>80) {\
		LOG_BLOCK(0);\
		if(WaitForSingleObject(MPID_Events[MPID_MyWorldRank],1)==WAIT_FAILED)\
			printf("WaitFailed %d\n",GetLastError());\
		LOG_BLOCK(1);\
	}\
	if(MPID_DeviceCheck( MPID_NOTBLOCKING ) >0) c=0;\
}
#define SIGNAL(p) if(mixed) SetEvent(MPID_Events[p]);
#else
#define IDLE(c) MPID_DeviceCheck( MPID_NOTBLOCKING );
#define SIGNAL(p)
#endif

#ifdef FOO
/* LOCAL copy of some of MPID_shmem */
int                 MPID_myid = -1;
int                 MPID_numids = 0;
static int	    MPID_pktflush;
extern MPID_SHMEM_globmem  *MPID_shmem;
extern MPID_PKT_T          *MPID_local;
extern VOLATILE MPID_PKT_T **MPID_incoming;
extern MPID_SHMEM_lglobmem MPID_lshmem;
#endif

/* 
   Common macro for checking the actual length (msglen) against the
   declared max length in a handle (dmpi_recv_handle).  
   Resets msglen if it is too long; also sets err to MPI_ERR_TRUNCATE.
   This will set the error field to be added to a handle "soon" 
   (Check for truncation)

   This does NOT call the MPID_ErrorHandler because that is for panic
   situations.
 */
#define MPID_CHK_MSGLEN(rhandle,msglen,err) \
if ((rhandle)->len < (msglen)) {\
    err = MPI_ERR_TRUNCATE;\
    rhandle->s.MPI_ERROR = MPI_ERR_TRUNCATE;\
    msglen = (rhandle)->len;\
    }
#define MPID_CHK_MSGLEN2(actlen,msglen,err) \
if ((actlen) < (msglen)) {\
    err = MPI_ERR_TRUNCATE;\
    msglen = (actlen);\
    }

#ifdef USE_NT2UNIX  /*Si*/
  #define __fastcall
#endif
/* Function prototypes for routines known only to the device */
extern MPID_Device *MPID_SHMEM_InitMsgPass ANSI_ARGS(( int *, char ***, 
						    int, int ));
extern MPID_Protocol *MPID_SHMEM_Short_setup ANSI_ARGS((void));
extern MPID_Protocol *MPID_SHMEM_Eagerb_setup ANSI_ARGS((void));
extern MPID_Protocol *MPID_SHMEM_Rndvb_setup ANSI_ARGS((void));
extern MPID_Protocol *MPID_SHMEM_Eagern_setup ANSI_ARGS((void));
#ifdef SINGLECOPY
extern MPID_Protocol *MPID_SHMEM_LEagern_setup ANSI_ARGS((void));
#endif
extern MPID_Protocol *MPID_SHMEM_Rndvn_setup ANSI_ARGS((void));
extern int MPID_SHMEM_Check_incoming ANSI_ARGS(( MPID_Device *, 
					      MPID_BLOCKING_TYPE));
extern int  MPID_CH_Init_hetero ANSI_ARGS(( int *, char *** ));
extern MPID_PKT_TSH *MPID_SHMEM_GetSendPkt ANSI_ARGS((int)); /* MPID_PKT_T -> MPID_PKT_TSH Si */
extern void *MPID_SetupGetAddress ANSI_ARGS(( void *, int *, int ));
extern void MPID_FreeGetAddress ANSI_ARGS(( void * ));
extern int MPID_PackMessageFree ANSI_ARGS((MPIR_SHANDLE *));
extern void MPID_PackMessage ANSI_ARGS((void *, int, struct MPIR_DATATYPE *, 
					struct MPIR_COMMUNICATOR *, int, 
					MPID_Msgrep_t, MPID_Msg_pack_t, 
					void **, int *, int *));
extern void MPID_UnpackMessageSetup ANSI_ARGS(( int, struct MPIR_DATATYPE *, 
						struct MPIR_COMMUNICATOR *,
						int, MPID_Msgrep_t, void **, 
						int *, int * ));
extern void MPID_SHMEM_init ANSI_ARGS(( int *,char** ));
extern void MPID_SHMEM_finalize ANSI_ARGS(());
extern int MPID_UnpackMessageComplete ANSI_ARGS(( MPIR_RHANDLE * ));
extern void MPID_SHMEM_FreeRndvBuf ANSI_ARGS((int,void *));
extern void *MPID_SHMEM_GetRndvBuf ANSI_ARGS((int));
extern int MPID_SHMEM_IsMemoryShared ANSI_ARGS((void *addr));
extern int MPID_SHMEM_InitEagerBufs ANSI_ARGS((int));
extern int __fastcall MPID_SHMEM_SendControl ANSI_ARGS(( MPID_PKT_TSH *, int, int ));  /* MPID_PKT_T -> MPID_PKT_TSH Si */
extern void MPID_SendCancelPacket ANSI_ARGS(( MPI_Request*,int* ));
extern int MPID_SHMEM_SendCancelPacket ANSI_ARGS(( MPIR_SHANDLE* ));
extern void MPID_SHMEM_SendCancelOkPacket ANSI_ARGS((MPID_PKT_TSH  *,int)); /* MPID_PKT_T -> MPID_PKT_TSH Si */
extern void MPID_SHMEM_RecvCancelOkPacket ANSI_ARGS((MPID_PKT_TSH  *,int)); /* MPID_PKT_T -> MPID_PKT_TSH Si */
extern int MPID_SHMEM_ReadControl ANSI_ARGS(( MPID_PKT_TSH **, int, int *, MPID_BLOCKING_TYPE )); /* MPID_PKT_T -> MPID_PKT_TSH Si */
extern int MPID_SHMEM_CopyFromProcess ANSI_ARGS((unsigned long,void *,void *,unsigned ,void *));
extern void __fastcall MPID_SHMEM_GetSendPacket(MPID_PKT_TSH **,int);  /* MPID_PKT_T -> MPID_PKT_TSH Si */
extern void __fastcall MPID_SHMEM_SetPacketReady(MPID_PKT_TSH *,int) ; /* MPID_PKT_T -> MPID_PKT_TSH Si */
#endif
