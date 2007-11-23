//
//   $Id: sci_priv.h 945 2001-06-15 15:41:49Z joachim $
//     contains all definitions/type related to use of SCI resources
//




//
//   constants 
// 
#define ADSCI_PAGESIZE 4096

#define ADSCI_SIZE_DATASGMT    (64*ADSCI_PAGESIZE)
#define ADSCI_SIZE_LOCKSGMT    (16*ADSCI_PAGESIZE)
#define ADSCI_SIZE_STATEKSGMT  (16*ADSCI_PAGESIZE)
#define ADSCI_SIZE_INTRPTSGMT  (4*ADSCI_PAGESIZE)


//
// types
//

#ifndef _SYS_TYPES_H
typedef unsigned int  uint;
~typedef unsigned long  ulong;
#endif

#define MACC_PRANK_WIDTH   12
#define MACC_SGMT_WIDTH    4
#define MACC_OFFSET_WIDTH  4

typedef struct _sci_connector {
    int node_id;
    int adapter;
    int sgmt_id;
    int intrpt_id;
} SCI_connector;

typedef struct _sci_memlocator {
    uint node_id:MACC_PRANK_WIDTH;
    uint sgmt_id:MACC_SGMT_WIDTH;
    uint offset:MACC_OFFSET_WIDTH;
} SCI_mloc;

typedef struct _sci_memaccessor {
    SCI_memlocator l;
    size_t stride;
} SCI_macc;

//
// globals
//


//
// macros
//
#define NO_FLAGS 0

#define CHECK_SISCI_ERROR (sisci_ec) if ((sisci_ec) != SISCI_ERR_OK) \
                                          fprintf (stderr, "SISCI call returned error 0x%x in %s:%d\n",
						   sisci_ec, __FILE__, __LINE__);

// initialization of memory accessors
#define MACC_INIT (macc, node_id, sgmt, offset, stride) (macc).l.node_id = node_id; \
                                                      (macc).l.sgmt = sgmt;\
                                                      (macc).l.offset = offset;\
                                                      (macc).stride = stride;

#define MLOC_INIT (mloc, node_id, sgmt, offset) (mloc).node_id = node_id; \
                                               (mloc).sgmt = sgmt;\
                                               (mloc).offset = offset;

// locking & unlocking for thread safety
#define SCI_THREAD_SAFE 0
#if SCI_THREAD_SAFE
#define LOCK(l) pthread_mutex_lock(l);
#define UNLOCK(l) pthread_mutex_unlock(l);
#else
#define LOCK(l) 
#define UNLOCK(l) 
#endif

// checked memory allocation 
#define ALLOCATE(ptr,type,size) \
    if (((ptr) = (type) malloc (size)) == NULL) { \
	DERROR("out of memory");\
    	return (MPI_ERR_EXHAUSTED); \
    }

#define CALLOCATE(ptr,type,nelem,elsize) \
    if (((ptr) = (type) calloc (nelem, elsize)) == NULL) { \
	DERROR("out of memory");\
    	return (MPI_ERR_EXHAUSTED); \
    }




// 
// Overrides for XEmacs and vim so that we get a uniform tabbing style.
// XEmacs/vim will notice this stuff at the end of the file and automatically
// adjust the settings for this buffer only.  This must remain at the end
// of the file.
// ---------------------------------------------------------------------------
// Local variables:
// c-indent-level: 3
// c-basic-offset: 3
// tab-width: 3
// End:
// vim:tw=0:ts=3:wm=0:
// 
