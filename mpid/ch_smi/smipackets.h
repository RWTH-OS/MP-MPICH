/* $Id$ */

/* 
   This file defines the packet/message format for the shared-memory
   system.
 */

#ifndef _MPID_SMI_PKT_DEF
#define _MPID_SMI_PKT_DEF

#include <stdio.h>

#include "smidef.h"
#include "smicheck.h"


/* 
   This packet size should be selected such that
   (s + r*(n+h)) + c*n \approx (s+r*n) + s+r*h
   where s = latency, r = time to send a byte, n = total message length, 
   h = header size, and c = time to copy a byte.  This condition reduces to
   c n \approx s
   For a typical system with
   s = 30us
   c = .03us/byte
   this gives
   n = s / c = 30 us / (.03us/byte) = 1000 bytes

   When the message does not fit into a single packet, ALL of the message
   should be placed in the "extension" packet (see below).  This removes 
   an extra copy from the code.
 */

/* macro to initialize the pkt descriptor and to get a send pkt */
#define MPID_GETSENDPKT(pd, _hsz, _hdr, _dsz, _data, _dst, _mode) \
    (pd).hsize  = (unsigned short)(_hsz); \
    (pd).header = (MPID_PKT_T *)(_hdr); \
    (pd).dsize  = _dsz; \
    (pd).data   = _data; \
    (pd).dest   = _dst; \
    MPID_SMI_GetSendPkt(_mode, &(pd));

/* macros to init different packet types */
#define MPID_INIT_PREPKT(pp, _mode, _cntxt_id, _lrank, _tag, _len, _send_id) \
    (pp).mode       = (_mode);     \
    (pp).context_id = (_cntxt_id); \
    (pp).lrank      = (_lrank);    \
    (pp).tag	    = (_tag);      \
    (pp).len	    = (_len);      \
    (pp).send_id    = (_send_id);  

#define MPID_INIT_EAGER_PREPKT(pp, _mode, _cntxt_id, _lrank, _tag, _len, _send_id, _offset) \
    (pp).mode       = (_mode);     \
    (pp).context_id = (_cntxt_id); \
    (pp).lrank      = (_lrank);    \
    (pp).tag	    = (_tag);      \
    (pp).len	    = (_len);      \
    (pp).send_id    = (_send_id);  \
    (pp).offset     = (_offset);

#define MPID_INIT_RNDVREQ_PREPKT(pp,_cntxt_id,_lrank,_tag,_len,_s_id,_sgmt_offset,_r_id,\
                              _sgmt_id,_adpt_nbr,_len_avail,_flags) \
    (pp).mode       = MPID_PKT_REQUEST_SEND; \
    (pp).context_id = (_cntxt_id); \
    (pp).lrank      = (_lrank);    \
    (pp).tag	    = (_tag);      \
    (pp).len	    = (_len);      \
    (pp).send_id    = (_s_id);     \
    (pp).recv_id    = (_r_id);     \
    (pp).sgmt_id    = (_sgmt_id);  \
    (pp).adpt_nbr   = (_adpt_nbr); \
    (pp).sgmt_offset= (_sgmt_offset);  \
    (pp).len_avail  = (_len_avail);\
    (pp).data_offset= 0; \
    (pp).flags      = (_flags); 

#define MPID_INIT_RNDVREQ_RDY_PREPKT(pp,_cntxt_id,_lrank,_tag,_len,_s_id,_sgmt_offset,_r_id,\
                              _sgmt_id,_adpt_nbr,_len_avail,_flags) \
    (pp).mode       = MPID_PKT_REQUEST_SEND_RDY; \
    (pp).context_id = (_cntxt_id); \
    (pp).lrank      = (_lrank);    \
    (pp).tag	    = (_tag);      \
    (pp).len	    = (_len);      \
    (pp).send_id    = (_s_id);     \
    (pp).recv_id    = (_r_id);     \
    (pp).sgmt_id    = (_sgmt_id);  \
    (pp).adpt_nbr   = (_adpt_nbr); \
    (pp).sgmt_offset= (_sgmt_offset);  \
    (pp).len_avail  = (_len_avail);\
    (pp).data_offset= 0; \
    (pp).flags      = (_flags); 

#define MPID_INIT_RNDVOK_PREPKT(pp,_len,_s_id,_sgmt_offset,_r_id,_sgmt_id,_adpt_nbr,_len_avail,\
                                _offset,_flags) \
    (pp).mode       = MPID_PKT_OK_TO_SEND; \
    (pp).context_id = 0; \
    (pp).lrank      = 0; \
    (pp).tag	    = 0; \
    (pp).len	    = (_len);      \
    (pp).send_id    = (_s_id);     \
    (pp).recv_id    = (_r_id);     \
    (pp).sgmt_id    = (_sgmt_id);  \
    (pp).adpt_nbr   = (_adpt_nbr); \
    (pp).sgmt_offset= (_sgmt_offset);     \
    (pp).len_avail  = (_len_avail);\
    (pp).data_offset= (_offset); \
    (pp).flags      = (_flags); 

#define MPID_INIT_RNDVOK_RDY_PREPKT(pp,_len,_s_id,_sgmt_offset,_r_id,_sgmt_id,_adpt_nbr,_len_avail,\
                                _offset,_flags) \
    (pp).mode       = MPID_PKT_OK_TO_SEND_RDY; \
    (pp).context_id = 0; \
    (pp).lrank      = 0; \
    (pp).tag	    = 0; \
    (pp).len	    = (_len);      \
    (pp).send_id    = (_s_id);     \
    (pp).recv_id    = (_r_id);     \
    (pp).sgmt_id    = (_sgmt_id);  \
    (pp).adpt_nbr   = (_adpt_nbr); \
    (pp).sgmt_offset= (_sgmt_offset);     \
    (pp).len_avail  = (_len_avail);\
    (pp).data_offset= (_offset); \
    (pp).flags      = (_flags); 

#define MPID_INIT_RNDVCONT_PREPKT(pp,_len,_s_id,_sgmt_offset,_r_id, _len_avail,_offset,_flags) \
    (pp).mode       = MPID_PKT_CONT; \
    (pp).context_id = 0; \
    (pp).lrank      = 0;    \
    (pp).tag	    = 0;      \
    (pp).len	    = (_len);      \
    (pp).send_id    = (_s_id);     \
    (pp).recv_id    = (_r_id);     \
    (pp).sgmt_id    = 0;  \
    (pp).adpt_nbr   = 0; \
    (pp).sgmt_offset= (_sgmt_offset);     \
    (pp).len_avail  = (_len_avail);\
    (pp).data_offset= (_offset); \
    (pp).flags      = (_flags); 

#define MPID_INIT_RNDVCONT_RDY_PREPKT(pp,_len,_s_id,_sgmt_offset,_r_id, _len_avail,_offset,_flags) \
    (pp).mode       = MPID_PKT_CONT_RDY; \
    (pp).context_id = 0; \
    (pp).lrank      = 0;    \
    (pp).tag	    = 0;      \
    (pp).len	    = (_len);      \
    (pp).send_id    = (_s_id);     \
    (pp).recv_id    = (_r_id);     \
    (pp).sgmt_id    = 0;  \
    (pp).adpt_nbr   = 0; \
    (pp).sgmt_offset= (_sgmt_offset);     \
    (pp).len_avail  = (_len_avail);\
    (pp).data_offset= (_offset); \
    (pp).flags      = (_flags); 

#define MPID_INIT_RNDVPART_PREPKT(pp,_len,_s_id,_sgmt_offset,_r_id, _len_avail,_offset) \
    (pp).mode       = MPID_PKT_PART_READY; \
    (pp).context_id = 0; \
    (pp).lrank      = 0;    \
    (pp).tag	    = 0;      \
    (pp).len	    = (_len);      \
    (pp).send_id    = (_s_id);     \
    (pp).recv_id    = (_r_id);     \
    (pp).sgmt_id    = 0;  \
    (pp).adpt_nbr   = 0; \
    (pp).sgmt_offset= (_sgmt_offset);     \
    (pp).len_avail  = (_len_avail);\
    (pp).data_offset= (_offset); \
    (pp).flags      = 0; 

#define MPID_INIT_CANCEL_PREPKT(pp, _mode, _cntxt_id, _lrank, _tag, _len, _s_id, _cncl, _r_id) \
    (pp).mode       = (_mode);     \
    (pp).context_id = (_cntxt_id); \
    (pp).lrank      = (_lrank);    \
    (pp).tag	    = (_tag);      \
    (pp).len	    = (_len);      \
    (pp).send_id    = (_s_id);     \
    (pp).cancel     = (_cncl);     \
    (pp).recv_id    = (_r_id);

#define MPID_INIT_RSRC_REQ_PREPKT(pp, _cntxt_id, _lrank, _tag, _rsrc_type) \
    (pp).mode       = MPID_PKT_RSRC_REQ; \
    (pp).context_id = (_cntxt_id); \
    (pp).lrank      = (_lrank);    \
    (pp).tag	    = (_tag);      \
    (pp).len	    = 0;           \
    (pp).send_id    = 0;           \
    (pp).rsrc_type  = (_rsrc_type);\
    (pp).have_released = 0;

#define MPID_INIT_RSRC_OK_PREPKT(pp, _cntxt_id, _lrank, _tag, _rsrc_type, _have_released) \
    (pp).mode       = MPID_PKT_RSRC_OK; \
    (pp).context_id = (_cntxt_id); \
    (pp).lrank      = (_lrank);    \
    (pp).tag	    = (_tag);      \
    (pp).len	    = 0;           \
    (pp).send_id    = 0;           \
    (pp).rsrc_type  = (_rsrc_type);\
    (pp).have_released = (_have_released);

#define MPID_INIT_PIPE_PREPKT(pp,  _cntxt_id, _lrank, _len, _sgmt_offset,_sgmt_id,_adpt_nbr,_len_avail,\
                               _flags) \
    (pp).mode       = MPID_PKT_PIPE_READY; \
    (pp).context_id = _cntxt_id; \
    (pp).lrank      = _lrank; \
    (pp).tag	    = 0; \
    (pp).len	    = (int)(_len); \
    (pp).send_id    = NULL; \
    (pp).sgmt_id    = (_sgmt_id);  \
    (pp).adpt_nbr   = (_adpt_nbr); \
    (pp).sgmt_offset= (_sgmt_offset); \
    (pp).len_avail  = (_len_avail);\
    (pp).flags      = (_flags); 

#define MPID_INIT_OS_PREPKT(pp, _cntxt_id, _lrank, _win_id, _ta_type, _accu_op, _data_tag,_nbr_ta, _data_len) \
	(pp).mode         = MPID_PKT_ONESIDED; \
	(pp).context_id   = _cntxt_id; \
	(pp).lrank        = _lrank; \
	(pp).target_winid = _win_id; \
	(pp).ta_type      = _ta_type; \
	(pp).accu_op      = _accu_op; \
	(pp).data_tag     = _data_tag; \
	(pp).nbr_ta       = _nbr_ta; \
	(pp).data_len     = _data_len; 


/*
   This is a very simple, open, single packet structure.  

   Similar games can be played for other design points.  Note that the 
   lrank could be determined by looking up the absolute rank in the 
   matching context_id; this approach may be cost effective if many small
   messages are sent on a slow system.
 */


/* 
   Here are all of the packet types.  

   There is no special support for ready-send messages.  It isn't hard
   to add, but at the level of hardware that a portable implementation
   can reach, there isn't much to do.

   There are three ways to send messages:
   SHORT (data in envelope)
   SEND_ADDRESS (data in shared memory, receiver frees)
   REQUEST_SEND (data not available until sender receives OK_TO_SEND_GET
                 and returns a CONT_GET.  Receiver may return OK_TO_SEND_GET
                 for multiple segments (allows large messages to be
		 sent with limited shared memory). 
   _NB	NRndvn packets

 */
#define MPID_PKT_LAST_MSG MPID_PKT_REQUEST_SEND_RDY
typedef enum { 
    /* short protocol msg */
    MPID_PKT_SHORT = 0, 
    /* eager protocol msg */
    MPID_PKT_SEND_ADDRESS = 1, 
    /* rendez-vous protocol msg (blocking/non-blocking) */
    MPID_PKT_REQUEST_SEND = 2,
	/* ready mode send request (blocking rndv) */
	MPID_PKT_REQUEST_SEND_RDY = 3,
	
	/* this is not a new message, but a request for a different send mode */
    MPID_PKT_REQUEST_SEND_NOZC = 4,

    /* ctrl pkts for non-blocking rndv */
    MPID_PKT_OK_TO_SEND = 5, 
    MPID_PKT_CONT = 6,
    MPID_PKT_PART_READY = 7,
	/* ready mode confirmation */
    MPID_PKT_OK_TO_SEND_RDY = 8, 
    MPID_PKT_CONT_RDY = 9,

    /* cancel packets */
    MPID_PKT_ANTI_SEND = 11,
    MPID_PKT_ANTI_SEND_OK = 12,

    /* packets for single sided communication */
    MPID_PKT_PUT = 13,
    MPID_PKT_GET = 14,
    MPID_PKT_ACCU = 15,
	MPID_PKT_ONESIDED = 16,

	/* resource management */
	MPID_PKT_RSRC_REQ = 20,
	MPID_PKT_RSRC_OK  = 21,

	/* broadcast Ready message */
	MPID_PKT_PIPE_READY = 22,

    /* explicit flow-control (not  used) */
    MPID_PKT_FLOW = 31
} MPID_Pkt_t;

   
#define MPID_PKT_MSGREP_DECL 

/* Explicit flow control.  When flow control is enabled, EVERY packet includes
   a flow word (int:32).  This word will (usually) contain two fields
   that indicate how much channel/buffer memory has be used since the 
   last message.

   Not used for ch_smi.  */
#ifdef MPID_FLOW_CONTROL
#define MPID_PKT_FLOW_DECL \
    int flow_info:32;
#else
#define MPID_PKT_FLOW_DECL 
#endif

/* "size" parameter to MPID_SMI_FreeRecvPkt() for freeing control messages */
#define IS_CTRL_MSG -1

/* Note that context_id and lrank may be unused; they are present in 
   case they are needed to fill out the word */
/* the previous members next, owner and src are not needed for all protocols in ch_smi */
/*    union _MPID_PKT_T *next; */    /* link to 'next' packet */
/*    int owner;               */    /* Owner of packet */
/*    int src:32;              */    /* Source of packet in COMM_WORLD system */
#define MPID_PKT_MODE  \
    unsigned int mode:5;             /* Contains MPID_Pkt_t */             \
    unsigned int context_id:16;      /* Context_id (= communicator) */     \
    unsigned int lrank:11;           /* Local rank in sending context */   \
    MPID_PKT_FLOW_DECL           /* Flow control info */

#define MPID_PKT_BASIC \
    MPID_PKT_MODE      \
    unsigned int      tag:32;             /* tag is full sizeof(int) */         \
    unsigned int      len:32;             /* Length of DATA */                  \
/* If you change the length of the tag field, change the defn of MPID_TAG_UB
   in mpid.h */

#define MPID_PKT_HEAD \
    MPID_PKT_BASIC     \
    MPID_Aint    send_id;     /* Id sent by SENDER, identifies MPI_Request */


#define MPID_PKT_IS_MSG(mode) ((mode) <= MPID_PKT_LAST_MSG)

/* 
   One unanswered question is whether it is better to send the length of
   a short message in the short packet types, or to compute it from the
   message-length provided by the underlying message-passing system.
   Currently, I'm planning to send it.  Note that for short messages, I 
   only need another 2 bytes to hold the length (1 byte if I restrict
   short messages to 255 bytes).  The tradeoff here is additional computation
   at sender and receiver versus reduced data-load on the connection between
   sender and receiver.
 */

/* This is the minimal packet */
typedef struct {
    MPID_PKT_MODE
    } MPID_PKT_MODE_T;

/* This is the minimal message packet */
typedef struct {
    MPID_PKT_HEAD
    } MPID_PKT_HEAD_T;

/* Short messages are sent eagerly (unless Ssend) */
typedef struct {
    MPID_PKT_HEAD
    volatile CSUM_VALUE_TYPE csum;    /* checksum for the data in the first transaction packet*/
#ifdef MPI_LINUX_ALPHA
    /* On Alpha, we need to expand the size of PKT_SHORT_T to 32 byte for better memcpy() performance.
       This means sacrifying short-msg payload for performance.*/
    int      align;
#endif
    char     buffer;        /* the data starts here */
} MPID_PKT_SHORT_T;

#ifndef MPI_LINUX_ALPHA
#define MPID_PKT_SHORT_SIZE (sizeof(MPID_PKT_HEAD_T) + sizeof (CSUM_VALUE_TYPE))
#else
#define MPID_PKT_SHORT_SIZE (sizeof(MPID_PKT_HEAD_T) + sizeof (CSUM_VALUE_TYPE) + sizeof(int))
#endif

/* Eager message can use this simple packet */
typedef struct {
    MPID_PKT_HEAD
    unsigned int offset;    /* Location of data in shared memory */
    } MPID_PKT_SEND_ADDRESS_T;


/* Note that recv_id, len_avail, and data_offset are needed only for
   partial transfers (which are very common, though.
   The same packet is used for all kinds of rendez-vous transfers. */
typedef struct {
    MPID_PKT_HEAD
    MPID_Aint  recv_id;       /* used by receiver for partial gets */
    int        sgmt_id;       /* the (SCI) sgmt id of the receive buffer */
    int        adpt_nbr;      /* the SCI adapter via which the sgmt is exported */
    ulong      sgmt_offset;   /* location of recv buffer inside the segment*/
    ulong      len_avail;     /* actual length available */
    ulong      data_offset;   /* current offset inside the msg to send (for partial transfers) */
	int        flags;         /* indication of buffer types etc. */  
} MPID_PKT_RNDV_T;

typedef struct {
    MPID_PKT_HEAD
    int        sgmt_id;       /* the (SCI) sgmt id of the receive buffer */
    int        adpt_nbr;      /* the SCI adapter via which the sgmt is exported */
    ulong      sgmt_offset;   /* location of recv buffer inside the segment*/
    ulong      len_avail;     /* actual length available */
	int        flags;         /* indication of buffer types etc. */  
} MPID_PKT_PIPE_T;

typedef struct {
    MPID_PKT_HEAD
    MPID_Aint recv_id;
    int  sgmt_id;
    int  adptr_nbr;
    int  sgmt_offset;
} MPID_PKT_ZEROCOPY_T;


typedef struct {
    MPID_PKT_HEAD
    int          cancel;        /* set to 1 if msg was cancelled - 0 otherwise */
    MPID_Aint    recv_id;       /* rhandle's address */
} MPID_PKT_ANTI_SEND_T;

typedef struct {
    MPID_PKT_HEAD
    MPID_SMI_rsrc_type_t  rsrc_type;    /* Ressource type to release / that was released. */
	int have_released;                  /* Flag: did the process release a ressource? */
} MPID_PKT_RSRC_T;

#ifdef MPID_FLOW_CONTROL
#  define MPID_SMI_BASIC_PKT_SIZE 16
#else
#  define MPID_SMI_BASIC_PKT_SIZE 12
#endif
#define MPID_SMI_SENDCTRL_PAD  (MPID_SMI_cfg.SENDCTRL_PAD)

#define MPID_SMI_PKT_MAX_SIZE \
			(MPID_SMI_SCI_TA_SIZE - CSUMHEAD_SIZE - \
			sizeof(MPID_SMI_MSGFLAG_T) - MPID_SMI_BASIC_PKT_SIZE - \
			MPID_SMI_SENDCTRL_PAD - 4)

/*
 * one-sided communication
 */

#define MPID_SMI_PUT_DATA_SIZE    (MPID_SMI_PKT_MAX_SIZE - 12)
#define MPID_SMI_ACCU_DATA_SIZE   (MPID_SMI_PKT_MAX_SIZE - 28)
#define MPID_SMI_PUT_PKT_SIZE     (sizeof (MPID_PKT_PUT_T) - 8)
#define MPID_SMI_ACCU_PKT_SIZE    (sizeof (MPID_PKT_ACCU_T))
#define MPID_SMI_PUT_DATA_OFFSET  (MPID_SMI_PUT_PKT_SIZE)
#define MPID_SMI_ACCU_DATA_OFFSET (MPID_SMI_ACCU_PKT_SIZE)
			
/* immediate data transport */
typedef struct {
	MPID_PKT_BASIC
	int		target_offset:32;
	int		numofdata:32;
	int		target_winid:31;
	int		inline_data:1;
	int		kind_dtype:32;
	int		size_dtype:32;
} MPID_PKT_PUT_T;

typedef struct {
    MPID_PKT_BASIC
	int	target_offset:32;
	int	numofdata:32;
	int	target_winid:31;
	int	do_remote_put:1;
	int	kind_dtype:32;
	int	size_dtype:32;
	int flag_id:32;
	int	flag_offset:32;
	int	origin_offset:32;
} MPID_PKT_GET_T;

typedef struct {
	MPID_PKT_BASIC
	int		target_offset:32;
	int		numofdata:32;
	int		op:32;
	int		kind_dtype:32;
	int		size_dtype:32;
	int		target_winid:31;
	int		inline_data:1;
	int		datasize:32;
} MPID_PKT_ACCU_T;

/* Delayed & gathered data transport for one-sided communication. */
/* XXX For now, only contiguous data is supported. Non-contiguous data
   would require to supply (and transmit, if necessary) the datatypes 
   for each individual transaction. */
typedef struct {
	MPID_PKT_BASIC
	int	   target_winid;

	int    ta_type;    /* put, get or accumulate; request (for all) or response (for get only) */
	int    accu_op;      /* accumulate operation */
	int    dtype;        /* basic MPI datatype for accumulate */

	int    data_tag;     /* tag for the following message containing the data (put, accumulate)
							or for the message to reply with (get) */
	int    nbr_ta;       /* number of transactions that have been gathered */
	int    data_len;    /* length of the data to be received. */
} MPID_PKT_ONESIDED_T;


typedef struct {
    MPID_PKT_BASIC
} MPID_PKT_FLOW_T;


/* We may want to make all of the packets an exact size (e.g., memory/cache
   page.  This is done by defining a pad */
#ifndef MPID_PKT_PAD
#define MPID_PKT_PAD 32
#endif

typedef union _MPID_PKT_T {
    MPID_PKT_HEAD_T          head;
    MPID_PKT_SHORT_T         short_pkt;
    MPID_PKT_SEND_ADDRESS_T  sendadd_pkt;
    MPID_PKT_RNDV_T          rndv_pkt;
	MPID_PKT_PUT_T			 put_pkt;
	MPID_PKT_GET_T			 get_pkt;
	MPID_PKT_ACCU_T			 accu_pkt;
    MPID_PKT_FLOW_T          flow_pkt;
	MPID_PKT_PIPE_T          pcast_pkt;
    char                     pad[MPID_PKT_PAD];
    } MPID_PKT_T;

extern FILE *MPID_TRACE_FILE;

/* data structure for everything MPID_SMI_SendControl() needs */
typedef struct {
    MPID_PKT_T *pkt;         /* ptr to where the packet is written (remote SCI memory) */
    MPID_SMI_MSGID_T msgid;  /* id of this packet */
    int dest;
    unsigned short hsize;         /* for all packets: effective size & data of header */
    MPID_PKT_T *header;
    unsigned int dsize;            /* for short messages only: inlined data */
    char *data;
} MPID_SMI_CTRLPKT_T;

#ifdef MPID_DEBUG_ALL
#define MPID_TRACE_CODE(name,channel) {if (MPID_TRACE_FILE){\
fprintf( MPID_TRACE_FILE,"[%d] %30s : %4d  (%s:%d)\n", MPID_SMI_myid, \
         name, channel, __FILE__, __LINE__ ); fflush( MPID_TRACE_FILE );}}
#define MPID_TRACE_CODE_PKT(name,channel,mode) {if (MPID_TRACE_FILE){\
fprintf( MPID_TRACE_FILE,"[%d] %30s : %4d (type %d)  (%s:%d)\n", \
	 MPID_SMI_myid, name, channel, mode, __FILE__, __LINE__ ); \
	 fflush( MPID_TRACE_FILE );}}
#else
#define MPID_TRACE_CODE(name,channel)
#define MPID_TRACE_CODE_PKT(name,channel,mode)
#endif

#endif


/*
 * Overrides for XEmacs and vim so that we get a uniform tabbing style.
 * XEmacs/vim will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:tw=0:ts=4:wm=0:
 */
