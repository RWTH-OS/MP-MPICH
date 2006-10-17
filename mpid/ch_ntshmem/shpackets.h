
/* 
   This file defines the packet/message format for the shared-memory
   system.
 */

#ifndef MPID_SHPKT_DEF
#define MPID_SHPKT_DEF
#include <stdio.h>
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

/*
   This is a very simple, open, single packet structure.  

   Similar games can be played for other design points.  Note that the 
   lrank could be determined by looking up the absolute rank in the 
   matching context_id; this approach may be cost effective if many small
   messages are sent on a slow system.
 */

#define MPID_MIN(a,b) ((a) < (b) ? (a) : (b))

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

 */
#define MPID_PKT_LAST_MSGSH MPID_PKT_REQUEST_SEND_GETSH
/* all enum types: type -> typeSH Si */
typedef enum {	MPID_PKT_SHORTSH=0,
				MPID_PKT_LONGSH=1,
				MPID_PKT_REQUEST_SENDSH=2,
				MPID_PKT_REQUEST_SEND_GETSH=3,
				MPID_PKT_OK_TO_SENDSH=4,
				MPID_PKT_OK_TO_SEND_GETSH = 5,
				MPID_PKT_CONT_GETSH = 6,
				MPID_PKT_ANTI_SENDSH=7,
				MPID_PKT_ANTI_SEND_OKSH=8,
				MPID_PKT_FLOWSH = 9,
				MPID_PKT_ACK_PROTOSH = 10
} MPID_Pkt_tSH; /* MPID_Pkt_t -> MPID_Pkt_tSHH Si */
/* Comments on packets - see the readme */
   
#define MPID_DO_HETERO(a) 
#define MPID_PKT_MSGREP_DECL 

#ifndef MPID_PKT_PRIVATE
#if defined(MPID_STATISTICS) && defined(LATENCY)
#define MPID_PKT_PRIVATE longlong_t time;
#else
#define MPID_PKT_PRIVATE 
#endif
#endif

#if 1
/* 1.1.2 Version */
/* Note that context_id and lrank may be unused; they are present in 
   case they are needed to fill out the word */
#define MPID_PKT_MODE  \
    MPID_PKT_PRIVATE   \
    volatile int ready;     /* Indicates packet is ready to be read */\
	unsigned mode/*:5*/;             /* Contains MPID_Pkt_t */             \
    unsigned context_id/*:16*/;      /* Context_id */                      \
    unsigned lrank/*:11*/;           /* Local rank in sending context */ 

    
#else
	/* 
   Flow control.  When flow control is enabled, EVERY packet includes
   a flow word (int:32).  This word will (usually) contain two fields
   that indicate how much channel/buffer memory has be used since the 
   last message
 */
#ifdef MPID_FLOW_CONTROL
#define MPID_PKT_FLOW_DECL int flow_info:32;
#else
#define MPID_PKT_FLOW_DECL
#endif


/* Note that context_id and lrank may be unused; they are present in 
   case they are needed to fill out the word */
#define MPID_PKT_MODE  \
    MPID_PKT_PRIVATE   \
    unsigned mode:5;             /* Contains MPID_Pkt_t */             \
    unsigned context_id:16;      /* Context_id */                      \
    unsigned lrank:11;           /* Local rank in sending context */   \
    int to:32;                   /* destination rank */                \
    int seqnum:32;               /* bytes sent / bytes received */     \
    union _MPID_PKT_T *next;     /* link to 'next' packet    */        \
    int owner;                   /* Owner of packet */                 \
    int src:32;                  /* Source of packet in COMM_WORLD system */ \
    MPID_PKT_FLOW_DECL           /* Flow control info */\
	volatile int ready;			 /* Indicates packet is ready to be read */

#endif

#define MPID_PKT_BASIC \
    MPID_PKT_MODE      \
    int      tag /*:32*/;             /* tag is full sizeof(int) */         \
    int      len/*:32*/;             /* Length of DATA */                  


/* If you change the length of the tag field, change the defn of MPID_TAG_UB
   in mpid.h */

#ifndef MPID_PKT_MAX_DATA_SIZE
#define MPID_PKT_MAX_DATA_SIZE 1024
#endif /* MPID_PKT_MAX_DATA_SIZE */


#if defined(SINGLECOPY) || defined(RNDV_STATIC) 
#define MPID_LONG_LEN 8704 /*(8.5*1024)*/
#endif

#define MPID_PKT_IS_MSGSH(mode) ((mode) <= MPID_PKT_LAST_MSGSH)

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
    } MPID_PKT_MODE_TSH; /* type -> typeSH Si */

/* This is the minimal message packet */
typedef struct {
    MPID_PKT_BASIC
    } MPID_PKT_HEAD_TSH; /* type -> typeSH Si */

/* Short messages are sent eagerly (unless Ssend) */
typedef struct { 
    MPID_PKT_BASIC
	  MPID_Aint send_id;       /* Id sent by SENDER, identifies MPI_Request */
    char     buffer[MPID_PKT_MAX_DATA_SIZE];
    } MPID_PKT_SHORT_TSH; /* type -> typeSH Si */

/* Eager message can use this simple packet */
typedef struct {
    MPID_PKT_BASIC
    void        *address;    /* Location of data in shared memory */
	MPID_Aint   send_id;
    } MPID_PKT_SEND_ADDRESS_TSH; /* type -> typeSH Si */

/* Note that recv_id, len_avail, and cur_offset are needed only for
   partial transfers.  The same packet type is
   used for all get operations so that it can be returned to the
   partner for updating. */
typedef struct {
    MPID_PKT_BASIC
    MPID_Aint    send_id;       /* Id sent by SENDER, identifies MPI_Request */
    void         *address;      /* Location of data ON SENDER */
    /* The following support partial sends */
    MPID_Aint    recv_id;       /* Used by receiver for partial gets */
    int          len_avail;     /* Actual length available */
    int          cur_offset;    /* Offset (for sender to use) */
    } MPID_PKT_GET_TSH;  /* MPID_PKT_GET_T -> MPID_PKT_GET_TSH Si */
/* Get done is the same type with a different mode */


#ifdef SINGLECOPY
typedef struct {
    MPID_PKT_BASIC
	unsigned int PID;
	int    *read_complete; /* The address of the flag that indicates completion on sender */
    void    *address;      /* Location of data ON SENDER (in its address room)*/
	MPID_Aint   send_id;
    } MPID_PKT_SEND_LADDRESS_TSH;/* MPID_PKT_SEND_LADDRESS_T ->MPID_PKT_SEND_LADDRESS_TSH Si */
#endif

typedef struct {
    MPID_PKT_BASIC
} MPID_PKT_FLOW_TSH;/* MPID_PKT_FLOW_T -> MPID_PKT_FLOW_TSH Si */

typedef struct {
    MPID_PKT_BASIC
    int          cancel;        /* set to 1 if msg was cancelled - 
				   0 otherwise */
    MPID_Aint    send_id;       /* Id sent by SENDER, identifies MPI_Request */
    MPID_Aint    recv_id;       /* rhandle's address */
} MPID_PKT_ANTI_SEND_TSH;   /* MPID_PKT_ANTI_SEND_T ->MPID_PKT_ANTI_SEND_TSH Si */

/* We may want to make all of the packets an exact size (e.g., memory/cache
   page.  This is done by defining a pad */
#ifndef MPID_PKT_PAD
#define MPID_PKT_PAD 128
#endif


typedef union _MPID_PKT_TSH {
    MPID_PKT_HEAD_TSH          head;  /* type -> typeSH Si */
    MPID_PKT_SHORT_TSH         short_pkt; /* type -> typeSH Si */
    MPID_PKT_SEND_ADDRESS_TSH  sendadd_pkt;  /* type -> typeSH Si */
    MPID_PKT_GET_TSH           get_pkt;  /* MPID_PKT_GET_T -> MPID_PKT_GET_TSH Si */
#ifdef SINGLECOPY
	MPID_PKT_SEND_LADDRESS_TSH lsendadd_pkt;  /* MPID_PKT_SEND_LADDRESS_T ->MPID_PKT_SEND_LADDRESS_TSH Si */
#endif
	MPID_PKT_ANTI_SEND_TSH     antisend_pkt; /* MPID_PKT_ANTI_SEND_T ->MPID_PKT_ANTI_SEND_TSH Si */
    MPID_PKT_FLOW_TSH          flow_pkt;     /* MPID_PKT_FLOW_T ->MPID_PKT_FLOW_TSH Si */
    char                     pad[MPID_PKT_PAD];
    } MPID_PKT_TSH; /* MPID_PKT_T -> MPID_PKT_TSH Si */

extern FILE *MPID_TRACE_FILE;

#ifdef MPID_DEBUG_ALL
#define MPID_TRACE_CODE(name,channel) {if (MPID_TRACE_FILE){\
fprintf( MPID_TRACE_FILE,"[%d] %20s on %4d at %s:%d\n", MPID_MyWorldRank, \
         name, channel, __FILE__, __LINE__ ); fflush( MPID_TRACE_FILE );}}
#define MPID_TRACE_CODE_X(name,longvalue) {if (MPID_TRACE_FILE){\
fprintf( MPID_TRACE_FILE,"[%d] %20s on %4d at %s:%lx\n", MPID_MyWorldRank, \
         name, longvalue, __FILE__, __LINE__ ); fflush( MPID_TRACE_FILE );}}
#define MPID_TRACE_CODE_PKT(name,channel,mode) {if (MPID_TRACE_FILE){\
fprintf( MPID_TRACE_FILE,"[%d] %20s on %4d (type %d) at %s:%d\n", \
	 MPID_MyWorldRank, name, channel, mode, __FILE__, __LINE__ ); \
	 fflush( MPID_TRACE_FILE );}}
#else
#define MPID_TRACE_CODE(name,channel)
#define MPID_TRACE_CODE_X(name,channel)
#define MPID_TRACE_CODE_PKT(name,channel,mode)
#endif


#endif
