#ifndef _MPID_REQ_H
#define _MPID_REQ_H

/*
 * $Id$
 *
 * Modified definitions of the request.  The "device" handle has been 
 * integrated.
 * 
 * Should consider separating persistent from non-persistent requests.
 * Note that much of the information a send request is not needed
 * once the send envelope is sent, unless it is a persistent request.
 * This includes the contextid, dest, tag, and datatype.  The comm
 * might be needed for error handling, but maybe not.
 *
 * If persistent is a handle_type, we save another store.
 */

#include "mpiops.h"

#ifndef MPIR_REQUEST_COOKIE

/* reqrndv.h contains the type definition for MPID_RNDV_T. The default 
   type is defined to "long", but it can be replaced by device-specific
   types by putting the required typedef into reqrndv.h . 
   NOTE: the size of the new type must not exceed sizeof(long) to avoid 
         differently sized handles in multiple devices. This means a 
         pointer to a specific structure must be used. */
#include <reqrndv.h>

#include "datatype.h"

#ifdef CH_GM_PRESENT
#include "../mpid/ch_gm/gmpi_debug_checksum_type.h"
#endif

/* MPIR_COMMON is a subset of the handle structures that contains JUST
   the handle type and the Cookie. */
/* User not yet supported */
typedef enum {
    MPIR_SEND,
    MPIR_RECV,
    MPIR_PERSISTENT_SEND,
    MPIR_PERSISTENT_RECV /*,
    MPIR_USER */
} MPIR_OPTYPE;

#define MPIR_REQUEST_COOKIE 0xe0a1beaf
typedef struct {
    MPIR_OPTYPE handle_type;
    MPIR_COOKIE                 /* Cookie to help detect valid item */
    int is_complete;
    int self_index;             /* Used when mapping to/from indices */
    int ref_count;              /* Used to handle freed (by user) but
				   not complete */
    } MPIR_COMMON;

/* In the case of a send handle, the information is provided separately.
   All we need is enough information to dispatch the message and to 
   deliver the data (in a rendezvous/get setting).  Some sophisticated
   devices may be able to use an MPI_Datatype directly; they should
   add an MPI_Datatype field if they need it. */
typedef struct _MPIR_SHANDLE MPIR_SHANDLE;
struct _MPIR_SHANDLE {
    MPIR_OPTYPE  handle_type;    
    MPIR_COOKIE                   /* Cookie to help detect valid item */
    int          is_complete;     /* Indicates if complete */
    int          self_index;      /* Used when mapping to/from indices */
    int          ref_count;       /* Used to handle freed (by user) but  not complete */
    int          is_cancelled;    /* Indicates message is cancelled */
    int          cancel_complete; /* Indicates when cancel call has  completed */
    int          partner;         /* Holds rank of process - used to cancel isend requests */
    int          partner_grank;   /* for cancelling via gateway device, we save here the rank in MPID_COMM_ALL */
    int          errval;          /* Holds any error code; 0 for none */
    unsigned int msgid;           /* used for cancelling via gateway device */
    MPI_Comm     comm;            /* Do we need this?  */
    MPI_Status   s;               /* Status of data */

    /* To postpone (and subsequently resume) send operations, the complete msg 
       information needs to be stored in the shandle. The msgrep and tag are 
       not required for other things here. */
    MPID_Msgrep_t msgrep;
    int           tag;
    int           context_id;
    int           src_lrank;


#if (defined WIN32)
    volatile int *flag;  /* this is for the WIN32 shmem protocols only */
#endif

    /* Device data */
    int           is_non_blocking;
    /* The following describes the buffer to be sent.  This may be ignored, 
       depending on the device. */
    void          *start,
	          *curr_buf; /* used for packing/unpacking in the device*/
    int		  act_index;
    int           bytes_as_contig,
	          rndv_buf_len;
    struct MPIR_DATATYPE* datatype;
        /* Rest of data */
    ASYNCSendId_t sid;              /* Id of non-blocking send, if used.
				       0 if no non-blocking send used, 
				       or if non-blocking send has 
				       completed */
    MPID_RNDV_T   recv_handle;      /* Holds 'transfer' handle for RNDV 
				       operations */
#ifdef CH_GM_PRESENT
    struct
      {
	void * ptr;
	unsigned long current_expected;
	unsigned long current_offset;
	unsigned long current_done;
	unsigned int rendez_vous;
      } gm;
#endif 

    /* New fields are the functions to call */
    /* Is test a field or a function ? */
    int (*test)   (MPIR_SHANDLE *);
    int (*push)   (MPIR_SHANDLE *);
    int (*wait)   (MPIR_SHANDLE *);
    int (*cancel) (MPIR_SHANDLE *);
    int (*finish) (MPIR_SHANDLE *);

  void* dev; /* <-- Pointer to the respective device */

    void *bsend;                /* Pointer to structure managed for buffered sends */
    volatile int is_valid;      /* For a new instance, this is set to false until all entries are valid 
				   (only required for multi-threaded devices) */
};


/* A receive request is VERY different from a send.  We need to 
 * keep the information about the message we want in the request,
 * as well as how to accept it.
 */
typedef struct _MPIR_RHANDLE MPIR_RHANDLE;
struct _MPIR_RHANDLE {
    MPIR_OPTYPE  handle_type;    
    MPIR_COOKIE                /* Cookie to help detect valid item */
    int          is_complete;  /* Indicates is complete */
    int          self_index;   /* Used when mapping to/from indices */
    int          ref_count;    /* Used to handle freed (by user) but
				  not complete */
    MPI_Status   s;            /* Status of data */
    int          contextid;    /* context id -- why? */
    void         *buf;         /* address of buffer */
    int          len;          /* length of buffer at bufadd in bytes */
    int          partner;      /* Holds rank of process - used for 
				  rendevous unexpected messages. 
				  XXX: what about 'from'? Seems to be the same! */

    /* Device data */
    int           is_non_blocking;
        /* Rest of data */
    ASYNCRecvId_t rid;              /* Id of non-blocking recv, if used.
				       0 if no non-blocking recv used.*/
    MPID_Aint     send_id;          /* Used for rendevous send; this is
				       needed when the incoming message
				       is unexpected. */
    MPID_RNDV_T   recv_handle;      /* Holds 'transfer' handle for RNDV 
				       operations */
    char          *unex_buf;        /* Holds body of unexpected message */
    int           from;             /* Absolute process number that sent
				       message; used only in rendevous 
				       messages */
  
#ifdef CH_GM_PRESENT
    struct
      {
	char *netbuf;
	unsigned long current_expected;
	unsigned long current_offset;
	unsigned long current_done;
	unsigned int in_pipe;
        struct pkt_cksum cksum;
      } gm;
#endif

        /* The following describes the user buffer to be received */
    void         *start;
    int          bytes_as_contig;
    int          count;
    struct MPIR_DATATYPE *datatype;   /* basic or derived datatype */
    struct MPIR_COMMUNICATOR *comm;
    MPID_Msgrep_t msgrep;     /* Message representation; used to indicate
				XDR, sender, receiver used */

    /* New fields are the functions to call */
    /* Is test a field or a function ? */
    int (*test)   (MPIR_RHANDLE *);
    /* Push is called to advance the completion; the second arg may be
       a pkt or, in the case of an unexpected receive, the saved request.
       In the unexpected case, the saved request may already be complete */
    int (*push)   (MPIR_RHANDLE *, void *);
    /* Status is saved in the request */
    int (*wait)   (MPIR_RHANDLE *);
    int (*cancel) (MPIR_RHANDLE *);
    int (*finish) (MPIR_RHANDLE *);

    void* dev; /* <-- Pointer to the respective device */

    /* For optimized implementations of reductions: perform the reduce 
       operation from within the incoming buffer, saving one memory copy
       -> pseudo-zero-copy! */
    struct MPIR_OP *op_ptr;
    int op_cnt;

    volatile int is_valid;      /* For a new instance, this is set to false until all entries 
				   are valid (only required for multi-threaded devices) */
};

typedef struct {
    MPIR_RHANDLE rhandle;
    int          active;
    int          perm_tag, perm_source, perm_count;
    void         *perm_buf;
    struct MPIR_DATATYPE *perm_datatype;
    struct MPIR_COMMUNICATOR *perm_comm;
    } MPIR_PRHANDLE;

typedef struct {
    MPIR_SHANDLE shandle;
    int          active;
    int          perm_tag, perm_dest, perm_count;
    void         *perm_buf;
    struct MPIR_DATATYPE *perm_datatype;
    struct MPIR_COMMUNICATOR *perm_comm;
    void         (*send) (struct MPIR_COMMUNICATOR *, void *, int, 
				    struct MPIR_DATATYPE *,
			  int, int, int, int, MPI_Request, int *, int );
                    /* IsendDatatype, IssendDatatype, Ibsend, IrsendDatatype */
    } MPIR_PSHANDLE;
	
/* This is an "extension" handle and is NOT part of the MPI standard.
   Defining it, however, introduces no problems with the standard, and
   it allows us to easily extent the request types.

   Note that this is not yet compatible with the essential fields of
   MPIR_COMMON.
 */
typedef struct {
    MPIR_OPTYPE handle_type;    
    MPIR_COOKIE                 /* Cookie to help detect valid item */
    int         is_complete;    /* Is request complete? */
    int         self_index;     /* Used when mapping to/from indices */
    int         ref_count;      /* Used to handle freed (by user) but
				   not complete */
    int         active;         /* Should this be ignored? */
    int         (*create_ureq) (MPI_Request);
    int         (*free_ureq)   (MPI_Request);
    int         (*wait_ureq)   (MPI_Request);
    int         (*test_ureq)   (MPI_Request);
    int         (*start_ureq)  (MPI_Request);
    int         (*cancel_ureq) (MPI_Request);
    void        *private_data;
} MPIR_UHANDLE;

#define MPIR_HANDLES_DEFINED

union MPIR_HANDLE {
    MPIR_OPTYPE   handle_type;   
    MPIR_COMMON   chandle;       /* common fields */
    MPIR_SHANDLE  shandle;
    MPIR_RHANDLE  rhandle;
    MPIR_PSHANDLE persistent_shandle;
    MPIR_PRHANDLE persistent_rhandle;
    MPIR_UHANDLE  uhandle;
};

#ifdef STDC_HEADERS
/* Prototype for memset() */
#include <string.h>
#elif defined(HAVE_STRING_H)
#include <string.h>
#elif defined(HAVE_MEMORY_H)
#include <memory.h>
#endif

#if 0 /* Do not use the macro used by mpich original */
      /* mp-mpich has a more complex init function   */
#ifdef DEBUG_INIT_MEM
#define MPID_Request_init( ptr, in_type ) { \
		      memset(ptr,0,sizeof(*(ptr)));\
		      (ptr)->handle_type = in_type;\
                      (ptr)->ref_count = 1;\
		      MPIR_SET_COOKIE((ptr),MPIR_REQUEST_COOKIE);}
#else
/* For now, turn on deliberate garbaging of memory to catch problems */
/* We should do this, but I want to get the release out */
/* 		      memset(ptr,0xfc,sizeof(*(ptr))); */
#define MPID_Request_init( ptr, in_type ) { \
		      memset(ptr,0,sizeof(*(ptr)));\
		      (ptr)->handle_type = in_type;\
                      (ptr)->ref_count = 1;\
		      MPIR_SET_COOKIE((ptr),MPIR_REQUEST_COOKIE);}
#endif
#endif /* 0 */

/* These have been added to provide a procedural interface to these 
   fields; the Globus device implements these in a different way */
#define MPID_SendRequestCancelled(r) ((r)->shandle.s.MPI_TAG == MPIR_MSG_CANCELLED)
#define MPID_SendRequestErrval(r) (r)->s.MPI_ERROR

#endif

#endif
