//
// $Id: sci_msg.h 945 2001-06-15 15:41:49Z joachim $
//
// SCI_msg is a low-level messaging facility for sending and receiving fixed-size
// messages via SCI. 
//

#include <vector>
#include <stack>

#include <sisci_api.h>

#define SCI_MSG_USE_CHECKSUM 0

#define SCI_MSG_DEFAULT_SLOTSIZE 128
#define SCI_MSG_MAX_SLOTSIZE     1024
#define SCI_MSG_DEFAULT_NBRSLOTS 16
#define SCI_MSG_MAXID            1021

#define SCI_MSG_TAG_MEMINIT  0

typedef uint SCI_msgid;
typedef uint SCI_csum;
typedef char[SCI_MSG_MAX_SLOTSIZE] SCI_maxslot;

//
// SCI packet (base of a message) 
//
typedef struct _sci_packet {
	SCI_msgid id;
#if SCI_MSG_USE_CHECKSUM
	SCI_csum csum;
#endif
	uint tag;
	uint len;
	char data;
} SCI_packet;
	
#define SCI_HEADER_SIZE (sizeof(SCI_msgid) + sizeof(uint) +sizeof(uint))

//
// SCI msg queues: queue for sending works with offsets (for remote memory access)
// queue for receiving works with addresses (faster and easier on local memory)
//
typedef struct _sci_msg_queue_loc {
	SCI_packet &base;   // base address (slot 0)
	SCI_packet &next;   // slot for next msg to send or recv
	SCI_packet &last;   // last slot (for fast wrap-around check)
	uint nbr_slots;     // nbr of slots in this queue
	unit slotsize;      // size of these slots

	SCI_msgid id;       // ID of next msg to send or recv

	ulong *msgs_read;   // counter for "nbr msg processed"
} SCI_msgqueue_loc;

typedef struct _sci_msg_queue_rmt {
	ulong base;   // base offset (slot 0)
	ulong next;   // offset for slot for next msg to send 
	ulong last;   // offset of last slot (for fast wrap-around check)
	uint nbr_slots;     // nbr of slots in this queue
	unit slotsize;      // size of these slots

	uint node_id;       // SCI node id for this queue
	uint sgmt_id;
	sci_remote_interrupt_t rmt_intrpt;
	
	SCI_msgid id;          // ID of next msg to send or recv
	ulong nbr_msgs_sent;   // nbr of msgs sent 

	ulong msgs_read; // offset for remote counter: nbr of messages read
} SCI_msgqueue_rmt;


//
// sending and receiving short "active" messages via SCI
//
class SCI_msg {
public:
	SCI_msg (vector<SCI_macc>& queue_macc, SCI_memaccessor& sci_macc, 
				SCI_descriptor& sci_desc, int pktsize = 0, int nbrpkts = 0);
	~SCI_msg();

	int rank();
	int size();
	vector<int> all();
	int max_payload();
  
	void reg_msgtype (int tag;  void (*handler)(int src, int tag, int len, void *data), uint flags = 0);
	void free_msgtype (int tag);  

	void send (int dst, int tag, int len, void *data);
	void bcast (vector<int>& dst_ranks, int root, int len, void *data);
	template<class T>
	void allgather (vector<int>& ranks, int len, void *send_data, vector<T>& recv_data);

	void poll (vector<int>& src_ranks, vector<int>& src_tags, int *recv_tag, void *recv_data);
	void free_msg (void *buf);
  
private:
	// SCI objects
	SCI_memaccessor& memacc;
	SCI_descriptor& desc;
	vector< SCI_connector > queue_mem_cnct;

	// message scheduler
	sci_callback_action_t msg_arrived (void *arg, sci_local_interrupt_t interrupt, sci_error_t status);

	// message queues - one out & in pair for each process
	vector< SCI_msgqueue_rmt > msg_out;
	vector< SCI_msgqueue_loc > msg_in;

	// interrupt 
	sci_desc_t local_intrpt_desc;
	uint local_intrpt_nbr;
	sci_local_interrupt_t local_intrpt;
	vector< sci_desc_t > rmt_intrpt_desc;
	vector< sci_remote_interrupt_t > rmt_intrpt;

	// message handlers
	hash_map< int, void (*handler)(int src, int tag, int len, void *data) > msg_handlers;
	uint next_tag;
	
	// temporary message buffers for fast allocation
	void *get_msg_buf();
	stack < void * > buffer_pool;

	// locks
	pthread_mutex_lock sndrcv_lock;

	int nbr_pkts, pkt_size, payload;
	int myrank, nbrprocs;

#if SCI_MSG_USE_CHECKSUM
	bool check_csum (SCI_csum csum, int len, void *src, void *dst);
	SCI_csum gen_csum (int len, void *data);
#endif
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
