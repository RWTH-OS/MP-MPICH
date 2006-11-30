//
// $Id$
//
// 'SCI_syncmod' (synchronization module) provide SCI-based locks and barriers. 
// The SCI_syncmod object provides the required infrastructure and can create
// new locks and barriers.
//

#include <set>

#include <sisci_api.h>

#include "sci_priv.h"
#include "sci_msg.h"

#define DEBUG 0
#if DEBUG
#define DBG_CMD(a) a
#else
#define DBG_CMD(a)
#fi

class SCI_lock;
class SCI_barrier;


typedef volatile uint atomic_val_t;   // value of atomic counter 
typedef uint atomic_cnt_t;            // id of atomic counter (is atomic_t in orig. syncmod)
typedef SCI_mloc SCI_syncid;

class SCI_syncmod {
public:
    SCI_syncmod (SCI_memaccessor &memacc, SCI_msg& msg_channel, int min_locks = 100, int min_barriers = 10);
    ~SCI_syncmod ();
    
    SCI_lock& create_lock (int locality = -1);
    SCI_lock& get_lock (SCI_syncid lock_id);
    void free_lock (SCI_lock& lock);
    
    SCI_barrier& create_barrier (vector<int>& members, int locality = -1);
    SCI_barrier& get_barrier (SCI_syncid barrier_id);
    void free_barrier (SCI_barrier& barrier);

private:
    // objects
    SCI_msg& msg;
    SCI_memaccessor& macc;
    SCI_lock *master_lock;
    
    // functions
    void alloc_atomic (uint size, atomic_cnt_t *cnt);
    void free_atomic (uint size, atomic_cnt_t *cnt);

    // addresses of sync memory
    atomic_val_t *atomstart;
    atomic_val_t *atomstart_direct;
    vector<uint *> syncMem;
    int sgmt_size;
    int lock_size;      // amount of memory (bytes) for one lock

    // SCI connectors
    SCI_connector atom_cnct;
    vector<SCI_connector> sync_cnct;   // these segments are only required for the barrier

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
