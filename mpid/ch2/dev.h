/*
 * $Id$
 *
 * This file defines the device structures.  There are several layers:
 *
 * The protocol: this indicates how a device sends and receives messages using
 * one particular approach
 *
 * The device: this indicates how a device choses particular protocols
 * 
 * The device_set: this is the container for all devices, and indicates
 * how to choose which device to use
 *
 */


#ifndef _MPID_DEV
#define _MPID_DEV

#include "mpichconf.h"

/* needed for MPIR_COMMUNICATOR */
#include "comm.h"

#include "sside_protocol.h"
#include "adi3types.h"

/* if the whole library (MPIR and MPID level) is supposed to be thread-safe 
   -> enable the relevant macros. They are called MPIR..., but cover both
   layers, MPIR and MPID.
   XXX we only support pthreads - is anything else required ?
   XXX is this the right place to define these macros ? */
#ifdef MPIR_USE_LIBTHREADS
#define MPIR_INIT_LOCK(mtx) pthread_mutex_init(&mtx, NULL);
#define MPIR_LOCK(mtx)      pthread_mutex_lock(&mtx);
#define MPIR_UNLOCK(mtx)    pthread_mutex_unlock(&mtx);
#else
/* no multi-threading */
#define MPIR_INIT_LOCK(mtx)
#define MPIR_LOCK(mtx)
#define MPIR_UNLOCK(mtx)
#endif

struct _MPID_Device;

typedef struct _MPID_Protocol MPID_Protocol;
struct _MPID_Protocol { 
    /* The datatype is passed if the protocol supports direct send/recv of non-contignous
       dataypes (see nc_enable in device struct). If not, a NULL ptr is passed. */
    int (*send)        (void *, int, int, int, int, int, MPID_Msgrep_t, struct MPIR_DATATYPE *);
    int (*recv)        (MPIR_RHANDLE *, int, void *);
    int (*isend)       (void *, int, int, int, int, int, MPID_Msgrep_t, MPIR_SHANDLE *, struct MPIR_DATATYPE *);
    int (*wait_send)   (MPIR_SHANDLE *);
    int (*push_send)   (MPIR_SHANDLE *);
    int (*cancel_send) (MPIR_SHANDLE *);
    int (*irecv)       (MPIR_RHANDLE *, int, void *);
    int (*wait_recv)   (MPIR_RHANDLE *, MPI_Status *);
    int (*push_recv)   (MPIR_RHANDLE *);
    int (*cancel_recv) (MPIR_RHANDLE *);
    int (*unex)        (MPIR_RHANDLE *, int, void *);
    int (*do_ack)      (void *, int);
    void (*delete)     (MPID_Protocol *);
};

int MPID_Protocol_call_send(int(void *, int, int, int, int, int, MPID_Msgrep_t, struct MPIR_DATATYPE *),
			    void *, int, int, int, int, int, MPID_Msgrep_t, struct MPIR_DATATYPE *,
			    struct _MPID_Device *this);
int MPID_Protocol_call_recv(int(MPIR_RHANDLE *, int, void *),
			    MPIR_RHANDLE *, int, void *,
			    struct _MPID_Device *this);
int MPID_Protocol_call_isend(int(void *, int, int, int, int, int, MPID_Msgrep_t, MPIR_SHANDLE *, struct MPIR_DATATYPE *),
			     void *, int, int, int, int, int, MPID_Msgrep_t, MPIR_SHANDLE *, struct MPIR_DATATYPE *,
			     struct _MPID_Device *this);
int MPID_Protocol_call_irecv(int(MPIR_RHANDLE *, int, void *),
			     MPIR_RHANDLE *, int, void *,
			     struct _MPID_Device *this);
int MPID_Protocol_call_unex(int(MPIR_RHANDLE *, int, void *),
			    MPIR_RHANDLE *, int, void *,
			    struct _MPID_Device *this);
int MPID_Protocol_call_do_ack(int(void *, int),
			      void*, int,
			      struct _MPID_Device *this);
void MPID_Protocol_call_delete(void(MPID_Protocol *),
			      MPID_Protocol *,
			      struct _MPID_Device *this);


/* 
 * The information on the data formats could be stored with each device,
 * but in some cases (particularly while receiving a packet) we may not want
 * to determine the device first.  In any event, this data is organized by
 * global rank of partner, and is stored separately.  
 * It could be in the MPID_DevSet, but I'm leaving it separate, at least for
 * the moment
 */


/* This is a particular form of device that allows for three protocol breaks  */
typedef struct _MPID_Device MPID_Device;
struct _MPID_Device {
    /* returns threshold value between short and eager protocols */ 
    int (*long_len)( int );
    /* returns threshold value between eager and rndv protovols */ 
    int (*vlong_len)( int );
    /* Standard protocols */
    MPID_Protocol *short_msg, *long_msg, *vlong_msg;
    /* Protocols for special send modes (often the same as above) */
    MPID_Protocol *eager, *rndv, *ready; 
    /* one-/single-sided communication */
    MPID_Sside_protocol	* sside;
    /* Mapping from global ranks to device relative rank.  May be 
       null to use grank directly */
    int *grank_to_devlrank;
    /* MMU functions */
    void *	  (*alloc_mem)(size_t, MPID_Info*);
    int		  (*free_mem)(void*);
    /* Routines to receive header (check/wait header) */
    int           (*check_device)(MPID_Device*, MPID_BLOCKING_TYPE);
    /* Run down and abort - do these need self (device)? */
    int           (*terminate)(MPID_Device *);
    int           (*abort)(struct MPIR_COMMUNICATOR *, int, char *);
    /* canceling messages is device specific */
    int           (*cancel)(MPIR_SHANDLE *);
    /* the device may offer a specific timing function that can be used instead
       of the generic MPI_Wtime implementation */
    double        (*wtime)(void);    
    /* get some kind of version information */
    void          (*get_version)(char *);
    /* device-specific communicator setup */
    int           (*comm_init)(struct MPIR_COMMUNICATOR *, struct MPIR_COMMUNICATOR *);
    int           (*comm_free)(struct MPIR_COMMUNICATOR *);
    /* setup of device-specific collective operations */
    int           (*collops_init)(struct MPIR_COMMUNICATOR *, MPIR_COMM_TYPE);
    /* buffer handling for persistent communication */
    int           (*persistent_init)(union MPIR_HANDLE *);
    int           (*persistent_free)(union MPIR_HANDLE *);
    /* switch for enabling the non-contiguous send/recv 
     0 = disabled, 1 = only for simple types (always MPI conform), 2 = full */
    unsigned int  nc_enable;
    /* device relative rank and number of procs that the device handles */
    int lrank;
    int lsize;
  
    /* pointer to the struct-covered global device data: */
    void*         global_data;

    /* This next field is used to link together all of the devices */
    struct _MPID_Device *next;
};


/* these are wrapper functions between ADI2 and the ch_device which set 
   the active_dev pointer in a multidevice environment: */
int   MPID_Device_call_long_len( int,  struct _MPID_Device *this );
int   MPID_Device_call_vlong_len( int,  struct _MPID_Device *this );
void *MPID_Device_call_alloc_mem(size_t, MPID_Info*,  struct _MPID_Device *this);
int   MPID_Device_call_free_mem(void*,  struct _MPID_Device *this);
int   MPID_Device_call_check_device(MPID_Device*, MPID_BLOCKING_TYPE);
int   MPID_Device_call_terminate(MPID_Device *);
int   MPID_Device_call_abort(struct MPIR_COMMUNICATOR *, int, char *,  struct _MPID_Device *this);
int   MPID_Device_call_cancel(MPIR_SHANDLE *,  struct _MPID_Device *this);
void  MPID_Device_call_get_version(char *,  struct _MPID_Device *this);
int   MPID_Device_call_persistent_init(union MPIR_HANDLE *,  struct _MPID_Device *this);
int   MPID_Device_call_persistent_free(union MPIR_HANDLE *,  struct _MPID_Device *this);


/* This is the container for ALL devices */
typedef struct {
  /* mapping from global ranks to devices.  Many entries in this array will 
     point to the same device */
  int         ndev; /* also used in WSock if shmem is used */
  MPID_Device **dev;
    
  /* List of all DIFFERENT devices.  */
  int         ndev_list;
  MPID_Device *dev_list;

  /* pointer to the currently active device: */
  MPID_Device *active_dev;

  /* These are freed but not completed requests.  We check them from 
     time to time.  This is here because it is global state related 
     to device processing.
  */
  MPI_Request req_pending;

} MPID_DevSet;


/* This is the structure that is used to specifiy the configuration for
   multi-device systems. */
typedef struct _MPID_Config MPID_Config;
struct _MPID_Config {
    /* The routine to initialize the device */
    MPID_Device *(*device_init) ( int *, char ***, int, int );
    /* The name of the routine if device_init is null for dynamic loading */
    char *device_init_name; 
    /* Number of partners served by this device */
    int  num_served;
    /* Array of global ranks served by this device */
    int  *granks_served;
    /* Next device (Null if this is last) */
    MPID_Config *next;
};

#endif
