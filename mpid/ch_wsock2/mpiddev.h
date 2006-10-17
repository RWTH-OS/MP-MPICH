/* Definitions for the device only 
   This is an example that can can be used by channel codes
 */
#ifndef MPID_DEV_H
#define MPID_DEV_H

#include "dev.h"

#include "statistic.h"

/* Globals - For the device */
extern int          MPID_n_pending;
extern MPID_DevSet *MPID_devset;
extern MPID_INFO   *MPID_tinfo;

#define MPIDTRANSPORT "ch_wsock"

/* packets.h include chdef.h and channel.h */
#include "packets.h"
#include "mpid_debug.h"

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
extern MPID_Device *MPID_CH_InitMsgPass ANSI_ARGS(( int *, char ***, 
						    int, int ));
extern MPID_Protocol *MPID_CH_Short_setup ANSI_ARGS((void));
extern MPID_Protocol *MPID_WSOCK_Eagerb_setup ANSI_ARGS((void));
extern MPID_Protocol *MPID_WSOCK_Rndvb_setup ANSI_ARGS((void));
extern MPID_Protocol *MPID_WSOCK_Eagern_setup ANSI_ARGS((void));
extern MPID_Protocol *MPID_CH_Rndvn_setup ANSI_ARGS((void));
extern int MPID_CH_Check_incoming ANSI_ARGS(( MPID_Device *, 
					      MPID_BLOCKING_TYPE));
extern int  MPID_CH_Init_hetero ANSI_ARGS(( int *, char *** ));
extern void MPID_CH_Pkt_pack ANSI_ARGS(( void *, int, int ));
extern void MPID_CH_Pkt_unpack ANSI_ARGS(( void *, int, int ));

extern int MPID_PackMessageFree ANSI_ARGS((MPIR_SHANDLE *));
extern void MPID_PackMessage ANSI_ARGS((void *, int, struct MPIR_DATATYPE *, 
					struct MPIR_COMMUNICATOR *, int, 
					MPID_Msgrep_t, MPID_Msg_pack_t, 
					void **, int *, int *));
extern void MPID_UnpackMessageSetup ANSI_ARGS(( int, struct MPIR_DATATYPE *, 
						struct MPIR_COMMUNICATOR *,
						int, MPID_Msgrep_t, void **, 
						int *, int * ));
extern int MPID_UnpackMessageComplete ANSI_ARGS(( MPIR_RHANDLE * ));

int MPID_WSOCK_SendCancelPacket ANSI_ARGS(( MPIR_SHANDLE* ));
int MPID_SHMEM_Eagern_send_local( void*, int, int, int, int, int,
							MPID_Msgrep_t, MPIR_SHANDLE *,int,struct MPIR_DATATYPE*);
int MPID_SHMEM_LEagern_isend( void*, int, int, int, int, int,
							MPID_Msgrep_t, MPIR_SHANDLE *,struct MPIR_DATATYPE*);

/* 
   Devices that provide their own datatype handling may need to provide their
   own MPI_Get_count and MPI_Get_elements routines.  They should provide
   MPID_Get_count and MPID_Get_elements, and define the macros
   MPID_HAS_GET_COUNT and MPID_HAS_GET_ELEMENTS in mpid.h .  The 
   definitions of the MPID versions is exactly the same as the MPI versions.
 */
#endif
