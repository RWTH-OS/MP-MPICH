//
// $Id: sci_syncmod.cc 889 2001-05-10 18:46:46Z joachim $
//
// Implementation of SCI_syncmod
//

#include <sci_sycnmod.h>

SCI_syncmod::SCI_syncmod (SCI_memaccessor &memacc, SCI_msg& msg_channel, int min_locks, int min_barriers)
{
  SCI_macc macc;
  SCI_connector cnct;
  SCI_syncid lock_id;
  vector<SCI_connector> sycn_mem_cnct;
    
  int sgmt_size = sizeof (uint) * 2*(min_locks + min_barriers);
  sync_mem_cnct.reserve(msg_channel.size());
  syncMem.reserve(msg_channel.size());
  
  // create central synchronization segment
  // (this should change to be a 'distributed' segment to improve scaling!)
  if (msg_channel.rank() == 0)
    atom_cnct = memacc.create_shmem (sgmt_size);

  msg_channel.bcast (msg_channel.all(), 0, sizeof(SCI_connector), &atom_cnct);
  
  // get normal and locking map of this segment
  MACC_INIT (macc, 0, atom_cnct.sgmt_id, 0, 0);
  atomstart_direct = (atomic_val_t) memacc.addr_lock (macc, sgmt_size, 0);
  atomstart = (atomic_val_t) memacc.addr_lock (macc, sgmt_size, SCI_ACC_ATOMIC);

  // now, each process creates & connects management segments
  cnct = memacc.create_shmem (sgmt_size);
  msg_channel.allgather (msg_channel.all(), sizeof(SCI_connector), (void *)&cnct, sycn_mem_cnct);  
  for (vector<SCI_connector>::iterator sync_mem_it = sycn_mem_cnct.begin(), int rank = 0;
       sync_mem_it != sycn_mem_cnct.end(); sync_mem_it++, rank++) {
    MACC_INIT (macc, rank, sync_mem_it.sgmt_id, 0, 0);
    syncMem.push_back (memacc.addr_lock (macc, sgmt_size, 0));
  }
		       
  // create master lock
  lock_size = 2 + msg.size();
  if (msg_channel.rank() == 0) {
    atomstart_direct[ATOMIC_MANAGER] = ;
    macc.store_barrier();

    master_lock = create_lock();
    lock_id = master_lock.get_id();
  }
  msg_channel.bcast (msg_channel.all(), 0, sizeof(SCI_syncid), &lock_id);
  if (msg_channel.rank() != 0) {
    master_lock = get_lock (lock_id);
  }
    
  return;
}

SCI_syncmod::~SCI_syncmod ()
{

  return;
}

//
// create a new global SCI lock
//
// orig call: sync_allocAtomic(uint size, atomic_t *atomic)
SCI_lock& SCI_syncmod::create_lock (int locality)
{
  SCI_lock *new_lock;
  SCI_macc lock_macc;
  atomic_val_t newlock_offset, tmp;

  master_lock->acquire();

  // 'locality' is ignored for now (we would need to create an atomic segment on
  // every node for this)
	
  // save old index of max. counter
  DBG_CMD (printf("sync_allocAtomic: lock done, reading manager\n"));
  tmp = atomstart_direct[ATOMIC_MANAGER];

  // increment max. index
  DBG_CMD (printf("sync_allocAtomic: got old value %i from slot %i\n", tmp, ATOMIC_MANAGER));
  newlock_offset = tmp + lock_size;

  // check if space is left
  DBG_CMD (printf("sync_allocAtomic: new locManager %i, comparing with end %i\n", newlock_offset,atomEnd));
  if ((newlock_offset < 0) || (newlock_offset >= sgmt_size/sizeof(atomic_val_t))) {
    // In the future, we should create a new lock memory segment here.
    // For now, we have to give up.
    master_lock->release();
    
    // XXX throw exception
    return ERR_SIMPLESYNC_OUTOFATOMIC;
  }

  // save new counter value
  DBG_CMD (printf("sync_allocAtomic: assigning locManager %i\n", newlock_offset));
  atomstart_direct[ATOMIC_MANAGER] = newlock_offset;

  // sync & release the lock 
  DBG_CMD (printf ("create_lock: locManager assigned %i at slot %i\n", newlock_offset, ATOMIC_MANAGER));
  DBG_CMD (printf ("create_lock: new value %i, doing sync\n",tmp));
  macc.store_barrier();
  master_lock->release();

  MACC_INIT(lock_macc, 0, atom_cnct.sgmt_id, newlock_offset/(lock_size*sizeof(atomic_val_t)), 
	    lock_size*sizeof(atomic_val_t));
  new_lock = new SCI_lock (lock_macc);

  return *new_lock;
}

void SCI_syncmod::free_lock (SCI_lock& lock)
{
  // not yet implemented; this would require a global management for the lock counters

  return;
}


SCI_barrier& SCI_syncmod::create_barrier (vector<int>& members, int locality)
{
  // not yet implemented (because barriers are not required for ad_sci 
  
  return new_barrier;
}

void SCI_syncmod::free_barrier (SCI_barrier& barrier) 
{
  // not yet implemented; this would require a global management for the barrier counters

  return;
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
