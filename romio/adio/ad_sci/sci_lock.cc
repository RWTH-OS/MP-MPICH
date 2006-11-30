//
// $Id$
//
//

SCI_lock::SCI_lock(SCI_memaccessor& global_macc, SCI_macc lock_macc, int size) {
  macc = global_macc;

  lock_size = size;
  lock_id   = lock_macc.l;
  atomic_direct = (atomic_cnt_t *)macc.addr (lock_macc, lock_size, 0);
  atomic_cntr   = (atomic_cnt_t *)macc.addr (lock_macc, lock_size, SCI_ACC_ATOMIC);

  rank = msg.rank();
  nproc = msg.size();
  valid = (atomic_cnt_t *)malloc (nproc * sizeof(atomic_cnt_t));

  // pthread_mutex lock for process-internal locking
  pthread_mutex_init (&loc_lock, NULL);

  return;
}

SCI_lock::~SCI_lock() {
  free (valid);
  pthread_mutex_destroy (&loc_lock);

  return;
}

SCI_syncid SCI_lock::get_id() {
  return lock_id;
}

// original call:  errCode_t sync_lock(int atomic)
void SCI_lock::acquire() {
  atomic_cnt_t ticket, access, test;
  uint      minimum, boundary, i;
  
  // do local lock first
  pthread_mutex_lock(&loc_lock);
  
  // mark wish 
  DBG_CMD (printf("Start lock on node %i / slot %i\n",locNodeNum,atomic));
  set_atomic_direct (2 + rank, 1);
  macc.store_barrier();
  
  // atomically increment ticket counter
  DBG_CMD (printf("Get ticket\n"));
  get_atomic (0, &ticket);
  DBG_CMD (printf("Ticket = %i\n",ticket));
  
  // mark the ticket in the slot
  set_atomic_direct (2 + rank, ticket+2);
  DBG_CMD (printf("Ticket set in wish list\n"));
  
  // test whether we have direct access to the lock (shortcut) 
  get_atomic_direct (1, &access);
  DBG_CMD (printf("Access token : %i\n",access));

  if (ticket == access) {
    // err=sci_acquire(); 
    DBG_CMD (printf("We got the lock the fast way\n"));
  } else {
    // lock is not free or error during inc, do longer version
    // flush the write buffers again (paranoia hack) 
    macc.store_barrier();
      
    // first run, check valid entries 
    boundary = -1;
    for (i = 0; i < nproc; i++) {
      get_atomic_direct (2+i, &(valid[i]));

      if (valid[i]) 
	boundary = i;
    }
    DBG_CMD (printf("Created valid field, boundary = %i\n",boundary));
      
    // now loop until we have the lock 
    do {
      DBG_CMD (printf("Test field: "));
      minimum = ticket + 2;
      for (i = 0; i <= boundary; i++) {
	if (valid[i]) {
	  get_atomic_direct (2+i, &test);
	  if (test == 0) {
	    valid[i] = 0;
	  } else {
	    if (test < minimum) 
	      minimum = test;
	  }
	  DBG_CMD (printf(" %i",test));
	}
	DBG_CMD (else printf(" -"));
      }
      DBG_CMD (printf(" : Wait loop end - minimum %i, ticket %i\n",minimum,ticket));
    } while (minimum != ticket + 2);
    
    // we got the lock, everything is fine, do acquire
    DBG_CMD (printf("We got the lock the slow way\n"));
    // err = sci_acquire();
  } 
  
  return err;
}

// original call:  errCode_t sync_lock(int atomic)
bool SCI_lock::acquire_try() {
  atomic_cnt_t ticket, access, test;
  uint      minimum, boundary, i;
  
  // do local lock first
  if (pthread_mutex_trylock(&loc_lock)
    return false;
  
  // mark wish 
  DBG_CMD (printf("Start lock on node %i / slot %i\n",locNodeNum,atomic));
  set_atomic_direct (2 + rank, 1);
  macc.store_barrier();
  
  // atomically increment ticket counter
  DBG_CMD (printf("Get ticket\n"));
  get_atomic (0, &ticket);
  DBG_CMD (printf("Ticket = %i\n",ticket));
  
  // mark the ticket in the slot
  set_atomic_direct (2 + rank, ticket+2);
  DBG_CMD (printf("Ticket set in wish list\n"));
  
  // test whether we have direct access to the lock (shortcut) 
  get_atomic_direct (1, &access);
  DBG_CMD (printf("Access token : %i\n",access));

  if (ticket == access) {
    // err=sci_acquire(); 
    DBG_CMD (printf("We got the lock the fast way\n"));
    return true;
  } else {
    // lock is not available

    // XXX: It could also be that the inc failed -> this should be checked!
    // However, if the caller tries again, he has chances to succeed.
    return false;
  }
}

// original call: sync_unlock(int atomic)
void SCI_lock::release() {
  atomic_cnt_t test;

  // as we still have the lock, we can modify the access counter non atomically
  get_atomic_direct (1, &test);
  set_atomic_direct (1, test+1);

  // now the lock is released, but we still have to undo the reservation 
  set_atomic_direct (2 + rank, 0);

  pthread_mutex_unlock (&loc_lock);

  macc.store_barrier();
  return err;
}

//
// atomic read & write
//
void SCI_lock::get_atomic (uint offset, atomic_cnt_t *val)
{
  atomic_cnt_t tmp;
    
  tmp = atomic_cntr[offset];
  *val = ((tmp >> 24) & 0x000000FF) | ((tmp >>  8) & 0x0000FF00) |
    ((tmp <<  8) & 0x00FF0000) | ((tmp << 24) & 0xFF000000);
    
  return;
}

void SCI_lock::get_atomic_direct (uint offset, atomic_cnt_t *val);
{
  atomic_cnt_t tmp;
    
  tmp = atomic_direct[offset];
  *val = ((tmp >> 24) & 0x000000FF) | ((tmp >>  8) & 0x0000FF00) |
    ((tmp <<  8) & 0x00FF0000) | ((tmp << 24) & 0xFF000000);

  return;
}

void SCI_lock::set_atomic_direct (uint offset, atomic_cnt_t val);
{
  atomic_cnt_t tmp;
    
  tmp = ((val >> 24) & 0x000000FF) | ((val >>  8) & 0x0000FF00) |
    ((val <<  8) & 0x00FF0000) | ((val << 24) & 0xFF000000);
  atomic_direct[offset] = tmp;
    
  // flush write buffers
  macc.store_barrier();

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
