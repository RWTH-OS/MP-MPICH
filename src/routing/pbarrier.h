/* $Id$ 
 * (c) 2005 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de 
 */
/* POSIX-based barrier for multiple threads on a single processor or SMP node */
#ifndef WIN32
#include <sys/errno.h>
#include <sys/types.h>
#include <pthread.h>
#else
#include "pthread.h"

#endif
#include <stdio.h>


typedef struct PBarrier  {
    volatile int maxcnt;	       /* maximum number of runners    */
    volatile int aborted;	       /* Boolean: if TRUE, barrier has been
				          aborted */
    void (* completion)(void *);
    void *compl_arg;

    struct _sb
    {
	pthread_cond_t wait_cv;	       /* cv for waiters at barrier    */
	pthread_mutex_t wait_lk;       /* mutex for waiters at barrier */
	volatile int runners;	       /* number of running threads    */
    }
    sb[2];
    struct _sb *sbp;		       /* current sub-barrier          */
} pbarrier_t;


/* prototypes */

int pbarrier_init (pbarrier_t *bp, int count, void (* compl_func)(void *), void *arg);
int pbarrier_set (pbarrier_t *bp, int nbr);
int pbarrier_abort (pbarrier_t *bp);
int pbarrier_wait (register pbarrier_t *bp);
int pbarrier_wait_and_inc (register pbarrier_t *bp);
int pbarrier_destroy (pbarrier_t *bp);

/* OSF/1 is not fully POSIX compliant */
/* therefor some "wrapper defines"    */

#if defined(OSF1_PARAGON)
#define PTHREAD_MUTEXATTR_INIT(attr)   pthread_mutexattr_create(attr)
#define PTHREAD_MUTEX_INIT(lock, attr) pthread_mutex_init((lock), *(attr))

#define PTHREAD_CONDATTR_INIT(attr)    pthread_condattr_create(attr)
#define PTHREAD_COND_INIT(cond, attr)  pthread_cond_init(cond, *(attr))

#define PTHREAD_ATTR_INIT(attr)        pthread_attr_create(attr)
#define PTHREAD_CREATE(thread,attr,start,arg)   pthread_create((thread),*(attr),(start),(void *)(arg))
#else
#define PTHREAD_MUTEXATTR_INIT(attr)   pthread_mutexattr_init(attr)
#define PTHREAD_MUTEX_INIT(lock, attr) pthread_mutex_init((lock),(attr))

#define PTHREAD_CONDATTR_INIT(attr)    pthread_condattr_init(attr)
#define PTHREAD_COND_INIT(cond, attr)  pthread_cond_init((cond),(attr))

#define PTHREAD_ATTR_INIT(attr)        pthread_attr_init(attr)
#define PTHREAD_CREATE(thread,attr,start,arg)   pthread_create((thread),(attr),(start),(void *)(arg))
#endif
