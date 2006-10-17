/* $Id$ 
 * (c) 2005 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de 
 */
/*======================================================================*/
/* barrier to synchronise POSIX threads on a single processor           */

#include "pbarrier.h"

pthread_mutexattr_t m_attr;
pthread_condattr_t  c_attr;

/*
 * barrier_init() - initialize a barrier variable.
 */
int
pbarrier_init(pbarrier_t *bp, int count, void (*compl_func)(void *), void *arg) 
{
  struct _sb *sbp;
  int	i, n;
  
  if (count < 1)
    return(EINVAL);
  
  n = PTHREAD_MUTEXATTR_INIT (&m_attr);
  n = PTHREAD_CONDATTR_INIT (&c_attr);

  bp->maxcnt  = count;
  bp->aborted = 0;
  bp->sbp     = &(bp->sb[0]);
  bp->completion = compl_func;
  bp->compl_arg  = arg;
  
  for (i = 0; i < 2; ++i) {
    sbp	 = &(bp->sb[i]);
    sbp->runners = count;
    
    if ((n = PTHREAD_MUTEX_INIT(&(sbp->wait_lk), &m_attr)))     
      return(n);
    
    if ((n = PTHREAD_COND_INIT(&(sbp->wait_cv), &c_attr)))
      return(n);
  }
  return(0);
}

/*
 * barrier_set() - tell the barrier how many threads will arrive
 *
 * USE WITH CAUTION, since it can not be told for sure if some thread
 * is still due to arrive at the barrier (that means, that the barrier 
 * is in use). If that is the case, the results are unpredictable. 
 * 
 * ONLY USE THIS FUNCTION IF YOU ARE SURE THAT THE BARRIER IS NOT IN USE
 * and if a destroy&init cycle would need too much time.
 */
int
pbarrier_set(pbarrier_t *bp, int nbr)
{
  register struct _sb *sbp = bp->sbp;

  pthread_mutex_lock(&sbp->wait_lk);
  /*  assert (bp->control == 0); */

  /* let's hope that the barrier is not in use 
     (the user of the barrier is responsible for that!) */
  bp->maxcnt = nbr;
  bp->sb[0].runners = nbr;
  bp->sb[1].runners = nbr;

  bp->aborted = 0;

  pthread_mutex_unlock(&sbp->wait_lk);
  return (1);
}

/* 
 * pbarrier_abort() - immediately "complete" a (running) barrier without waiting
 *                    for the initial number of threads
 *
 * THIS FUNCTION SHOULD NOT BE USED AT ALL. The only place where it may make sense
 * is if problems occur during some kind of initialization. A Barrier that has been 
 * aborted can not be used again until re-initialized via pbarrier_set()
 *
 */
int
pbarrier_abort(pbarrier_t *bp)
{
  register struct _sb *sbp = bp->sbp;

  pthread_mutex_lock (&sbp->wait_lk);

  /* abort the barrier */
  bp->aborted = 1;
  sbp->runners--;

  /* wake up the waiters */
  pthread_cond_broadcast(&sbp->wait_cv);

  pthread_mutex_unlock (&sbp->wait_lk);
  return (bp->maxcnt - sbp->runners);
} 
  
/*
 * pbarrier_wait() - wait at a barrier for everyone to arrive.
 *
 */
int
pbarrier_wait(register pbarrier_t *bp)
{
  register struct _sb *sbp = bp->sbp;
  int	count	 = bp->maxcnt;
  
  pthread_mutex_lock (&sbp->wait_lk);

  if ( sbp->runners == 1 || bp->aborted) {	/* last thread to reach barrier	*/
      if (bp->maxcnt != 1) {
	  /* do the completion action */
	  if (bp->completion != NULL) 
	      (bp->completion)(bp->compl_arg);
	  
	  /* barrier complete: reset runner count and switch sub-barriers	*/
	  bp->aborted  = 0;
	  sbp->runners = count;
	  bp->sbp = ( bp->sbp == &bp->sb[0] ) ? &bp->sb[1] : &bp->sb[0];

	  /* wake up the waiters */
	  if (!bp->aborted) 
	      pthread_cond_broadcast(&sbp->wait_cv);	 	  
      }
  } else {
      sbp->runners--;   /* one less runner  */
      while (sbp->runners != count) 
	  pthread_cond_wait( &sbp->wait_cv, &sbp->wait_lk );
  }
  
  pthread_mutex_unlock(&sbp->wait_lk);
  return(0);
}

/*
 * pbarrier_wait_and_inc() - wait at a barrier for everyone to arrive and 
 *                           increment the number of participants when leaving
 *                           the barrier
 */
int pbarrier_wait_and_inc (register pbarrier_t *bp)
{
  register struct _sb *sbp = bp->sbp;
  int	count	 = bp->maxcnt;
  
  pthread_mutex_lock (&sbp->wait_lk);

  if ( sbp->runners == 1 || bp->aborted) {	/* last thread to reach barrier	*/
      if (bp->maxcnt != 1) {
	  /* do the completion action */
	  if (bp->completion != NULL) 
	      (bp->completion)(bp->compl_arg);
	  
	  /* barrier complete: reset runner count and switch sub-barriers	*/
	  bp->aborted  = 0;
	  bp->sb[0].runners = bp->maxcnt + 1;
	  bp->sb[1].runners = bp->maxcnt + 1;
	  bp->maxcnt++;
	  bp->sbp = ( bp->sbp == &bp->sb[0] ) ? &bp->sb[1] : &bp->sb[0];

	  /* wake up the waiters */
	  if (!bp->aborted)
	      pthread_cond_broadcast(&sbp->wait_cv);
      }
  } else {
      sbp->runners--;   /* one less runner  */
      while (sbp->runners != bp->maxcnt)
	  pthread_cond_wait( &sbp->wait_cv, &sbp->wait_lk);
  }
  
  pthread_mutex_unlock(&sbp->wait_lk);
  return(0);
}

/*
 * pbarrier_destroy() - destroy a barrier
 */
int
pbarrier_destroy(pbarrier_t *bp) 
{
    int i,n;

    for (i = 0; i < 2; ++i) { 
	if ((n = pthread_cond_destroy(&bp->sb[i].wait_cv)))
	   return(n);

	if ((n = pthread_mutex_destroy(&bp->sb[i].wait_lk)))
	   return(n);
    }
    return(0);
}
