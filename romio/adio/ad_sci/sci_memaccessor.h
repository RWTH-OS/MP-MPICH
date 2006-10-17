//
// $Id: sci_memaccessor.h,v 1.7 2001/06/15 15:41:49 joachim Exp $
//
// Access memory from local or remote SCI segments
//  - stride-based access via SCI_macc descriptors
//  - dynamic connection management for efficient resource usage
//  - monitoring of SCI connetivity via segment callbacks
//

#include <hash_map>

#include <sisci_api.h>

#include "sci_priv.h"
#include "sci_descriptor.h"

// flags for segment creation & connection
#define SCI_ACC_ATOMIC        1
#define SCI_ACC_READONLY      2
#define SCI_ACC_NOFLUSH       4
#define SCI_ACC_BARRIER       8
#define SCI_ACC_ASYNC         16
#define SCI_ACC_CREATE_INTRPT 32

typedef struct _sci_sgmt_info {
	// location of the segment
	uint node_id;
	uint sgmt_id;

	// states of the segment
	bool is_local;
	bool is_readonly;
	bool is_atomic;
	bool disconnect_requested;
	bool is_accessable;

	// SISCI resources
	sci_desc_t desc;
	union segment {
		sci_local_segment_t local;
		sci_remote_segment_t remote;
	}
	sci_map_t map, atomic_map, readonly_map;
	sci_sequence_t sequence;
	sci_sequence_status_t seq_status;

	// control the access to the segment in case of SCI errors
	pthread_cond_t access_cv;

	// mapping properties
	char *addr, *atomic_addr, *readonly_addr;
	uint offset, size;

	// locking 
	uint lock_cntr;
	uint cnct_cntr;
} SCI_sgmt_info;


class SCI_memaccessor {
public:
	SCI_memaccessor (int adapter = -1) : sci_desc();
	~SCI_memaccessor ();
    
	// create local shared memory
	SCI_connector create_shmem (int size, uint flags = 0, int adapter = -1);
	void remove_shmem (SCI_connector& cnct);
    
	// get and put data via memory accessors
	bool get (SCI_macc& macc, size_t len, void *buf, uint flags = 0);
	bool put (SCI_macc& macc, size_t len, void *buf, uint flags = 0);
	// for asynchronous get & put operations: test/wait for completion
	bool test (uint flags = 0);
	bool wait (uint flags = 0);
    
	// get an address range, guarantee connectivity until address is released
	void *range (SCI_macc& macc, size_t len);
	void *range_lock (SCI_macc& macc, size_t len, uint flags = 0);
	void lock_release (SCI_macc macc);
	// locked memory can be accessed faster via the address 
	bool copy (void *dst, void *src, size_t len, uint flags = 0);
    
	// consistency
	void store_barrier(int prank = -1);
	void load_barrier(int prank = -1);
    
	// info
	uint local_adapter();
	uint local_nodeid();

private:
	// member ojects 
	SCI_descriptor sci_desc;
	pthread_mutex_t access_lock;
    
	// segment callback functions
	sci_callback_action_t cb_rmt (void *arg, sci_remote_segment_t segment, 
											sci_segment_cb_reason_t reason, sci_error_t status);
	sci_callback_action_t cb_loc (void *arg, sci_local_segment_t segment,
											sci_segment_cb_reason_t reason, unsigned int nodeId, 
											unsigned int localAdapterNo, sci_error_t error);
	// remote segment management
	SCI_sgmt_info& connect_sgmt (SCI_macc macc, size_t len, uint flags = 0);
	SCI_sgmt_info& reconnect_sgmt (SCI_sgmt_info sgmt, SCI_macc macc, size_t len, uint flags = 0);
	SCI_sgmt_info& remap_sgmt (SCI_sgmt_info& sgmt, size_t len, uint flags);	
	bool disconnect_sgmt (SCI_sgmt_info sgmt, bool do_lock = true);

	// the current segment - a kind of lookup cache
	SCI_sgmt_info& sgmt_info; 
	// all information for the connected segments (key = node_id + sgmt_id)
	hash_map<ulong, SCI_sgmt_info, hash<ulong>, equal_to<ulong> > sci_memory_sgmts;

	// various status information
	uint adapter_nbr;
	uint local_node_id;
	uint last_sgmt_id = 0;
	uint pagesize;
}


// retrieve the information for the related segment - maybe it is "cached"
// in the localt sgmt_info variable; if not -> get it from the hash_map
// If not in the map, we need to connect first.
#define GET_SGMT_INFO (macc, len, flags) if ((sgmt_info.node_id != (macc).l.node_id) \
															|| (sgmt_info.sgmt_id != (macc).l.sgmt_id)) \
 { \
	ulong sgmt_key = macc.l.node_id << MACC_PRANK_WIDTH + macc.l.sgmt_id; \
	hash_map<ulong, SCI_sgmt_info, hash<ulong>, equal_to<ulong> >::iterator sgmt_it = \
		find (sci_memory_sgmts.begin(), sci_memory_sgmts.end(), sgmt_key); \
	if (sgmt_it == sci_memory_sgmts.end()) { \
		sgmt_info = connect_sgmt (macc, len, flags); \
	} else { \
		sgmt_info = *sgmt_it; \
	} \
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
