/* $Id$ */

#ifndef _MPID_SMI_EAGER_H
#define _MPID_SMI_EAGER_H

#include "smimem.h"

#include "mpiimpl.h"
#include "mpiops.h"
#include "../util/stack.h"
#include "reqalloc.h"
#include "mpid.h"

#include "smidef.h"
#include "smidebug.h"
#include "smisync.h"
#include "smidev.h"
#include "smistat.h"
#include "smiregionmngmt.h"
#include "direct_ff.h"
#include "smicoll.h"

/* set to '1' to trace eager msg send/recv */
#define DO_EAGER_DEBUG  0

#if DO_EAGER_DEBUG
#define EAGER_DEBUG(dbg) dbg
#else
#define EAGER_DEBUG(dbg) 
#endif

/* the Eager protocol uses shandle->sid[0] to sid[2] for various purposes */
#define EAGERN_ISEND_ID_N_LOCK 0
#define EAGERN_ISEND_TRANSFER  1
#define EAGERN_ISEND_MODE      2

/* constants to indicate if data transfe is complete */
#define EAGERN_ISEND_TRANSFER_UNCOMPLETE 0
#define EAGERN_ISEND_TRANSFER_COMPLETE   1

/* numbers for the three possible Eagern Isend modes, used in 
   shandle->sid[EAGERN_ISEND_MODE] */
#define EAGERN_ISEND_MODE_BLOCKING 0
#define EAGERN_ISEND_MODE_PIO      1
#define EAGERN_ISEND_MODE_DMA      2


/* This information is required to connect to the eager buffers of a process */
typedef struct {
    int sgmt_id;    
    int adptr_nbr;
    int bufsize;    /* size of eager buffers, may differ among processes */
    int nbr_bufs;   /* number of eager buffers, may differ among processes */
    int nbr_cncts;
    int mem_size;   /* bufsize * nbr_bufs => may differ among processes */
    int mem_offset; /* offset into eager memory where "our" buffers begin */
} MPID_SMI_Eagern_connect_info_t;

/* Information about incoming buffers needed to receive eager messages */
typedef struct {
    int bufsize;    /* size of eager buffers, may differ among processes */
    int nbr_bufs;   /* number of eager buffers, may differ among processes */
} MPID_SMI_Eagern_receive_info_t;

/* This structure allows to choose between a fixed number of fixed-sized eager buffers
   and one large ring buffer at startup. */   
/* Added another (void *) to SendCopy and RecvCopy, so for non-contig send/recv
   also the datatype can be handled */
typedef struct {
    int (*init) (void);
    void (*init_complete) (void);
    int (*connect) (int);
    void (*disconnect) (int);
    int (*free_buf) (void *, int, int);
    void* (*get_buf) (int, int);
    void (*sendcpy) (int, void *, void *, int, struct MPIR_DATATYPE *);
    void (*recvcpy) (int, void *, void *, int, struct MPIR_DATATYPE *, struct MPIR_OP *);
    void (*delete) (void);
} MPID_SMI_Eagern_int_t;

/* imports */
/* these variables are declared in smieager.c */
extern MPID_SMI_Eagern_int_t MPID_SMI_Eagern_int;
extern int *MPID_SMI_Shregid_eagerbufs;
extern int *MPID_SMI_eagerseg_connected;
extern int MPID_SMI_Locregid_eagerbufs;
extern char *MPID_SMI_local_eagerbufs;
extern char *MPID_SMI_global_eagerbufs;
extern char **MPID_SMI_incoming_eagerbufs;
extern char **MPID_SMI_outgoing_eagerbufs;
extern int MPID_SMI_eager_disconnects;
extern int *MPID_SMI_eager_align_buf;
extern MPID_SMI_Eagern_connect_info_t *MPID_SMI_Eagern_connect_info;

/* from smishort.c */
extern char **MPID_SMI_shmem_short;

/* declared and initialized in smiinit.c */
extern int *MPID_SMI_localRankForProc;

/* implemented in smideager.c */
int MPID_SMI_dEager_init( void );
void MPID_SMI_dEager_init_complete ( void );
void MPID_SMI_dEager_meminit( int );
int MPID_SMI_dEager_connect( int );
void MPID_SMI_dEager_disconnect( int );
void *MPID_SMI_dEager_get_buf ( int, int );
int MPID_SMI_dEager_free_buf ( void *, int, int );
void MPID_SMI_dEager_sendcpy( int, void *, void *, int, struct MPIR_DATATYPE *);
void MPID_SMI_dEager_recvcpy( int, void *, void *, int, struct MPIR_DATATYPE *, struct MPIR_OP *op_ptr);
void MPID_SMI_dEager_delete( void );

/* implemented in smiseager.c */
int MPID_SMI_sEager_init ( void );
void MPID_SMI_sEager_init_complete ( void );
void MPID_SMI_sEager_meminit ( int );
int MPID_SMI_sEager_connect( int );
void MPID_SMI_sEager_disconnect( int );
void *MPID_SMI_sEager_get_buf ( int, int );
int MPID_SMI_sEager_free_buf ( void *, int, int );
void MPID_SMI_sEager_sendcpy( int, void *, void *, int, struct MPIR_DATATYPE *);
void MPID_SMI_sEager_recvcpy( int, void *, void *, int, struct MPIR_DATATYPE *, struct MPIR_OP *op_ptr);
void MPID_SMI_sEager_delete( void );

void MPID_SMI_Eagern_init_complete(void);

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
 * vim:ts=4:sw=4:
 */
