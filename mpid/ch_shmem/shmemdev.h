/* $Id$ */
/* Definitions for the device only 
   This is an example that can can be used by channel codes
 */
#ifndef _3452342_SHMEMDEV_H
#define _3452342_SHMEMDEV_H

/* shmem-mpid contains device-specific definitions */
#if defined(HAVE_SHMEM_MPID_H) && !defined(SHMEM_MPID_INC)
#define SHMEM_MPID_INC
#include "shmem-mpid.h"
#endif

#if !defined(VOLATILE)
#if (HAS_VOLATILE || defined(__STDC__))
#define VOLATILE volatile
#else
#define VOLATILE
#endif
#endif

#include "dev.h"

/* Globals - For the device */
extern int          MPID_n_pending;
extern MPID_DevSet *MPID_devset;
extern MPID_INFO   *MPID_tinfo;

#include "shmempackets.h"
#include "shmemdebug.h"

#define MPIDTRANSPORT "ch_shmem"
#define MPIDPATCHLEVEL 2.0

#include "shmemdef.h"

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

/* Function prototypes for routines known only to the device */
extern MPID_Device *MPID_SHMEM_InitMsgPass ( int *, char ***, int, int );
extern MPID_Protocol *MPID_SHMEM_Short_setup (void);
extern MPID_Protocol *MPID_SHMEM_Eagerb_setup (void);
extern MPID_Protocol *MPID_SHMEM_Rndvb_setup (void);
extern MPID_Protocol *MPID_SHMEM_Eagern_setup (void);
extern MPID_Protocol *MPID_SHMEM_Rndvn_setup (void);
extern int MPID_SHMEM_Check_incoming ( MPID_Device *, MPID_BLOCKING_TYPE);
extern int  MPID_CH_Init_hetero ( int *, char *** );
extern MPID_PKT_T *MPID_SHMEM_GetSendPkt (int);
extern void *MPID_SHMEM_SetupGetAddress ( void *, int *, int );
extern void MPID_SHMEM_FreeGetAddress ( void * );
extern int MPID_PackMessageFree (MPIR_SHANDLE *);
extern void MPID_PackMessage (void *, int, struct MPIR_DATATYPE *, 
					struct MPIR_COMMUNICATOR *, int, 
					MPID_Msgrep_t, MPID_Msg_pack_t, 
					void **, int *, int *);
extern void MPID_UnpackMessageSetup ( int, struct MPIR_DATATYPE *, 
						struct MPIR_COMMUNICATOR *,
						int, MPID_Msgrep_t, void **, 
						int *, int * );
extern int MPID_UnpackMessageComplete ( MPIR_RHANDLE * );

/* Internal device routines */
extern int MPID_SHMEM_ReadControl ( MPID_PKT_T **, int, int * );
extern int MPID_SHMEM_SendControl ( MPID_PKT_T *, int, int );
extern void MPID_SHMEM_FreeRecvPkt ( MPID_PKT_T * );

/* Internal debugging routines */
extern int MPID_Print_packet ( FILE *, MPID_PKT_T * );
extern void MPID_Print_rhandle ( FILE *, MPIR_RHANDLE * );
extern void MPID_Print_shandle ( FILE *, MPIR_SHANDLE * );

/* Routines used to cancel sends */
extern int MPID_SHMEM_SendCancelPacket ( MPIR_SHANDLE * );
extern void MPID_SHMEM_SendCancelOkPacket ( MPID_PKT_T *, int );
extern void MPID_SHMEM_RecvCancelOkPacket ( MPID_PKT_T *, int );


/* 
 * We can communicate some information to the device by way of attributes 
 * (communicator construction should have had info!).  The following 
 * include file simply defines the GET/SET operations as empty.
 */

#endif /* _3452342_SHMEMDEV_H */

