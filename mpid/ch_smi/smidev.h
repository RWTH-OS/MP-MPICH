/* $Id$ */

/* Definitions for the device only  */
#ifndef _MPID_SMI_DEV_H
#define _MPID_SMI_DEV_H

#if !defined(VOLATILE)
#if (HAS_VOLATILE || defined(__STDC__))
#define VOLATILE volatile
#else
#define VOLATILE
#endif
#endif

#include "smidef.h"

extern MPID_SMI_LOCK_T MPID_SMI_incoming_lck;
extern MPID_SMI_LOCK_T MPID_SMI_freepkts_lck;
extern MPID_SMI_LOCK_T MPID_SMI_getpkts_lck;
extern MPID_SMI_LOCK_T MPID_SMI_connect_lck;
extern MPID_SMI_LOCK_T MPID_SMI_allocate_lck;
extern MPID_SMI_LOCK_T MPID_SMI_async_check_lck;
extern MPID_SMI_LOCK_T MPID_SMI_counters_lck;
extern MPID_SMI_LOCK_T MPID_SMI_waitmsg_lck;

/* include the generic device datastructures */
#include "dev.h"

/* Globals - For the device */
extern int          MPID_n_pending;
extern MPID_DevSet *MPID_devset;
extern MPID_INFO   *MPID_tinfo;

#include "smipackets.h"
#include "smidebug.h"
#include "smieager.h"
#include "smistat.h"
#include "smiostypes.h"

/* version information */
#define MPIDTRANSPORT "ch_smi 6.0"
#define MPIDPATCHLEVEL 2.9
#define p2p_lock_name "lock-free"

#ifdef MPID_USE_DEVTHREADS
#define MPID_SMI_THREADS_INFO "enabled"
#else
#define MPID_SMI_THREADS_INFO "disabled"
#endif

#ifdef MPID_SMI_STATISTICS
#define MPID_SMI_STATISTICS_INFO "enabled"
#else
#define MPID_SMI_STATISTICS_INFO "disabled"
#endif

#ifdef MPID_DEBUG_ALL  
#define MPID_SMI_DEBUG_INFO "enabled"
#else
#define MPID_SMI_DEBUG_INFO "disabled"
#endif

#include "smidef.h"
#include "smi.h"

/* Abort with some information */
#define MPID_ABORT(msg) { fprintf (stderr, "Internal abort by process %d (%s:%d)\n",  \
                                     MPID_SMI_myid, __FILE__, __LINE__); \
                           MPID_SMI_Abort (NULL, 1, msg); }
/* convenience macro */
#define MPID_ASSERT(cond, msg) if (!(cond)) { MPID_ABORT(msg); }

/* macro to give up processor */
#ifndef WIN32
#include <sched.h>
#include <pthread.h>
#define MPID_SMI_YIELD sched_yield()
#else
#include <winbase.h>
#include <wtypes.h>
#define MPID_SMI_YIELD Sleep(0)
#endif

/* macro for calling SMI functions with exit on failure */
#define SMIcall(SMI_FUNCTION) { smi_error_t SMI_error; \
 char error_msg[128]; \
 if ((SMI_error = SMI_FUNCTION) != SMI_SUCCESS) { \
    sprintf (error_msg, "Aborting with SMI error %d (%s:%d)\n", SMI_error,__FILE__,__LINE__); \
    MPID_SMI_Abort (NULL, -1, error_msg); \
 } }

#define OK_FOR_DMA(_dma_addr,_dma_size) ( (MPID_SMI_DMA_OFFSET_ALIGN != 0) && \
					  (MPID_SMI_DMA_SIZE_ALIGN != 0) && \
					  (((size_t)_dma_addr) % MPID_SMI_DMA_OFFSET_ALIGN == 0) \
                                  && (((size_t)_dma_size) % MPID_SMI_DMA_SIZE_ALIGN == 0) )

/* align size of memory block to multiple of stream-buffer size */
#define MPID_SMI_STREAMBUF_ALIGN(bufsize)    if ((bufsize > 0) && ((bufsize) & (MPID_SMI_STREAMSIZE - 1))) { \
	bufsize = ((bufsize)/MPID_SMI_STREAMSIZE + 1) * MPID_SMI_STREAMSIZE; }

#define MPID_SMI_PAGESIZE_ALIGN(bufsize)    if ((bufsize > 0) && ((bufsize) & (MPID_SMI_PAGESIZE - 1))) { \
	bufsize = ((bufsize)/MPID_SMI_PAGESIZE + 1) * MPID_SMI_PAGESIZE; }

/* from smishort.c */
extern MPID_SMI_lShortrecv_t *MPID_SMI_lShortrecv;
extern MPID_SMI_lShortsend_t *MPID_SMI_lShortsend;


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
extern MPID_Device *MPID_CH_SMI_InitMsgPass ( int *, char ***, int, int );

/* from smipriv.h */
unsigned int MPID_SMI_ld(unsigned int);
unsigned int MPID_SMI_rotate_right(unsigned int, unsigned int, unsigned int);
unsigned int MPID_SMI_Get_Slots(unsigned int);
unsigned int MPID_SMI_msb(unsigned int);
void MPID_SMI_Lowlevel_Init(void);
void *MPID_SMI_shmalloc(int, int);
void MPID_SMI_shfree(char *);
void MPID_SMI_lbarrier(void);
void MPID_SMI_Get_version(char *);
void MPID_SMI_finalize(void);
int MPID_SMI_pkt_avail(void);
int MPID_SMI_ReadControl(MPID_PKT_T **, int *);
void MPID_SMI_FreeRecvPkt(MPID_PKT_T *, int, int);
int MPID_SMI_GetSendPkt(int, MPID_SMI_CTRLPKT_T *);
int MPID_SMI_SendControl(MPID_SMI_CTRLPKT_T *);
void MPID_SMI_Send_postpone (MPIR_SHANDLE *);
void MPID_SMI_Recv_postpone (MPIR_RHANDLE *);
void MPID_SMI_Check_postponed(void);

/* other */
unsigned long MPID_SMI_crc32(unsigned char *buf, int len, unsigned char id);
int MPID_SMI_Collops_init(struct MPIR_COMMUNICATOR*, MPIR_COMM_TYPE);
MPID_Protocol *MPID_SMI_Short_setup (void);
MPID_Protocol *MPID_SMI_Eagerb_setup (void);
MPID_Protocol *MPID_SMI_Rndvb_setup (void);
MPID_Protocol *MPID_SMI_Eagern_setup (void);
MPID_Protocol *MPID_SMI_Rndv_setup (void);
void MPID_SMI_Brndv_memsetup (void);

int MPID_SMI_Check_incoming ( MPID_Device *, MPID_BLOCKING_TYPE);
void *MPID_SMI_Async_devcheck (void *args);

void MPID_SMI_Rndv_free_recvbuf ( ulong sgmt_offset, int from_drank );
int MPID_SMI_Nbrndv_push_recv( void *, int );
int MPID_SMI_Rndvn_irecv_nozc (void *in_pkt, int from_grank );

int MPID_SMI_Persistent_init (union MPIR_HANDLE *req);
int MPID_SMI_Persistent_free (union MPIR_HANDLE *req);

int MPID_PackMessageFree (MPIR_SHANDLE *);
void MPID_PackMessage (void *, int, struct MPIR_DATATYPE *, 
			      struct MPIR_COMMUNICATOR *, int, 
			      MPID_Msgrep_t, MPID_Msg_pack_t, 
			      void **, int *, int *);
void MPID_UnpackMessageSetup ( int, struct MPIR_DATATYPE *, 
				      struct MPIR_COMMUNICATOR *,
				      int, MPID_Msgrep_t, void **, 
				      int *, int * );
int MPID_UnpackMessageComplete ( MPIR_RHANDLE * );

/* from smicancel.c */
int MPID_SMI_SendCancelPacket(MPIR_SHANDLE *);
void MPID_SMI_SendCancelOkPacket(MPID_PKT_T  *, int);
void MPID_SMI_RecvCancelOkPacket(MPID_PKT_T *, int);

/* from smidelayedos.c */
int MPID_SMI_Os_delayed_store (MPID_Win *, MPID_SMI_Delayed_ta_t *);
int MPID_SMI_Os_delayed_flush (MPID_Win *, int);
int MPID_SMI_Os_delayed_process (MPID_PKT_T *, int);

/* Internal device routines */
void MPID_SMI_init (void);
int MPID_SMI_Shmem_init ( void );
int MPID_SMI_Short_init ( void );
void MPID_SMI_Short_GetConfig (int *, int *);
int MPID_SMI_Eagern_init ( void );
void MPID_SMI_Eagern_GetConfig (int *, int *);
void MPID_SMI_Eagern_preconnect( void );
int MPID_SMI_Rndvn_init ( void  );
void MPID_SMI_Rndvn_GetConfig (int *, int *);
int MPID_SMI_Abort ( struct MPIR_COMMUNICATOR *, int, char * );

void MPID_SMI_rsrc_req( void *, int  );
void MPID_SMI_rsrc_ack( void *, int  );


/* needed in smieager.c */
int MPID_SMI_Rndv_isend ( void *, int, int, int, int, int, 
				  MPID_Msgrep_t, MPIR_SHANDLE *, struct MPIR_DATATYPE * );
int MPID_SMI_Rndv_send ( void *, int, int, int, int, int, MPID_Msgrep_t, struct MPIR_DATATYPE * );
void *MPID_SMI_locmalloc ( int );

/* sending a message to myself */
int MPID_SMI_Send_self ( void *, int, int, int, int, int, 
					  MPID_Msgrep_t);
int MPID_SMI_Isend_self ( void *, int, int, int, int, int, 
					  MPID_Msgrep_t, MPIR_SHANDLE *);

/* Internal debugging routines */
extern int MPID_Print_packet ( FILE *, MPID_PKT_T * );
extern void MPID_Print_rhandle ( FILE *, MPIR_RHANDLE * );
extern void MPID_Print_shandle ( FILE *, MPIR_SHANDLE * );

#endif
