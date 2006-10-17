/* Thread-related definitions and macros */

#ifndef _MPID_THREADS_H_
#define _MPID_THREADS_H_

#include "mpichconf.h"

/*
 * Thread definitions.  We show an example of pthreads, as well as
 * a default set for no threading.  
 */
#if defined MPID_USE_DEVTHREADS || defined MPIR_USE_LIBTHREADS
/*#  ifndef WIN32*/
#    include <pthread.h>
/*#  endif*/
#ifndef _REENTRANT
#define _REENTRANT
#endif
#define MPID_THREAD_DS_LOCK_DECLARE    pthread_mutex_t mutex
#define MPID_THREAD_DS_LOCK_INIT(p) pthread_mutex_init(&(p)->mutex, NULL)
#define MPID_THREAD_DS_LOCK(p)       pthread_mutex_lock(&(p)->mutex)
#define MPID_THREAD_DS_UNLOCK(p)     pthread_mutex_unlock(&(p)->mutex)
#define MPID_THREAD_DS_LOCK_FREE(p) pthread_mutex_destroy(&(p)->mutex)

#else

#define MPID_THREAD_DS_LOCK_DECLARE unsigned short dummy_mutex
#define MPID_THREAD_DS_LOCK_INIT(p) 
#define MPID_THREAD_DS_LOCK(p)      
#define MPID_THREAD_DS_UNLOCK(p)    
#define MPID_THREAD_DS_LOCK_FREE(p) 

#endif


/* define type and macros for mutex locks among threads */
#if defined MPID_USE_DEVTHREADS || defined MPIR_USE_LIBTHREADS
/* thread type definitons */
#ifndef WIN32
#define MPID_LOCK_T pthread_mutex_t
#else
/* XXX dummy */
#define MPID_LOCK_T int
#endif

#define MPID_INIT_LOCK(mtx) pthread_mutex_init(mtx, NULL)
#define MPID_LOCK(mtx) pthread_mutex_lock(mtx)
#define MPID_UNLOCK(mtx) pthread_mutex_unlock(mtx)
#define MPID_TRYLOCK(mtx) pthread_mutex_trylock(mtx)
#define MPID_DESTROY_LOCK(mtx) pthread_mutex_destroy (mtx)

#else 

#define MPID_LOCK_T int
/* no multi-threading */
#define MPID_INIT_LOCK(mtx)
#define MPID_LOCK(mtx) 
#define MPID_UNLOCK(mtx)
#define MPID_TRYLOCK(mtx) 1
#define MPID_DESTROY_LOCK(mtx)

#endif

#define MPID_LOCK_INIT(mtx) MPID_INIT_LOCK(mtx)
#define MPID_LOCK_DESTROY(mtx) MPID_DESTROY_LOCK(mtx)

#endif

