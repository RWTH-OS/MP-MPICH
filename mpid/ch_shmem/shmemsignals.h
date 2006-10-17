/* $Id */

#ifndef _23452542_SHMEMSIGNALS_H
#define _23452542_SHMEMSIGNALS_H

/* 
   The create_procs routine keeps track of the processes (stores the
   rc from PROCESS_FORK) and is prepared to kill the children if it
   receives a SIGCHLD.  One problem is making sure that the kill code
   isn't invoked during a normal shutdown.  This is handled by turning
   off the signals while in the rundown part of the code; this introduces
   a race condition in failures that I'm not prepared for yet.

   This is complicated by the decision of POSIX to chose unreliable signals
   as the default signal behavior (probably to ensure the adoption of 
   Windows NT).

   The interface is this:
   To set a reliable signal handler, 
   SIGNAL_HAND_SET(signame,sigf)

   To set a reliable signal handler and get the old handler back
   SIGNAL_HAND_SET_RET(signame,sigf,oldsigf)

   Before exiting a signal handler (needed with unreliable signals to 
   re-establish the handler!)
   SIGNAL_HAND_CLEANUP(signame,sigf)

   Finally, signal handler declarations are a mess.  Some systems even
   require users to declare them in ways that don't match the definition
   or need!  To declare a signal handler, use

   SIGNAL_HAND_DECL(sigf)
   It will use arguments sig [,code, scp] .  Depend ONLY on sig.

   Finally, it is sometimes necessary to block signals.  This is done
   with SIGNAL_BLOCK(signal) and SIGNAL_UNBLOCK (which restores the 
   previous signal state).
   
 */
#if defined(HAVE_SIGACTION)
/*
 * In the case where SA_RESETHAND is supported (i.e., reliable signals), 
 * we can use that and don't need to reset the handler.  Otherwise, we'll
 * have to.
 */
#if defined(SA_RESETHAND)
/* Here is the most reliable version.  Systems that don't provide
   SA_RESETHAND are basically broken at a deep level. 
 */
#define SIGNAL_HAND_SET_RET(signame,sigf,oldsigf) {\
struct sigaction oldact;\
sigaction( signame, (struct sigaction *)0, &oldact );\
oldsigf = oldact.sa_handler;\
oldact.sa_handler = sigf;\
oldact.sa_flags   = oldact.sa_flags & ~(SA_RESETHAND);\
sigaddset( &oldact.sa_mask, signame );\
sigaction( signame, &oldact, (struct sigaction *)0 );}

#define SIGNAL_HAND_SET(signame,sigf) {\
struct sigaction oldact;\
sigaction( signame, (struct sigaction *)0, &oldact );\
oldact.sa_handler = sigf;\
oldact.sa_flags   = oldact.sa_flags & ~(SA_RESETHAND);\
sigaddset( &oldact.sa_mask, signame );\
sigaction( signame, &oldact, (struct sigaction *)0 );}

#define SIGNAL_HAND_CLEANUP(signame,sigf)

#else
/* If SA_RESETHAND is not defined, we hope that by masking off the
   signal we're catching that it won't deliver that signal to SIG_DFL
 */
#define SIGNAL_HAND_SET_RET(signame,sigf,oldsigf) {\
struct sigaction oldact;\
sigaction( signame, (struct sigaction *)0, &oldact );\
oldsigf = oldact.sa_handler;\
oldact.sa_handler = sigf;\
sigaddset( &oldact.sa_mask, signame );\
sigaction( signame, &oldact, (struct sigaction *)0 );}

#define SIGNAL_HAND_SET(signame,sigf) {\
struct sigaction oldact;\
sigaction( signame, (struct sigaction *)0, &oldact );\
oldact.sa_handler = sigf;\
sigaddset( &oldact.sa_mask, signame );\
sigaction( signame, &oldact, (struct sigaction *)0 );}

#define SIGNAL_HAND_CLEANUP(signame,sigf) SIGNAL_HAND_SET(signame,sigf)
#endif /* SA_RESETHAND */

#elif defined(HAVE_SIGNAL)
/* Assumes reliable signals. The ';' in the definitions keep the emacs
   indentation code from doing stupid things */
#define SIGNAL_HAND_SET_RET(signame,sigf,oldsigf) \
oldsigf = signal(signame,sigf);
#define SIGNAL_HAND_SET(signame,sigf) \
(void) signal(signame,sigf);
#define SIGNAL_HAND_CLEANUP(signame,sigf)

#elif defined(HAVE_SIGSET)
#define SIGNAL_HAND_SET_RET(signame,sigf,oldsigf) \
oldsigf = sigset(signame,sigf);
#define SIGNAL_HAND_SET(signame,sigf) \
(void) sigset(signame,sigf);
#define SIGNAL_HAND_CLEANUP(signame,sigf) 

#else
/* no signal handlers! */
'Error - no signal handler available'
#endif

/* Signal handler declarations */
#if defined(HAVE_SIGHAND3)
#define SIGNAL_HAND_DECL(sigf) \
RETSIGTYPE sigf( sig, code, scp )\
int sig, code; struct sigcontext *scp;
#elif defined(HAVE_SIGHAND4)
#define SIGNAL_HAND_DECL(sigf) \
RETSIGTYPE sigf( sig, code, scp, addr )\
int sig, code; struct sigcontext *scp;char *addr;
#else
#define SIGNAL_HAND_DECL(sigf) \
RETSIGTYPE sigf( sig )\
int sig;
#endif

#endif /* _23452542_SHMEMSIGNALS_H */

