#ifndef MPID_LOCK_H
#define MPID_LOCK_H

/* these are default definitions, the macros may have been defined elsewhere before */

/* definition of MPID_LOCK */

#ifndef MPID_LOCK
# ifdef MPID_ENABLE_THREADS
#  ifndef WIN32
#   define MPID_LOCK(mutex)		pthread_mutex_lock (mutex)
#  else
    /* has to be done */
#   define MPID_LOCK(mutex)
#  endif
# else
#  define MPID_LOCK(mutex)
# endif
#endif

/* definition of MPID_UNLOCK */

#ifndef MPID_UNLOCK
# ifdef MPID_ENABLE_THREADS
#  ifndef WIN32
#   define MPID_UNLOCK(mutex)	pthread_mutex_unlock (mutex)
#  else
    /* has to be done */
#   define MPID_UNLOCK(mutex)
#  endif
# else
#  define MPID_UNLOCK(mutex)
# endif
#endif

/* definition of MPID_TRYLOCK */

#ifndef MPID_TRYLOCK
# ifdef MPID_ENABLE_THREADS
#  ifndef WIN32
#   define MPID_TRYLOCK(mutex)	pthread_mutex_trylock (mutex)
#  else
    /* has to be done */
#   define MPID_TRYLOCK(mutex) 1
#  endif
# else
#  define MPID_TRYLOCK(mutex) 1
# endif
#endif


#endif	/* MPID_LOCK_H */


/*
 * Overrides for XEmacs and vim so that we get a uniform tabbing style.
 * XEmacs/vim will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:tw=0:ts=4:wm=0:
 */



