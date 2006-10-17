/* general declarations needed by p2p that don't fit elsewhere */

#if !defined(P2P_EXTERN)
#define P2P_EXTERN extern
#endif

/* Handle argument declarations for ANSI C and C++ compilers */

#ifndef ANSI_ARGS
#    if defined(__STDC__) || defined(__cplusplus)
#        define ANSI_ARGS(a) a
#    else
#        define ANSI_ARGS(a) ()
#    endif
#endif

/* Definitions for improving Cache locality */

#define MPID_CACHE_ALIGN	/* could become machine-dependent  */

#if defined(MPID_CACHE_ALIGN)
#    define PAD(n) char pad[n];
#else
#    define PAD(n)
#endif

/* It is often advisable to flush cache of shared memory objects when
   they are no longer needed.  This macro does that; if it is undefined,
   then nothing happens 
 */
#ifndef MPID_FLUSH_CACHE
#    define  MPID_FLUSH_CACHE(addr,size)
#endif

/* function declarations for p2p */

double p2p_wtime       ANSI_ARGS((void));
void   p2p_init        ANSI_ARGS((int,int));
void   p2p_shfree      ANSI_ARGS((char *));
void   p2p_cleanup     ANSI_ARGS((void));
void   p2p_error       ANSI_ARGS((char *, int));
void   p2p_setpgrp     ANSI_ARGS((void));
void   p2p_yield       ANSI_ARGS((void));
void   p2p_kill_procs  ANSI_ARGS((void)); 
void   p2p_clear_signal ANSI_ARGS((void)); 
void   p2p_cleanup     ANSI_ARGS((void)); 
void p2p_create_procs  ANSI_ARGS((int,int,char **));
void p2p_wtime_init	   ANSI_ARGS(());


#ifdef USE_DISTRIB_SHMALLOC
    void *p2p_shmalloc ANSI_ARGS((int, int));
#else
    void *p2p_shmalloc ANSI_ARGS((int));
#endif
