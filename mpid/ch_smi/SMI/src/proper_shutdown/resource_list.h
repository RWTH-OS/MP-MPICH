/* $Id$ */

#ifndef _SMI_RESOURCE_LIST_H_
#define _SMI_RESOURCE_LIST_H_

#include <stdlib.h>

#include "env/general_definitions.h"


typedef struct rs_tempfile_t_ {
  char szName[256];
  int fd;
} rs_tempfile_t;

typedef struct rs_tempstream_t_ {
  char szName[256];
  FILE *fd;
} rs_tempstream_t;

typedef struct rs_thread_create_t_ {
    void *(*start_routine)(void *);
    void *arg;
} rs_thread_create_t;

typedef enum  rs_resource_type_t_ {
#ifndef NO_SISCI
  rtSciDesc,
  rtDmaQueue,
  rtLocInterrupt,
  rtRmtInterrupt,
  rtSequence,
  rtSegment,
  rtConnection,
  rtMap,
#endif 
  rtSharedMemory,
  rtTempFile,
  rtTempStream,
#ifndef DISABLE_THREADS
  rtThread,
#endif
  rtNone
} rs_resource_type_t;

typedef union rs_resource_t_ {
#ifndef NO_SISCI
  sci_desc_t SciDesc;
  sci_dma_queue_t DmaQueue;
  sci_local_interrupt_t LocInterrupt;
  sci_remote_interrupt_t RmtInterrupt;
  sci_sequence_t Sequence;
  sci_local_segment_t Segment;
  sci_remote_segment_t Connection;
  sci_map_t Map;
#endif 
  int ShmId;
  rs_tempfile_t TempFile;
  rs_tempstream_t TempStream;
#ifndef DISABLE_THREADS
  pthread_t thread_id;
#endif
  void* None;
} rs_resource_t;

typedef struct rs_node_t_ {
  rs_resource_t Resource;
  rs_resource_type_t ResourceType;
  struct rs_node_t_ *pNext;
} rs_node_t;

/* module-internal prototypes  */
int rs_is_equal(rs_node_t* pNode,rs_resource_t Resource, rs_resource_type_t ResourceType);
void rs_mk_resource(rs_resource_t* newResource, void *pResource, rs_resource_type_t ResourceType);
rs_node_t* rs_node_search_pre(rs_node_t* pRoot, void* pResource, rs_resource_type_t ResourceType);
rs_node_t* rs_node_search(rs_node_t* pRoot, void* pResource, rs_resource_type_t ResourceType);
int rs_node_push(rs_node_t* pRoot, void* pResource, rs_resource_type_t ResourceType);
int rs_node_pop(rs_node_t* pRoot, rs_resource_t* pResource, rs_resource_type_t* pResourceType);
void rs_FreeResource(rs_resource_t Resource, rs_resource_type_t ResourceType, sci_error_t* error); 
int rs_node_remove(rs_node_t* pRoot, void* pResource, rs_resource_type_t ResourceType);

/* SMI-internal prototypes */
int _smi_init_resource_list(void);
int _smi_clear_all_resources(void);

#ifndef NO_SISCI
void rs_SCIOpen(sci_desc_t   *sd,
		unsigned int flags,
		sci_error_t  *error);

void rs_SCIClose(sci_desc_t sd,
                  unsigned int flags,
                  sci_error_t *error);

void rs_SCICreateSegment(sci_desc_t             sd,
		      sci_local_segment_t    *segment,
		      unsigned int           segmentId,
		      size_t                 size,
		      sci_cb_local_segment_t callback,
		      void                   *callbackArg, 
		      unsigned int           flags,
		      sci_error_t            *error);

void rs_SCIConnectSegment( sci_desc_t sd,
			   sci_remote_segment_t* segment,
			   unsigned int nodeId,
			   unsigned int segmentId,
			   unsigned int localAdapterNo,
			   sci_cb_remote_segment_t callback,
			   void* callbackArg,
			   unsigned int timeout,
			   unsigned int flags,
			   sci_error_t* error);

volatile void *rs_SCIMapLocalSegment (sci_local_segment_t segment,
                             sci_map_t           *map,
                             size_t              offset,
                             size_t		         size,
                             void                *addr,
                             unsigned int        flags,
                             sci_error_t         *error);

volatile void *rs_SCIMapRemoteSegment (sci_remote_segment_t segment,
                              sci_map_t            *map,
                              size_t               offset,
                              size_t               size,
                              void                *addr,
                              unsigned int         flags,
                              sci_error_t          *error);

void rs_SCIUnmapSegment(sci_map_t    map,
                         unsigned int flags,
                         sci_error_t  *error);

void rs_SCIDisconnectSegment( sci_remote_segment_t segment,
			      unsigned int flags,
			      sci_error_t* error);

void rs_SCIRemoveSegment(sci_local_segment_t segment,
                          unsigned int        flags, 
                          sci_error_t         *error);

void rs_SCICreateMapSequence(sci_map_t   map, 
                           sci_sequence_t *sequence, 
                           unsigned int   flags, 
                           sci_error_t    *error);

void rs_SCIRemoveSequence(sci_sequence_t sequence, 
                           unsigned int   flags, 
                           sci_error_t    *error);

void rs_SCICreateDMAQueue(sci_desc_t      sd,
                           sci_dma_queue_t *dq,
                           unsigned int    localAdapterNo,
                           size_t          maxEntries,
                           unsigned int    flags,
                           sci_error_t     *error);

void rs_SCIRemoveDMAQueue(sci_dma_queue_t dq,
			  unsigned int    flags,
			  sci_error_t     *error);

void rs_SCICreateInterrupt(sci_desc_t            sd,
                            sci_local_interrupt_t *interrupt,
                            unsigned int          localAdapterNo,
                            unsigned int          *interruptNo,
                            sci_cb_interrupt_t    callback,
                            void                  *callbackArg,
                            unsigned int          flags,
                            sci_error_t           *error);

void rs_SCIRemoveInterrupt( sci_local_interrupt_t interrupt,
                            unsigned int          flags,
                            sci_error_t           *error);

void rs_SCIConnectInterrupt(sci_desc_t            sd,
                            sci_remote_interrupt_t *interrupt,
			    unsigned int           nodeId,
			    unsigned int           localAdapterNo,
			    unsigned int           interruptNo,
			    unsigned int           timeout,
			    unsigned int           flags,
			    sci_error_t            *error);

void rs_SCIDisconnectInterrupt(sci_remote_interrupt_t interrupt,
			       unsigned int          flags,
			       sci_error_t           *error);
#endif 

int rs_shmget(key_t key, size_t size, int shmflg);
int rs_shmctl(int shmid, int cmd, struct shmid_ds *buf);

int rs_CreateTempfile(const char* szName, int oflag);
void rs_RemoveTempfile(char* szName);
FILE *rs_CreateTempstream(const char* szName, const char *mode);
void rs_RemoveTempstream(char* szName);

#ifndef DISABLE_THREADS
int rs_pthread_create (pthread_t  * thread, pthread_attr_t * attr, 
		       void * (*start_routine)(void *), void * arg);
void rs_pthread_exit(void *retval);
int rs_pthread_cancel(pthread_t thread);
int rs_pthread_join(pthread_t th, void **thread_return);
#endif

#endif



