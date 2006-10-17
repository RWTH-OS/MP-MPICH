//
// $Id: sci_transport.h,v 1.1 2001/11/15 18:41:20 joachim Exp $
//
// Transport arbitrary amounts of data via SCI.
// 

#include <map>

#include "sisci_api.h"

#include "scipriv.h"

enum tmode { PIO_RECV, PIO_SEND, RDMA_RECV, RDMA_SEND, 
	     PIO_FRECV, PIO_FSEND, RDMA_FRECV, RDMA_FSEND };

struct _SCI_mode_handle {
  tmode mode;
} mode_h;

struct _SCI_recv_handle {
  void  *local_buf;           // local destination buffer
  ulong  local_buf_offset;    // current offset in this buffer
  ulong  buf_len;
  
  void *incoming_buf;
  void *fill_buf, *read_buf;

  pthread_mutex_t *complete_lock;
} recv_h;

struct _SCI_frecv_handle {
  void  *incoming_buf;         // incoming buffer for recv operations
  ulong  incoming_buf_offset;
  FILE  *local_file;
  ulong  local_file_offset;    // current offset in this file
  
  pthread_mutex_t *complete_lock;
} frecv_h;

// send from a local buffer to a remote buffer
struct _SCI_send_handle {
  void  *local_buf;           // local src or destination buffer
  ulong  local_buf_offset;    // current offset in this buffer

  pthread_mutex_t *complete_lock; 
} send_h;

struct _SCI_fsend_handle {
  void  **outgoing_buf;         // local outgoing buffers for pipelined send operations
  ulong outgoing_buf_offset;
  FILE  *local_file;
  ulong  local_file_offset;    // current offset in this file  
  SCI_connector remote_buf;

  pthread_mutex_t *complete_lock;
  pthread_mutex_t *lower_lock;
  pthread_mutex_t *upper_lock;
} fsend_h;


union _SCI_thandle {
  mode_h;
  recv_h;
  frecv_h;
  send_h;
  fsend_h;
} SCI_thandle;

typedef SCI_thandle *SCI_thandle_t;

class SCI_transport {
public:
  SCI_transport (SCI_msg &_msg, SCI_memaccessor &_macc, 
		 int pool_size = (1024*1024), int transporter_size = (256*1024));
  ~SCI_transport ();

  // memory-to-memory
  recv (void *dest_buf, size_t len, uint from);
  send (void *src_buf, size_t len, uint to);
  irecv (void *dest_buf, size_t len, uint from, SCI_thandle_t *handle);

  // file-to-memory and vice versa
  frecv (FILE *f, size_t offset, size_t len, uint from);
  fsend (FILE *f, size_t offset, size_t len, uint to);
  ifrecv (FILE *f, size_t offset, size_t len, uint from, SCI_thandle_t *handle);

  // completion
  wait (SCI_thandle_t *handle);
  
private:
  SCI_memaccessor macc;
  SCI_msg msg;
  SCI_connector incoming_buf;
  
}
