//
// $Id: sci_lock.h 889 2001-05-10 18:46:46Z joachim $
//
// Inter-process locks based on SCI fetch-and-add memory
//

#include "sci_syncmod.h"
#include "sci_memaccessor.h"
#include "sci_msg.h"

class SCI_lock {
public:
    SCI_lock (SCI_memaccessor& global_macc, SCI_msg& global_msg, SCI_macc lock_macc);
    ~SCI_lock ();
    
    // the id is needed to allow other processes to use this lock, too
    SCI_syncid get_id();

    void acquire();
    bool acquire_try();

    void release();

private:
    SCI_memaccessor& macc;
    SCI_msg &msg;

    void get_atomic (uint offset, atomic_val_t *val);
    void get_atomic_direct (uint offset, atomic_val_t *val);
    void set_atomic_direct (uint offset, atomic_val_t val);
    
    SCI_syncid lock_id;
    phtread_mutex_t loc_lock;
    atomic_cnt_t *atomic_cntr, *atomic_direct;
    atomic_cnt_t *valid;  // local valid field for acquire()
    int rank, nproc;
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
